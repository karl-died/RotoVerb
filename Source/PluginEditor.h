/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

using namespace juce;
//==============================================================================
/**
*/
class RotoVerbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RotoVerbAudioProcessorEditor (RotoVerbAudioProcessor&,
                                  AudioProcessorValueTreeState&);
    ~RotoVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RotoVerbAudioProcessor& audioProcessor;

    AudioProcessorValueTreeState& valueTreeState;

    Label distanceLabel;
    Slider distanceSlider;
    std::unique_ptr<SliderAttachment> distanceAttachment;
    
    Label chorusLabel;
    Slider chorusSlider;
    std::unique_ptr<SliderAttachment> chorusAttachment;
    
    Label decayLabel;
    Slider decaySlider;
    std::unique_ptr<SliderAttachment> decayAttachment;
    
    Label densityLabel;
    Slider densitySlider;
    std::unique_ptr<SliderAttachment> densityAttachment;
    
    Label dampLabel;
    Slider dampSlider;
    std::unique_ptr<SliderAttachment> dampAttachment;

    Label mixLabel;
    Slider mixSlider;
    std::unique_ptr<SliderAttachment> mixAttachment;
    
    ComboBox matrixMenu;
    
    TextButton holdButton;
    std::unique_ptr<ButtonAttachment> holdAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotoVerbAudioProcessorEditor)
};
