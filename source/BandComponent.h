#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class BandComponent : public juce::Component
{
public:
    BandComponent();

    void paint (juce::Graphics& g) override;
    void resized() override;

    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer dragConstrainer;

    int left;
    int minimumLeft;
    bool isDraggable = true;

private:
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;

    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };

    bool dragging = false;
    int dragStartX;
    int dragStartWidth;
};