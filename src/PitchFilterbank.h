/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef PITCH_FILTERBANK_H
#define PITCH_FILTERBANK_H

#include "Types.h"

class PitchFilterbank
{
public:
    PitchFilterbank(int sampleRate, float tuningFrequency);
    ~PitchFilterbank();

    void reset();
    
    RealBlock process(const RealSequence &);

    RealBlock getRemainingOutput();

private:
    class D;
    D *m_d;

    PitchFilterbank(const PitchFilterbank &); // not supplied
    PitchFilterbank &operator=(const PitchFilterbank &); // not supplied
};

#endif
