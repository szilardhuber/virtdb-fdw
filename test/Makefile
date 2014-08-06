all: gtest_test

BUILD_ROOT := $(shell pwd)/../
include ../common.mk

CXXFLAGS += $(GTEST_CFLAGS)
LDFLAGS += $(GTEST_LDFLAGS)

TESTS := *_test.hh

gtest_test: gtest_test.o $(COMMON_OBJECTS)
	g++ -o $@ $< $(COMMON_OBJS) $(LDFLAGS)

%.o: %.cc $(TESTS)
	g++ -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f *.o gtest_test
