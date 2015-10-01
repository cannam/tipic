/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Filter.h"

#include <bqvec/VectorOps.h>
#include <bqvec/Allocators.h>

#include <stdexcept>

using namespace std;
using namespace breakfastquay;

Filter::Filter(Parameters params)
{
    if (params.a.empty()) {
        m_fir = true;
        if (params.b.empty()) {
            throw logic_error("Filter must have at least one pair of coefficients");
        }
    } else {
        m_fir = false;
        if (params.a.size() != params.b.size()) {
            throw logic_error("Inconsistent numbers of filter coefficients");
        }
    }
    
    m_sz = int(params.b.size());
    m_order = m_sz - 1;

    // We keep some empty space at the start of the buffer, and
    // encroach gradually into it as we add individual sample
    // calculations at the start. Then when we run out of space, we
    // move the buffer back to the end and begin again. This is
    // significantly faster than moving the whole buffer along in
    // 1-sample steps every time.
    
    m_offmax = 20;
    m_offa = m_offmax;
    m_offb = m_offmax;

    if (m_fir) {
        m_a = 0;
        m_bufa = 0;
    } else {
        m_a = allocate<double>(m_sz);
        m_bufa = allocate_and_zero<double>(m_order + m_offmax);
        v_copy(m_a, &params.a[0], m_sz);
    }
    
    m_b = allocate<double>(m_sz);
    v_copy(m_b, &params.b[0], m_sz);
    m_bufb = allocate_and_zero<double>(m_sz + m_offmax);
}

Filter::~Filter()
{
    deallocate(m_a);
    deallocate(m_bufa);
    deallocate(m_b);
    deallocate(m_bufb);
}

void
Filter::reset()
{
    m_offb = m_offmax;
    m_offa = m_offmax;
    v_zero(m_bufb, m_sz + m_offmax);
    if (!m_fir) v_zero(m_bufa, m_order + m_offmax);
}

void
Filter::process(const double *const BQ_R__ in,
		double *const BQ_R__ out,
		const int n)
{
    for (int s = 0; s < n; ++s) {

        if (m_offb > 0) --m_offb;
        else {
            v_move(m_bufb + m_offmax + 1, m_bufb, m_sz - 1);
            m_offb = m_offmax;
        }
        m_bufb[m_offb] = in[s];

	double b_sum = v_multiply_and_sum(m_b, m_bufb + m_offb, m_sz);

        double outval;

        if (m_fir) {

            outval = b_sum;

        } else {

            double a_sum = v_multiply_and_sum(m_a + 1, m_bufa + m_offa, m_order);
            outval = b_sum - a_sum;

            if (m_offa > 0) --m_offa;
            else {
                v_move(m_bufa + m_offmax + 1, m_bufa, m_order - 1);
                m_offa = m_offmax;
            }
            m_bufa[m_offa] = outval;
        }
        
	out[s] = outval;
    }
}

