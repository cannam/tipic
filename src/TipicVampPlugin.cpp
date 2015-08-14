
#include "TipicVampPlugin.h"

Tipic::Tipic(float inputSampleRate) :
    Plugin(inputSampleRate)
{
}

Tipic::~Tipic()
{
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
    return list;
}

float
Tipic::getParameter(string identifier) const
{
    return 0;
}

void
Tipic::setParameter(string identifier, float value) 
{
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
    d.sampleRate = 4410.0 / m_inputSampleRate;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool
Tipic::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    return true;
}

void
Tipic::reset()
{
    // Clear buffers, reset stored values, etc
}

Tipic::FeatureSet
Tipic::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    return FeatureSet();
}

Tipic::FeatureSet
Tipic::getRemainingFeatures()
{
    return FeatureSet();
}

