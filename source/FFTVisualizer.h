#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
//#include <juce_gui_basics/components/juce_Component.h>
#include "FFTProcessor.h"
#include "PluginEditor.h"
#include "utils.h"

class PluginProcessor;

class FFTVisualizer : public juce::Component, juce::Timer
{
public:
    FFTVisualizer();

    float scopeDataL[FFTProcessor::numBins];
    float scopeDataR[FFTProcessor::numBins];

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

    PluginProcessor* processorRef;

    float freqMax = 20000;
    float freqMin = 20;
    float skewFactor = 1;

    float mindB = -100.0f;
    float maxdB = 0.0f;

private:
    void drawNextFrameOfSpectrum();
    void drawFrame (juce::Graphics&);

    float previousFreqMax;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTVisualizer)
};