/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef OCTAVEFOLD_H
#define OCTAVEFOLD_H

#include <vector>

class OctaveFold
{
public:
    static std::vector<double> process(std::vector<double> in) {
	std::vector<double> out(12, 0.0);
	for (int i = 0; i < int(in.size()); ++i) {
	    out[(i+1) % 12] += in[i];
	}
	return std::move(out);
    }
};

#endif

