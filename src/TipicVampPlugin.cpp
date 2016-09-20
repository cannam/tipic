
#include "TipicVampPlugin.h"

#include "PitchFilterbank.h"
#include "CRP.h"
#include "Chroma.h"
#include "CENS.h"
#include "FeatureDownsample.h"

#include <iostream>
#include <sstream>

using namespace std;

static const float defaultTuningFrequency = 440.f;

Tipic::Tipic(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_stepSize(0),
    m_blockSize(0),
    m_tuningFrequency(defaultTuningFrequency),
    m_filterbank(0),
    m_crp(0),
    m_chroma(0),
    m_logChroma(0),
    m_cens(0),
    m_pitchOutputNo(-1),
    m_cpOutputNo(-1),
    m_clpOutputNo(-1),
    m_censOutputNo(-1),
    m_crpOutputNo(-1)
{
}

Tipic::~Tipic()
{
    delete m_filterbank;
    delete m_crp;
    delete m_chroma;
    delete m_logChroma;
    delete m_cens;

    for (auto &d: m_downsamplers) delete d.second;
}

string
Tipic::getIdentifier() const
{
    return "tipic";
}

string
Tipic::getName() const
{
    return "TIPIC";
}

string
Tipic::getDescription() const
{
    return "Chroma and pitch features, including DCT-reduced chroma with extra timbre invariance. Based on the MATLAB Chroma Toolbox by Müller and Ewert, adapted to use causal filters.";
}

string
Tipic::getMaker() const
{
    return "Queen Mary, University of London";
}

int
Tipic::getPluginVersion() const
{
    return 1;
}

string
Tipic::getCopyright() const
{
    return "Methods by Sebastian Ewert and Meinard Müller, pluguin by Chris Cannam. GPL licence.";
}

Tipic::InputDomain
Tipic::getInputDomain() const
{
    return TimeDomain;
}

size_t
Tipic::getPreferredBlockSize() const
{
    return 0;
}

size_t 
Tipic::getPreferredStepSize() const
{
    return 0;
}

size_t
Tipic::getMinChannelCount() const
{
    return 1;
}

size_t
Tipic::getMaxChannelCount() const
{
    return 1;
}

Tipic::ParameterList
Tipic::getParameterDescriptors() const
{
    ParameterList list;

    ParameterDescriptor desc;
    desc.identifier = "tuning";
    desc.name = "Tuning Frequency";
    desc.unit = "Hz";
    desc.description = "Frequency of concert A";
    desc.minValue = 360;
    desc.maxValue = 500;
    desc.defaultValue = defaultTuningFrequency;
    desc.isQuantized = false;
    list.push_back(desc);
    
    return list;
}

float
Tipic::getParameter(string identifier) const
{
    if (identifier == "tuning") {
        return m_tuningFrequency;
    }
    return 0;
}

void
Tipic::setParameter(string identifier, float value) 
{
    if (identifier == "tuning") {
        m_tuningFrequency = value;
    }
}

Tipic::ProgramList
Tipic::getPrograms() const
{
    ProgramList list;
    return list;
}

string
Tipic::getCurrentProgram() const
{
    return ""; // no programs
}

void
Tipic::selectProgram(string name)
{
}

static vector<string> noteNames
    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static std::string noteName(int i)
{
    string name = noteNames[i % 12];
    int oct = i / 12 - 1;
    ostringstream sstr;
    sstr << i << " " << name << oct << ends;
    return sstr.str();
}

Tipic::OutputList
Tipic::getOutputDescriptors() const
{
    OutputList list;

    string downIdSuffix = "-smoothed";
    string downNamePrefix = "Smoothed ";
    string downDescSuffix = ", smoothed by 10x downsampling";
    
    OutputDescriptor d;
    d.identifier = "pitch";
    d.name = "Pitch Representation";
    d.description = "Short-time energy content of the signal within 88 semitone-tuned frequency bands";
    d.unit = "";
    d.hasFixedBinCount = true;
    int min = 0, max = 0;
    PitchFilterbank::getPitchRange(min, max);
    d.binCount = max - min + 1;
    d.binNames.clear();
    for (int p = min; p <= max; ++p) {
	d.binNames.push_back(noteName(p));
    }
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = PitchFilterbank::getOutputSampleRate();
    d.hasDuration = false;
    m_pitchOutputNo = list.size();
    list.push_back(d);

    d.identifier += downIdSuffix;
    d.name = downNamePrefix + d.name;
    d.description += downDescSuffix;
    d.sampleRate /= 10.0;
    list.push_back(d);
    
    d.identifier = "chroma";
    d.name = "Chroma Pitch Features";
    d.description = "CP Chroma Pitch features derived by summing the Pitch Representation into a single octave";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 12;
    d.binNames = noteNames;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = PitchFilterbank::getOutputSampleRate();
    d.hasDuration = false;
    m_cpOutputNo = list.size();
    list.push_back(d);

    d.identifier += downIdSuffix;
    d.name = downNamePrefix + d.name;
    d.description += downDescSuffix;
    d.sampleRate /= 10.0;
    list.push_back(d);

    d.identifier = "clp";
    d.name = "Chroma Log Pitch Features";
    d.description = "CLP Chroma Logarithmic Pitch features derived by summing log of the Pitch Representation energy values into a single octave";
    d.sampleRate = PitchFilterbank::getOutputSampleRate();
    m_clpOutputNo = list.size();
    list.push_back(d);

    d.identifier += downIdSuffix;
    d.name = downNamePrefix + d.name;
    d.description += downDescSuffix;
    d.sampleRate /= 10.0;
    list.push_back(d);

    d.identifier = "cens";
    d.name = "Chroma Energy Normalised Statistics Features";
    d.description = "CENS statistical features based on L1 normalized pitch energy distribions";
    d.sampleRate = PitchFilterbank::getOutputSampleRate() / 10.0;
    m_censOutputNo = list.size();
    list.push_back(d);

    d.identifier = "crp";
    d.name = "Chroma DCT-Reduced Pitch Features";
    d.description = "CRP Chroma DCT-Reduced Log Pitch features, providing some timbre-invariance by discarding timbre-related information from lower cepstral coefficients";
    d.sampleRate = PitchFilterbank::getOutputSampleRate();
    m_crpOutputNo = list.size();
    list.push_back(d);

    d.identifier += downIdSuffix;
    d.name = downNamePrefix + d.name;
    d.description += downDescSuffix;
    d.sampleRate /= 10.0;
    list.push_back(d);

    return list;
}

bool
Tipic::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (m_inputSampleRate > 192000) {
	cerr << "ERROR: Tipic::initialise: Max sample rate 192000 exceeded "
	     << "(requested rate = " << m_inputSampleRate << ")" << endl;
	return false;
    }
    
    if (m_pitchOutputNo < 0) {
	// getOutputDescriptors has never been called, it sets up the
	// outputNo members
	(void)getOutputDescriptors();
    }
    if (m_pitchOutputNo < 0 ||
	m_cpOutputNo < 0 ||
	m_clpOutputNo < 0 ||
	m_censOutputNo < 0 ||
	m_crpOutputNo < 0) {
	throw std::logic_error("setup went wrong");
    }
    
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) {
	cerr << "ERROR: Tipic::initialise: wrong number of channels supplied (only 1 supported)" << endl;
	return false;
    }

    m_stepSize = stepSize;
    m_blockSize = blockSize;
    
    if (m_stepSize != m_blockSize) {
	cerr << "ERROR: initialise: step size and block size must be equal" << endl;
	return false;
    }
    
    reset();
    
    return true;
}

