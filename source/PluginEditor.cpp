#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    bypassButtonAtt = new juce::AudioProcessorValueTreeState::ButtonAttachment (processorRef.apvts, processorRef.getParamString (processorRef.Parameter::bypass), bypassButton);

    addAndMakeVisible (fftVis);
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

    bandComp1 = std::make_unique<BandComponent>();
    addAndMakeVisible (*bandComp1);
    bandComp1->isDraggable = false;
    bandComp1->left = margin;
    bandComp1->minimumLeft = bandComp1->left;
    bandComp1->bandID = (int) bandComponents.size();
    processorRef.addBand (getFreqFromLeft (bandComp1->left));
    bandComponents.push_back (std::move (bandComp1));
    auto newBandRemoveButton = std::make_unique<juce::TextButton> ("-");
    bandRemoveButtons.push_back (std::move (newBandRemoveButton));

    addAndMakeVisible (newBandButton);
    newBandButton.onClick = [&] {
        newBand();
    };

    addAndMakeVisible (bypassButton);

    addAndMakeVisible (spectralSlider);
    spectralSlider.setRange (-1, 1);
    spectralSlider.setValue (0.0);
    spectralSlider.onValueChange = [this] {
        *processorRef.fft[0].spectralSliderValue = -spectralSlider.getValue();
        *processorRef.fft[1].spectralSliderValue = spectralSlider.getValue();
    };

    addAndMakeVisible (freqMaxSlider);
    freqMaxSlider.setRange (1000, 20000);
    freqMaxSlider.onValueChange = [this] {
        fftVis.freqMax = freqMaxSlider.getValue();
    };
    freqMaxSlider.setValue (20000);

    addAndMakeVisible (skewFactorSlider);
    skewFactorSlider.setRange (0.1, 1);
    skewFactorSlider.setValue (1);
    skewFactorSlider.onValueChange = [this] {
        fftVis.skewFactor = skewFactorSlider.getValue();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 500);
    //setResizable (true, true);
    //setResizeLimits (500, 300, 10000, 10000);
}

PluginEditor::~PluginEditor()
{
    bandComponents.clear();
}

void PluginEditor::newBand()
{
    auto newLeft = bandComponents[bandComponents.size() - 1]->left + 100;
    if (newLeft + 50 > getLocalBounds().reduced (margin).getRight())
    {
        std::cout << getLocalBounds().reduced (margin).getRight() << std::endl;
        return;
    }

    std::unique_ptr<BandComponent> newBandComponent = std::make_unique<BandComponent>();
    addAndMakeVisible (*newBandComponent);
    newBandComponent->left = newLeft;
    newBandComponent->bandID = (int) bandComponents.size();
    processorRef.addBand (getFreqFromLeft (newBandComponent->left));
    bandComponents.push_back (std::move (newBandComponent));

    auto newBandRemoveButton = std::make_unique<juce::TextButton> ("-");
    addAndMakeVisible (*newBandRemoveButton);
    int newBandIndex = (int) bandComponents.size() - 1;
    newBandRemoveButton->onClick = [&, newBandIndex] {
        removeBand (newBandIndex);
    };
    bandRemoveButtons.push_back (std::move (newBandRemoveButton));

    resized();

    std::cout << "size after new band: " << bandComponents.size() << std::endl;
}

void PluginEditor::removeBand (int bandID)
{
    jassert (bandID < (int) bandComponents.size());

    bandComponents.erase (bandComponents.begin() + bandID);
    bandRemoveButtons.erase (bandRemoveButtons.begin() + bandID);
    processorRef.removeBand(bandID);

    for (int i = 0; i < (int) bandComponents.size(); ++i)
    {
        // update bandIDs
        bandComponents[i]->bandID = i;

        // update removeButtons onClick
        bandRemoveButtons[i]->onClick = [&, i] {
            removeBand (i);
        };
    }

    resized();

    std::cout << "size after removal: " << bandComponents.size() << std::endl;
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    // g.setColour (juce::Colours::white);
    // g.setFont (16.0f);
    //auto area = getLocalBounds();
    //auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto b = getLocalBounds().reduced (margin);

    // layout the positions of your child components here
    fftVis.setBounds (b);

    if (bandComponents.size() == 1)
        bandComponents[0]->setBounds (b);
    else
    {
        // update minimumLeft
        for (int i = 0; i < bandComponents.size() - 1; ++i)
        {
            auto newMinLeft = 0;
            newMinLeft = bandComponents[i]->left + 50;

            bandComponents[i + 1]->minimumLeft = newMinLeft;
        }

        // set bounds
        jassert (bandComponents.size() == bandRemoveButtons.size());
        for (int i = bandComponents.size() - 1; i >= 0; --i)
        {
            if (i == 0)
            { // left-most band
                bandComponents[i]->setBounds (b.withRight (bandComponents[i + 1]->left));
            }
            else if (i == bandComponents.size() - 1)
            { // right-most band
                bandComponents[i]->setBounds (b.withLeft (bandComponents[i]->left));
            }
            else
            {
                bandComponents[i]->setBounds (b.withRight (bandComponents[i + 1]->left).withLeft (bandComponents[i]->left));
            }

            bandRemoveButtons[i]->setBounds (bandComponents[i]->getBounds().withWidth (40).withHeight (20));
        }
    }

    updateProcessorValues();

    inspectButton.setBounds (getLocalBounds().withWidth (100).withHeight (50).withY (0));

    newBandButton.setBounds (getLocalBounds().withWidth (100).withHeight (50).withY (getLocalBounds().getBottom() - 50));
    bypassButton.setBounds (getLocalBounds().withWidth (50).withHeight (50).withY (getLocalBounds().getBottom() - 50));

    spectralSlider.setBounds (b.withHeight (100));

    freqMaxSlider.setBounds (b.withWidth (b.getWidth() / 2).withX (b.getWidth() / 2 + 50).withHeight (50).withY (b.getHeight() + 25));
    skewFactorSlider.setBounds (b.withWidth (b.getWidth() / 2).withHeight (50).withY (b.getHeight() + 25));
}

void PluginEditor::mouseDoubleClick (const juce::MouseEvent& event)
{
    newBand();
}

double inline PluginEditor::getFreqFromLeft (int left)
{
    auto b = getLocalBounds().reduced (margin);

    auto value = juce::jmap ((double) left, (double) b.getX(), (double) b.getX() + b.getWidth(), 0.0, 1.0);

    return value * fftVis.freqMax;
}

void PluginEditor::updateProcessorValues()
{
    for (int i = 0; i < bandComponents.size(); ++i)
    {
        processorRef.updateBand (i, getFreqFromLeft (bandComponents[i]->left));
    }

    // int bandIndex = 0;
    // for (int i = 0; i < FFTProcessor::numBins; ++i)
    // {
    //     // Left
    //     processorRef.fft[0].newSpectralMultipliers[i] =
    //         // Right
    //         processorRef.fft[1].newSpectralMultipliers[i] =
    // }

    // processorRef.fft[0].spectralMultipliersChanged.store (true);
    // processorRef.fft[1].spectralMultipliersChanged.store (true);
    // *processorRef.fft[0].spectralSliderValue = -spectralSlider.getValue();
    // *processorRef.fft[1].spectralSliderValue = spectralSlider.getValue();
}