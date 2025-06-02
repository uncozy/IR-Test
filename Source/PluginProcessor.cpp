#include "PluginProcessor.h"
#include "PluginEditor.h"

IRLoaderAudioProcessor::IRLoaderAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState(*this, nullptr, "Parameters",
                     {
                         std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f),
                         std::make_unique<juce::AudioParameterFloat>("gain", "Gain", -24.0f, 24.0f, 0.0f)
                     })
{
    mixParameter = valueTreeState.getRawParameterValue("mix");
    gainParameter = valueTreeState.getRawParameterValue("gain");
}

IRLoaderAudioProcessor::~IRLoaderAudioProcessor()
{
}

const juce::String IRLoaderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IRLoaderAudioProcessor::acceptsMidi() const
{
    return false;
}

bool IRLoaderAudioProcessor::producesMidi() const
{
    return false;
}

bool IRLoaderAudioProcessor::isMidiEffect() const
{
    return false;
}

double IRLoaderAudioProcessor::getTailLengthSeconds() const
{
    return impulseLoaded ? impulseBuffer.getNumSamples() / getSampleRate() : 0.0;
}

int IRLoaderAudioProcessor::getNumPrograms()
{
    return 1;
}

int IRLoaderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IRLoaderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String IRLoaderAudioProcessor::getProgramName (int index)
{
    return {};
}

void IRLoaderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void IRLoaderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    convolution.prepare(spec);
}

void IRLoaderAudioProcessor::releaseResources()
{
    convolution.reset();
}

bool IRLoaderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void IRLoaderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (!impulseLoaded)
        return;

    // Create a copy of the dry signal
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Process with convolution
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);

    // Apply gain
    float gainValue = juce::Decibels::decibelsToGain(gainParameter->load());
    buffer.applyGain(gainValue);

    // Mix dry and wet signals
    float mixValue = mixParameter->load();
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* wetData = buffer.getWritePointer(channel);
        auto* dryData = dryBuffer.getReadPointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            wetData[sample] = dryData[sample] * (1.0f - mixValue) + wetData[sample] * mixValue;
        }
    }
}

bool IRLoaderAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* IRLoaderAudioProcessor::createEditor()
{
    return new IRLoaderAudioProcessorEditor (*this);
}

void IRLoaderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void IRLoaderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (valueTreeState.state.getType()))
            valueTreeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void IRLoaderAudioProcessor::loadImpulseResponse(const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader != nullptr)
    {
        impulseBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&impulseBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        convolution.loadImpulseResponse(std::move(impulseBuffer),
                                       reader->sampleRate,
                                       juce::dsp::Convolution::Stereo::yes,
                                       juce::dsp::Convolution::Trim::yes,
                                       juce::dsp::Convolution::Normalise::yes);
        
        impulseLoaded = true;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IRLoaderAudioProcessor();
}
