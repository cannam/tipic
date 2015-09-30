/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef FEATURE_DOWNSAMPLE_H
#define FEATURE_DOWNSAMPLE_H

#include <vector>

#include "Types.h"

//!!! doc: class maintains state

class Filter;

class FeatureDownsample
{
public:
    struct Parameters {
	int featureSize;
	int downsampleFactor;
	int windowLength;
	Parameters() :
	    featureSize(1),
	    downsampleFactor(10),
	    windowLength(41)
	{ }
    };

    FeatureDownsample(Parameters params);
    ~FeatureDownsample();

    void reset();
    RealBlock process(const RealBlock &in);
    
private:
    Parameters m_params;
    std::vector<Filter *> m_filters;
    int m_toNext;
};

#endif
