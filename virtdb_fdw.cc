#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

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

namespace { namespace virtdb_fdw_priv {

static void
cbGetForeignRelSize( PlannerInfo *root,
                     RelOptInfo *baserel,
                     Oid foreigntableid )
{
}

static void
cbGetForeignPaths( PlannerInfo *root,
                   RelOptInfo *baserel,
                   Oid foreigntableid)
{
}

static ForeignScan
*cbGetForeignPlan( PlannerInfo *root,
                   RelOptInfo *baserel,
                   Oid foreigntableid,
                   ForeignPath *best_path,
                   List *tlist,
                   List *scan_clauses )
{
  Index scan_relid = baserel->relid;

  scan_clauses = extract_actual_clauses(scan_clauses, false);

  ForeignScan * ret =
    make_foreignscan(
      tlist,
      scan_clauses,
      scan_relid,
      // no expressions to evaluate:
      NIL,
      // passing private state:
      reinterpret_cast<List*>(baserel->fdw_private));

  return ret;
}

static void
cbBeginForeignScan( ForeignScanState *node,
                    int eflags )
{
}

static TupleTableSlot *
cbIterateForeignScan(ForeignScanState *node)
{
  TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
  ExecClearTuple(slot);
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

