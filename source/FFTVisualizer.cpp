
#include "FFTVisualizer.h"

FFTVisualizer::FFTVisualizer()
{
    startTimer (50);

    for (int i = 0; i < FFTProcessor::fftSize; ++i)
    {
        scopeDataL[i] = 0;
        scopeDataR[i] = 0;
    }
}

void FFTVisualizer::timerCallback()
{
    if (processorRef->fft[0].readyToDisplay.load() && processorRef->fft[1].readyToDisplay.load())
    {
        drawNextFrameOfSpectrum();
        processorRef->fft[0].readyToDisplay.store (false);
        processorRef->fft[1].readyToDisplay.store (false);
        repaint();
    }
}

void FFTVisualizer::drawNextFrameOfSpectrum()
{
    for (int i = 0; i < FFTProcessor::numBins; ++i)
    {
        // auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) FFTProcessor::numBins) * skewFactor);
        // auto fftDataIndex = juce::jlimit (0, FFTProcessor::numBins, (int) (skewedProportionX * (float) FFTProcessor::numBins));
        auto fftDataIndex = skewIndex (i, skewFactor);

        auto levelL = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fft[0].fftDisplayable[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        scopeDataL[i] = levelL;

        auto levelR = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fft[1].fftDisplayable[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
            mindB,
            maxdB,
            0.0f,
            1.0f);
        scopeDataR[i] = levelR;
    }
}

void FFTVisualizer::drawFrame (juce::Graphics& g)
{
    // index that has freqMax: sampleRate / fftSize * i = freqMax
    int indexMax = freqMax * FFTProcessor::fftSize / processorRef->getSampleRate();
    indexMax = skewIndex (indexMax, 1 / skewFactor);
    if (indexMax == 0)
        return; // avoid jmap assertion error

    auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();

    // set lines colour
    g.setColour (juce::Colours::grey.darker());

    // every 1000k or so, draw a vertical line
    for (int freq = 1000; freq < freqMax; freq += 1000)
    {
        g.drawLine ({ (float) juce::jmap (freq, 0, (int) freqMax, 0, width),
            (float) height,
            (float) juce::jmap (freq, 0, (int) freqMax, 0, width),
            0.0f });
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
        auto iSkewed = skewIndex (i, 1 / skewFactor);

        g.setColour (juce::Colours::beige);
        g.drawLine ({ (float) juce::jmap (iSkewed - 1, 0, indexMax, 0, width),
            juce::jmap (scopeDataL[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (iSkewed, 0, indexMax, 0, width),
            juce::jmap (scopeDataL[i], 0.0f, 1.0f, (float) height, 0.0f) });

        g.setColour (juce::Colours::darkcyan);
        g.drawLine ({ (float) juce::jmap (iSkewed - 1, 0, indexMax, 0, width),
            juce::jmap (scopeDataR[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
            (float) juce::jmap (iSkewed, 0, indexMax, 0, width),
            juce::jmap (scopeDataR[i], 0.0f, 1.0f, (float) height, 0.0f) });
    }
}

void FFTVisualizer::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::black);

    //g.fillAll();

    drawFrame (g);
}