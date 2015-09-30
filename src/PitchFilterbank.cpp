
#include "PitchFilterbank.h"

#include "Resampler.h"
#include "Filter.h"

#include "delays.h"
#include "filter-a.h"
#include "filter-b.h"

#include <deque>
#include <map>
#include <stdexcept>
#include <iostream>

#include <cmath>

using namespace std;

static const int HIGHEST_FILTER_INDEX_AT_882 = 38;   // MIDI pitch 59
static const int HIGHEST_FILTER_INDEX_AT_4410 = 74;  // MIDI pitch 95
static const int HIGHEST_FILTER_INDEX_AT_22050 = 87; // MIDI pitch 108
static const int HIGHEST_FILTER_INDEX = HIGHEST_FILTER_INDEX_AT_22050;

class PitchFilterbank::D
{
public:
    D(int sampleRate, double tuningFrequency) :
	m_nfilters(HIGHEST_FILTER_INDEX + 1),
	m_sampleRate(sampleRate),
	m_tuningFrequency(tuningFrequency)
    {
	// To handle a non-440 tuning frequency, we resample the input
	// by this tuning ratio and then adjust the output block
	// timings accordingly. Ratio is calculated on the basis that
	// for tuning freq >440 we want to lower the pitch of the
	// input audio by slowing it down, therefore we want to
	// pretend that it came in at a lower sample rate than it
	// really did, and for >440 the opposite applies. The
	// effective input sample rate is the rate at which we pretend
	// the audio was supplied.
	m_tuningRatio = 440.0 / m_tuningFrequency;
	m_effectiveInputSampleRate = int(round(m_sampleRate * m_tuningRatio));

	//!!! todo: tuning frequency adjustment
	// * resample input by a small amount
	// * adjust output block timings by a small amount

	//!!! nb "we use forward-backward filtering such that the
	// resulting output signal has precisely zero phase distortion
	// and a magnitude modified by the square of the filter’s
	// magnitude response" -- we are not doing forward-backward
	// here & so need to adapt magnitudes in compensation to match
	// original
	
	m_resamplers[882] = new Resampler(m_effectiveInputSampleRate, 882);
	m_resamplers[4410] = new Resampler(m_effectiveInputSampleRate, 4410);
	m_resamplers[22050] = new Resampler(m_effectiveInputSampleRate, 22050);
	
	for (int i = 0; i < m_nfilters; ++i) {
	    int ix = i + 20;
	    int coeffs = sizeof(filter_a[0]) / sizeof(filter_a[0][0]);
	    vector<double> a(filter_a[ix], filter_a[ix] + coeffs);
	    vector<double> b(filter_b[ix], filter_b[ix] + coeffs);
	    m_filters.push_back(new Filter({ a, b }));
	    m_toCompensate.push_back(totalDelay(i));
	}

	m_filtered.resize(m_nfilters);
	m_energies.resize(m_nfilters);
    }
	
    ~D() {
	for (auto f: m_filters) delete f;
	for (auto r: m_resamplers) delete r.second;
    }

    int getSampleRate() const { return m_sampleRate; }
    double getTuningFrequency() const { return m_tuningFrequency; }

    RealBlock process(const RealSequence &in) {

	for (auto r: m_resamplers) {
	    m_resampled[r.first] = r.second->process(in.data(), in.size());
	}

	for (int i = 0; i < m_nfilters; ++i) {
	    int rate = filterRate(i);
	    if (m_resampled.find(rate) == m_resampled.end()) {
		throw logic_error("No resampled output for rate of filter");
	    }
	    pushFiltered(i, m_resampled[rate], false);
	}

	return energiesFromFiltered(false);
    }

    RealBlock getRemainingOutput() {

	for (auto r: m_resamplers) {
	    RealSequence in(r.second->getLatency(), 0.0);
	    m_resampled[r.first] = r.second->process(in.data(), in.size());
	}

	for (int i = 0; i < m_nfilters; ++i) {
	    int rate = filterRate(i);
	    pushFiltered(i, m_resampled[rate], true);
	}
	
	return energiesFromFiltered(true);
    }
    
