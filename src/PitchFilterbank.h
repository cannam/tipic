/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef PITCH_FILTERBANK_H
#define PITCH_FILTERBANK_H

#include <vector>

class PitchFilterbank
{
public:
    PitchFilterbank(int sampleRate);
    ~PitchFilterbank();

    /// A series of real-valued samples ordered in time.
    typedef std::vector<double> RealSequence;

    /// A series of real-valued samples ordered by bin (frequency or similar).
    typedef std::vector<double> RealColumn;

    /// A matrix of real-valued samples, indexed by time then bin number.
    typedef std::vector<RealColumn> RealBlock;

    RealBlock process(const RealSequence &);

    RealBlock getRemainingOutput();

private:
    class D;
    D *m_d;
};

#endif
