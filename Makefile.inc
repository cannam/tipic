
SRC_DIR  := src

VAMPSDK_DIR  ?= ../vamp-plugin-sdk

CQ_DIR	     ?= constant-q-cpp
BQVEC_DIR    ?= bqvec

PLUGIN_EXT	?= .so

CXX	?= g++
CC	?= gcc
AR	?= ar
RANLIB	?= ranlib

CFLAGS := $(CFLAGS)
CXXFLAGS := $(CFLAGS) -I. -I$(VAMPSDK_DIR) -I$(CQ_DIR) -I$(BQVEC_DIR) -I$(BQVEC_DIR)/bqvec -I$(CQ_DIR)/src/dsp $(CXXFLAGS)

LDFLAGS := $(LDFLAGS) 
PLUGIN_LDFLAGS := $(LDFLAGS) $(PLUGIN_LDFLAGS)

LIBRARY	       := libtipic.a
PLUGIN	       := tipic-vamp$(PLUGIN_EXT)

PUBLIC_HEADERS	:=

LIB_HEADERS	:= $(SRC_DIR)/delays.h $(SRC_DIR)/filter-a.h $(SRC_DIR)/filter-b.h $(SRC_DIR)/Filter.h $(SRC_DIR)/PitchFilterbank.h $(SRC_DIR)/DCT.h $(SRC_DIR)/Types.h $(SRC_DIR)/CRP.h
LIB_SOURCES	:= $(SRC_DIR)/Filter.cpp $(SRC_DIR)/PitchFilterbank.cpp $(SRC_DIR)/DCT.cpp $(SRC_DIR)/CRP.cpp
LIB_OBJECTS	:= $(LIB_SOURCES:.cpp=.o)
LIB_OBJECTS	:= $(LIB_OBJECTS:.c=.o)

PLUGIN_HEADERS  := $(SRC_DIR)/TipicVampPlugin.h
PLUGIN_SOURCES  := $(SRC_DIR)/TipicVampPlugin.cpp $(SRC_DIR)/libmain.cpp
PLUGIN_OBJECTS	:= $(PLUGIN_SOURCES:.cpp=.o)
PLUGIN_OBJECTS	:= $(PLUGIN_OBJECTS:.c=.o)

BQVEC_HEADERS	:= $(BQVEC_DIR)/Allocators.h $(BQVEC_DIR)/Restrict.h $(BQVEC_DIR)/VectorOps.h
BQVEC_SOURCES	:= $(BQVEC_DIR)/src/Allocators.cpp
BQVEC_OBJECTS	:= $(BQVEC_SOURCES:.cpp=.o)
BQVEC_OBJECTS	:= $(BQVEC_OBJECTS:.c=.o)

TEST_SOURCES	:= $(SRC_DIR)/test-filter.cpp $(SRC_DIR)/test-dct.cpp
TEST_OBJECTS	:= $(TEST_SOURCES:.cpp=.o)
TEST_OBJECTS	:= $(TEST_OBJECTS:.c=.o)

OTHER_OBJECTS	:= $(CQ_DIR)/src/dsp/FFT.o $(CQ_DIR)/src/ext/kissfft/kiss_fft.o $(CQ_DIR)/src/ext/kissfft/tools/kiss_fftr.o

HEADERS	     := $(PUBLIC_HEADERS) $(LIB_HEADERS) $(PLUGIN_HEADERS) $(BQVEC_HEADERS)
SOURCES	     := $(PUBLIC_SOURCES) $(LIB_SOURCES) $(PLUGIN_SOURCES) $(BQVEC_SOURCES) $(TEST_SOURCES)
OBJECTS	     := $(SOURCES:.cpp=.o)
OBJECTS	     := $(OBJECTS:.c=.o)

LIBS	:= $(CQ_DIR)/libcq.a $(VAMPSDK_DIR)/libvamp-sdk.a

all: constant-q-cpp $(LIBRARY) $(PLUGIN) tests

.PHONY: constant-q-cpp
constant-q-cpp: 
	$(MAKE) -C $@ -f Makefile$(MAKEFILE_EXT) libcq.a

$(PLUGIN):	$(PLUGIN_OBJECTS) $(LIB_OBJECTS) $(BQVEC_OBJECTS) $(LIBS)
	$(CXX) -o $@ $^ $(LIBS) $(PLUGIN_LDFLAGS)

$(LIBRARY):    $(LIB_OBJECTS) $(BQVEC_OBJECTS) $(OTHER_OBJECTS)
	$(RM) -f $@
	$(AR) cr $@ $^
	$(RANLIB) $@

.PHONY:	tests
tests:	test-dct test-filter
	./test-dct
	./test-filter

test-dct:	  $(TEST_OBJECTS) $(LIBRARY)
	$(CXX) -o $@ src/test-dct.o $(LIBRARY)

test-filter:	  $(TEST_OBJECTS) $(LIBRARY)
	$(CXX) -o $@ src/test-filter.o $(LIBRARY)

clean:		
	rm -f $(OBJECTS)
	$(MAKE) -C constant-q-cpp -f Makefile$(MAKEFILE_EXT) clean

distclean:	clean
	rm -f $(PLUGIN)

depend:
	makedepend -I$(BQVEC_DIR) -Y -fMakefile.inc $(SOURCES) $(HEADERS)

# DO NOT DELETE

src/Filter.o: src/Filter.h bqvec/bqvec/Restrict.h bqvec/bqvec/VectorOps.h
src/Filter.o: bqvec/bqvec/Restrict.h bqvec/bqvec/Allocators.h
src/Filter.o: bqvec/bqvec/VectorOps.h
src/PitchFilterbank.o: src/PitchFilterbank.h src/Filter.h
src/PitchFilterbank.o: bqvec/bqvec/Restrict.h src/delays.h src/filter-a.h
src/PitchFilterbank.o: src/filter-b.h
src/TipicVampPlugin.o: src/TipicVampPlugin.h src/PitchFilterbank.h
src/TipicVampPlugin.o: bqvec/bqvec/Range.h bqvec/bqvec/VectorOps.h
src/TipicVampPlugin.o: bqvec/bqvec/Restrict.h
src/libmain.o: src/TipicVampPlugin.h src/PitchFilterbank.h
src/Filter.o: bqvec/bqvec/Restrict.h
src/TipicVampPlugin.o: src/PitchFilterbank.h
