/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MatrixReverb.h"
#include "Schwamm.h"

using namespace juce;
//==============================================================================
/**
*/
class RotoVerbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RotoVerbAudioProcessor();
    ~RotoVerbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setMatrix(const char* name);
    std::map<std::string, const float*>& getMatrices();
    std::string getCurrentMatrixName();
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotoVerbAudioProcessor)
    
    SmoothedParameter<float>* mix;
    std::atomic<float>* densityParam;
    std::atomic<float>* distanceParam;
    std::atomic<float> schwammDelayParam{0};
    
    StringArray matrixNames{};
    
    AudioProcessorValueTreeState parameters;
    AudioBuffer<float> reverbBuffer;
    MatrixReverb* reverb;
    Schwamm* schwamm;
};
