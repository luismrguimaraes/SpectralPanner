#pragma once

#include <juce_dsp/juce_dsp.h>

/**
  STFT analysis and resynthesis of audio data.

  Each channel should have its own FFTProcessor.
 */
class FFTProcessor
{
public:
    FFTProcessor();

    int getLatencyInSamples() const { return fftSize; }

    void reset();
    float processSample(float sample, bool bypassed);
    void processBlock(float* data, int numSamples, bool bypassed);

    // The FFT has 2^order points and fftSize/2 + 1 bins.
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;      // 1024 samples
    static constexpr int numBins = fftSize / 2 + 1;    // 513 bins
    static constexpr int overlap = 4;                  // 75% overlap
    static constexpr int hopSize = fftSize / overlap;  // 256 samples

    // The FFT working space. Contains interleaved complex numbers.
    std::array<float, fftSize * 2> fftData;

    std::array<float, fftSize> fftDisplayable{};
    std::atomic<bool> readyToDisplay = false;
    std::unique_ptr<double> spectralSliderValue = std::make_unique<double>(0.0);

private:
    void processFrame(bool bypassed);
    void processSpectrum(float* data, int numBins);

    // Gain correction for using Hann window with 75% overlap.
    static constexpr float windowCorrection = 2.0f / 3.0f;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // Counts up until the next hop.
    int count = 0;

    // Write position in input FIFO and read position in output FIFO.
    int pos = 0;

    // Circular buffers for incoming and outgoing audio data.
    std::array<float, fftSize> inputFifo;
    std::array<float, fftSize> outputFifo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTProcessor)
};