/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef LOGCOMPRESS_H
#define LOGCOMPRESS_H

#include <vector>
#include <cmath>

class LogCompress
{
public:
    static std::vector<double> process(std::vector<double> in,
				       double factor,
				       double addTerm) {
	std::vector<double> out;
	out.reserve(in.size());
	for (auto x: in) {
	    out.push_back(log10(addTerm + factor * x));
	}
	return std::move(out);
    }
};

#endif

