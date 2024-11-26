#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginEditor.h"
class PluginEditor;

class BandComponent : public juce::Component
{
public:
    BandComponent();

    void paint (juce::Graphics& g) override;
    void resized() override;

    float left;
    float minimumLeft;
    bool isDraggable = true;

    int bandID;
    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };

private:
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;


    bool dragging = false;
    int dragStartX;
    int dragStartWidth;

    PluginEditor* editor;
};