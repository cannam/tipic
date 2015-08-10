/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Filter.h"

#include <bqvec/VectorOps.h>
#include <bqvec/Allocators.h>

#include <stdexcept>

using namespace std;
using namespace breakfastquay;

Filter::Filter(Parameters params)
{
    if (params.a.size() != params.b.size()) {
	throw logic_error("Inconsistent numbers of filter coefficients");
    }
    if (params.a.size() < 1) {
	throw logic_error("Filter must have at least one pair of coefficients");
    }
    m_sz = int(params.a.size());
    m_order = m_sz - 1;
    m_a = allocate<double>(m_sz);
    m_b = allocate<double>(m_sz);
    v_copy(m_a, &params.a[0], m_sz);
    v_copy(m_b, &params.b[0], m_sz);
    m_inbuf = allocate_and_zero<double>(m_sz);
    m_outbuf = allocate_and_zero<double>(m_order);
}

Filter::~Filter()
{
    deallocate(m_a);
    deallocate(m_b);
    deallocate(m_inbuf);
    deallocate(m_outbuf);
}

void
Filter::reset()
{
    v_zero(m_inbuf, m_sz);
    v_zero(m_outbuf, m_order);
}

void
Filter::process(const double *const BQ_R__ in,
		double *const BQ_R__ out,
		const int n)
{
    for (int s = 0; s < n; ++s) {

	v_move(m_inbuf + 1, m_inbuf, m_order);
	m_inbuf[0] = in[s];

	double b_sum = v_multiply_and_sum(m_b, m_inbuf, m_sz);
	double a_sum = v_multiply_and_sum(m_a + 1, m_outbuf, m_order);

	double outval = b_sum - a_sum;

	v_move(m_outbuf + 1, m_outbuf, m_order - 1);
	m_outbuf[0] = outval;
	out[s] = outval;
    }
}

