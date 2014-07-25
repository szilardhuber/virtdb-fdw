BUILD_ROOT := $(shell pwd)
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
ZMQ_CFLAGS := $(shell pkg-config --cflags libzmq) -I./src/cppzmq
PROTOBUF_LDFLAGS := $(shell pkg-config --libs protobuf)
PROTOBUF_CFLAGS := $(shell pkg-config --cflags protobuf)
PG_CPPFLAGS := $(ZMQ_CFLAGS) $(PROTOBUF_CFLAGS)
PG_LIBS := -lstdc++ $(ZMQ_LDFLAGS) $(PROTOBUF_LDFLAGS)
# 
GTEST_PATH := $(BUILD_ROOT)/src/gtest-1.7.0
GTEST_CONFIG_STATUS := $(GTEST_PATH)/config.status
# FIXME integrate libtool better ...
GTEST_LIBDIR := $(GTEST_PATH)/lib/.libs/

# FIXME on Windows
FIX_CXX_11_BUG =
ifeq ($(shell uname), 'Linux')
FIX_CXX_11_BUG =  -Wl,--no-as-needed
endif

CXXFLAGS += -std=c++11 -fPIC -pthread $(FIX_CXX_11_BUG)

include $(PGXS)

LDFLAGS += $(FIX_CXX_11_BUG) $(PG_LIBS)

all: $(EXTENSION)--$(EXTVERSION).sql gtest-pkg-build-all test-build-all

gtest-pkg-build-all: gtest-pkg-configure gtest-pkg-lib

gtest-pkg-configure: $(GTEST_CONFIG_STATUS)

$(GTEST_CONFIG_STATUS):
	@echo "doing configure in gtest"
	@cd $(GTEST_PATH)
	./configure
	@echo "configure done in gtest"
	@cd $(BUILD_ROOT)

# NOTE: assumption: the libdir will be created during build
gtest-pkg-lib: $(GTEST_LIBDIR)

$(GTEST_LIBDIR):
	@echo "building the gtest package"
	@make -C $(GTEST_PATH)
	@echo "building finished in gtest package"

gtest-pkg-clean:
	@echo "cleaning the gtest package"
	@make -C $(GTEST_PATH) clean
	@echo "cleaning finished in gtest package"

test-build-all:
	@echo "building tests"
	make -C test/ all

test-build-clean:
	@echo "cleaning tests"
	make -C test/ clean

src/virtdb_fdw.o: $(PROTO_OBJECTS)

%.pb.o: %.pb.cc

%.pb.cc: %.proto
	protoc -I src/proto --cpp_out=src/proto/ $<

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	echo $< $@
	cp $< $@

virtdb-clean: test-build-clean gtest-pkg-clean
	rm -f $(PROTO_OBJECTS) $(OBJS) $(shell find ./ -name "*.pb.*") $(EXTENSION)--$(EXTVERSION).sql
