/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "CRP.h"

#include "DCT.h"
#include "Normalise.h"
#include "LogCompress.h"
#include "OctaveFold.h"
#include "DCTReduce.h"

#include <cmath>
#include <iostream>

using namespace std;

CRP::~CRP()
{
    delete m_dctReduce;
}

RealBlock
CRP::process(const RealBlock &in)
{
    if (in.empty()) {
	return in;
    }
    if (!m_dctReduce) {
	m_size = in[0].size();
	m_dctReduce = new DCTReduce(m_size, m_params.coefficientsToDrop);
    }

    RealBlock out;

    for (RealColumn col: in) {

	if (m_params.applyLogCompression) {
            col = LogCompress::process(col, m_params.logFactor, m_params.logAddTerm);
	}

        out.push_back(Normalise::normalise
                      (OctaveFold::process
                       (m_dctReduce->process(col)),
                       m_params.normP));
    }        

    return out;
}

