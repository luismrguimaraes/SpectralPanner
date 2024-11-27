#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    bypassButtonAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processorRef.apvts, processorRef.getParamString (processorRef.Parameter::bypass), bypassButton);

    fftVis = std::make_unique<FFTVisualizer>();
    addAndMakeVisible (*fftVis);
    fftVis->processorRef = &processorRef;

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

    // auto bandComp1 = std::make_unique<BandComponent>();
    // addAndMakeVisible (*bandComp1);
    // bandComp1->isDraggable = false;
    // bandComp1->left = margin;
    // bandComp1->minimumLeft = bandComp1->left;
    // bandComp1->bandID = (int) bandComponents.size();
    // processorRef.addBand (getFreqFromLeft (bandComp1->left));
    // bandComponents.push_back (std::move (bandComp1));
    // auto newBandRemoveButton = std::make_unique<juce::TextButton> ("-");
    // bandRemoveButtons.push_back (std::move (newBandRemoveButton));

    addAndMakeVisible (newBandButton);
    newBandButton.onClick = [&] {
        newBand();
    };

    addAndMakeVisible (bypassButton);

    // addAndMakeVisible (spectralSlider);
    // spectralSlider.setRange (-1, 1);
    // spectralSlider.setValue (0.0);
    // spectralSlider.onValueChange = [this] {
    //     *processorRef.fft[0].spectralSliderValue = -spectralSlider.getValue();
    //     *processorRef.fft[1].spectralSliderValue = spectralSlider.getValue();
    // };

    // addAndMakeVisible (freqMaxSlider);
    freqMaxSlider.setRange (1000, 20000);
    freqMaxSlider.onValueChange = [this] {
        fftVis->freqMax = freqMaxSlider.getValue();
    };
    freqMaxSlider.setValue (20000);

    // addAndMakeVisible (skewFactorSlider);
    skewFactorSlider.setRange (0.1, 1);
    skewFactorSlider.setValue (1);
    skewFactorSlider.onValueChange = [this] {
        fftVis->skewFactor = skewFactorSlider.getValue();
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 500);
    //setResizable (true, true);
    //setResizeLimits (500, 300, 10000, 10000);
}

PluginEditor::~PluginEditor()
{
    // bandComponents.clear();
    // bandRemoveButtons.clear();

    initing = true;

    std::cout << "destroying editor" << std::endl;
}

void PluginEditor::newBand (bool _initing)
{
    if (!_initing && !processorRef.canAddBand())
    {
        std::cout << "Cannot add new band" << std::endl;
        return;
    }

    auto isFirstBand = (int) bandComponents.size() == 0;
    float newLeft = margin;
    if (!isFirstBand)
        newLeft = bandComponents[bandComponents.size() - 1]->left + 100.f;
    auto noSpaceForNewBand = newLeft + 50 > getLocalBounds().reduced (margin).getRight();
    if (!_initing && noSpaceForNewBand)
    {
        std::cout << "no space for new band"
                  << " " << getLocalBounds().getWidth() << std::endl;
        return;
    }

    std::cout << "adding band" << std::endl;

    // -- band components--
    std::unique_ptr<BandComponent> newBandComponent = std::make_unique<BandComponent>();
    addAndMakeVisible (*newBandComponent);
    if (isFirstBand)
    {
        newBandComponent->isDraggable = false;
        newBandComponent->minimumLeft = newLeft;
    }
    if (_initing)
    {
        auto b = getLocalBounds().reduced (margin);
        newBandComponent->left = juce::jmap (processorRef.getBand ((int) bandComponents.size()), 0.f, 20000.f, (float) b.getX(), (float) b.getX() + b.getWidth());
    }
    else
        newBandComponent->left = newLeft;
    newBandComponent->bandID = (int) bandComponents.size();

    if (!_initing)
        processorRef.addBand (getFreqFromLeft (newBandComponent->left));
    bandComponents.push_back (std::move (newBandComponent));
    // -- band components--

    // -- band remove buttons --
    auto newBandRemoveButton = std::make_unique<juce::TextButton> ("-");
    if (!isFirstBand)
    {
        addAndMakeVisible (*newBandRemoveButton);
        int newBandIndex = (int) bandComponents.size() - 1;
        newBandRemoveButton->onClick = [&, newBandIndex] {
            removeBand (newBandIndex);
        };
    }
    bandRemoveButtons.push_back (std::move (newBandRemoveButton));
    // -- band remove buttons --

    // -- slider attachments --
    auto lastBandIndex = (int) bandComponents.size() - 1;
    auto parameterIdentifier = processorRef.getParamString (processorRef.Parameter::bandSlider) + juce::String (lastBandIndex);
    auto newSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, parameterIdentifier, bandComponents[lastBandIndex]->slider);
    auto param = processorRef.apvts.getParameter (parameterIdentifier);
    if (!_initing)
        param->setValueNotifyingHost (param->getDefaultValue());
    bandSliderAttachments.push_back (std::move (newSliderAttach));
    // -- slider attachments --

    if (!_initing)
    {
        // resized();
        updateProcessorValues();
    }

    std::cout << "size after new band: " << bandComponents.size() << std::endl;
    // update newBandButton visibility
    newBandButton.setVisible (processorRef.canAddBand());
}

