/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "FeatureDownsample.h"

#include "Filter.h"
#include "Window.h"

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
    Window<double> w(HanningWindow, params.windowLength - 1);
    vector<double> wd(w.getWindowData());
    wd.push_back(wd[0]);

    // FIR filter
    for (int i = 0; i < m_params.featureSize; ++i) {
	m_filters.push_back(new Filter({ {}, wd }));
    }

    m_toNext = params.downsampleFactor;
}

FeatureDownsample::~FeatureDownsample()
{
    for (auto &f: m_filters) delete f;
}

void
FeatureDownsample::reset()
{
    for (auto &f: m_filters) f->reset();
}

RealBlock
FeatureDownsample::process(const RealBlock &in)
{
    //!!! todo: adjust for delay
    
    RealBlock out;
    for (const auto &col: in) {
	RealColumn outcol;
	--m_toNext;
	for (int i = 0; i < m_params.featureSize; ++i) {
	    double val = 0.0;
	    m_filters[i]->process(&col[i], &val, 1);
	    if (m_toNext == 0) {
		outcol.push_back(val);
	    }
	}
	if (m_toNext == 0) {
	    out.push_back(outcol);
	    m_toNext = m_params.downsampleFactor;
	}
    }
    return out;
}

