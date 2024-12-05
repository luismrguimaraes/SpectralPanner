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
    std::tuple<float, float> processSample (float sampleL, float sampleR, bool bypassed);
    void processBlock (float* dataL, float* dataR, int numSamples, bool bypassed);

    void setSampleRate (int sampleRate);
    int getSampleRate();
    void setPanLaw (float panLawDB);

    // The FFT has 2^order points and fftSize/2 + 1 bins.
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numBins = fftSize / 2 + 1;
    static constexpr int overlap = 4; // 75% overlap
    static constexpr int hopSize = fftSize / overlap;

    std::array<float, fftSize> fftDisplayableL {};
    std::array<float, fftSize> fftDisplayableR {};
    std::atomic<bool> readyToDisplay = false;
    std::unique_ptr<double> spectralSliderValue = std::make_unique<double> (0.0);

    std::vector<float> spectralPanValues;

private:
    void processFrame (bool bypassed);
    void processSpectrum (float* dataL, float* dataR, int numBins);

    // Gain correction for using Hann window with 75% overlap.
    static constexpr float windowCorrection = 2.0f / 3.0f;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // The FFT working space. Contains interleaved complex numbers.
    std::array<float, fftSize * 2> fftDataL;
    std::array<float, fftSize * 2> fftDataR;

    // Counts up until the next hop.
    int count = 0;

    // Write position in input FIFO and read position in output FIFO.
    int pos = 0;

    // Circular buffers for incoming and outgoing audio data.
    std::array<float, fftSize> inputFifoL;
    std::array<float, fftSize> inputFifoR;
    std::array<float, fftSize> outputFifoL;
    std::array<float, fftSize> outputFifoR;

    int sampleRate;
    float panLawDB = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTProcessor)
};