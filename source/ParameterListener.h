#include "PluginEditor.h"

class MyParameterListener : public juce::AudioProcessorParameter::Listener
{
public:
    // Constructor
    MyParameterListener() {}

    // Destructor
    ~MyParameterListener() {}

    // Called when a parameter's value changes
    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        // Handle the parameter value change
        juce::Logger::writeToLog ("Parameter " + juce::String(parameterIndex) + " changed to " + juce::String(newValue));
    }

    // Called when a parameter's gesture changes (e.g., a user starts or stops adjusting the parameter)
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override
    {
        // Handle the parameter gesture change
        juce::Logger::writeToLog ("Parameter " + juce::String(parameterIndex) + (gestureIsStarting ? " started" : " stopped") + " being adjusted.");
    }
};
