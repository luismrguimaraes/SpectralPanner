#pragma once

#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include "BandComponent.h"
#include "FFTProcessor.h"
#include "FFTVisualizer.h"
#include "PluginProcessor.h"
#include "utils.h"

class BandComponent;
class PluginProcessor;
class FFTVisualizer;

class PluginEditor : public juce::AudioProcessorEditor, juce::AsyncUpdater
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void handleAsyncUpdate() override;
    void updateProcessorValues();

    void removeBand (int bandID);

    void updateEditorValues();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    juce::TextButton newBandButton { "New band" };
    void newBand (bool _initing = false);
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAtt;

    juce::Slider spectralSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };
    juce::Slider freqMaxSlider;
    juce::Slider skewFactorSlider;
    std::unique_ptr<FFTVisualizer> fftVis;

    std::vector<std::unique_ptr<BandComponent>> bandComponents;
    std::vector<std::unique_ptr<juce::TextButton>> bandRemoveButtons;

    double getFreqFromLeft (float left);
    bool initing = true;

    void mouseDoubleClick (const juce::MouseEvent& event) override;

    int margin = 50;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
