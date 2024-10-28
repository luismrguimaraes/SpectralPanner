#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "FFTProcessor.h"

class BandComponent : public juce::Component
{
public:
    BandComponent();

    void paint (juce::Graphics &g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent & 	event	) override;
    void mouseDrag(const juce::MouseEvent &event)  override;
    void mouseUp (const juce::MouseEvent &event) override;

    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer dragConstrainer;

    int minimumLeft;
private:
    juce::Slider slider{juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow};
    int margin = 0;
    int mouseDownMargin = 20; 

    bool dragging = false;
    int dragStartX;
    int dragStartWidth;
};