#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() : fft (fftOrder),
                               window (fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)
{
    // Note that the window is of length `fftSize + 1` because JUCE's windows
    // are symmetrical, which is wrong for overlap-add processing. To make the
    // window periodic, set size to 1025 but only use the first 1024 samples.

    for (int i = 0; i < numBins; ++i)
    {
        spectralPanValues.push_back (0.0);
    }
}

void FFTProcessor::reset()
{
    count = 0;
    pos = 0;

    // Zero out the circular buffers.
    std::fill (inputFifoL.begin(), inputFifoL.end(), 0.0f);
    std::fill (outputFifoL.begin(), outputFifoL.end(), 0.0f);
    std::fill (inputFifoR.begin(), inputFifoR.end(), 0.0f);
    std::fill (outputFifoR.begin(), outputFifoR.end(), 0.0f);

    // reset spectral multipliers arrays
    spectralPanValues.clear();
    for (int i = 0; i < numBins; ++i)
    {
        spectralPanValues.push_back (0.0);
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

void FFTProcessor::processBlock (float* dataL, float* dataR, int numSamples, bool bypassed)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // data[i]
        auto tuple = processSample (dataL[i], dataR[i], bypassed);
        dataL[i] = std::get<0> (tuple);
        dataR[i] = std::get<1> (tuple);
    }
}

std::tuple<float, float> FFTProcessor::processSample (float sampleL, float sampleR, bool bypassed)
{
    // Push the new sample value into the input FIFO.
    inputFifoL[pos] = sampleL;
    inputFifoR[pos] = sampleR;

    // Read the output value from the output FIFO. Since it takes fftSize
    // timesteps before actual samples are read from this FIFO instead of
    // the initial zeros, the sound output is delayed by fftSize samples,
    // which we will report as our latency.
    float outputSampleL = outputFifoL[pos];
    float outputSampleR = outputFifoR[pos];

    // Once we've read the sample, set this position in the FIFO back to
    // zero so we can add the IFFT results to it later.
    outputFifoL[pos] = 0.0f;
    outputFifoR[pos] = 0.0f;

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

    return std::make_tuple (outputSampleL, outputSampleR);
}

void FFTProcessor::processFrame (bool bypassed)
{
    const float* inputPtrL = inputFifoL.data();
    float* fftPtrL = fftDataL.data();

    const float* inputPtrR = inputFifoR.data();
    float* fftPtrR = fftDataR.data();

    // Copy the input FIFO into the FFT working space in two parts.
    std::memcpy (fftPtrL, inputPtrL + pos, (fftSize - pos) * sizeof (float));
    if (pos > 0)
    {
        std::memcpy (fftPtrL + fftSize - pos, inputPtrL, pos * sizeof (float));
    }
    std::memcpy (fftPtrR, inputPtrR + pos, (fftSize - pos) * sizeof (float));
    if (pos > 0)
    {
        std::memcpy (fftPtrR + fftSize - pos, inputPtrR, pos * sizeof (float));
    }

    // Apply the window to avoid spectral leakage.
    window.multiplyWithWindowingTable (fftPtrL, fftSize);
    window.multiplyWithWindowingTable (fftPtrR, fftSize);

    if (!bypassed)
    {
        // Perform the forward FFT.
        fft.performRealOnlyForwardTransform (fftPtrL, true);
        fft.performRealOnlyForwardTransform (fftPtrR, true);

        // Do stuff with the FFT data.
        processSpectrum (fftPtrL, fftPtrR, numBins);

        // Perform the inverse FFT.
        fft.performRealOnlyInverseTransform (fftPtrL);
        fft.performRealOnlyInverseTransform (fftPtrR);
    }

    // Apply the window again for resynthesis.
    window.multiplyWithWindowingTable (fftPtrL, fftSize);
    window.multiplyWithWindowingTable (fftPtrR, fftSize);

    // Scale down the output samples because of the overlapping windows.
    for (int i = 0; i < fftSize; ++i)
    {
        fftPtrL[i] *= windowCorrection;
        fftPtrR[i] *= windowCorrection;
    }

    // Add the IFFT results to the output FIFO.
    for (int i = 0; i < pos; ++i)
    {
        outputFifoL[i] += fftDataL[i + fftSize - pos];
        outputFifoR[i] += fftDataR[i + fftSize - pos];
    }
    for (int i = 0; i < fftSize - pos; ++i)
    {
        outputFifoL[i + pos] += fftDataL[i];
        outputFifoR[i + pos] += fftDataR[i];
    }

    readyToDisplay.store (true);
}

void FFTProcessor::setPanLaw (float _panLawDB)
{
    if ((_panLawDB > 0.f && _panLawDB < 6.f) || juce::approximatelyEqual (_panLawDB, 0.f))
        panLawDB = _panLawDB;
}

void FFTProcessor::processSpectrum (float* dataL, float* dataR, int _numBins)
{
    // The spectrum data is floats organized as [re, im, re, im, ...]
    // but it's easier to deal with this as std::complex values.
    auto* cdataL = reinterpret_cast<std::complex<float>*> (dataL);
    auto* cdataR = reinterpret_cast<std::complex<float>*> (dataR);

    // skip bin 0
    for (int i = 1; i < _numBins; ++i)
    {
        float panLawGain = juce::Decibels::decibelsToGain (panLawDB);

        // Stereo Panning
        if (spectralPanValues[i] > 0.0f)
        {
            cdataR[i] += cdataL[i] * spectralPanValues[i];
            cdataL[i] *= 1 - spectralPanValues[i];
        }
        else if (spectralPanValues[i] < 0.0f)
        {
            cdataL[i] += cdataR[i] * std::abs (spectralPanValues[i]);
            cdataR[i] *= 1 - std::abs (spectralPanValues[i]);
        }

        // Usually we want to work with the magnitude and phase rather
        // than the real and imaginary parts directly.
        float magnitudeL = std::abs (cdataL[i]);
        float phaseL = std::arg (cdataL[i]);
        float magnitudeR = std::abs (cdataR[i]);
        float phaseR = std::arg (cdataR[i]);

        // This is where you'd do your spectral processing...

        // float threeDBGain = juce::Decibels::decibelsToGain (3.0);
        // float minusThreeDBGain = juce::Decibels::decibelsToGain (-3.0);
        // float dbLawGain = juce::Decibels::decibelsToGain (6.0);

        // Stereo Balance Panning
        // if (spectralPanValues[i] > 0.0f)
        // {
        //     magnitudeL *= juce::jmap (spectralPanValues[i], 1.f, 0.f);
        //     magnitudeR *= juce::jmap (spectralPanValues[i], 1.f, panLawGain);
        // }
        // else if (spectralPanValues[i] < 0.0f)
        // {
        //     magnitudeL *= juce::jmap (spectralPanValues[i], 0.f, -1.f, 1.f, panLawGain);
        //     magnitudeR *= juce::jmap (spectralPanValues[i], 0.f, -1.f, 1.f, 0.f);
        // }

        // fill/update fftDisplayable
        fftDisplayableL[i] = magnitudeL;
        fftDisplayableR[i] = magnitudeR;

        // Convert magnitude and phase back into a complex number.
        cdataL[i] = std::polar (magnitudeL, phaseL);
        cdataR[i] = std::polar (magnitudeR, phaseR);
    }

    // std::cout << *spectralSliderValue << std::endl;
    // std::cout << _numBins << std::endl;
}