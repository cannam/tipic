
#include "TipicVampPlugin.h"

#include "PitchFilterbank.h"
#include "CRP.h"
#include "Chroma.h"

#include <bqvec/Range.h>
#include <bqvec/VectorOps.h>

#include <iostream>
#include <sstream>

using namespace std;

using namespace breakfastquay;

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
    m_pitchOutputNo(-1),
    m_cpOutputNo(-1),
    m_clpOutputNo(-1),
    m_crpOutputNo(-1)
{
}

Tipic::~Tipic()
{
    delete m_filterbank;
    delete m_crp;
    delete m_chroma;
    delete m_logChroma;
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

    OutputDescriptor d;
    d.identifier = "pitch";
    d.name = "Pitch Features";
    d.description = "";
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
    d.sampleRate = 22050 / 2205; //!!! get block size & hop from filterbank
    d.hasDuration = false;
    m_pitchOutputNo = list.size();
    list.push_back(d);

    d.identifier = "cp";
    d.name = "Chroma Pitch Features";
    d.description = "";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 12;
    d.binNames = noteNames;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = 22050 / 2205; //!!! get block size & hop from filterbank
    d.hasDuration = false;
    m_cpOutputNo = list.size();
    list.push_back(d);

    d.identifier = "clp";
    d.name = "Chroma Log Pitch Features";
    d.description = "";
    m_clpOutputNo = list.size();
    list.push_back(d);

    d.identifier = "crp";
    d.name = "Chroma DCT-Reduced Log Pitch Features";
    d.description = "";
    m_crpOutputNo = list.size();
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
    }
    
    m_filterbank->reset();
}

Tipic::FeatureSet
Tipic::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    RealSequence in;
    in.resize(m_blockSize);
    v_convert(in.data(), inputBuffers[0], m_blockSize);
    
    RealBlock pitchFiltered = m_filterbank->process(in);

    RealBlock cp = m_chroma->process(pitchFiltered);
    RealBlock clp = m_logChroma->process(pitchFiltered);
    RealBlock crp = m_crp->process(pitchFiltered);

    FeatureSet fs;
    addFeatures(fs, m_pitchOutputNo, pitchFiltered);
    addFeatures(fs, m_cpOutputNo, cp);
    addFeatures(fs, m_clpOutputNo, clp);
    addFeatures(fs, m_crpOutputNo, crp);
    return fs;
}

Tipic::FeatureSet
Tipic::getRemainingFeatures()
{
    RealBlock pitchFiltered = m_filterbank->getRemainingOutput();

    RealBlock cp = m_chroma->process(pitchFiltered);
    RealBlock clp = m_logChroma->process(pitchFiltered);
    RealBlock crp = m_crp->process(pitchFiltered);

    FeatureSet fs;
    addFeatures(fs, m_pitchOutputNo, pitchFiltered);
    addFeatures(fs, m_cpOutputNo, cp);
    addFeatures(fs, m_clpOutputNo, clp);
    addFeatures(fs, m_crpOutputNo, crp);
    return fs;
}

void
Tipic::addFeatures(FeatureSet &fs, int outputNo, const RealBlock &block)
{
    for (int i = 0; in_range_for(block, i); ++i) {
	Feature f;
	int h = block[i].size();
	f.values.resize(h);
	v_convert(f.values.data(), block[i].data(), h);
	fs[outputNo].push_back(f);
    }
}
