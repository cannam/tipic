/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef CRP_H
#define CRP_H

#include "Types.h"
#include "DCTReduce.h"

//!!! Downsampling/temporal smoothing not yet implemented

//!!! doc: this class has no history, can be used in more than one
//!!! "channel" at once

class CRP
{
public:
    struct Parameters {
	int coefficientsToDrop;
	bool applyLogCompression;
        double logFactor;
        double logAddTerm;
        int normP; // 0 = no normalisation, 1 = L^1, 2 = L^2
        double normThresh;
	Parameters() :
            coefficientsToDrop(55),
            applyLogCompression(true),
            logFactor(1000.0),
            logAddTerm(1.0),
            normP(2),
            normThresh(1e-6)
        { }
    };
	
    CRP(Parameters params);
    ~CRP();

    RealBlock process(const RealBlock &in);
    
private:
    Parameters m_params;
    DCTReduce m_dctReduce;
};

#endif

