/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "FeatureDownsample.h"

#include "Filter.h"
#include "Window.h"
#include "Normalise.h"

#include <stdexcept>

using namespace std;

FeatureDownsample::FeatureDownsample(Parameters params) :
    m_params(params)
{
    if (params.downsampleFactor < 2 || params.windowLength < 2) {
	throw invalid_argument
	    ("Expected downsampleFactor and windowLength each to be at least 2");
    }

    // Our windows are periodic rather than symmetric, but we want a
    // symmetric window here
    Window<double> w(HanningWindow, params.windowLength + 1);
    vector<double> wdat(w.getWindowData());;
    vector<double> wd(wdat.begin()+1, wdat.end());

    double divisor = 0.0;
    for (auto x: wd) divisor += x;
    for (auto &x: wd) x /= divisor;

    // FIR filter
    for (int i = 0; i < m_params.featureSize; ++i) {
	m_filters.push_back(new Filter({ {}, wd }));
    }

    m_toNext = 1;
    m_toDrop = m_params.windowLength / 2;
    m_inCount = 0;
    m_outCount = 0;
}

FeatureDownsample::~FeatureDownsample()
{
    for (auto &f: m_filters) delete f;
}

void
FeatureDownsample::reset()
{
    for (auto &f: m_filters) f->reset();
    m_toNext = 1;
    m_toDrop = m_params.windowLength / 2;
    m_inCount = 0;
    m_outCount = 0;
}

RealBlock
FeatureDownsample::process(const RealBlock &in)
{
    RealBlock out;
    
    for (const auto &col: in) {
	RealColumn outcol;
	if (m_toDrop > 0) {
	    --m_toDrop;
	} else {
	    --m_toNext;
	}
	for (int i = 0; i < m_params.featureSize; ++i) {
	    double val = 0.0;
	    m_filters[i]->process(&col[i], &val, 1);
	    if (m_toNext == 0) {
		outcol.push_back(val);
	    }
	}
	if (m_toNext == 0) {
	    out.push_back(Normalise::normalise
			  (outcol, m_params.normP, m_params.normThresh));
	    m_toNext = m_params.downsampleFactor;
	    ++m_outCount;
	}
	++m_inCount;
    }
    
    return out;
}

RealBlock
FeatureDownsample::getRemainingOutput()
{
    RealBlock pad(m_params.windowLength, RealColumn(m_params.featureSize, 0.0));
    RealBlock tail = process(pad);
    int expected = m_inCount / m_params.downsampleFactor;
    RealBlock out;
    for (int i = 0;
	 m_outCount < expected && i < int(tail.size());
	 ++i, ++m_outCount) {
	out.push_back(Normalise::normalise
		      (tail[i], m_params.normP, m_params.normThresh));
    }
    return out;
}


