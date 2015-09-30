/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef FILTER_H
#define FILTER_H

#include <bqvec/Restrict.h>

#include <vector>

class Filter
{
public:
    struct Parameters {
        std::vector<double> a;
        std::vector<double> b;
    };

    /**
     * Construct an IIR filter with numerators b and denominators
     * a. To make an FIR filter instead (i.e. with denominator = 1),
     * leave the vector a in the param struct empty. Otherwise, the
     * vectors a and b must have the same number of values.
     */
    Filter(Parameters params);
    
    ~Filter();

    void reset();

    /**
     * Filter the input sequence \arg in of length \arg n samples, and
     * write the resulting \arg n samples into \arg out. There must be
     * enough room in \arg out for \arg n samples to be written.
     */
    void process(const double *const BQ_R__ in,
                 double *const BQ_R__ out,
                 const int n);

private:
    int m_order;
    int m_sz;
    double *m_a;
    double *m_b;
    double *m_bufa;
    double *m_bufb;
    int m_offa;
    int m_offb;
    int m_offmax;
    bool m_fir;

    Filter(const Filter &); // not supplied
    Filter &operator=(const Filter &); // not supplied
};
    
#endif
