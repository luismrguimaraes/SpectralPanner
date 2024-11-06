#include "BandComponent.h"

BandComponent::BandComponent()
    : dragger(), dragConstrainer()
{
    addAndMakeVisible (slider);
    slider.setRange (-1, 1, 0.1);
    slider.setValue (0.0);
}

void BandComponent::paint (juce::Graphics& g)
{
    if (isDraggable)
    {
        auto b = getLocalBounds();
        g.setColour (juce::Colours::yellowgreen);
        juce::Rectangle<int> rect (5, b.getHeight());
        rect.setX (b.getX());
        g.fillRect (rect);
    }
}

void BandComponent::resized()
{
    auto b = getLocalBounds();

    auto newBounds = b.reduced (0, b.getHeight() / 3);
    newBounds = newBounds.withSizeKeepingCentre (juce::jmin (newBounds.getHeight(), b.getWidth()), newBounds.getHeight());

    if (isDraggable)
    {
        // get space for boundary line
        auto xIncrement = 5;
        newBounds = newBounds.withX (newBounds.getX() + xIncrement).withWidth (newBounds.getWidth() - xIncrement);
    }
    slider.setBounds (newBounds);
}

void BandComponent::mouseDown (const juce::MouseEvent& event)
{
    std::cout << event.getMouseDownPosition().toString() << std::endl;

    if (isDraggable)
    {
        std::cout << "draggin" << std::endl;
        dragging = true;
        dragStartX = getBounds().getX();
        dragStartWidth = getBounds().getWidth();
        std::cout << dragStartX << std::endl;
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

        getParentComponent()->resized();
    }
}

void BandComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
    getParentComponent()->mouseDoubleClick (event);
}