void PluginEditor::removeBand (int bandID)
{
    jassert (bandID < (int) bandComponents.size());

    std::cout << "Removing band " << bandID << std::endl;

    bandSliderAttachments.erase (bandSliderAttachments.begin() + bandID); // MUST RUN BEFORE bandComponents.erase() !!
    bandComponents.erase (bandComponents.begin() + bandID);
    bandRemoveButtons.erase (bandRemoveButtons.begin() + bandID);
    processorRef.removeBand (bandID);

    for (int i = 0; i < (int) bandComponents.size(); ++i)
    {
        // update bandIDs
        bandComponents[i]->bandID = i;

        // update removeButtons onClick
        bandRemoveButtons[i]->onClick = [&, i] {
            removeBand (i);
        };

        // update slider attachments
        if (i >= bandID)
        {
            // // move attachments after the band removed 1 band back

            auto paramID = processorRef.getParamString (processorRef.Parameter::bandSlider) + juce::String (i);
            auto nextParamID = processorRef.getParamString (processorRef.Parameter::bandSlider) + juce::String (i + 1);
            std::cout << nextParamID << std::endl;

            // remove attachment
            bandSliderAttachments[i] = nullptr;

            // create new attachment with updated value
            auto newSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.apvts, paramID, bandComponents[i]->slider);
            processorRef.apvts.getParameter (paramID)->setValueNotifyingHost (processorRef.apvts.getParameter (nextParamID)->getValue());
            bandSliderAttachments[i] = std::move (newSliderAttach);
        }
    }

    // reset last slider value param
    auto lastParamID = processorRef.getParamString (processorRef.Parameter::bandSlider) + juce::String (processorRef.bandNMax - 1);
    processorRef.apvts.getParameter (lastParamID)->setValueNotifyingHost (processorRef.apvts.getParameter (lastParamID)->getDefaultValue());

    // resized();
    updateProcessorValues();

    std::cout << "size after removal: " << bandComponents.size() << std::endl;
    // update newBandButton visibility
    newBandButton.setVisible (processorRef.canAddBand());
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
    if (initing)
    {
        for (int i = 0; i < processorRef.getBandsInUse(); ++i)
        {
            newBand (true);
        }
        initing = false;
    }

    jassert (bandComponents.size() == bandRemoveButtons.size());
    jassert (bandComponents.size() == bandSliderAttachments.size());

    auto b = getLocalBounds().reduced (margin);

    // layout the positions of your child components here
    fftVis->setBounds (b);

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

    inspectButton.setBounds (getLocalBounds().withWidth (100).withHeight (50).withY (0));

    newBandButton.setBounds (getLocalBounds().withWidth (200).withHeight (50).withY (getLocalBounds().getBottom() - 50));
    bypassButton.setBounds (getLocalBounds().withWidth (50).withHeight (50).withY (getLocalBounds().getBottom() - 50));

    spectralSlider.setBounds (b.withHeight (100));

    freqMaxSlider.setBounds (b.withWidth (b.getWidth() / 2).withX (b.getWidth() / 2 + 50).withHeight (50).withY (b.getHeight() + 25));
    skewFactorSlider.setBounds (b.withWidth (b.getWidth() / 2).withHeight (50).withY (b.getHeight() + 25));
}

void PluginEditor::mouseDoubleClick (const juce::MouseEvent& event)
{
    newBand();
}

double inline PluginEditor::getFreqFromLeft (float left)
{
    auto b = getLocalBounds().reduced (margin);

    double value = 0;
    if (juce::approximatelyEqual (b.getX(), b.getX() + b.getWidth()))
    {
        std::cout << "getFreqFromLeft error" << std::endl;
        return value;
    };

    value = juce::jmap ((float) left, (float) b.getX(), (float) b.getX() + b.getWidth(), 0.f, 1.f);
    return value * fftVis->freqMax;
}

void PluginEditor::updateProcessorValues()
{
    for (int i = 0; i < bandComponents.size(); ++i)
    {
        // std::cout << "updating band " << i << " with value " << getFreqFromLeft (bandComponents[i]->left) << std::endl;
        processorRef.updateBand (i, getFreqFromLeft (bandComponents[i]->left));
    }
}

// Call when processor values (without attachments) changed (bands frequency and bands in use)
void PluginEditor::updateEditorValues()
{
    if (!initing)
        triggerAsyncUpdate();
}
void PluginEditor::handleAsyncUpdate()
{
    auto b = getLocalBounds().reduced (margin);
    // // update editor values
    while ((int) bandComponents.size() > processorRef.getBandsInUse())
    {
        removeBand (bandComponents[(int) bandComponents.size() - 1]->bandID);
    }
    while ((int) bandComponents.size() < processorRef.getBandsInUse())
    {
        newBand (true);
    }
    for (int i = 1; i < processorRef.getBandsInUse(); ++i)
    {
        auto value = juce::jmap (processorRef.getBand (i), 0.f, 20000.f, (float) b.getX(), (float) b.getX() + b.getWidth());
        //     auto min = juce::jmap ((int) processorRef.getBand (i - 1) + 50, 0, 20000, b.getX(), b.getX() + b.getWidth());
        //     auto max = -1;
        //     if (i < processorRef.getBandsInUse() - 1)
        //         max = juce::jmap ((int) processorRef.getBand (i + 1) - 50, 0, 20000, b.getX(), b.getX() + b.getWidth());

        //     if (value < min)
        //         value = min;
        //     if (value > max)
        //         value = max;

        bandComponents[i]->left = value;

        // std::cout << "value: " << value << std::endl;
    }

    // // render
    resized();
}