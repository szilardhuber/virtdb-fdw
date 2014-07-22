#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"


// protocol buffer
#include "proto/data.pb.h"

// ZeroMQ
#include <zmq.hpp>

#include "virtdb_fdw.h" // pulls in some postgres headers
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
// standard headers
#include <atomic>
#include <exception>
#include <sstream>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory>

zmq::context_t* zmq_context = NULL;
namespace { namespace virtdb_fdw_priv {

// We dont't do anything here right now, it is intended only for optimizations.
static void
cbGetForeignRelSize( PlannerInfo *root,
                     RelOptInfo *baserel,
                     Oid foreigntableid )
{
    elog(LOG, "[%s] - Default row estimation: %f", __func__, baserel->rows);
    elog(LOG, "[%s] - Default row width: %d", __func__, baserel->width);
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
    elog(LOG, "[%s] - Total cost: %.2f", __func__, total_cost);

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

    ForeignScan * ret =
        make_foreignscan(
            tlist,
            scan_clauses,
            scan_relid,
            NIL,
            NIL);

    return ret;
}

// Serializes a Protobuf message to ZMQ via REQ-REP method.
// will be consolidated but this is also for rapid development.
static void
sendMessage(std::shared_ptr<::google::protobuf::Message> message)
{
    zmq::socket_t socket (zmq_context, ZMQ_REQ);
    socket.connect ("tcp://localhost:55555");
    std::string str;
    message->SerializeToString(&str);
    int sz = str.length();
    zmq::message_t query(sz);
    memcpy(query.data (), str.c_str(), sz);
    socket.send (query);
}

// Here we will convert the expressions to a serializable format and pass them
// over the API but for now it only displays debug log.
static void
interpretExpression( Expr* clause )
{
    static int level = 0;
    using virtdb::interface::pb::Expression;
    std::shared_ptr<Expression> expression(new Expression);
    expression->set_variable("Expr->type");
    expression->set_operand("=");
    std::ostringstream s;
    s << clause->type;
    expression->set_value(s.str().c_str());
    sendMessage(expression);
    elog(LOG, "[%s] - On level: %d", __func__, level);
    elog(LOG, "[%s] - Filter expression type: %d", __func__, clause->type);
    if (IsA(clause, BoolExpr))
    {
        const BoolExpr* bool_expression = reinterpret_cast<const BoolExpr*>(clause);
        ListCell* current = bool_expression->args->head;
        for (int i = 0; i < bool_expression->args->length; i++)
        {
            Expr * expr = (Expr *)current->data.ptr_value;
            level++;
            interpretExpression(expr);
            level--;
            current = current->next;
        }
    }
}

// It is just a hack enabling rapid development. Will be implemented correctly.
static bool
hasMoreData(bool reset = false)
{
    static int counter = 0;
    if (reset)
    {
        counter = 0;
        return true;
    }
    if (counter++ < 2)
    {
        return true;
    }
    return false;
}

static void
cbBeginForeignScan( ForeignScanState *node,
                    int eflags )
{
    elog(LOG, "[%s]", __func__);
    ListCell   *l;
    foreach(l, node->ss.ps.plan->qual)
    {
        Expr* clause = (Expr*) lfirst(l);
        interpretExpression(clause);
    }
    hasMoreData(true); //TODO: hack remove
}

static TupleTableSlot *
cbIterateForeignScan(ForeignScanState *node)
{
    struct AttInMetadata * meta = TupleDescGetAttInMetadata(node->ss.ss_currentRelation->rd_att);
    elog(LOG, "[%s]", __func__);
    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    try
    {
        ExecClearTuple(slot);
        if (hasMoreData()) {
            for (int i = 0; i < node->ss.ss_currentRelation->rd_att->natts; i++)
            {
                slot->tts_isnull[i] = false;
                switch( meta->tupdesc->attrs[i]->atttypid )
                {
                    case INT4OID: {
                        slot->tts_values[i] = Int32GetDatum(i);
                        break;
                    }
                    case INT8OID: {
                        slot->tts_values[i] = Int64GetDatum(i);
                        break;
                    }
                    case FLOAT8OID:  {
                        slot->tts_values[i] = Float8GetDatum(i);
                        break;
                    }
                    case VARCHAROID: {
                        if (false) {  //TODO: disabled until we get real data
                            // bytea *vcdata = reinterpret_cast<bytea *>(palloc(field_bytes[i] + VARHDRSZ));
                            // ::memcpy( VARDATA(vcdata), it.data_->get<char>(i,it.row_,fb), field_bytes[i] );
                            // SET_VARSIZE(vcdata, field_bytes[i] + VARHDRSZ);
                            // slot->tts_values[i] = PointerGetDatum(vcdata);
                        } else {
                            slot->tts_isnull[i] = true;
                        }
                        break;
                    }
                    case NUMERICOID: {
                        if (false) {
                            // slot->tts_values[i] =
                            //     DirectFunctionCall3( numeric_in,
                            //                             CStringGetDatum(it.data_->get<char>(c,it.row_,fb)),
                            //                             ObjectIdGetDatum(InvalidOid),
                            //                             Int32GetDatum(state->meta_->tupdesc->attrs[c]->atttypmod) );
                        } else {
                            slot->tts_isnull[i] = true;
                        }
                        break;
                    }
                    case DATEOID: {
                        if (false) {
                            // slot->tts_values[i] =
                            //     DirectFunctionCall1( date_in, CStringGetDatum(it.data_->get<char>(c,it.row_,fb)));
                        } else {
                            slot->tts_isnull[i] = true;
                        }
                        break;
                    }
                    case TIMEOID:
                    {
                        if (false) {
                            // slot->tts_values[i] =
                            //     DirectFunctionCall1( time_in, CStringGetDatum(it.data_->get<char>(c,it.row_,fb)));
                        } else {
                            slot->tts_isnull[i] = true;
                        }
                        break;
                    }
                    default: {
                        slot->tts_isnull[i] = true;
                        break;
                    }
                }
            }
            ExecStoreVirtualTuple(slot);
        }
    }
    catch(const std::exception & e)
    {
        elog(ERROR, "[%s:%d] internal error in %s: %s",__FILE__,__LINE__,__func__,e.what());
    }
    return slot;
}

static void
cbReScanForeignScan( ForeignScanState *node )
{
}

static void
cbEndForeignScan(ForeignScanState *node)
{
}

}}

// C++ implementation of the forward declared function
extern "C" {

void PG_init_virtdb_fdw_cpp(void)
{
    using virtdb::interface::pb::Data;
    std::shared_ptr<Data> data_ptr(new Data);
    zmq_context = new zmq::context_t (1);
}

void PG_fini_virtdb_fdw_cpp(void)
{
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
