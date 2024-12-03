#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() : fft (fftOrder),
                               window (fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)
{
    // Note that the window is of length `fftSize + 1` because JUCE's windows
    // are symmetrical, which is wrong for overlap-add processing. To make the
    // window periodic, set size to 1025 but only use the first 1024 samples.

    for (int i = 0; i < numBins; ++i)
    {
        spectralMultipliers.push_back (0.0);
    }
}

void FFTProcessor::reset()
{
    count = 0;
    pos = 0;

    // Zero out the circular buffers.
    std::fill (inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill (outputFifo.begin(), outputFifo.end(), 0.0f);

    // reset spectral multipliers arrays
    spectralMultipliers.clear();
    spectralMultipliersChanged.store (false);
    for (int i = 0; i < numBins; ++i)
    {
        spectralMultipliers.push_back (0.0);
    }
}

void FFTProcessor::setSampleRate (int _sampleRate)
{
    sampleRate = _sampleRate;
}

int FFTProcessor::getSampleRate()
{
    return sampleRate;
}

void FFTProcessor::processBlock (float* data, int numSamples, bool bypassed)
{
    for (int i = 0; i < numSamples; ++i)
    {
        data[i] = processSample (data[i], bypassed);
    }
}

float FFTProcessor::processSample (float sample, bool bypassed)
{
    // Push the new sample value into the input FIFO.
    inputFifo[pos] = sample;

    // Read the output value from the output FIFO. Since it takes fftSize
    // timesteps before actual samples are read from this FIFO instead of
    // the initial zeros, the sound output is delayed by fftSize samples,
    // which we will report as our latency.
    float outputSample = outputFifo[pos];

    // Once we've read the sample, set this position in the FIFO back to
    // zero so we can add the IFFT results to it later.
    outputFifo[pos] = 0.0f;

    // Advance the FIFO index and wrap around if necessary.
    pos += 1;
    if (pos == fftSize)
    {
        pos = 0;
    }

    // Process the FFT frame once we've collected hopSize samples.
    count += 1;
    if (count == hopSize)
    {
        count = 0;
        processFrame (bypassed);
    }

    return outputSample;
}

void FFTProcessor::processFrame (bool bypassed)
{
    const float* inputPtr = inputFifo.data();
    float* fftPtr = fftData.data();

    // Copy the input FIFO into the FFT working space in two parts.
    std::memcpy (fftPtr, inputPtr + pos, (fftSize - pos) * sizeof (float));
    if (pos > 0)
    {
        std::memcpy (fftPtr + fftSize - pos, inputPtr, pos * sizeof (float));
    }

    // Apply the window to avoid spectral leakage.
    window.multiplyWithWindowingTable (fftPtr, fftSize);

    if (!bypassed)
    {
        // Perform the forward FFT.
        fft.performRealOnlyForwardTransform (fftPtr, true);

        // Do stuff with the FFT data.
        processSpectrum (fftPtr, numBins);

        // Perform the inverse FFT.
        fft.performRealOnlyInverseTransform (fftPtr);
    }

    // Apply the window again for resynthesis.
    window.multiplyWithWindowingTable (fftPtr, fftSize);

    // Scale down the output samples because of the overlapping windows.
    for (int i = 0; i < fftSize; ++i)
    {
        fftPtr[i] *= windowCorrection;
    }

    // Add the IFFT results to the output FIFO.
    for (int i = 0; i < pos; ++i)
    {
        outputFifo[i] += fftData[i + fftSize - pos];
    }
    for (int i = 0; i < fftSize - pos; ++i)
    {
        outputFifo[i + pos] += fftData[i];
    }

    readyToDisplay.store (true);
}

void FFTProcessor::processSpectrum (float* data, int _numBins)
{
    // The spectrum data is floats organized as [re, im, re, im, ...]
    // but it's easier to deal with this as std::complex values.
    auto* cdata = reinterpret_cast<std::complex<float>*> (data);

    // skip bin 0
    for (int i = 1; i < _numBins; ++i)
    {
        // Usually we want to work with the magnitude and phase rather
        // than the real and imaginary parts directly.
        float magnitude = std::abs (cdata[i]);
        float phase = std::arg (cdata[i]);

        // This is where you'd do your spectral processing...

        // do parameter smoothing ??
        // apply panning
        // float threeDBGain = juce::Decibels::decibelsToGain (3.0);
        // float minusThreeDBGain = juce::Decibels::decibelsToGain (-3.0);
        float sixDBGain = juce::Decibels::decibelsToGain (6.0);
        if (spectralMultipliers[i] > 0.0f)
        {
            magnitude *= juce::jmap (spectralMultipliers[i], 1.f, sixDBGain);
        }
        else if (spectralMultipliers[i] < 0.0f)
        {
            magnitude *= juce::jmap (spectralMultipliers[i], 0.f, -1.f, 1.f, 0.f);

            std::cout << juce::jmap (spectralMultipliers[i], 0.f, -1.f, 1.f, 0.f) << std::endl;
        }

        // magnitude *= (1.0 + spectralMultipliers[i]) / 2.0;

        // fill/update fftDisplayable
        fftDisplayable[i] = magnitude;

        // Convert magnitude and phase back into a complex number.
        cdata[i] = std::polar (magnitude, phase);
    }

    // std::cout << *spectralSliderValue << std::endl;
    // std::cout << _numBins << std::endl;
}