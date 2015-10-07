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

#include "CENS.h"

#include "maths/MathUtilities.h"
#include "OctaveFold.h"
#include "Resize.h"

#include <cmath>
#include <iostream>

using namespace std;

static Quantize::Parameters
qparams(CENS::Parameters params)
{
    Quantize::Parameters qp;
    qp.steps = params.quantSteps;
    qp.weights = params.quantWeights;
    return qp;
}

CENS::CENS(Parameters params) :
    m_params(params),
    m_quantize(qparams(params))
{
}

CENS::~CENS()
{
}

RealBlock
CENS::process(const RealBlock &in)
{
    if (in.empty()) {
	return in;
    }

    RealBlock out;

    for (RealColumn col: in) {

        out.push_back(m_quantize.process
		      (MathUtilities::normaliseLp
		       (OctaveFold::process
			(Resize::process(col)),
			m_params.normP, m_params.normThresh)));
    }        

    return out;
}

