EXTENSION = src/virtdb_fdw
EXTVERSION = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

all: $(EXTENSION)--$(EXTVERSION).sql gtest-pkg-build-all test-build-all

BUILD_ROOT := $(shell pwd)
include ./common.mk

MODULE_big = virtdb_fdw
OBJS = src/virtdb_fdw_main.o src/virtdb_fdw.o $(PROTO_OBJECTS)
SHLIB_LINK = -lstdc++
DATA = $(EXTENSION)--$(EXTVERSION).sql
EXTRA_CLEAN = $(EXTENSION)--$(EXTVERSION).sql
PG_CONFIG ?= $(shell which pg_config)
ifeq ($(PG_CONFIG), )
$(info $$PG_CONFIG is [${PG_CONFIG}])
PG_CONFIG = $(shell which /usr/local/pgsql/bin/pg_config)
endif
PGXS := $(shell $(PG_CONFIG) --pgxs)
PG_CPPFLAGS := $(ZMQ_CFLAGS) $(PROTOBUF_CFLAGS)
PG_LIBS := -lstdc++ $(ZMQ_LDFLAGS) $(PROTOBUF_LDFLAGS)

include $(PGXS)

LDFLAGS += $(PG_LIBS)

gtest-pkg-build-all: gtest-pkg-configure gtest-pkg-lib

gtest-pkg-configure: $(GTEST_CONFIG_STATUS)

$(GTEST_CONFIG_STATUS):
	@echo "doing configure in gtest in " $(GTEST_PATH) 
	cd $(GTEST_PATH) ; ./configure
	@echo "configure done in gtest"

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

$(EXTENSION)--$(EXTVERSION).sql: $(EXTENSION).sql
	echo $< $@
	cp $< $@

virtdb-clean: test-build-clean gtest-pkg-clean clean
	rm -f $(PROTO_OBJECTS) $(OBJS) $(shell find ./ -name "*.pb.*") $(EXTENSION)--$(EXTVERSION).sql
