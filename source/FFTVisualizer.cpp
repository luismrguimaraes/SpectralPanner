
#include "FFTVisualizer.h"

FFTVisualizer::FFTVisualizer(){
    startTimer (50);

    for (int i = 0; i<FFTProcessor::fftSize; ++i){
        scopeDataL[i] = 0;
        scopeDataR[i] = 0;
    }
}

void FFTVisualizer::timerCallback()
{
    if (processorRef->fft[0].readyToDisplay.load() && processorRef->fft[1].readyToDisplay.load()){
        drawNextFrameOfSpectrum();
        processorRef->fft[0].readyToDisplay.store(false);
        processorRef->fft[1].readyToDisplay.store(false);
        repaint();
    }
}

void FFTVisualizer::drawNextFrameOfSpectrum()
{
    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    for (int i = 0; i < FFTProcessor::fftSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) FFTProcessor::fftSize) * 1.f);
        auto fftDataIndex = juce::jlimit (0, FFTProcessor::fftSize, (int) (skewedProportionX * (float) FFTProcessor::fftSize));
        
        auto levelL = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fft[0].fftDisplayable[fftDataIndex])
                                                            - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
                                    mindB, maxdB, 0.0f, 1.0f);
        scopeDataL[i] = levelL;

        auto levelR = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef->fft[1].fftDisplayable[fftDataIndex])
                                                            - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
                                    mindB, maxdB, 0.0f, 1.0f);
        scopeDataR[i] = levelR;
    }
}

void FFTVisualizer::drawFrame (juce::Graphics& g)
{
    for (int i = 1; i < FFTProcessor::fftSize; ++i)
    {
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();

        g.setColour(juce::Colours::beige);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeDataL[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                        (float) juce::jmap (i,     0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeDataL[i],     0.0f, 1.0f, (float) height, 0.0f) });
        g.setColour(juce::Colours::darkcyan);
        g.drawLine ({ (float) juce::jmap (i - 1, 0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeDataR[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                        (float) juce::jmap (i,     0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeDataR[i],     0.0f, 1.0f, (float) height, 0.0f) });
    }
}

void FFTVisualizer::paint(juce::Graphics &g){
    g.setColour(juce::Colours::black);

    //g.fillAll();

    drawFrame(g);
}