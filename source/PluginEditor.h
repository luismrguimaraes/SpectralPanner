#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include "BandComponent.h"
#include "FFTProcessor.h"
#include "FFTVisualizer.h"
#include "utils.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void updateProcessorValues();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };

    juce::TextButton newBandButton { "New band" };
    void newBand();

    juce::Slider delayMsSlider;
    juce::Slider spectralSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };
    juce::Slider freqMaxSlider;
    juce::Slider skewFactorSlider;
    FFTVisualizer fftVis;
    std::unique_ptr<BandComponent> bandComp1;
    //BandComponent bandComp2;

    std::vector<std::unique_ptr<BandComponent>> bandComponents;
    double getFreqFromLeft (int left);

    void mouseDoubleClick (const juce::MouseEvent& event) override;

    int margin = 50;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
