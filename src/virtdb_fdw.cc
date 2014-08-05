#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

#include "expression.hh"
#include "query.hh"
#include "receiver_thread.hh"
#include "data_handler.hh"
#include "virtdb_fdw.h" // pulls in some postgres headers

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

zmq::context_t* zmq_context = NULL;
receiver_thread* worker_thread = NULL;

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
    zmq::socket_t socket (*zmq_context, ZMQ_REQ);
    socket.connect ("tcp://localhost:55555");
    std::string str;
    message.SerializeToString(&str);
    int sz = str.length();
    zmq::message_t query(sz);
    memcpy(query.data (), str.c_str(), sz);
    socket.send (query);
}


static void
cbBeginForeignScan( ForeignScanState *node,
                    int eflags )
{
    ListCell   *l;
    struct AttInMetadata * meta = TupleDescGetAttInMetadata(node->ss.ss_currentRelation->rd_att);
    Filter* filterChain = new OpExprFilter();
    filterChain->Add(new NullTestFilter());
    filterChain->Add(new AnyAllFilter());
    filterChain->Add(new BoolExprFilter());
    filterChain->Add(new DefaultFilter());
    try
    {
        virtdb::Query query;

        // Table name
        std::string table_name{RelationGetRelationName(node->ss.ss_currentRelation)};
        for (auto& c: table_name)
        {
            c = ::toupper(c);
        }
        query.set_table_name( table_name );

        // Columns - TODO No aggregates are handled here
        int n = node->ss.ps.plan->targetlist->length;
        ListCell* cell = node->ss.ps.plan->targetlist->head;
        for (int i = 0; i < n; i++)
        {
            if (!IsA(lfirst(cell), TargetEntry))
            {
                continue;
            }
            TargetEntry* target_entry = reinterpret_cast<TargetEntry*> (lfirst(cell));
            query.add_column( target_entry->resname );
            // elog(LOG, "\t\tColumn No: %d", target_entry->resno);
            cell = cell->next;
        }

        // Filters
        foreach(l, node->ss.ps.plan->qual)
        {
            Expr* clause = (Expr*) lfirst(l);
            query.add_filter( filterChain->Apply(clause, meta) );
        }

        // Limit
        // From: http://www.postgresql.org/docs/9.2/static/fdw-callbacks.html
        // Information about the table to scan is accessible through the ForeignScanState node
        // (in particular, from the underlying ForeignScan plan node, which contains any
        // FDW-private information provided by GetForeignPlan).
        ForeignScan *plan = reinterpret_cast<ForeignScan *>(node->ss.ps.plan);
        if (plan->fdw_private)
        {
            query.set_limit( lfirst_int(plan->fdw_private->head) );
        }

        // Schema

        // UserToken

        // AccessInfo

        // Prepare for getting data
        worker_thread->add_query(node, query);

        send_message( query.get_message()) ;
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
            for (int i = 0; i < node->ss.ss_currentRelation->rd_att->natts; i++)
            {
                if (handler->is_null(i))
                {
                    slot->tts_isnull[i] = true;
                }
                else
                {
                    slot->tts_isnull[i] = false;
                    switch( meta->tupdesc->attrs[i]->atttypid )
                    {
                        case VARCHAROID: {
                            const std::string* const data = handler->get_string(i);
                            if (data)
                            {
                                bytea *vcdata = reinterpret_cast<bytea *>(palloc(data->size() + VARHDRSZ));
                                ::memcpy( VARDATA(vcdata), data->c_str(), data->size() );
                                SET_VARSIZE(vcdata, data->size() + VARHDRSZ);
                                slot->tts_values[i] = PointerGetDatum(vcdata);
                            }
                            else {
                                slot->tts_isnull[i] = true;
                            }
                            break;
                        }
                        default: {
                            elog(ERROR, "Unhandled attribute type: %d", meta->tupdesc->attrs[i]->atttypid);
                            slot->tts_isnull[i] = true;
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
        zmq_context = new zmq::context_t (1);
        worker_thread = new receiver_thread();
        new std::thread(&receiver_thread::run, worker_thread);
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
