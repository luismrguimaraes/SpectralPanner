#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
//#include <juce_gui_basics/components/juce_Component.h>
#include "FFTProcessor.h"
#include "PluginProcessor.h"
#include "utils.h"

class FFTVisualizer : public juce::Component, juce::Timer
{
public:
    FFTVisualizer();

    float scopeDataL[FFTProcessor::fftSize];
    float scopeDataR[FFTProcessor::fftSize];

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

    PluginProcessor* processorRef;

    float freqMax = 30000;
    float skewFactor = 1;

private:
    void drawNextFrameOfSpectrum();
    void drawFrame (juce::Graphics&);

    float previousFreqMax = freqMax;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTVisualizer)
};