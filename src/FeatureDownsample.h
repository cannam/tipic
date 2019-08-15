/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Tipic

    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef FEATURE_DOWNSAMPLE_H
#define FEATURE_DOWNSAMPLE_H

#include <vector>

#include "Types.h"

/**
 * Downsample a feature by a factor in time.
 *
 * This class retains internal history, so a single instance per
 * channel or feature type is required. It is also not thread-safe.
 */
class Filter;

class FeatureDownsample
{
public:
    struct Parameters {
	int featureSize;
	int downsampleFactor;
	int windowLength;
        int normP; // 0 = no normalisation, 1 = L^1, 2 = L^2
        double normThresh;
	Parameters() :
	    featureSize(1),
	    downsampleFactor(10),
	    windowLength(41),
	    normP(2),
	    normThresh(1e-6)
	{ }
    };

    FeatureDownsample(Parameters params);
    ~FeatureDownsample();

    void reset();
    RealBlock process(const RealBlock &in);
    RealBlock getRemainingOutput();
    
private:
    Parameters m_params;
    std::vector<Filter *> m_filters;
    int m_toDrop;
    int m_toNext;
    int m_inCount;
    int m_outCount;
};

#endif
