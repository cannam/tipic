/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef CHROMA_H
#define CHROMA_H

#include "Types.h"

//!!! Downsampling/temporal smoothing not yet implemented

//!!! doc: this class has no history, can be used in more than one
//!!! "channel" at once

class Chroma
{
public:
    struct Parameters {
	bool applyLogCompression;
	double logFactor;
	double logAddTerm;
        int normP; // 0 = no normalisation, 1 = L^1, 2 = L^2
        double normThresh;
	Parameters() :
            applyLogCompression(false),
            logFactor(100.0),
            logAddTerm(1.0),
            normP(2),
            normThresh(1e-6)
        { }
    };

    Chroma(Parameters params) : m_params(params) { }
    ~Chroma();

    RealBlock process(const RealBlock &in);

private:
    Parameters m_params;
};

#endif

