#pragma once

#include "FFTProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_processors/juce_audio_processors.h>

// import Signalsmith's DSP library, and ignore its warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "dsp/spectral.h"
#pragma clang diagnostic pop

#if (MSVC)
    #include "ipps.h"
#endif

class PluginEditor;

class PluginProcessor : public juce::AudioProcessor, juce::AudioProcessorParameter::Listener
{
public:
    // KEEP THIS HERE. for some reason parameterValueChanged was using this with value
    // true, when it was the last public declaration.
    std::atomic<bool> editorCreated = false;

    enum Parameter {
        bypass,
        band,
        bandSlider,
        bandsInUse
    };
    static juce::String getParamString (Parameter param)
    {
        switch (param)
        {
            case bypass:
                return "bypass";
            case band:
                return "band";
            case bandSlider:
                return "bandSlider";
            case bandsInUse:
                return "bandsInUse";
        }
    }
    float getBand (int index);
    int updateBand (int index, double value);
    void addBand (double value);
    int removeBand (int index = -1);
    bool canAddBand();
    int const bandNMax = 10;

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

    int getBandsInUse();
    float getBandSliderValue (int);
    // We need a separate FFTProcessor for each channel.
    FFTProcessor fft[2];

    juce::AudioProcessorParameter* getBypassParameter() const override;
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    juce::AudioParameterFloat* getBandParameter (int bandIndex);
    juce::AudioParameterInt* getBandsInUseParameter();
    juce::AudioProcessorEditor* editor = nullptr;

    void parameterValueChanged (int, float) override;
    void parameterGestureChanged (int, bool) override;

    void updateFFTProcessorMultipliers();
};
