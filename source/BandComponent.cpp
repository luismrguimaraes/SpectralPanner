#include "BandComponent.h"

BandComponent::BandComponent()
{
    addAndMakeVisible (slider);
    // slider.setRange (-1, 1);
    slider.setValue (0.0);

    addAndMakeVisible (label);
    label.setInterceptsMouseClicks (false, false);
}

void BandComponent::paint (juce::Graphics& g)
{
    if (isDraggable)
    {
        auto b = getLocalBounds();
        g.setColour (juce::Colours::yellowgreen);
        juce::Rectangle<int> rect (separatorWidth, b.getHeight());
        rect.setX (b.getX());
        g.fillRect (rect);
    }
}

void BandComponent::resized()
{
    auto b = getLocalBounds();

    auto newSliderBounds = b.reduced (0, b.getHeight() / 3);
    newSliderBounds = newSliderBounds.withSizeKeepingCentre (juce::jmin (newSliderBounds.getHeight(), b.getWidth()), newSliderBounds.getHeight());

    if (isDraggable)
    {
        // get space for boundary line
        auto xIncrement = separatorWidth;
        newSliderBounds = newSliderBounds.withX (newSliderBounds.getX() + xIncrement).withWidth (newSliderBounds.getWidth() - xIncrement);

        label.setBounds (b.withX (b.getX() + xIncrement).withHeight (100).withWidth (juce::jmin (100, b.getWidth()) - xIncrement));
    }
    slider.setBounds (newSliderBounds);
    if (newSliderBounds.getWidth() < 90)
    {
        slider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    }
    else
    {
        slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    }
}

void BandComponent::mouseDown (const juce::MouseEvent& event)
{
    // std::cout << event.getMouseDownPosition().toString() << std::endl;

    if (isDraggable)
    {
        // std::cout << "draggin" << std::endl;
        dragging = true;
        dragStartX = getBounds().getX();
        dragStartWidth = getBounds().getWidth();
        // std::cout << dragStartX << std::endl;
    }
}

void BandComponent::mouseUp (const juce::MouseEvent& event)
{
    if (dragging)
        dragging = false;
}

void BandComponent::mouseDrag (const juce::MouseEvent& event)
{
    if (dragging)
    {
        auto increment = event.getScreenX() - event.getMouseDownScreenX();
        auto newWidth = dragStartWidth - increment;
        auto newLeft = dragStartX + increment;
        if (newWidth >= 50 && newLeft > minimumLeft)
            left = newLeft;
        else if (newLeft <= minimumLeft)
        {
            left = minimumLeft;
        }
        else if (newWidth < 50)
        {
            // maximum left: dragStartWidth - increment = 50
            left = dragStartX + dragStartWidth - 50;
        }

        // getParentComponent()->resized();
        editor = (PluginEditor*) getParentComponent();
        editor->updateProcessorValues();
    }
}

void BandComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
    getParentComponent()->mouseDoubleClick (event);
}