#pragma once

#include "FFTProcessor.h"

inline int skewIndex (int index, float skewFactor)
{
    auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) index / (float) FFTProcessor::numBins) * skewFactor);
    auto skewedIndex = juce::jlimit (0, FFTProcessor::numBins, (int) (skewedProportionX * (float) FFTProcessor::numBins));

    return skewedIndex;
}

inline int binFromFreq (double freq, int samplerate)
{
    return freq / samplerate * FFTProcessor::fftSize;
}

inline double freqFromBin (int bin, int samplerate)
{
    return (double) bin / FFTProcessor::fftSize * samplerate;
}

inline double logScale0To1 (double freq, float min = 20.f, float max = 20000.f)
{
    //(\log(x)-\log\left(20)\right)/\left(\log(20000)-\log\left(20))\right)\right)
    auto value0To1 = (std::log (freq) - std::log (min)) / (std::log (max) - std::log (min));
    return value0To1;
}

inline double logScaleFrom0To1 (double value0To1, float min = 20.f, float max = 20000.f)
{
    // freqLogValue == std::log(freq)
    auto freqLogValue = value0To1 * (std::log (max) - std::log (min)) + std::log (min);

    auto freq = std::exp (freqLogValue);
    return freq;
}