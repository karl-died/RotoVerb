/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RotoVerbAudioProcessor::RotoVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
parameters(*this, nullptr, Identifier("RotoVerb"),
           {
    std::make_unique<AudioParameterFloat> (ParameterID {"distance", 1 },
                                           "Distance",
                                           NormalisableRange<float>(0.001f, 1.0f, 0.001f, 1.0f),
                                           0.15f),
    std::make_unique<AudioParameterFloat> (ParameterID {"rotation", 1 },
                                           "Rotation",
                                           NormalisableRange<float>(0.0f, 1.0f, 0.00001f, 0.3333f),
                                           0.0f),
    std::make_unique<AudioParameterFloat> (ParameterID {"dispersion", 1 },
                                           "Dispersion",
                                           NormalisableRange<float>(0.0f, 1.0f, 0.0001f, 1.0f),
                                           0.0f),
    std::make_unique<AudioParameterFloat> (ParameterID {"decayS", 1},
                                           "Decay",
                                           NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3333333f),
                                           1.0f),
    std::make_unique<AudioParameterFloat> (ParameterID {"density", 1},
                                           "Density",
                                           NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f),
                                           0.4f),
    std::make_unique<AudioParameterFloat> (ParameterID {"damp", 1},
                                           "Damp",
                                           NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f),
                                           0.5f),
    std::make_unique<AudioParameterInt> (ParameterID {"matrix", 1},
                                            "Matrix",
                                            0,
                                            100,
                                            0),
    std::make_unique<AudioParameterFloat> (ParameterID {"mix", 1},
                                           "Mix",
                                           NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
                                           0.5f)
        })
#endif
{
    reverb = new MatrixReverb(parameters.getRawParameterValue("distance"),
                            parameters.getRawParameterValue("rotation"),
                            parameters.getRawParameterValue("dispersion"),
                            parameters.getRawParameterValue("decayS"),
                            parameters.getRawParameterValue("damp"),
                            parameters.getRawParameterValue("density"));
  
    distanceParam = parameters.getRawParameterValue("distance");
    densityParam = parameters.getRawParameterValue("density");

    schwamm = new Schwamm(&schwammDelayParam,
                          parameters.getRawParameterValue("density"));
  
    mix = new SmoothedParameter<float>(parameters.getRawParameterValue("mix"));
  
    for(const auto& matrix : Data::matrices)
        matrixNames.add(matrix.first);
}

RotoVerbAudioProcessor::~RotoVerbAudioProcessor()
{
    delete reverb;
    delete mix;
    delete schwamm;
}

//==============================================================================
const juce::String RotoVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RotoVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RotoVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RotoVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RotoVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RotoVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RotoVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RotoVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RotoVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void RotoVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RotoVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb->setSamplerate(sampleRate);
    schwamm->setSampleRate(sampleRate);
}

void RotoVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RotoVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RotoVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    reverbBuffer.makeCopyOf(buffer);
    schwammDelayParam = *densityParam * *distanceParam;
    
    //buffer.applyGainRamp(0, buffer.getNumSamples(), 1.0 - mix->getValue(), 1.0 - mix->getTargetValue());
    
    reverb->processBlock(reverbBuffer);
    schwamm->process(reverbBuffer);
    
    float m0 = mix->getValue();
    float m1 = mix->getTargetValue();
    float dm = (m1 - m0) / buffer.getNumSamples();
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* reverbChannelData = reverbBuffer.getReadPointer(channel);
        auto* channelData = buffer.getWritePointer(channel);
        for(int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            float m = m0 + dm * sample;
            float dry = 1 - (m * m);
            float wet = 1 - ((m - 1) * (m - 1));
            channelData[sample] = dry * channelData[sample] + wet * reverbChannelData[sample];
        }
    }
    
    mix->update();
    
}
//==============================================================================
bool RotoVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RotoVerbAudioProcessor::createEditor()
{
    return new RotoVerbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RotoVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RotoVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (ValueTree::fromXml (*xmlState));
    
    RangedAudioParameter* matrixParam = parameters.getParameter("matrix");
    AudioParameterInt* matrixValue = (AudioParameterInt*) matrixParam;
    int matrixIndex = *matrixValue;
    String matrixName = matrixNames[matrixIndex];
    
    setMatrix(matrixName.getCharPointer());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RotoVerbAudioProcessor();
}


void RotoVerbAudioProcessor::setMatrix(const char* name)
{
    int index = matrixNames.indexOf(name);
    RangedAudioParameter* matrixParam = parameters.getParameter("matrix");
    AudioParameterInt* matrixValue = (AudioParameterInt*) matrixParam;
    *matrixValue = index;
    
    reverb->setMatrix(name);
}

std::map<std::string, const float*>& RotoVerbAudioProcessor::getMatrices()
{
    return reverb->getMatrices();
}

std::string RotoVerbAudioProcessor::getCurrentMatrixName()
{
    return reverb->getCurrentMatrixName();
}
