/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef CRP_H
#define CRP_H

#include "Types.h"

class DCT;

//!!! Downsampling/temporal smoothing not yet implemented

class CRP
{
public:
    struct Parameters {
	int coefficientsToDrop;
	bool applyLogCompression;
	Parameters() : coefficientsToDrop(54), applyLogCompression(true) { }
    };
	
    CRP(Parameters params) : m_params(params), m_size(0), m_dct(0) { }
    ~CRP();

    RealBlock process(const RealBlock &in);
    
private:
    Parameters m_params;
    int m_size;
    DCT *m_dct;
};

#endif

