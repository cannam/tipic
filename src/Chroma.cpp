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

#include "Chroma.h"

#include "maths/MathUtilities.h"
#include "LogCompress.h"
#include "OctaveFold.h"
#include "Resize.h"

#include <cmath>
#include <iostream>

using namespace std;

Chroma::Chroma(Parameters params) :
    m_params(params)
{
}

Chroma::~Chroma()
{
}

RealBlock
Chroma::process(const RealBlock &in)
{
    if (in.empty()) {
	return in;
    }

    RealBlock out;

    for (RealColumn col: in) {

	if (m_params.applyLogCompression) {
            col = LogCompress::process(col, m_params.logFactor, m_params.logAddTerm);
	}

        out.push_back(MathUtilities::normaliseLp
                      (OctaveFold::process
		       (Resize::process(col)),
		       m_params.normP, m_params.normThresh));
    }        

    return out;
}

