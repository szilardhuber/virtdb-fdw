#include "virtdb_fdw.h"

/* forward declarations of the PG C handlers */
extern void _PG_init(void);
extern void _PG_fini(void);
extern Datum virtdb_fdw_status(PG_FUNCTION_ARGS);
extern Datum virtdb_fdw_handler(PG_FUNCTION_ARGS);
extern Datum virtdb_fdw_validator(PG_FUNCTION_ARGS);

/* postgres integration */
PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(virtdb_fdw_status);
PG_FUNCTION_INFO_V1(virtdb_fdw_handler);
PG_FUNCTION_INFO_V1(virtdb_fdw_validator);

/* implementation is forward everything to C++ */
extern void _PG_init(void) { PG_init_virtdb_fdw_cpp(); }
extern void _PG_fini(void) { PG_fini_virtdb_fdw_cpp(); }
extern Datum virtdb_fdw_status(PG_FUNCTION_ARGS) { return virtdb_fdw_status_cpp(fcinfo); }
extern Datum virtdb_fdw_handler(PG_FUNCTION_ARGS) { return virtdb_fdw_handler_cpp(fcinfo); }
extern Datum virtdb_fdw_validator(PG_FUNCTION_ARGS) { return virtdb_fdw_validator_cpp(fcinfo); }

