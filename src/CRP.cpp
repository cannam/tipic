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

#include "CRP.h"

#include "maths/MathUtilities.h"
#include "LogCompress.h"
#include "OctaveFold.h"
#include "Resize.h"

#include <cmath>
#include <iostream>

using namespace std;

CRP::CRP(Parameters params) :
    m_params(params),
    m_dctReduce(120, params.coefficientsToDrop)
{}

CRP::~CRP()
{}

RealBlock
CRP::process(const RealBlock &in)
{
    if (in.empty()) {
	return in;
    }

    RealBlock out;

    for (RealColumn col: in) {
        out.push_back(process(col));
    }

    return out;
}

RealColumn
CRP::process(RealColumn col)
{
    if (m_params.applyLogCompression) {
        col = LogCompress::process
            (col, m_params.logFactor, m_params.logAddTerm);
    }

    return MathUtilities::normaliseLp
        (OctaveFold::process
         (m_dctReduce.process
          (Resize::process(col))),
         m_params.normP, m_params.normThresh);
}

