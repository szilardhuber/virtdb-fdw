MODULE_big = virtdb_fdw
PROTO_OBJECTS = src/proto/common.pb.o src/proto/data.pb.o
OBJS = src/virtdb_fdw_main.o src/virtdb_fdw.o $(PROTO_OBJECTS)
EXTENSION = src/virtdb_fdw
EXTVERSION = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
SHLIB_LINK = -lstdc++
DATA = $(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = $(EXTENSION)--$(EXTVERSION).sql
PG_CONFIG ?= $(shell which pg_config)
ifeq ($(PG_CONFIG), )
$(info $$PG_CONFIG is [${PG_CONFIG}])
PG_CONFIG = $(shell which /usr/local/pgsql/bin/pg_config)
endif
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

LDFLAGS += $(FIX_CXX_11_BUG) $(PG_LIBS)

all: $(EXTENSION)--$(EXTVERSION).sql

src/virtdb_fdw.o: $(PROTO_OBJECTS)

%.pb.o: %.pb.cc

%.pb.cc: %.proto
	protoc -I src/proto --cpp_out=src/proto/ $<

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	echo $< $@
	cp $< $@
