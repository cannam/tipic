
#include "TipicVampPlugin.h"

#include <bqvec/Range.h>
#include <bqvec/VectorOps.h>

#include <iostream>

using namespace std;

using namespace breakfastquay;

static const float defaultTuningFrequency = 440.f;

Tipic::Tipic(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_stepSize(0),
    m_blockSize(0),
    m_tuningFrequency(defaultTuningFrequency),
    m_filterbank(0),
    m_pitchOutputNo(-1)
{
}

Tipic::~Tipic()
{
    delete m_filterbank;
}

string
Tipic::getIdentifier() const
{
    return "tipic";
}

string
Tipic::getName() const
{
    return "Timbre-Invariant Pitch Chroma";
}

string
Tipic::getDescription() const
{
    // Return something helpful here!
    return "";
}

string
Tipic::getMaker() const
{
    // Your name here
    return "";
}

int
Tipic::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
Tipic::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "";
}

Tipic::InputDomain
Tipic::getInputDomain() const
{
    return TimeDomain;
}

size_t
Tipic::getPreferredBlockSize() const
{
    return 0; // 0 means "I can handle any block size"
}

size_t 
Tipic::getPreferredStepSize() const
{
    return 0; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
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

Tipic::OutputList
Tipic::getOutputDescriptors() const
{
    OutputList list;

    OutputDescriptor d;
    d.identifier = "pitch";
    d.name = "Pitch Features";
    d.description = "";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 88;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = 22050 / 2205; //!!! get block size & hop from filterbank
    d.hasDuration = false;
    m_pitchOutputNo = list.size();
    list.push_back(d);

    return list;
}

bool
Tipic::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (m_pitchOutputNo < 0) {
	// getOutputDescriptors has never been called, it sets up the
	// outputNo members
	(void)getOutputDescriptors();
    }
    
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) {
	cerr << "ERROR: initialise: wrong number of channels supplied (only 1 supported)" << endl;
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
    }
    m_filterbank->reset();
}

Tipic::FeatureSet
Tipic::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    PitchFilterbank::RealSequence in;
    in.resize(m_blockSize);
    v_convert(in.data(), inputBuffers[0], m_blockSize);
    
    PitchFilterbank::RealBlock pitchFiltered = m_filterbank->process(in);

    FeatureSet fs;
    addPitchFeatures(fs, pitchFiltered);
    return fs;
}

Tipic::FeatureSet
Tipic::getRemainingFeatures()
{
    PitchFilterbank::RealBlock pitchFiltered = m_filterbank->getRemainingOutput();

    FeatureSet fs;
    addPitchFeatures(fs, pitchFiltered);
    return fs;
}

void
Tipic::addPitchFeatures(FeatureSet &fs, const PitchFilterbank::RealBlock &block)
{
    for (int i = 0; in_range_for(block, i); ++i) {
	Feature f;
	int h = block[i].size();
	f.values.resize(h);
	v_convert(f.values.data(), block[i].data(), h);
	fs[m_pitchOutputNo].push_back(f);
    }
}

