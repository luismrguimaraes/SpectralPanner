#pragma once

#include "FFTProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

// import Signalsmith's DSP library, and ignore its warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "dsp/spectral.h"
#pragma clang diagnostic pop

#if (MSVC)
    #include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    enum Parameter {
        bypass,
        bands

    };
    static std::string getParamString (Parameter param)
    {
        switch (param)
        {
            case bypass:
                return "bypass";
            case bands:
                return "bands";
        }
    }
    float getBand (int index);
    int updateBand (int index, int value);
    void addBand (int value);
    int removeBand (int index);

    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // signalsmith::spectral::STFT<float> stft{1, 512, 64};
    // std::atomic<bool> stftReady = false;

    juce::AudioProcessorParameter* getBypassParameter() const override;
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // We need a separate FFTProcessor for each channel.
    FFTProcessor fft[2];

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    juce::var bandsArr;
};
