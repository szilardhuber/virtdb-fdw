
GTEST_PATH := ../src/gtest-1.7.0/
GTEST_INCLUDE := $(GTEST_PATH)/include
GTEST_LIBDIR := $(GTEST_PATH)/lib/.libs
GTEST_LIBS :=  $(GTEST_LIBDIR)/libgtest.a

# FIXME on Windows
FIX_CXX_11_BUG =
ifeq ($(shell uname), 'Linux')
FIX_CXX_11_BUG =  -Wl,--no-as-needed
endif

CXXFLAGS += -std=c++11 -fPIC -pthread $(FIX_CXX_11_BUG) -I$(GTEST_INCLUDE)
LDFLAGS += $(FIX_CXX_11_BUG) -L$(GTEST_LIBDIR) $(GTEST_LIBS)

all: gtest_test

gtest_test: gtest_test.o
	g++ -o $@ $< $(LDFLAGS)

%.o: %.cc
	g++ -c -o $@ $< $(CXXFLAGS)

gtest_test: gtest_test.o

clean:
	rm -f *.o gtest_test


