/*
  ==============================================================================

    Schwamm.cpp
    Created: 13 Feb 2025 11:24:06am
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#include "Schwamm.h"

Schwamm::Schwamm(std::atomic<float>* spreadParameter, std::atomic<float>* densityParameter) : spreadParameter(spreadParameter), densityParameter(densityParameter)
{
    for(int stage = 0; stage < numStages; stage++)
        delayScales[stage] = 1.0 / sqrt(numStages - stage);


    for(int channel = 0; channel < 2; channel++)
        for(int stage = 0; stage < numStages; stage++)
            allPassArray[channel][stage] = new AllPass(*spreadParameter * delayScales[stage],
                                              *densityParameter,
                                              44100);
}

void Schwamm::setSampleRate(int sampleRate)
{
    for(int stage = 0; stage < numStages; stage++)
        delayScales[stage] = 1.0 / sqrt(numStages - stage);
    
    
    for(int channel = 0; channel < 2; channel++)
        for(int stage = 0; stage < numStages; stage++)
            allPassArray[channel][stage]->setSampleRate(sampleRate);
}

void Schwamm::process(AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    for(int channel = 0; channel < 2; channel++)
    {
        for(int stage = 0; stage < numStages; stage++)
        {
            allPassArray[channel][stage]->setDelayMs(minSpread + (maxSpread - minSpread) * *spreadParameter * delayScales[stage]);
            allPassArray[channel][stage]->setFeedbackPercent(minDensity + *densityParameter * maxDensity);
        }
    }
    
    for(int channel = 0; channel < 2; channel++)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            for(int stage = 0; stage < numStages; stage++)
                channelData[sample] = allPassArray[channel][stage]->process(channelData[sample]);
        }
    }
}
