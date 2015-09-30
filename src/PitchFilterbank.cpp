
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
	m_tuningFrequency(tuningFrequency),
	m_blockNo(0)
    {
	// To handle a non-440Hz tuning frequency, we resample the
	// input and then adjust the output block timings
	// accordingly. For a tuning freq >440 we want to lower the
	// pitch of the input audio by slowing it down, therefore we
	// want to pretend that it came in at a lower sample rate than
	// it really did; for >440 the opposite applies. The effective
	// input sample rate is the rate at which we pretend the audio
	// was supplied. Rounding to the nearest int (because our
	// resampler only supports integer rates) gives around 0.1Hz
	// quantization close to 440Hz in 44.1kHz audio -- we could do
	// better by using multiples of our source and target sample
	// rates, but I think it probably isn't necessary.
	m_effectiveInputRate =
	    int(round(m_sampleRate * (440.0 / m_tuningFrequency)));

	//!!! nb "we use forward-backward filtering such that the
	// resulting output signal has precisely zero phase distortion
	// and a magnitude modified by the square of the filter’s
	// magnitude response" -- we are not doing forward-backward
	// here & so need to adapt magnitudes in compensation to match
	// original

	double snr = 50.0;
	double bw = 0.05;
	m_resamplers[882] = new Resampler(m_effectiveInputRate, 882, snr, bw);
	m_resamplers[4410] = new Resampler(m_effectiveInputRate, 4410, snr, bw);
	m_resamplers[22050] = new Resampler(m_effectiveInputRate, 22050, snr, bw);
	
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

    struct WindowPosition {
	uint64_t start;
	int size;
	double factor;
    };
    
    WindowPosition windowPosition(int block, int i) {

	int hop = 2205;

	double rate = filterRate(i);
	double topRate = 22050.0;
	double rateRatio = topRate / rate;
	double tuningRatio = m_sampleRate / double(m_effectiveInputRate);
	double sizeRatio = tuningRatio / rateRatio;

	uint64_t start(round((hop * uint64_t(block)) * sizeRatio));
	int size(round((hop * 2) * sizeRatio));

	return { start, size, rateRatio };
    }
    
    RealBlock energiesFromFiltered(bool drain) {

	for (int i = 0; i < m_nfilters; ++i) {

	    WindowPosition here = windowPosition(m_blockNo, i);
	    WindowPosition next = windowPosition(m_blockNo + 1, i);

	    int n = here.size;
	    int hop = next.start - here.start;

	    unsigned int minReq = n;
	    if (drain) minReq = hop;

	    // we use a separate buffer for each filter (not just one
	    // per resampling ratio) because each filter has a
	    // different delay, which we are compensating for when
	    // first filling the buffer.
	    
	    while (m_filtered[i].size() >= minReq) {
		double energy = calculateEnergy(m_filtered[i], n, here.factor);
		m_energies[i].push_back(energy);
		m_filtered[i] = RealSequence(m_filtered[i].begin() + hop,
					     m_filtered[i].end());
	    }
	}

	++m_blockNo;
	
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
    int m_effectiveInputRate;
    double m_tuningFrequency;

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
    int m_blockNo;

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

double
PitchFilterbank::getOutputSampleRate()
{
    return 22050.0 / 2205.0;
}

