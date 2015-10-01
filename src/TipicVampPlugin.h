#ifndef TIPIC_VAMP_PLUGIN_H
#define TIPIC_VAMP_PLUGIN_H

#include <vamp-sdk/Plugin.h>

#include "Types.h"

class PitchFilterbank;
class CRP;
class Chroma;
class CENS;
class FeatureDownsample;

using std::string;

class Tipic : public Vamp::Plugin
{
public:
    Tipic(float inputSampleRate);
    virtual ~Tipic();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    int m_stepSize;
    int m_blockSize;
    float m_tuningFrequency;
    PitchFilterbank *m_filterbank;
    CRP *m_crp;
    Chroma *m_chroma;
    Chroma *m_logChroma;
    CENS *m_cens;
    mutable int m_pitchOutputNo;
    mutable int m_cpOutputNo;
    mutable int m_clpOutputNo;
    mutable int m_censOutputNo;
    mutable int m_crpOutputNo;
    std::map<int, FeatureDownsample *> m_downsamplers;

    void addFeatures(FeatureSet &, int outputNo, const RealBlock &, bool final);
};


#endif
