
SRC_DIR  := src

VAMPSDK_DIR  ?= ../vamp-plugin-sdk

QMDSP_DIR    := qm-dsp

PLUGIN_EXT	?= .so

CXX	?= g++
CC	?= gcc
AR	?= ar
RANLIB	?= ranlib

CFLAGS := $(CFLAGS) -I$(QMDSP_DIR)/ext/kissfft -I$(QMDSP_DIR)/ext/kissfft/tools -Dkiss_fft_scalar=double
CXXFLAGS := $(CFLAGS) -I. -I$(VAMPSDK_DIR) -I$(QMDSP_DIR) $(CXXFLAGS)

LDFLAGS := $(LDFLAGS) 
PLUGIN_LDFLAGS := $(LDFLAGS) $(PLUGIN_LDFLAGS)

LIBRARY	       := libtipic.a
PLUGIN	       := tipic-vamp$(PLUGIN_EXT)

PUBLIC_HEADERS	:=

LIB_HEADERS	:= \
		$(SRC_DIR)/delays.h \
		$(SRC_DIR)/filter-a.h \
		$(SRC_DIR)/filter-b.h \
		$(SRC_DIR)/PitchFilterbank.h \
		$(SRC_DIR)/Types.h \
		$(SRC_DIR)/CRP.h \
		$(SRC_DIR)/LogCompress.h \
		$(SRC_DIR)/OctaveFold.h \
		$(SRC_DIR)/Resize.h \
		$(SRC_DIR)/Chroma.h \
		$(SRC_DIR)/FeatureDownsample.h \
		$(SRC_DIR)/Quantize.h \
		$(SRC_DIR)/CENS.h \
		$(QMDSP_DIR)/dsp/signalconditioning/Filter.h \
		$(QMDSP_DIR)/dsp/transforms/DCT.h \
		$(QMDSP_DIR)/dsp/transforms/FFT.h \
		$(QMDSP_DIR)/dsp/rateconversion/Resampler.h \
		$(QMDSP_DIR)/maths/MathUtilities.h \
		$(QMDSP_DIR)/base/Window.h \
		$(QMDSP_DIR)/base/KaiserWindow.h \
		$(QMDSP_DIR)/base/SincWindow.h \
		$(QMDSP_DIR)/ext/kissfft/tools/kiss_fftr.h
		
LIB_SOURCES	:= \
		$(SRC_DIR)/PitchFilterbank.cpp \
		$(SRC_DIR)/CRP.cpp \
		$(SRC_DIR)/Chroma.cpp \
		$(SRC_DIR)/FeatureDownsample.cpp \
		$(SRC_DIR)/CENS.cpp \
		$(QMDSP_DIR)/dsp/signalconditioning/Filter.cpp \
		$(QMDSP_DIR)/dsp/transforms/DCT.cpp \
		$(QMDSP_DIR)/dsp/transforms/FFT.cpp \
		$(QMDSP_DIR)/dsp/rateconversion/Resampler.cpp \
		$(QMDSP_DIR)/maths/MathUtilities.cpp \
		$(QMDSP_DIR)/base/KaiserWindow.cpp \
		$(QMDSP_DIR)/base/SincWindow.cpp \
		$(QMDSP_DIR)/ext/kissfft/kiss_fft.c \
		$(QMDSP_DIR)/ext/kissfft/tools/kiss_fftr.c
		
LIB_OBJECTS	:= $(LIB_SOURCES:.cpp=.o)
LIB_OBJECTS	:= $(LIB_OBJECTS:.c=.o)

PLUGIN_HEADERS  := $(SRC_DIR)/TipicVampPlugin.h
PLUGIN_SOURCES  := $(SRC_DIR)/TipicVampPlugin.cpp $(SRC_DIR)/libmain.cpp
PLUGIN_OBJECTS	:= $(PLUGIN_SOURCES:.cpp=.o)
PLUGIN_OBJECTS	:= $(PLUGIN_OBJECTS:.c=.o)

TEST_SOURCES	:= $(SRC_DIR)/test-filter.cpp $(SRC_DIR)/test-dct.cpp $(SRC_DIR)/test-normalise.cpp
TEST_OBJECTS	:= $(TEST_SOURCES:.cpp=.o)
TEST_OBJECTS	:= $(TEST_OBJECTS:.c=.o)

HEADERS	     := $(PUBLIC_HEADERS) $(LIB_HEADERS) $(PLUGIN_HEADERS)
SOURCES	     := $(PUBLIC_SOURCES) $(LIB_SOURCES) $(PLUGIN_SOURCES) $(TEST_SOURCES)
OBJECTS	     := $(SOURCES:.cpp=.o)
OBJECTS	     := $(OBJECTS:.c=.o)

