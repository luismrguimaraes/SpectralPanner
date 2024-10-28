#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), delayMsSlider()
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible(fftVis);
    fftVis.processorRef = &processorRef;

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

    addAndMakeVisible(bandComp1);
    bandComp1.minimumLeft = margin;
    bandComp1.isDraggable = false;
    addAndMakeVisible(bandComp2);
    bandComp2.left = 200;
    std::cout << bandComp2.left << std::endl;
    bandComp2.minimumLeft = margin + 50;

    addAndMakeVisible(newBandButton);
    inspectButton.onClick = [&] {
        newBand();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::newBand(){
    
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (16.0f);
    //auto area = getLocalBounds();
    //auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto b = getLocalBounds().reduced(margin);

    // layout the positions of your child components here
    fftVis.setBounds(b);

    bandComp2.setBounds(b.withLeft(bandComp2.left));
    bandComp1.setBounds(b.withRight(bandComp2.left));

    //inspectButton.setBounds (b.withSizeKeepingCentre(100, 50));

    newBandButton.setBounds (b.withSizeKeepingCentre(100, 50));

    delayMsSlider.setBounds(b);

    spectralSlider.setBounds(b.withHeight(100));
}
