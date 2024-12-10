
#include "FFTVisualizer.h"

FFTVisualizer::FFTVisualizer()
{
    startTimer ((1.0 / 30.0) * 1000.0); // 30 fps

    for (int i = 0; i < FFTProcessor::numBins; ++i)
    {
        scopeDataL[i] = 0;
        scopeDataR[i] = 0;
    }
}

void FFTVisualizer::timerCallback()
{
    if (processorRef->fftProc.readyToDisplay.load())
    {
        drawNextFrameOfSpectrum();
        processorRef->fftProc.readyToDisplay.store (false);
        repaint();
    }
}

void FFTVisualizer::drawNextFrameOfSpectrum()
{
    for (int i = 0; i < FFTProcessor::numBins; ++i)
    {
        auto fftDataIndex = i;

        auto levelL = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fftProc.fftDisplayableL[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        scopeDataL[i] = levelL;

        auto levelR = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fftProc.fftDisplayableR[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        scopeDataR[i] = levelR;
    }
}

void FFTVisualizer::drawFrame (juce::Graphics& g)
{
    int indexMax = binFromFreq (freqMax, processorRef->getSampleRate());
    if (indexMax == 0)
        return; // avoid jmap assertion error

    auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();

    bool bypassed = processorRef->apvts.getRawParameterValue (processorRef->getParamString (processorRef->Parameter::bypass))->load();

    // set lines colour
    g.setColour (juce::Colours::grey.darker());

    // draw a vertical lines
    for (int freq = freqMin; freq < freqMax;)
    {
        g.drawLine ({ (float) juce::jmap (logScale0To1 (freq), 0.0, (double) width),
            (float) height,
            (float) juce::jmap (logScale0To1 (freq), 0.0, (double) width),
            0.0f });

        if (freq < 100)
        {
            freq += 10;
        }
        else if (freq < 1000)
        {
            freq += 100;
        }
        else if (freq < 10000)
        {
            freq += 1000;
        }
        else if (freq < 100000)
        {
            freq += 10000;
        }
    }

    // horizontal line every 6 db
    for (float db = maxdB; db > mindB; db -= 6)
    {
        g.drawLine ({ 0.0f,
            (float) juce::jmap (db, mindB, maxdB, 0.0f, (float) height),
            (float) width,
            (float) juce::jmap (db, mindB, maxdB, 0.0f, (float) height) });
    }

    for (int i = 1; i < FFTProcessor::numBins; ++i)
    {
        auto binFreq = freqFromBin (i, processorRef->getSampleRate());
        auto previousBinFreq = freqFromBin (i - 1, processorRef->getSampleRate());

        if (previousBinFreq < freqMin)
        {
            continue;
        }

        if (!bypassed)
            g.setColour (juce::Colours::beige);
        else
            g.setColour (juce::Colours::beige.darker());
        g.drawLine ({ (float) juce::jmap (logScale0To1 (previousBinFreq), 0.0, (double) width),
            juce::jmap (scopeDataL[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (logScale0To1 (binFreq), 0.0, (double) width),
            juce::jmap (scopeDataL[i], 0.0f, 1.0f, (float) height, 0.0f) });

        if (!bypassed)
            g.setColour (juce::Colours::darkcyan);
        else
            g.setColour (juce::Colours::darkcyan.darker());
        g.drawLine ({ (float) juce::jmap (logScale0To1 (previousBinFreq), 0.0, (double) width),
            juce::jmap (scopeDataR[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (logScale0To1 (binFreq), 0.0, (double) width),
            juce::jmap (scopeDataR[i], 0.0f, 1.0f, (float) height, 0.0f) });
    }
}

void FFTVisualizer::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::black);

    //g.fillAll();

    drawFrame (g);
}