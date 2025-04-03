/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RotoVerbAudioProcessorEditor::RotoVerbAudioProcessorEditor (RotoVerbAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    auto addLabel = [this] (Label& label, std::string name, int x, int y, int w, int h)
    {
        label.setText(name, dontSendNotification);
        label.setJustificationType(Justification::centred);
        label.setBounds(x, y, w, h);
        addAndMakeVisible(label);
    };
    
    auto addSlider = [this] (Slider& slider, std::unique_ptr<SliderAttachment>& attachment, std::string name, int x, int y, int w, int h)
    {
        slider.setBounds(x, y, w, h);
        slider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, 100, 15);
        addAndMakeVisible(slider);
        attachment.reset(new SliderAttachment(this->valueTreeState, name, slider));
    };
    
    int xCounter = 0;
    int xIncrement = 120;
    
    addLabel(distanceLabel, "Distance", xCounter * xIncrement, 10, 180, 50);
    addSlider(distanceSlider, distanceAttachment, "distance", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
    
    addLabel(chorusLabel, "Chorus", xCounter * xIncrement, 10, 180, 50);
    addSlider(chorusSlider, chorusAttachment, "chorus", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
    
    addLabel(densityLabel, "Density", xCounter * xIncrement, 10, 180, 50);
    addSlider(densitySlider, densityAttachment, "density", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
    
    addLabel(decayLabel, "Decay", xCounter * xIncrement, 10, 180, 50);
    addSlider(decaySlider, decayAttachment, "decayS", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
    
    addLabel(dampLabel, "Damp", xCounter * xIncrement, 10, 180, 50);
    addSlider(dampSlider, dampAttachment, "damp", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
        
    addLabel(mixLabel, "Mix", xCounter * xIncrement, 10, 180, 50);
    addSlider(mixSlider, mixAttachment, "mix", xCounter * xIncrement, 50, 180, 115);
    
    xCounter++;
    
    matrixMenu.setBounds(50, 210, 180, 30);
    
    const auto matrices = audioProcessor.getMatrices();
    int id = 1;
    for(const auto& matrix : matrices)
        matrixMenu.addItem(matrix.first, id++);
    
    matrixMenu.onChange = [this] {
        int currentId = matrixMenu.getSelectedId();
        int index = matrixMenu.indexOfItemId(currentId);
        String matrixName = matrixMenu.getItemText(index);
        audioProcessor.setMatrix(matrixName.getCharPointer());
    };
    
    matrixMenu.setText(audioProcessor.getCurrentMatrixName());
    addAndMakeVisible(matrixMenu);
    
    holdButton.setButtonText("Hold");
    holdButton.setBounds(300, 210, 50, 30);
    holdButton.setToggleable(true);
    addAndMakeVisible(holdButton);
    holdAttachment.reset(new ButtonAttachment(this->valueTreeState, "hold", holdButton));
    
    setSize (xCounter * xIncrement + 60, 280);
}

RotoVerbAudioProcessorEditor::~RotoVerbAudioProcessorEditor()
{
}

//==============================================================================
void RotoVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void RotoVerbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