LIBS	     := $(VAMPSDK_DIR)/libvamp-sdk.a

all: $(LIBRARY) $(PLUGIN)

$(PLUGIN):	$(PLUGIN_OBJECTS) $(LIB_OBJECTS) $(LIBS)
	$(CXX) -o $@ $^ $(LIBS) $(PLUGIN_LDFLAGS)

$(LIBRARY):    $(LIB_OBJECTS)
	$(RM) -f $@
	$(AR) cr $@ $^
	$(RANLIB) $@

clean:		
	rm -f $(OBJECTS)

distclean:	clean
	rm -f $(PLUGIN)

depend:
	makedepend -Y -fMakefile.inc $(SOURCES) $(HEADERS)
	
# DO NOT DELETE

src/PitchFilterbank.o: src/PitchFilterbank.h src/Types.h src/delays.h
src/PitchFilterbank.o: src/filter-a.h src/filter-b.h
src/CRP.o: src/CRP.h src/Types.h src/DCTReduce.h src/LogCompress.h
src/CRP.o: src/OctaveFold.h src/Resize.h
src/Chroma.o: src/Chroma.h src/Types.h src/LogCompress.h src/OctaveFold.h
src/Chroma.o: src/Resize.h
src/FeatureDownsample.o: src/FeatureDownsample.h src/Types.h
src/CENS.o: src/CENS.h src/Types.h src/Quantize.h src/OctaveFold.h
src/CENS.o: src/Resize.h
qm-dsp/dsp/signalconditioning/Filter.o: qm-dsp/dsp/signalconditioning/Filter.h
qm-dsp/dsp/transforms/DCT.o: qm-dsp/dsp/transforms/DCT.h
qm-dsp/dsp/transforms/DCT.o: qm-dsp/dsp/transforms/FFT.h
qm-dsp/dsp/transforms/FFT.o: qm-dsp/dsp/transforms/FFT.h
qm-dsp/dsp/rateconversion/Resampler.o: qm-dsp/dsp/rateconversion/Resampler.h
qm-dsp/maths/MathUtilities.o: qm-dsp/maths/MathUtilities.h
qm-dsp/maths/MathUtilities.o: qm-dsp/maths/nan-inf.h
qm-dsp/base/KaiserWindow.o: qm-dsp/base/KaiserWindow.h
qm-dsp/base/SincWindow.o: qm-dsp/base/SincWindow.h
qm-dsp/ext/kissfft/kiss_fft.o: qm-dsp/ext/kissfft/_kiss_fft_guts.h
qm-dsp/ext/kissfft/kiss_fft.o: qm-dsp/ext/kissfft/kiss_fft.h
qm-dsp/ext/kissfft/tools/kiss_fftr.o: qm-dsp/ext/kissfft/tools/kiss_fftr.h
qm-dsp/ext/kissfft/tools/kiss_fftr.o: qm-dsp/ext/kissfft/kiss_fft.h
qm-dsp/ext/kissfft/tools/kiss_fftr.o: qm-dsp/ext/kissfft/_kiss_fft_guts.h
src/TipicVampPlugin.o: src/TipicVampPlugin.h src/Types.h
src/TipicVampPlugin.o: src/PitchFilterbank.h src/CRP.h src/DCTReduce.h
src/TipicVampPlugin.o: src/Chroma.h src/CENS.h src/Quantize.h
src/TipicVampPlugin.o: src/FeatureDownsample.h
src/libmain.o: src/TipicVampPlugin.h src/Types.h
src/test-filter.o: qm-dsp/dsp/signalconditioning/Filter.h
src/test-dct.o: qm-dsp/dsp/transforms/DCT.h qm-dsp/dsp/transforms/FFT.h
src/PitchFilterbank.o: src/Types.h
src/CRP.o: src/Types.h src/DCTReduce.h
src/Chroma.o: src/Types.h
src/FeatureDownsample.o: src/Types.h
src/CENS.o: src/Types.h src/Quantize.h
qm-dsp/dsp/transforms/DCT.o: qm-dsp/dsp/transforms/FFT.h
qm-dsp/maths/MathUtilities.o: qm-dsp/maths/nan-inf.h
qm-dsp/ext/kissfft/tools/kiss_fftr.o: qm-dsp/ext/kissfft/kiss_fft.h
src/TipicVampPlugin.o: src/Types.h
