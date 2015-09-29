/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef CRP_H
#define CRP_H

#include "Types.h"

class DCTReduce;

//!!! Downsampling/temporal smoothing not yet implemented

class CRP
{
public:
    struct Parameters {
	int coefficientsToDrop;
	bool applyLogCompression;
        double logFactor;
        double logAddTerm;
        int normP;
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
	
    CRP(Parameters params) : m_params(params), m_dctReduce(0) { }
    ~CRP();

    RealBlock process(const RealBlock &in);
    
private:
    Parameters m_params;
    DCTReduce *m_dctReduce;
};

#endif

