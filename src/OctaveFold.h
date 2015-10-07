/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Tipic

    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

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

