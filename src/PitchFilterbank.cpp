
#include "PitchFilterbank.h"

#include "Resampler.h"
#include "Filter.h"

#include "delays.h"
#include "filter-a.h"
#include "filter-b.h"

#include <stdexcept>

using namespace std;

static const int HIGHEST_FILTER_INDEX_AT_882 = 38;   // MIDI pitch 59
static const int HIGHEST_FILTER_INDEX_AT_4410 = 74;  // MIDI pitch 95
static const int HIGHEST_FILTER_INDEX_AT_22050 = 87; // MIDI pitch 108
static const int HIGHEST_FILTER_INDEX = HIGHEST_FILTER_INDEX_AT_22050;

class PitchFilterbank::D
{
public:
    D(int sampleRate) :
	m_sampleRate(sampleRate),
	m_to22050(sampleRate, 22050),
	m_to4410(sampleRate, 4410),
	m_to882(sampleRate, 882)
    {
	for (int i = 0; i <= HIGHEST_FILTER_INDEX; ++i) {
	    int ix = i + 20;
	    int coeffs = sizeof(filter_a[0]) / sizeof(filter_a[0][0]);
	    vector<double> a(filter_a[ix], filter_a[ix] + coeffs);
	    vector<double> b(filter_b[ix], filter_b[ix] + coeffs);
	    m_filters.push_back(new Filter({ a, b }));
	}
    }
	
    ~D() {
	for (int i = 0; i <= HIGHEST_FILTER_INDEX; ++i) {
	    delete m_filters[i];
	}
    }

    int getSampleRate() const { return m_sampleRate; }
    
    /// A series of real-valued samples ordered in time.
    typedef vector<double> RealSequence;

    /// A series of real-valued samples ordered by bin (frequency or similar).
    typedef vector<double> RealColumn;

    /// A matrix of real-valued samples, indexed by time then bin number.
    typedef vector<RealColumn> RealBlock;

    RealBlock process(const RealSequence &in) {

	m_at22050 = m_to22050.process(in.data(), in.size());
	m_at4410  =  m_to4410.process(in.data(), in.size());
	m_at882   =   m_to882.process(in.data(), in.size());

	for (int i = 0; i <= HIGHEST_FILTER_INDEX; ++i) {

	    if (i <= HIGHEST_FILTER_INDEX_AT_882) {
		pushFiltered(i, m_at882);
	    } else if (i <= HIGHEST_FILTER_INDEX_AT_4410) {
		pushFiltered(i, m_at4410);
	    } else {
		pushFiltered(i, m_at22050);
	    }
	}

	//!!! todo make this known through api. these values are at 22050Hz
	int windowSize = 4410;

	RealBlock energies(HIGHEST_FILTER_INDEX + 1);
	int cols = 0;
	
	while (m_filtered[HIGHEST_FILTER_INDEX].size() >= unsigned(windowSize)) {
	    //!!! Quite inefficient -- we're counting everything
	    //!!! twice. Since there is no actual window shape, isn't
	    //!!! the overlap just averaging?
	    for (int i = 0; i <= HIGHEST_FILTER_INDEX; ++i) {
		int n = windowSize;
		double factor = 1.0;
		if (i <= HIGHEST_FILTER_INDEX_AT_882) {
		    factor = 22050.0 / 882.0;
		} else if (i <= HIGHEST_FILTER_INDEX_AT_4410) {
		    factor = 22050.0 / 4410.0;
		}
		//!!! Problem -- this is not an integer, for
		//!!! fs=882 (it's 176.4)
		n = n / factor;
		double energy = calculateEnergy(m_filtered[i], n, factor);
		energies[i].push_back(energy);
		m_filtered[i] =
		    RealSequence(m_filtered[i].begin() + n/2, m_filtered[i].end());
	    }
	    ++cols;
	}

	RealBlock out(cols);
	for (int j = 0; j < cols; ++j) {
	    for (int i = 0; i <= HIGHEST_FILTER_INDEX; ++i) {
		out[j].push_back(energies[i][j]);
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
	    energy += seq[i] * seq[i] * factor;
	}
	return energy;
    }
    
private:
    int m_sampleRate;
    
    Resampler m_to22050;
    Resampler m_to4410;
    Resampler m_to882;

    RealSequence m_at22050;
    RealSequence m_at4410;
    RealSequence m_at882;

    RealSequence m_filtered[HIGHEST_FILTER_INDEX + 1];
    
    Resampler &resamplerFor(int filterIndex) {
	if (filterIndex < 0) {
	    throw std::logic_error("Filter index is negative");
	} else if (filterIndex <= HIGHEST_FILTER_INDEX_AT_882) {
	    return m_to882;
	} else if (filterIndex <= HIGHEST_FILTER_INDEX_AT_4410) {
	    return m_to4410;
	} else if (filterIndex <= HIGHEST_FILTER_INDEX_AT_22050) {
	    return m_to22050;
	} else {
	    throw std::logic_error("Filter index out of expected range");
	}
    }

    // This vector is initialised with 88 filter instances.
    // m_filters[n] (for n from 0 to 87) is for MIDI pitch 21+n, so we
    // span the MIDI range from pitch 21 to 108. Then m_filters[n] has
    // coefficients drawn from filter_a[20+n] and filter_b[20+n], and
    // has effective delay filter_delay[20+n].
    vector<Filter *> m_filters;

    int filterIndexForMidiPitch(int pitch) const {
	return pitch - 21;
    }
    int filterDelay(int filterIndex) const {
	return filter_delay[20 + filterIndex];
    }
    int totalDelay(int filterIndex) const {
	return filterDelay(filterIndex) +
	    const_cast<D *>(this)->resamplerFor(filterIndex).getLatency();
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

