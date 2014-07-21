MODULE_big = virtdb_fdw
OBJS = virtdb_fdw_main.o virtdb_fdw.o protobuf/data.pb.o
EXTENSION = virtdb_fdw
EXTVERSION = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
SHLIB_LINK = -lstdc++ 
DATA = $(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = $(EXTENSION)--$(EXTVERSION).sql
PG_CONFIG ?= $(shell which pg_config)
PGXS := $(shell $(PG_CONFIG) --pgxs)
ZMQ_LDFLAGS := $(shell pkg-config --libs libzmq)
ZMQ_CFLAGS := $(shell pkg-config --cflags libzmq)
PROTOBUF_LDFLAGS := $(shell pkg-config --libs protobuf)
PROTOBUF_CFLAGS := $(shell pkg-config --cflags protobuf)
PG_CPPFLAGS := $(ZMQ_CFLAGS) $(PROTOBUF_CFLAGS)
PG_LIBS := -lstdc++ $(ZMQ_LDFLAGS) $(PROTOBUF_LDFLAGS)

# FIXME on Windows
FIX_CXX_11_BUG = 
ifeq ($(shell uname), 'Linux')
FIX_CXX_11_BUG =  -Wl,--no-as-needed
endif

CXXFLAGS += -std=c++11 -fPIC -pthread $(FIX_CXX_11_BUG)

include $(PGXS)

LDFLAGS += -lstdc++ $(FIX_CXX_11_BUG)

all: $(EXTENSION)--$(EXTVERSION).sql

protobuf/data.pb.o: protobuf/data.pb.cc

protobuf/data.pb.cc: protobuf/data.proto
	make -C protobuf all

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	cp $< $@ 

