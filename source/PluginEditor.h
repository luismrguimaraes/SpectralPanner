#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include "FFTProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };

    juce::Slider delayMsSlider;
    float scopeData[FFTProcessor::fftSize];
    std::vector<float *> scopeDataStorage{};
    int scopeDataStorageMaxSize = 64;

    void drawNextFrameOfSpectrum();
    void drawFrame(juce::Graphics&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
