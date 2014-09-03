#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

#include "expression.hh"
#include "query.hh"
#include "receiver_thread.hh"
#include "data_handler.hh"
#include "virtdb_fdw.h" // pulls in some postgres headers
#include "postgres_util.hh"

// ZeroMQ
#include "cppzmq/zmq.hpp"

// more postgres headers
extern "C" {
    #include <utils/memutils.h>
    #include <foreign/fdwapi.h>
    #include <foreign/foreign.h>
    #include <utils/rel.h>
    #include <utils/builtins.h>
    #include <utils/date.h>
    #include <utils/syscache.h>
    #include <optimizer/pathnode.h>
    #include <optimizer/planmain.h>
    #include <optimizer/restrictinfo.h>
    #include <optimizer/clauses.h>
    #include <catalog/pg_type.h>
    #include <catalog/pg_operator.h>
    #include <access/transam.h>
    #include <access/htup_details.h>
    #include <funcapi.h>
    #include <nodes/print.h>
    #include <nodes/makefuncs.h>
    #include <miscadmin.h>
}

#include "filter.hh"
#include "anyallfilter.hh"
#include "boolexprfilter.hh"
#include "defaultfilter.hh"
#include "nulltestfilter.hh"
#include "opexprfilter.hh"

#include <logger.hh>
#include <util.hh>
#include <connector.hh>

// standard headers
#include <atomic>
#include <exception>
#include <sstream>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory>
#include <future>

using namespace virtdb;
using namespace virtdb::connector;

extern zmq::context_t* zmq_context;
receiver_thread* worker_thread = NULL;
endpoint_client*  ep_clnt;
log_record_client* log_clnt;

