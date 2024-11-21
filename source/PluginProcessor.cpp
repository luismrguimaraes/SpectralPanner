#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
#if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
    #endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
{
}

float PluginProcessor::getBand (int index)
{
    // std::cout << " arr size " << bandsArr.size() << std::endl;
    // if (index > bandsArr.size() - 1)
    //     return -1;
    // return bandsArr[index];

    auto bandParam = getBandParameter (index);
    return *bandParam;
}

int PluginProcessor::updateBand (int index, double value)
{
    // if (index > bandsArr.size() - 1)
    //     return -1;
    // auto arr = bandsArr.getArray();
    // arr->set (index, value);

    // // update FFTProcessor spectralMultipliers?
    // // ...

    // std::cout << "size " << bandsArr.size() << std::endl;
    // std::cout << "index " << index << " value " << value << std::endl;

    auto bandParam = getBandParameter (index);
    *bandParam = value;
    std::cout << "value received: " << value << std::endl;
    std::cout << "new value: " << bandParam->getCurrentValueAsText() << std::endl;
    std::cout << "bands in use: " << getBandsInUse() << std::endl;

    return 0;
}

void PluginProcessor::addBand (double value)
{
    // bandsArr.append (value);

    jassert (canAddBand());

    auto bandParam = getBandParameter (getBandsInUse());
    *bandParam = value;

    auto bandsInUseParam = getBandsInUseParameter();
    *bandsInUseParam = *bandsInUseParam + 1;
    std::cout << "param value just set: " << *bandsInUseParam << std::endl;
    std::cout << "actual: " << getBandsInUse() << std::endl;
}

bool PluginProcessor::canAddBand()
{
    return getBandsInUse() < bandNMax;
}

int PluginProcessor::removeBand (int index)
{
    // if (index > bandsArr.size() - 1)
    //     return -1;
    // bandsArr.remove (index);

    auto bandsInUseParam = getBandsInUseParameter();
    *bandsInUseParam = *bandsInUseParam - 1;
    return 0;
}

juce::AudioParameterFloat* PluginProcessor::getBandParameter (int bandIndex)
{
    auto param = (juce::AudioParameterFloat*) apvts.getParameter (getParamString (Parameter::band) + juce::String (bandIndex));

    std::cout << param->getParameterID() << std::endl;
    return param;
}

juce::AudioParameterInt* PluginProcessor::getBandsInUseParameter()
{
    return (juce::AudioParameterInt*) apvts.getParameter (getParamString (Parameter::bandsInUse));
}
int PluginProcessor::getBandsInUse()
{
    auto bandsInUseParam = getBandsInUseParameter();

    return *bandsInUseParam;
}

juce::AudioProcessorParameter* PluginProcessor::getBypassParameter() const
{
    return apvts.getParameter (getParamString (Parameter::bypass));
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    setLatencySamples (fft[0].getLatencyInSamples());

    fft[0].reset();
    fft[0].setSampleRate (sampleRate);
    fft[1].reset();
    fft[1].setSampleRate (sampleRate);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (/*layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     &&*/
        layouts.getMainOutputChannelSet()
        != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
    #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
#endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, numSamples);
    }

    bool bypassed = apvts.getRawParameterValue (getParamString (Parameter::bypass))->load();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        fft[channel].processBlock (channelData, numSamples, bypassed);
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    editor = new PluginEditor (*this);
    return editor;
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (getParamString (Parameter::bypass), 1),
        "Bypass",
        false));

    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (getParamString (Parameter::bandsInUse), 1),
        "Bands in use",
        1,
        20,
        1));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::band) + "0", 1),
        "Band 1 frequency",
        0.f,
        20000.f,
        0.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::band) + "1", 1),
        "Band 2 frequency",
        0.f,
        20000.f,
        0.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::band) + "2", 1),
        "Band 3 frequency",
        0.f,
        20000.f,
        0.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::band) + "3", 1),
        "Band 4 frequency",
        0.f,
        20000.f,
        0.f));

    // juce::ValueTree bands = juce::ValueTree ("Bands");
    // layout.add (bands.begin(), bands.end());

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
