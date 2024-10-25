#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FFTProcessor.h"

// import Signalsmith's DSP library, and ignore its warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "dsp/spectral.h"
#include "dsp/delay.h"
#pragma clang diagnostic pop

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
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

    std::unique_ptr<juce::AudioParameterInt> delayMs = std::make_unique<juce::AudioParameterInt> ("paramID", "Parameter Name", 0, 5000, 800);

    // signalsmith::spectral::STFT<float> stft{1, 512, 64};
    // std::atomic<bool> stftReady = false;

    juce::AudioProcessorParameter* getBypassParameter() const override;
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // We need a separate FFTProcessor for each channel.
    FFTProcessor fft[2];

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    int previousDelayMs;
	double decayGain = 0.5;

    float wet = 0.5;

	int delaySamples;
    using Delay = signalsmith::delay::Delay<float, signalsmith::delay::InterpolatorNearest>;
    Delay delay;

};
