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

#ifndef CENS_H
#define CENS_H

#include "Types.h"
#include "Quantize.h"

class CENS
{
public:
    struct Parameters {
    public:
	std::vector<double> quantSteps;
	std::vector<double> quantWeights;
        int normP; // 0 = no normalisation, 1 = L^1, 2 = L^2
        double normThresh;
	Parameters() :
	    quantSteps({ 0.4, 0.2, 0.1, 0.05 }),
	    quantWeights({ 0.25, 0.25, 0.25, 0.25 }),
            normP(1),
            normThresh(1e-3)
        { }
    };

    CENS(Parameters params);
    ~CENS();

    RealBlock process(const RealBlock &in);

private:
    Parameters m_params;
    Quantize m_quantize;
};

#endif

