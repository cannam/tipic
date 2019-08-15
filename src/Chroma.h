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

#ifndef CHROMA_H
#define CHROMA_H

#include "Types.h"

/**
 * Turn features obtained from a PitchFilterbank into pitch chroma
 * features (without the timbre-invariant DCT step used for CRP
 * features). No downsampling/smoothing is applied.
 *
 * This class retains no internal history, so a single instance could
 * be used for multiple channels at once, interleaved, within a single
 * thread. It is not thread-safe.
 */
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

    Chroma(Parameters params);
    ~Chroma();

    RealBlock process(const RealBlock &in);

private:
    Parameters m_params;
};

#endif

