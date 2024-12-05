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
    editorCreated.store (false);
}

void PluginProcessor::updateFFTProcessorPanValues()
{
    int firstBandBin;
    int lastBandBin;
    for (int band = 0; band < getBandsInUse(); ++band)
    {
        firstBandBin = binFromFreq (getBand (band), getSampleRate());
        bool bandIsLast = band == getBandsInUse() - 1;
        if (!bandIsLast)
            lastBandBin = binFromFreq (getBand (band + 1), getSampleRate());
        else
            lastBandBin = FFTProcessor::numBins - 1;

        auto sliderValue = getBandSliderValue (band);

        std::cout << "band " << band << " with freq " << getBand (band) << " value " << sliderValue << std::endl;
        std::cout << firstBandBin << " " << lastBandBin << std::endl;

        for (int bin = firstBandBin; bin <= lastBandBin; bin++)
        {
            // fft[0].spectralPanValues.at (bin) = -sliderValue;
            // fft[1].spectralPanValues.at (bin) = sliderValue;
            fftProc.spectralPanValues.at (bin) = sliderValue;
        }
    }
}

void PluginProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    // Handle the parameter value change
    // juce::Logger::writeToLog ("Parameter " + juce::String (parameterIndex) + " changed to " + juce::String (newValue));

    // update PluginEditor
    if (editor != nullptr)
    {
        if (editorCreated.load())
        {
            PluginEditor* ed = (PluginEditor*) editor;
            if (ed != nullptr)
            {
                std::cout << "editorCreated: " << editorCreated << std::endl;
                ed->updateEditorValues();
            }
            else
            {
                std::cout << "editor not nullptr, but is after cast" << std::endl;
            }
        }
        else
        {
            std::cout << "----------------------editor not created" << std::endl;
        }
    }
    else
    {
        std::cout << "Editor is nullptr" << std::endl;
    }

    updateFFTProcessorPanValues();
    fftProc.setPanLaw (apvts.getRawParameterValue (getParamString (Parameter::panLaw))->load());
}

void PluginProcessor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
    // Handle the parameter gesture change
    juce::Logger::writeToLog ("Parameter " + juce::String (parameterIndex) + (gestureIsStarting ? " started" : " stopped") + " being adjusted.");
}

// Get band start frequency
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

    // // update FFTProcessor spectralPanValues?
    // // ...

    // std::cout << "size " << bandsArr.size() << std::endl;
    // std::cout << "index " << index << " value " << value << std::endl;

    auto bandParam = getBandParameter (index);
    *bandParam = value;
    // std::cout << "value received: " << value << std::endl;
    // std::cout << "new value: " << bandParam->getCurrentValueAsText() << std::endl;
    // std::cout << "bands in use: " << getBandsInUse() << std::endl;

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
    // std::cout << "param value just set: " << *bandsInUseParam << std::endl;
    // std::cout << "actual: " << getBandsInUse() << std::endl;
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

float PluginProcessor::getBandSliderValue (int sliderIndex)
{
    auto paramID = getParamString (Parameter::bandSlider) + juce::String (sliderIndex);
    auto param = (juce::AudioParameterFloat*) apvts.getParameter (paramID);

    return *param;
}

juce::AudioParameterFloat* PluginProcessor::getBandParameter (int bandIndex)
{
    auto param = (juce::AudioParameterFloat*) apvts.getParameter (getParamString (Parameter::band) + juce::String (bandIndex));

    // std::cout << param->getParameterID() << std::endl;
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

    setLatencySamples (fftProc.getLatencyInSamples());

    fftProc.reset();
    fftProc.setSampleRate (sampleRate);

    updateFFTProcessorPanValues();
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

    auto* channelDataL = buffer.getWritePointer (0);
    auto* channelDataR = buffer.getWritePointer (1);
    fftProc.processBlock (channelDataL, channelDataR, numSamples, bypassed);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    std::cout << "creating editor" << std::endl;

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

    auto paramInt = std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (getParamString (Parameter::bandsInUse), 1),
        "Bands in use",
        1,
        bandNMax,
        1);
    paramInt->addListener (this);
    layout.add (std::move (paramInt));

    // Band
    auto paramFloat = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::band) + "0", 1),
        "Band 1 start frequency",
        0.f,
        20000.f,
        0.f);
    paramFloat->addListener (this);
    layout.add (std::move (paramFloat));

    // Slider
    juce::NormalisableRange<float> sliderLogRange (-1.0f, 1.0f, 0.0001f, 0.5f, true);
    paramFloat = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (getParamString (Parameter::bandSlider) + "0", 1),
        "Band 1 Value",
        sliderLogRange,
        0.f);
    paramFloat->addListener (this);
    layout.add (std::move (paramFloat));

    int bandsParamsRemaining = bandNMax - 1;
    for (int i = 0; i < bandsParamsRemaining; i++)
    {
        // Band
        paramFloat = std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID (getParamString (Parameter::band) + juce::String (i + 1), 1),
            juce::String ("Band " + juce::String (i + 2) + " start frequency"),
            0.f,
            20000.f,
            0.f);
        paramFloat->addListener (this);
        layout.add (std::move (paramFloat));

        // Slider
        paramFloat = std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID (getParamString (Parameter::bandSlider) + juce::String (i + 1), 1),
            juce::String ("Band " + juce::String (i + 2) + " Value"),
            sliderLogRange,
            0.f);
        paramFloat->addListener (this);
        layout.add (std::move (paramFloat));
    }

    // Pan Law
    paramInt = std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (getParamString (Parameter::panLaw), 1),
        "Pan Law",
        0.f,
        6.f,
        0.f);
    paramInt->addListener (this);
    layout.add (std::move (paramInt));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