void
Tipic::reset()
{
    if (!m_filterbank) {

	m_filterbank = new PitchFilterbank(m_inputSampleRate, m_tuningFrequency);

	m_crp = new CRP({});

	m_chroma = new Chroma({});

	Chroma::Parameters params;
	params.applyLogCompression = true;
	m_logChroma = new Chroma(params);

	m_cens = new CENS({});
    }
    
    m_filterbank->reset();

    for (auto &d: m_downsamplers) d.second->reset();
}

Tipic::FeatureSet
Tipic::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    RealSequence in;
    in.resize(m_blockSize);
    for (int i = 0; i < m_blockSize; ++i) {
	in[i] = inputBuffers[0][i];
    }
    
    RealBlock pitchFiltered = m_filterbank->process(in);

    RealBlock cp = m_chroma->process(pitchFiltered);
    RealBlock clp = m_logChroma->process(pitchFiltered);
    RealBlock cens = m_cens->process(pitchFiltered);
    RealBlock crp = m_crp->process(pitchFiltered);

    FeatureSet fs;
    addFeatures(fs, m_pitchOutputNo, pitchFiltered, false);
    addFeatures(fs, m_cpOutputNo, cp, false);
    addFeatures(fs, m_clpOutputNo, clp, false);
    addFeatures(fs, m_censOutputNo, cens, false);
    addFeatures(fs, m_crpOutputNo, crp, false);
    return fs;
}

Tipic::FeatureSet
Tipic::getRemainingFeatures()
{
    RealBlock pitchFiltered = m_filterbank->getRemainingOutput();

    RealBlock cp = m_chroma->process(pitchFiltered);
    RealBlock clp = m_logChroma->process(pitchFiltered);
    RealBlock cens = m_cens->process(pitchFiltered);
    RealBlock crp = m_crp->process(pitchFiltered);

    FeatureSet fs;
    addFeatures(fs, m_pitchOutputNo, pitchFiltered, true);
    addFeatures(fs, m_cpOutputNo, cp, true);
    addFeatures(fs, m_clpOutputNo, clp, true);
    addFeatures(fs, m_censOutputNo, cens, true);
    addFeatures(fs, m_crpOutputNo, crp, true);
    return fs;
}

void
Tipic::addFeatures(FeatureSet &fs, int outputNo, const RealBlock &block, bool final)
{
    if (block.empty()) return;

    int downsampledOutputNo = outputNo + 1;
    if (outputNo == m_censOutputNo) {
	// CENS exists only in downsampled form
	downsampledOutputNo = outputNo;
    }

    int n = block.size();
    
    if (outputNo != downsampledOutputNo) {
	for (int i = 0; i < n; ++i) {
	    Feature f;
	    int h = block[i].size();
	    f.values.resize(h);
	    for (int j = 0; j < h; ++j) {
		f.values[j] = block[i][j];
	    }
	    fs[outputNo].push_back(f);
	}
    }
    
    if (m_downsamplers.find(outputNo) == m_downsamplers.end()) {
	FeatureDownsample::Parameters params;
	params.featureSize = block[0].size();
	m_downsamplers[outputNo] = new FeatureDownsample(params);
    }

    RealBlock downsampled = m_downsamplers[outputNo]->process(block);

    if (final) {
	RealBlock remaining = m_downsamplers[outputNo]->getRemainingOutput();
	downsampled.insert(downsampled.end(), remaining.begin(), remaining.end());
    }

    n = downsampled.size();
    
    for (int i = 0; i < n; ++i) {
	Feature f;
	int h = downsampled[i].size();
	f.values.resize(h);
	for (int j = 0; j < h; ++j) {
	    f.values[j] = downsampled[i][j];
	}
	fs[downsampledOutputNo].push_back(f);
    }
}