namespace virtdb_fdw_priv {


// We dont't do anything here right now, it is intended only for optimizations.
static void
cbGetForeignRelSize( PlannerInfo *root,
                     RelOptInfo *baserel,
                     Oid foreigntableid )
{
}

// We also don't intend to put this to the public API for now so this
// default implementation is enough.
static void
cbGetForeignPaths( PlannerInfo *root,
                    RelOptInfo *baserel,
                    Oid foreigntableid)
{
    Cost startup_cost = 0;
    Cost total_cost = startup_cost + baserel->rows * baserel->width;

    add_path(baserel,
             reinterpret_cast<Path *>(create_foreignscan_path(
                 root,
                 baserel,
                 baserel->rows,
                 startup_cost,
                 total_cost,
                 // no pathkeys:  TODO! check-this!
                 NIL,
                 NULL,
                 // no outer rel either:  TODO! check-this!
                 NIL
            )));
}

// Maybe in a later version we could provide API for extracting clauses
// but this is good enough for now to just leave in all of them.
static ForeignScan
*cbGetForeignPlan( PlannerInfo *root,
                   RelOptInfo *baserel,
                   Oid foreigntableid,
                   ForeignPath *best_path,
                   List *tlist,
                   List *scan_clauses )
{
    Index scan_relid = baserel->relid;
    if (scan_clauses != NULL)
    {
        elog(LOG, "[%s] - Length of clauses BEFORE extraction: %d",
                    __func__, scan_clauses->length);
    }

    // Remove pseudo-constant clauses
    scan_clauses = extract_actual_clauses(scan_clauses, false);
    if (scan_clauses != NULL)
    {
        elog(LOG, "[%s] - Length of clauses AFTER extraction: %d",
                    __func__, scan_clauses->length);
    }

    // 1. make sure floating point representation doesn't trick us
    // 2. only set the limit if this is a single table
    List* limit = NULL;
    size_t nrels = bms_num_members(root->all_baserels);
    if( nrels == 1 && root->limit_tuples > 0.9 )
    {
        limit = list_make1_int(0.1+root->limit_tuples);
    }

    ForeignScan * ret =
        make_foreignscan(
            tlist,
            scan_clauses,
            scan_relid,
            NIL,
            limit);

    return ret;
}

// Serializes a Protobuf message to ZMQ via REQ-REP method.
// will be consolidated but this is also for rapid development.
static void
send_message(const ::google::protobuf::Message& message)
{
    try {
        zmq::socket_t socket (*zmq_context, ZMQ_PUSH);
        socket.connect ("tcp://localhost:45186");
        std::string str;
        message.SerializeToString(&str);
        int sz = str.length();
        zmq::message_t query_message(sz);
        memcpy(query_message.data (), str.c_str(), sz);
        socket.send (query_message);
    }
    catch (const zmq::error_t& err)
    {
        elog(ERROR, "Error num: %d", err.num());
    }
}

static void
cbBeginForeignScan( ForeignScanState *node,
                    int eflags )
{
    // elog_node_display(INFO, "node: ", node->ss.ps.plan, true);

    ListCell   *l;
    struct AttInMetadata * meta = TupleDescGetAttInMetadata(node->ss.ss_currentRelation->rd_att);
    filter* filterChain = new op_expr_filter();
    filterChain->add(new nulltest_filter());
    filterChain->add(new any_all_filter());
    filterChain->add(new bool_expr_filter());
    filterChain->add(new default_filter());
    try
    {
        virtdb::query query_data;

        // Table name
        std::string table_name{RelationGetRelationName(node->ss.ss_currentRelation)};
        for (auto& c: table_name)
        {
            c = ::toupper(c);
        }
        query_data.set_table_name( table_name );

        // Columns
        int n = node->ss.ps.plan->targetlist->length;
        ListCell* cell = node->ss.ps.plan->targetlist->head;
        for (int i = 0; i < n; i++)
        {
            if (!IsA(lfirst(cell), TargetEntry))
            {
                continue;
            }
            Expr* expr = reinterpret_cast<Expr*> (lfirst(cell));
            const Var* variable = get_variable(expr);
            if (variable != nullptr)
            {
                // elog(LOG, "Column: %s (%d)", meta->tupdesc->attrs[variable->varattno-1]->attname.data, variable->varattno-1);
                query_data.add_column( variable->varattno-1, meta->tupdesc->attrs[variable->varattno-1]->attname.data );
            }
            cell = cell->next;
        }

        // Filters
        foreach(l, node->ss.ps.plan->qual)
        {
            Expr* clause = (Expr*) lfirst(l);
            query_data.add_filter( filterChain->apply(clause, meta) );
        }

        // Limit
        // From: http://www.postgresql.org/docs/9.2/static/fdw-callbacks.html
        // Information about the table to scan is accessible through the ForeignScanState node
        // (in particular, from the underlying ForeignScan plan node, which contains any
        // FDW-private information provided by GetForeignPlan).
        ForeignScan *plan = reinterpret_cast<ForeignScan *>(node->ss.ps.plan);
        if (plan->fdw_private)
        {
            query_data.set_limit( lfirst_int(plan->fdw_private->head) );
        }

        // Schema

        // UserToken

        // AccessInfo

        // Prepare for getting data
        worker_thread->add_query(node, query_data);

        send_message( query_data.get_message()) ;
    }
    catch(const std::exception & e)
    {
        elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__,e.what());
    }
}

static TupleTableSlot *
cbIterateForeignScan(ForeignScanState *node)
{
    struct AttInMetadata * meta = TupleDescGetAttInMetadata(node->ss.ss_currentRelation->rd_att);
    data_handler* handler = worker_thread->get_data_handler(node);
    if (!handler->received_data())
    {
        worker_thread->wait_for_data(node);
    }
    if (handler->has_data())
    {
        TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
        ExecClearTuple(slot);
        try
        {
            handler->read_next();

            for (int column_id : handler->column_ids())
            {
                if (handler->is_null(column_id))
                {
                    slot->tts_isnull[column_id] = true;
                }
                else
                {
                    slot->tts_isnull[column_id] = false;
                    switch( meta->tupdesc->attrs[column_id]->atttypid )
                    {
                        case VARCHAROID: {
                            const std::string* const data = handler->get<std::string>(column_id);
                            elog(LOG, "VARCHAROID data: %s", data->c_str());
                            if (data)
                            {
                                bytea *vcdata = reinterpret_cast<bytea *>(palloc(data->size() + VARHDRSZ));
                                ::memcpy( VARDATA(vcdata), data->c_str(), data->size() );
                                SET_VARSIZE(vcdata, data->size() + VARHDRSZ);
                                slot->tts_values[column_id] = PointerGetDatum(vcdata);
                            }
                            else {
                                slot->tts_isnull[column_id] = true;
                            }
                            break;
                        }
                        case INT4OID: {
                            const int32_t* const data = handler->get<int32_t>(column_id);
                            if (data)
                            {

                            }
                            elog(LOG, "INT4OID data: %d", *data);
                            break;
                        }
                        default: {
                            elog(ERROR, "Unhandled attribute type: %d", meta->tupdesc->attrs[column_id]->atttypid);
                            slot->tts_isnull[column_id] = true;
                            break;
                        }
                    }
                }
            }
            ExecStoreVirtualTuple(slot);
        }
        catch(const std::exception & e)
        {
            elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__, e.what());
        }
        return slot;
    }
    else
    {
        // return NULL if there is no more data.
        return NULL;
    }
}

