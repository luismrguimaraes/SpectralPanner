#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include "FFTProcessor.h"
#include "FFTVisualizer.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };

    juce::Slider delayMsSlider;

    juce::Slider spectralSlider{juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow};
    // float scopeDataL[FFTProcessor::fftSize];
    // float scopeDataR[FFTProcessor::fftSize];

    // void drawNextFrameOfSpectrum();
    // void drawFrame(juce::Graphics&);

    FFTVisualizer fftVis;

    int margin = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
