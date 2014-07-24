CREATE OR REPLACE FUNCTION virtdb_fdw_status()
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION virtdb_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION virtdb_fdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER virtdb_fdw
HANDLER virtdb_fdw_handler
VALIDATOR virtdb_fdw_validator;

-- NOTE : we need to 'create extension virtdb_fdw;' in order to use this


