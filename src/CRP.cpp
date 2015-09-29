/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "CRP.h"

#include "Normalise.h"
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

	if (m_params.applyLogCompression) {
            col = LogCompress::process(col, m_params.logFactor, m_params.logAddTerm);
	}

        out.push_back(Normalise::normalise
                      (OctaveFold::process
                       (m_dctReduce.process
                        (Resize::process(col))),
                       m_params.normP, m_params.normThresh));
    }        

    return out;
}

