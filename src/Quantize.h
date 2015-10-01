/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include <vector>
#include <stdexcept>

class Quantize
{
public:
    class Parameters {
    public:
	std::vector<double> steps;
	std::vector<double> weights;
	Parameters() :
	    steps({ 0.4, 0.2, 0.1, 0.05 }),
	    weights({ 0.25, 0.25, 0.25, 0.25 }) { }
    };

    Quantize(Parameters params) : m_params(params) {
	if (params.steps.empty()) {
	    throw std::invalid_argument("Quantize steps must not be empty");
	}
	if (params.steps.size() != params.weights.size()) {
	    throw std::invalid_argument("Must have same number of quantize steps and weights");
	}
    }
    ~Quantize() { }

    std::vector<double> process(const std::vector<double> &in) {
	int n = in.size();
	int m = m_params.steps.size();
	std::vector<double> out(n, 0.0);
	for (int i = 0; i < n; ++i) {
	    for (int j = 0; j < m; ++j) {
		if (in[i] > m_params.steps[j]) {
		    out[i] += m_params.weights[j];
		}
	    }
	}
	return out;
    }

private:
    Parameters m_params;
};

#endif
