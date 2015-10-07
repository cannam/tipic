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

#ifndef PITCH_FILTERBANK_H
#define PITCH_FILTERBANK_H

#include "Types.h"

class PitchFilterbank
{
public:
    PitchFilterbank(int sampleRate, double tuningFrequency);
    ~PitchFilterbank();

    void reset();
    
    RealBlock process(const RealSequence &);

    RealBlock getRemainingOutput();

    static void getPitchRange(int &minMidiPitch, int &maxMidiPitch);

    static double getOutputSampleRate();
    
private:
    class D;
    D *m_d;

    PitchFilterbank(const PitchFilterbank &); // not supplied
    PitchFilterbank &operator=(const PitchFilterbank &); // not supplied
};

#endif
