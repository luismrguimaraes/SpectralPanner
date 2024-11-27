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
    return bin / FFTProcessor::fftSize * samplerate;
}
