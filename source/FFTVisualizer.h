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

    float freqMax;
    float skewFactor = 1;

    float mindB = -100.0f;
    float maxdB = 0.0f;

private:
    void drawNextFrameOfSpectrum();
    void drawFrame (juce::Graphics&);

    float previousFreqMax;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTVisualizer)
};