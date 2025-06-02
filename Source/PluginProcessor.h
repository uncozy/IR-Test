#pragma once

#include <JuceHeader.h>

class IRLoaderAudioProcessor : public juce::AudioProcessor
{
public:
    IRLoaderAudioProcessor();
    ~IRLoaderAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Custom methods
    void loadImpulseResponse(const juce::File& file);
    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }

private:
    juce::AudioProcessorValueTreeState valueTreeState;
    
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> impulseBuffer;
    
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* gainParameter = nullptr;
    
    bool impulseLoaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRLoaderAudioProcessor)
};