static void
cbReScanForeignScan( ForeignScanState *node )
{
    elog(LOG, "[%s]", __func__);
}

static void
cbEndForeignScan(ForeignScanState *node)
{
    worker_thread->remove_query(node);
}

}

// C++ implementation of the forward declared function
extern "C" {

void PG_init_virtdb_fdw_cpp(void)
{
    try
    {
        zmq_context = new zmq::context_t(1);

        worker_thread = new receiver_thread();
        auto thread = new std::thread(&receiver_thread::run, worker_thread);
        thread->detach();

        ep_clnt = new endpoint_client("tcp://127.0.0.1:65001", "generic_fdw");
        log_clnt = new log_record_client(*ep_clnt);


    }
    catch(const std::exception & e)
    {
        elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__,e.what());
    }
}

void PG_fini_virtdb_fdw_cpp(void)
{
    delete zmq_context;
    worker_thread->stop();
    delete worker_thread;
    delete log_clnt;
    delete ep_clnt;
}

Datum virtdb_fdw_status_cpp(PG_FUNCTION_ARGS)
{
    char * v = (char *)palloc(4);
    strcpy(v,"XX!");
    PG_RETURN_CSTRING(v);
}

Datum virtdb_fdw_handler_cpp(PG_FUNCTION_ARGS)
{
    FdwRoutine *fdw_routine = makeNode(FdwRoutine);

    // must define these
    fdw_routine->GetForeignRelSize    = virtdb_fdw_priv::cbGetForeignRelSize;
    fdw_routine->GetForeignPaths      = virtdb_fdw_priv::cbGetForeignPaths;
    fdw_routine->GetForeignPlan       = virtdb_fdw_priv::cbGetForeignPlan;
    fdw_routine->BeginForeignScan     = virtdb_fdw_priv::cbBeginForeignScan;
    fdw_routine->IterateForeignScan   = virtdb_fdw_priv::cbIterateForeignScan;
    fdw_routine->ReScanForeignScan    = virtdb_fdw_priv::cbReScanForeignScan;
    fdw_routine->EndForeignScan       = virtdb_fdw_priv::cbEndForeignScan;

    // optional fields will be NULL for now
    fdw_routine->AddForeignUpdateTargets  = NULL;
    fdw_routine->PlanForeignModify        = NULL;
    fdw_routine->BeginForeignModify       = NULL;
    fdw_routine->ExecForeignInsert        = NULL;
    fdw_routine->ExecForeignUpdate        = NULL;
    fdw_routine->ExecForeignDelete        = NULL;
    fdw_routine->EndForeignModify         = NULL;

    // optional EXPLAIN support is also omitted
    fdw_routine->ExplainForeignScan    = NULL;
    fdw_routine->ExplainForeignModify  = NULL;

    // optional ANALYZE support is also omitted
    fdw_routine->AnalyzeForeignTable  = NULL;

    PG_RETURN_POINTER(fdw_routine);
}

Datum virtdb_fdw_validator_cpp(PG_FUNCTION_ARGS)
{
    // TODO
    PG_RETURN_VOID();
}

}

#pragma GCC diagnostic pop
