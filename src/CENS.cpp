/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "CENS.h"

#include "Normalise.h"
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
		      (Normalise::normalise
		       (OctaveFold::process
			(Resize::process(col)),
			m_params.normP, m_params.normThresh)));
    }        

    return out;
}

