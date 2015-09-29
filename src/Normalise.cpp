/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Normalise.h"

#include <cmath>

using namespace std;

double
Normalise::norm(vector<double> v, int p)
{
    double tot = 0.0;
    for (auto x: v) tot += abs(pow(x, p));
    return pow(tot, 1.0 / p);
}

vector<double>
Normalise::normalise(vector<double> v, int p, double threshold)
{
    if (p == 0) return v; // no normalisation
    int n = v.size();
    double nv = norm(v, p);
    if (nv < threshold) {
	return vector<double>(n, 1.0 / pow(n, 1.0 / p)); // unit vector
    }
    vector<double> out;
    for (auto x: v) out.push_back(x / nv);
    return out;
}

