#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class IRLoaderAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::FileDragAndDropTarget
{
public:
    IRLoaderAudioProcessorEditor (IRLoaderAudioProcessor&);
    ~IRLoaderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    IRLoaderAudioProcessor& audioProcessor;
    
    juce::TextButton loadButton;
    juce::Label fileLabel;
    juce::Slider mixSlider;
    juce::Slider gainSlider;
    juce::Label mixLabel;
    juce::Label gainLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    
    bool isDragOver = false;
    juce::String currentFileName = "No IR loaded";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRLoaderAudioProcessorEditor)
};
