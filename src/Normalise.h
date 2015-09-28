/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef NORMALISE_H
#define NORMALISE_H

#include <vector>

class Normalise
{
public:
    static double norm(std::vector<double> v,
		       int p = 2); // L^p norm

    static std::vector<double> normalise(std::vector<double> v,
					 int p = 2,
					 double threshold = 1e-6);
};

#endif
