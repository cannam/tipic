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

#ifndef CRP_H
#define CRP_H

#include "Types.h"
#include "DCTReduce.h"

/**
 * Turn features obtained from a PitchFilterbank into timbre-invariant
 * pitch chroma (CRP) features. No downsampling/smoothing is applied.
 *
 * This class retains no internal history, so a single instance could
 * be used for multiple channels at once, interleaved, within a single
 * thread. It is not thread-safe.
 */
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

    /**
     * Process a block as produced by PitchFilterbank.
     */
    RealBlock process(const RealBlock &in);

    /**
     * Process a single column.
     */
    RealColumn process(RealColumn col);
    
private:
    Parameters m_params;
    DCTReduce m_dctReduce;
};

#endif

