/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "DCT.h"

#include <cmath>

DCT::DCT(int n) :
    m_n(n),
    m_scaled(n, 0.0),
    m_time2(n*4, 0.0),
    m_freq2r(n*4, 0.0),
    m_freq2i(n*4, 0.0),
    m_fft(n*4)
{
    m_scale = m_n * sqrt(2.0 / m_n);
}

DCT::~DCT()
{
}

void
DCT::forward(const double *in, double *out)
{
    for (int i = 0; i < m_n; ++i) {
	m_time2[i*2 + 1] = in[i];
	m_time2[m_n*4 - i*2 - 1] = in[i];
    }

    m_fft.forward(m_time2.data(), m_freq2r.data(), m_freq2i.data());

    for (int i = 0; i < m_n; ++i) {
	out[i] = m_freq2r[i];
    }
}

void
DCT::forwardUnitary(const double *in, double *out)
{
    forward(in, out);
    for (int i = 0; i < m_n; ++i) {
	out[i] /= m_scale;
    }
    out[0] /= sqrt(2.0);
}

void
DCT::inverse(const double *in, double *out)
{
    for (int i = 0; i < m_n; ++i) {
	m_freq2r[i] = in[i];
    }
    for (int i = 0; i < m_n; ++i) {
	m_freq2r[m_n*2 - i] = -in[i];
    }
    m_freq2r[m_n] = 0.0;

    for (int i = 0; i <= m_n*2; ++i) {
	m_freq2i[i] = 0.0;
    }
    
    m_fft.inverse(m_freq2r.data(), m_freq2i.data(), m_time2.data());

    for (int i = 0; i < m_n; ++i) {
	out[i] = m_time2[i*2 + 1];
    }
}

void
DCT::inverseUnitary(const double *in, double *out)
{
    for (int i = 0; i < m_n; ++i) {
	m_scaled[i] = in[i] * m_scale;
    }
    m_scaled[0] *= sqrt(2.0);
    inverse(m_scaled.data(), out);
}

