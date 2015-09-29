/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef DCTREDUCE_H
#define DCTREDUCE_H

#include "DCT.h"
#include <vector>

class DCTReduce
{
public:
    DCTReduce(int size, int coefficientsToDrop) :
	m_size(size),
	m_dct(size),
	m_dctOut(size),
	m_coefficientsToDrop(coefficientsToDrop)
    { } 

    std::vector<double> process(std::vector<double> in) {
	std::vector<double> out(in.size());
	m_dct.forward(in.data(), m_dctOut.data());
	for (int i = 0; i < m_coefficientsToDrop && i < m_size; ++i) {
	    m_dctOut[i] = 0.0;
	}
	m_dct.inverse(m_dctOut.data(), out.data());
	return std::move(out);
    }
    
private:
    int m_size;
    DCT m_dct;
    std::vector<double> m_dctOut;
    int m_coefficientsToDrop;
};

#endif

