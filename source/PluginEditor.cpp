#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), delayMsSlider()
{
    juce::ignoreUnused (processorRef);

    for (int i = 0; i<FFTProcessor::fftSize; ++i){
        scopeDataL[i] = 0;
        scopeDataR[i] = 0;
    }

    addAndMakeVisible (inspectButton);
    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    //addAndMakeVisible (delayMsSlider);
    delayMsSlider.setRange(0, 2000, 1);
    delayMsSlider.onValueChange = [this] { *processorRef.delayMs = delayMsSlider.getValue();};

    addAndMakeVisible(spectralSlider);
    spectralSlider.setRange(-1, 1);
    spectralSlider.setValue(0.0);
    spectralSlider.onValueChange = [this] { 
        *processorRef.fft[0].spectralSliderValue = -spectralSlider.getValue();
        *processorRef.fft[1].spectralSliderValue = spectralSlider.getValue();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    startTimer (50);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::drawNextFrameOfSpectrum()
{
    auto mindB = JUCE_LIVE_CONSTANT(-100.0f);
    auto maxdB =    JUCE_LIVE_CONSTANT(0.0f);

    for (int i = 0; i < FFTProcessor::fftSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) FFTProcessor::fftSize) * 1.f);
        auto fftDataIndex = juce::jlimit (0, FFTProcessor::fftSize, (int) (skewedProportionX * (float) FFTProcessor::fftSize));
        
        auto levelL = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef.fft[0].fftDisplayable[fftDataIndex])
                                                            - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
                                    mindB, maxdB, 0.0f, 1.0f);
        scopeDataL[i] = levelL;

        auto levelR = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef.fft[1].fftDisplayable[fftDataIndex])
                                                            - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
                                    mindB, maxdB, 0.0f, 1.0f);
        scopeDataR[i] = levelR;
    }
}

void PluginEditor::timerCallback()
{
    if (processorRef.fft[0].readyToDisplay.load() && processorRef.fft[1].readyToDisplay.load()){
        drawNextFrameOfSpectrum();
        processorRef.fft[0].readyToDisplay.store(false);
        processorRef.fft[1].readyToDisplay.store(false);
        repaint();
    }
}

void PluginEditor::drawFrame (juce::Graphics& g)
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

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    //auto area = getLocalBounds();
    //auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);

    drawFrame(g);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));

    delayMsSlider.setBounds(getLocalBounds());

    spectralSlider.setBounds(getLocalBounds());
}
