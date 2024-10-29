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

    bandComp1 = std::make_unique<BandComponent>();
    addAndMakeVisible(*bandComp1);
    bandComp1->minimumLeft = margin;
    bandComp1->isDraggable = false;
    bandComp1->left = 0;
    bandComponents.push_back(std::move(bandComp1));

    addAndMakeVisible(newBandButton);
    newBandButton.onClick = [&] {
        newBand();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
}

PluginEditor::~PluginEditor()
{
    bandComponents.clear();
}

void PluginEditor::newBand(){
    auto newLeft = bandComponents[bandComponents.size()-1]->left + 100;
    if (newLeft + 50 > getLocalBounds().reduced(margin).getRight()){
        std::cout << getLocalBounds().reduced(margin).getRight() << std::endl;
        return;
    }


    std::unique_ptr<BandComponent> newBandComponent = std::make_unique<BandComponent>();
    addAndMakeVisible(*newBandComponent);
    newBandComponent->left = newLeft;
    bandComponents.push_back(std::move(newBandComponent));

    resized();

    std::cout << "size: " << bandComponents.size() << std::endl;
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

    //bandComp2.setBounds(b.withLeft(bandComp2.left));
    //bandComp1.setBounds(b.withRight(bandComp2.left));

    if (bandComponents.size() == 1)
        bandComponents[0]->setBounds(b);
    else{
        // update minimumLeft
        for (int i = 0; i < bandComponents.size() -1; ++i){
            auto newMinLeft = 0;
            if (i == 0)
                newMinLeft = margin + bandComponents[i]->left + 50;
            else
                newMinLeft = bandComponents[i]->left + 50;
            
            bandComponents[i+1]->minimumLeft = newMinLeft;
        }

        // set bounds
        for (int i = bandComponents.size() -1; i >= 0; --i){

            auto left = 0;
            auto right = 0;

            if (i == 0){ // left-most band
                bandComponents[i]->setBounds(b.withRight(bandComponents[i+1]->left));
            }
            else if (i == bandComponents.size() -1){ // right-most band
                bandComponents[i]->setBounds(b.withLeft(bandComponents[i]->left));
            }
            else{
                bandComponents[i]->setBounds(b.withRight(bandComponents[i+1]->left).withLeft(bandComponents[i]->left));
            }
        }
    }

    //inspectButton.setBounds (b.withSizeKeepingCentre(100, 50));

    newBandButton.setBounds (getLocalBounds().withWidth(100).withHeight(50).withY(getLocalBounds().getBottom() - 50));

    delayMsSlider.setBounds(b);

    spectralSlider.setBounds(b.withHeight(100));
}
