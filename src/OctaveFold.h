/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef OCTAVEFOLD_H
#define OCTAVEFOLD_H

#include <vector>

class OctaveFold
{
public:
    /**
     * Take an 88-bin pitch feature and sum it into a 12-bin
     * octave. Each bin gets summed into chroma bin corresponding to
     * its MIDI pitch modulo 12. The first 20 MIDI pitches are
     * missing, so bin number n (for n from 0 to 87) is for MIDI pitch
     * 21+n.
     */
    static std::vector<double> process(std::vector<double> in) {
	std::vector<double> out(12, 0.0);
	for (int i = 0; i < int(in.size()); ++i) {
	    out[(21+i) % 12] += in[i];
	}
	return std::move(out);
    }
};

#endif

