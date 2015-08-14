
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

using namespace std;

static const int HIGHEST_FILTER_INDEX_AT_882 = 38;   // MIDI pitch 59
static const int HIGHEST_FILTER_INDEX_AT_4410 = 74;  // MIDI pitch 95
static const int HIGHEST_FILTER_INDEX_AT_22050 = 87; // MIDI pitch 108
static const int HIGHEST_FILTER_INDEX = HIGHEST_FILTER_INDEX_AT_22050;

class PitchFilterbank::D
{
public:
    D(int sampleRate) :
	m_nfilters(HIGHEST_FILTER_INDEX + 1),
	m_sampleRate(sampleRate)
    {
	m_resamplers[882] = new Resampler(sampleRate, 882);
	m_resamplers[4410] = new Resampler(sampleRate, 4410);
	m_resamplers[22050] = new Resampler(sampleRate, 22050);
	
	for (int i = 0; i < m_nfilters; ++i) {
	    int ix = i + 20;
	    int coeffs = sizeof(filter_a[0]) / sizeof(filter_a[0][0]);
	    vector<double> a(filter_a[ix], filter_a[ix] + coeffs);
	    vector<double> b(filter_b[ix], filter_b[ix] + coeffs);
	    m_filters.push_back(new Filter({ a, b }));
	}

	m_filtered.resize(m_nfilters);
	m_energies.resize(m_nfilters);
    }
	
    ~D() {
	for (auto f: m_filters) delete f;
	for (auto r: m_resamplers) delete r.second;
    }

    int getSampleRate() const { return m_sampleRate; }
    
    /// A series of real-valued samples ordered in time.
    typedef vector<double> RealSequence;

    /// A series of real-valued samples ordered by bin (frequency or similar).
    typedef vector<double> RealColumn;

    /// A matrix of real-valued samples, indexed by time then bin number.
    typedef vector<RealColumn> RealBlock;

    RealBlock process(const RealSequence &in) {

	cerr << "process" << endl;
	
	for (auto r: m_resamplers) {
	    m_resampled[r.first] = r.second->process(in.data(), in.size());
	}

	for (int i = 0; i < m_nfilters; ++i) {
	    int rate = filterRate(i);
	    if (m_resampled.find(rate) == m_resampled.end()) {
		throw logic_error("No resampled output for rate of filter");
	    }
	    pushFiltered(i, m_resampled[rate]);
	}

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
	    
	    while (m_filtered[i].size() >= unsigned(n)) {
		double energy = calculateEnergy(m_filtered[i], n, factor);
		m_energies[i].push_back(energy);
		m_filtered[i] = RealSequence(m_filtered[i].begin() + hop,
					     m_filtered[i].end());
	    }
	}

	int minCols = 0;
	for (int i = 0; i < m_nfilters; ++i) {
	    int n = m_energies[i].size();
	    if (i == 0 || n < minCols) {
		minCols = n;
	    }
	}
	
	RealBlock out(minCols);
	cerr << "process: minCols = " << minCols << endl;
	for (int j = 0; j < minCols; ++j) {
	    for (int i = 0; i < m_nfilters; ++i) {
		out[j].push_back(m_energies[i][0]);
		m_energies[i].pop_front();
	    }
	}

	return out;
    }

    RealBlock getRemainingOutput() {
	//!!! for now! but we do have some buffered
	return RealBlock();
    }
    
    void pushFiltered(int i, const RealSequence &resampled) {
	int n = resampled.size();
	RealSequence filtered(n, 0.0);
	m_filters[i]->process(resampled.data(), filtered.data(), n);
	m_filtered[i].insert(m_filtered[i].end(), filtered.begin(), filtered.end());
    }	

    double calculateEnergy(const RealSequence &seq, int n, double factor) {
	double energy = 0.0;
	for (int i = 0; i < n; ++i) {
	    energy += seq[i] * seq[i];
	}
	return energy * factor;
    }
    
private:
    int m_nfilters;
    int m_sampleRate;

    // This vector is initialised with 88 filter instances.
    // m_filters[n] (for n from 0 to 87) is for MIDI pitch 21+n, so we
    // span the MIDI range from pitch 21 to 108. Then m_filters[n] has
    // coefficients drawn from filter_a[20+n] and filter_b[20+n], and
    // has effective delay filter_delay[20+n].
    vector<Filter *> m_filters;

    map<int, Resampler *> m_resamplers; // rate -> resampler
    map<int, RealSequence> m_resampled;
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
    int filterDelay(int filterIndex) const {
	return filter_delay[20 + filterIndex];
    }
    int totalDelay(int filterIndex) const {
	return filterDelay(filterIndex) +
	    const_cast<D *>(this)->resamplerFor(filterIndex)->getLatency();
    }
};

PitchFilterbank::PitchFilterbank(int sampleRate) :
    m_d(new D(sampleRate))
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
    delete m_d;
    m_d = new D(rate);
}

PitchFilterbank::RealBlock
PitchFilterbank::process(const RealSequence &in)
{
    return m_d->process(in);
}

PitchFilterbank::RealBlock
PitchFilterbank::getRemainingOutput()
{
    return m_d->getRemainingOutput();
}
