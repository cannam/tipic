/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef RESIZE_H
#define RESIZE_H

#include <vector>

class Resize
{
public:
    // The chroma processing chain requires a 120-bin pitch filterbank
    // output, even though ours only actually contains 88 bins. Zero
    // pad at both ends here.
    static std::vector<double> process(std::vector<double> in) {
	std::vector<double> resized(20, 0.0);
	resized.insert(resized.end(), in.begin(), in.end());
	resized.resize(120);
	return resized;
    }
};

#endif