    RealBlock energiesFromFiltered(bool drain) {
	
	//!!! todo make this known through api. these values are at 22050Hz
	int windowSize = 4410;

	//!!! This is all quite inefficient -- we're counting
	//!!! everything twice. Since there is no actual window shape,
	//!!! isn't the overlap just averaging?

	for (int i = 0; i < m_nfilters; ++i) {

	    double factor = 22050.0 / filterRate(i);
	    //!!! Problem -- this is not an integer, for
	    //!!! fs=882 (it's 176.4)
	    int n = windowSize / factor;
	    int hop = n / 2;

	    unsigned int minReq = n;
	    if (drain) minReq = hop;

	    //!!! strictly, we don't actually need to store the
	    //!!! filtered outputs since our overlapped windows are
	    //!!! not shaped -- each one is the sum of two half-size
	    //!!! windows, so we can just push the energies of those
	    //!!! directly. that's a TODO
	    
	    while (m_filtered[i].size() >= minReq) {
		double energy = calculateEnergy(m_filtered[i], n, factor);
		m_energies[i].push_back(energy);
		m_filtered[i] = RealSequence(m_filtered[i].begin() + hop,
					     m_filtered[i].end());
	    }
	}

	int minCols = 0, maxCols = 0;
	for (int i = 0; i < m_nfilters; ++i) {
	    int n = m_energies[i].size();
	    if (i == 0 || n < minCols) minCols = n;
	    if (i == 0 || n > maxCols) maxCols = n;
	}

	RealBlock out;
	
	if (drain) {
	    out.resize(maxCols);
	    for (int j = 0; j < maxCols; ++j) {
		for (int i = 0; i < m_nfilters; ++i) {
		    if (m_energies[i].size() == 0) {
			out[j].push_back(0.0);
		    } else {
			out[j].push_back(m_energies[i][0]);
			m_energies[i].pop_front();
		    }
		}
	    }
	} else {
	    out.resize(minCols);
	    for (int j = 0; j < minCols; ++j) {
		for (int i = 0; i < m_nfilters; ++i) {
		    out[j].push_back(m_energies[i][0]);
		    m_energies[i].pop_front();
		}
	    }
	}

	return out;
    }
    
    void pushFiltered(int i, const RealSequence &resampled, bool drain) {

	RealSequence in(resampled);
	if (drain) {
	    RealSequence pad(filterDelay(i), 0.0);
	    in.insert(in.end(), pad.begin(), pad.end());
	}
	
	int n = in.size();
	RealSequence filtered(n, 0.0);
	m_filters[i]->process(in.data(), filtered.data(), n);

	int pushStart = 0, pushCount = n;

	if (m_toCompensate[i] > 0) {
	    pushCount = max(pushCount - m_toCompensate[i], 0);
	    int compensating = n - pushCount;
	    pushStart += compensating;
	    m_toCompensate[i] -= compensating;
	    if (m_toCompensate[i] < 0) {
		throw logic_error("Compensated for more latency than exists");
	    }
	}
	
	m_filtered[i].insert(m_filtered[i].end(),
			     filtered.begin() + pushStart,
			     filtered.begin() + pushStart + pushCount);
    }	

    double calculateEnergy(const RealSequence &seq, int n, double factor) {
	double energy = 0.0;
	int sz = seq.size();
	if (n > sz) n = sz;
	for (int i = 0; i < n; ++i) {
	    energy += seq[i] * seq[i];
	}
	return energy * factor;
    }
    
private:
    int m_nfilters;
    int m_sampleRate;
    int m_effectiveInputSampleRate;
    double m_tuningFrequency;
    double m_tuningRatio;

    // This vector is initialised with 88 filter instances.
    // m_filters[n] (for n from 0 to 87) is for MIDI pitch 21+n, so we
    // span the MIDI range from pitch 21 to 108. Then m_filters[n] has
    // coefficients drawn from filter_a[20+n] and filter_b[20+n], and
    // has effective delay filter_delay[20+n].
    vector<Filter *> m_filters;

    map<int, Resampler *> m_resamplers; // rate -> resampler
    map<int, RealSequence> m_resampled;
    vector<int> m_toCompensate; // latency remaining at start, per filter
    vector<RealSequence> m_filtered;
    vector<deque<double>> m_energies;

    Resampler *resamplerFor(int filterIndex) {
	int rate = filterRate(filterIndex);
	if (m_resamplers.find(rate) == m_resamplers.end()) {
	    throw std::logic_error("Filter index has unknown or unexpected rate");
	}
	return m_resamplers[rate];
    }

    int filterRate(int filterIndex) const {
	if (filterIndex <= HIGHEST_FILTER_INDEX_AT_882) {
	    return 882;
	} else if (filterIndex <= HIGHEST_FILTER_INDEX_AT_4410) {
	    return 4410;
	} else {
	    return 22050;
	}
    }
    int resamplerDelay(int filterIndex) const {
	return const_cast<D *>(this)->resamplerFor(filterIndex)->getLatency();
    }
    int filterDelay(int filterIndex) const {
	return filter_delay[20 + filterIndex];
    }
    int totalDelay(int filterIndex) const {
	return resamplerDelay(filterIndex) + filterDelay(filterIndex);
    }
};

PitchFilterbank::PitchFilterbank(int sampleRate, double tuningFrequency) :
    m_d(new D(sampleRate, tuningFrequency))
{
}

PitchFilterbank::~PitchFilterbank()
{
    delete m_d;
}

void
PitchFilterbank::reset()
{
    int rate = m_d->getSampleRate();
    double freq = m_d->getTuningFrequency();
    delete m_d;
    m_d = new D(rate, freq);
}

RealBlock
PitchFilterbank::process(const RealSequence &in)
{
    return m_d->process(in);
}

RealBlock
PitchFilterbank::getRemainingOutput()
{
    return m_d->getRemainingOutput();
}

void
PitchFilterbank::getPitchRange(int &minMidiPitch, int &maxMidiPitch)
{
    minMidiPitch = 21;
    maxMidiPitch = 108;
}


