MODULE_big = virtdb_fdw
OBJS = virtdb_fdw_main.o virtdb_fdw.o
EXTENSION = virtdb_fdw
EXTVERSION = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
PG_CPPFLAGS = -I../.. 
PG_LIBS = -lstdc++ 
SHLIB_LINK = -lstdc++ 
DATA = $(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = $(EXTENSION)--$(EXTVERSION).sql
PG_CONFIG ?= $(shell which pg_config)
PGXS := $(shell $(PG_CONFIG) --pgxs)

# FIXME on Windows
FIX_CXX_11_BUG := ""
ifeq ($(shell uname), 'Linux')
FIX_CXX_11_BUG =  -Wl,--no-as-needed
endif

CXXFLAGS += -std=c++11 -fPIC -pthread $(FIX_CXX_11_BUG)

include $(PGXS)

LDFLAGS += -lstdc++ -pthread $(FIX_CXX_11_BUG)

all: $(EXTENSION)--$(EXTVERSION).sql

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	cp $< $@ 

