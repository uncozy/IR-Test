#include "PluginProcessor.h"
#include "PluginEditor.h"

IRLoaderAudioProcessorEditor::IRLoaderAudioProcessorEditor (IRLoaderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
    
    // Load button
    loadButton.setButtonText("Load IR");
    loadButton.onClick = [this]() {
        juce::FileChooser chooser("Select an impulse response file...",
                                 juce::File{},
                                 "*.wav; *.aif; *.aiff");
        
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            audioProcessor.loadImpulseResponse(file);
            currentFileName = file.getFileName();
            repaint();
        }
    };
    addAndMakeVisible(loadButton);
    
    // File label
    fileLabel.setText(currentFileName, juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileLabel);
    
    // Mix slider
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(mixSlider);
    
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);
    
    // Gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gainSlider.setRange(-24.0, 24.0, 0.1);
    gainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(gainSlider);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    
    // Attach sliders to parameters
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "mix", mixSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "gain", gainSlider);
}

IRLoaderAudioProcessorEditor::~IRLoaderAudioProcessorEditor()
{
}

void IRLoaderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    if (isDragOver)
    {
        g.setColour(juce::Colours::lightblue.withAlpha(0.3f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::lightblue);
        g.drawRect(getLocalBounds(), 2);
    }
    
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("IR Loader", getLocalBounds().removeFromTop(40),
                      juce::Justification::centred, 1);
}

void IRLoaderAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area
    bounds.removeFromTop(40);
    
    // Load button and file label
    auto topSection = bounds.removeFromTop(80);
    loadButton.setBounds(topSection.removeFromTop(30).reduced(20, 5));
    fileLabel.setBounds(topSection.reduced(10, 5));
    
    // Controls section
    auto controlsArea = bounds.reduced(20, 10);
    auto sliderWidth = controlsArea.getWidth() / 2 - 10;
    
    // Mix control
    auto mixArea = controlsArea.removeFromLeft(sliderWidth);
    mixLabel.setBounds(mixArea.removeFromTop(20));
    mixSlider.setBounds(mixArea);
    
    controlsArea.removeFromLeft(20); // spacing
    
    // Gain control
    auto gainArea = controlsArea;
    gainLabel.setBounds(gainArea.removeFromTop(20));
    gainSlider.setBounds(gainArea);
}

bool IRLoaderAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".wav") ||
            file.endsWithIgnoreCase(".aif") ||
            file.endsWithIgnoreCase(".aiff"))
            return true;
    }
    return false;
}

void IRLoaderAudioProcessorEditor::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    isDragOver = true;
    repaint();
}

void IRLoaderAudioProcessorEditor::fileDragExit(const juce::StringArray& files)
{
    isDragOver = false;
    repaint();
}

void IRLoaderAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    isDragOver = false;
    
    if (files.size() > 0)
    {
        juce::File file(files[0]);
        audioProcessor.loadImpulseResponse(file);
        currentFileName = file.getFileName();
        fileLabel.setText(currentFileName, juce::dontSendNotification);
    }
    
    repaint();
}
