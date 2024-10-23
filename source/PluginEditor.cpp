#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), delayMsSlider()
{
    juce::ignoreUnused (processorRef);

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

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    startTimer (50);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::drawNextFrameOfSpectrum()
{
    auto mindB = -100.0f;
    auto maxdB =    0.0f;

        for (int i = 0; i < FFTProcessor::fftSize; ++i)                         // [3]
        {
            //auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) FFTProcessor::fftSize) * 0.2f);
            //auto fftDataIndex = juce::jlimit (0, FFTProcessor::fftSize / 2, (int) (skewedProportionX * (float) FFTProcessor::fftSize * 0.5f));
            auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (processorRef.fft[0].fftData[i])
                                                               - juce::Decibels::gainToDecibels ((float) FFTProcessor::fftSize)),
                                     mindB, maxdB, 0.0f, 1.0f);
        scopeData[i] = level; //std::abs(processorRef.stft.spectrum[i]->imag()) + std::abs(processorRef.stft.spectrum[i]->real());
        
        // scopeData[i] = std::abs(*processorRef.stft.spectrum[i]);
        if (scopeData[i] < 0.f || scopeData[i] > 1.f)
             std::cout << scopeData[i] << std::endl;
        }
}

void PluginEditor::timerCallback()
{
    if (processorRef.fft[0].ready.load()){
        drawNextFrameOfSpectrum();
        processorRef.fft[0].ready.store(false);
        repaint();
    }
}

void PluginEditor::drawFrame (juce::Graphics& g)
{
    for (int i = 1; i < FFTProcessor::fftSize; ++i)
    {
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();

        g.drawLine ({ (float) juce::jmap (i - 1, 0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                        (float) juce::jmap (i,     0, FFTProcessor::fftSize - 1, 0, width),
                                juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) });
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
}
