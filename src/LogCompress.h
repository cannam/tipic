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

