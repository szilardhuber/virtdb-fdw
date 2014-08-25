# BUILD_ROOT is required
# XXX EXTRA_CLEAN = $(EXTENSION)--$(EXTVERSION).sql
COMMON_FILE_NAMES := expression.cc query.cc receiver_thread.cc data_handler.cc
PROTO_FILE_NAMES := common.proto meta_data.proto db_config.proto data.proto
PROTO_FILES := $(patsubst %.proto,$(BUILD_ROOT)/src/proto/%.proto,$(PROTO_FILE_NAMES))
COMMON_SOURCES :=  $(patsubst %.cc,$(BUILD_ROOT)/src/%.cc,$(COMMON_FILE_NAMES))
PROTO_SOURCES := $(patsubst %.proto,%.pb.cc,$(PROTO_FILES))
PROTO_OBJECTS := $(patsubst %.pb.cc,%.pb.o,$(PROTO_SOURCES))
ZMQ_LDFLAGS := $(shell pkg-config --libs libzmq)
ZMQ_CFLAGS := $(shell pkg-config --cflags libzmq) -I$(BUILD_ROOT)/src/cppzmq
PROTOBUF_LDFLAGS := $(shell pkg-config --libs protobuf)
PROTOBUF_CFLAGS := $(shell pkg-config --cflags protobuf)
PROTOBUF_PATH := $(BUILD_ROOT)/src/proto/
GTEST_PATH := $(BUILD_ROOT)/src/gtest
GTEST_CONFIG_STATUS := $(GTEST_PATH)/config.status
# FIXME integrate libtool better ...
GTEST_LIBDIR := $(GTEST_PATH)/lib/.libs/
GTEST_INCLUDE := $(GTEST_PATH)/include
GTEST_LIBS :=  $(GTEST_LIBDIR)/libgtest.a
GTEST_LDFLAGS := $(GTEST_LIBS) -L$(GTEST_LIBDIR)
GTEST_CFLAGS := -I$(GTEST_INCLUDE)

COMMON_OBJS := $(patsubst %.cc,%.o,$(COMMON_SOURCES))

# FIXME on Windows
FIX_CXX_11_BUG :=
LINUX_LDFLAGS :=
ifeq ($(shell uname), Linux)
FIX_CXX_11_BUG :=  -Wl,--no-as-needed
LINUX_LDFLAGS :=  -pthread
endif



CXXFLAGS += -std=c++11 -fPIC $(FIX_CXX_11_BUG) $(LINUX_LDFLAGS) $(PROTOBUF_CFLAGS) $(ZMQ_CFLAGS) $(CPPFLAGS) -g3
LDFLAGS += $(FIX_CXX_11_BUG) $(LINUX_LDFLAGS) $(PROTOBUF_LDFLAGS) $(ZMQ_LDFLAGS) -g3

%.pb.o: %.pb.cc

%.pb.cc: %.proto
	protoc -I $(BUILD_ROOT)/src/proto --cpp_out=$(BUILD_ROOT)/src/proto/ $<
