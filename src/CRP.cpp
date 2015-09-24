/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "CRP.h"

#include "DCT.h"

#include <cmath>
#include <iostream>

using namespace std;

CRP::~CRP()
{
    delete m_dct;
}

RealBlock
CRP::process(const RealBlock &in)
{
    if (in.empty()) {
	return in;
    }
    if (!m_dct) {
	m_size = in[0].size();
	m_dct = new DCT(m_size);
    }

    int bins = 12;
    RealBlock out;
    RealColumn dctOut(m_size);

    for (RealColumn col: in) {

	RealColumn crp(bins);

	if (int(col.size()) != m_size) {
	    cerr << "ERROR: Inconsistent value count in pitch column: found "
		 << col.size() << " where previous column(s) have had " << m_size
		 << endl;
	    throw std::logic_error("Inconsistent value count in pitch column");
	}
    
	if (m_params.applyLogCompression) {
	    for (auto &v: col) {
		//!!! These numbers are parameters in the MATLAB
		v = log10(1.0 + 1000.0 * v);
	    }
	}

	m_dct->forward(col.data(), dctOut.data());

	for (int i = 0; i < m_params.coefficientsToDrop; ++i) {
	    dctOut[i] = 0.0;
	}

	m_dct->inverse(dctOut.data(), col.data());

	for (int i = 0; i < m_size; ++i) {
	    crp[i % bins] += col[i];
	}

	//!!! normalise as in normalizeFeature.m
	
	out.push_back(crp);
    }

    return out;
}

