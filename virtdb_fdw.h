#ifndef _VIRTDB_FDW_H_INCLUDED_
#define _VIRTDB_FDW_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <postgres.h>
#include <fmgr.h>

/* forwarding function entry points so we can implement them in C++ */
void PG_init_virtdb_fdw_cpp(void);
void PG_fini_virtdb_fdw_cpp(void);
Datum virtdb_fdw_status_cpp(PG_FUNCTION_ARGS);
Datum virtdb_fdw_handler_cpp(PG_FUNCTION_ARGS);
Datum virtdb_fdw_validator_cpp(PG_FUNCTION_ARGS);

extern zmq::context_t zmq_context;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_VIRTDB_FDW_H_INCLUDED_*/
