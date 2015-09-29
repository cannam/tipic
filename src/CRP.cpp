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

    // The chroma processing chain requires a 120-bin pitch filterbank
    // output, even though ours only actually contains 88 bins.

    int size = 120;
    
    if (!m_dctReduce) {
	m_dctReduce = new DCTReduce(size, m_params.coefficientsToDrop);
    }

    RealBlock out;

    for (RealColumn col: in) {

	if (m_params.applyLogCompression) {
            col = LogCompress::process(col, m_params.logFactor, m_params.logAddTerm);
	}

        RealColumn resized(20, 0.0);
        resized.insert(resized.end(), col.begin(), col.end());
        resized.resize(size);
        
        out.push_back(Normalise::normalise
                      (OctaveFold::process
                       (m_dctReduce->process(resized)),
                       m_params.normP, m_params.normThresh));
    }        

    return out;
}

