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

#ifndef TIPIC_TYPES_H
#define TIPIC_TYPES_H

#include <vector>

/// A series of real-valued samples ordered in time.
typedef std::vector<double> RealSequence;

/// A series of real-valued samples ordered by bin (frequency or similar).
typedef std::vector<double> RealColumn;

/// A matrix of real-valued samples, indexed by time then bin number.
typedef std::vector<RealColumn> RealBlock;

#endif

