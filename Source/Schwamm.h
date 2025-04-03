/*
  ==============================================================================

    Schwamm.h
    Created: 13 Feb 2025 11:23:59am
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once
#include <atomic>
#include <array>
#include <vector>
#include "AllPass.h"
#include <JuceHeader.h>

using namespace juce;

class Schwamm
{
public:
    Schwamm(std::atomic<float>* spreadParameter,
            std::atomic<float>* densityParameter);
    
    void process(AudioBuffer<float>& buffer);
    void setSampleRate(int sampleRate);
    
private:
    std::atomic<float>* spreadParameter;
    std::atomic<float>* densityParameter;
    const float minDensity = 0.01;
    const float maxDensity = 50;
    const float minSpread = 4.0;
    const float maxSpread = 20.0;
    
    static const int numStages = 3;
    double delayScales[numStages];
    std::array<std::array<AllPass*, numStages>, 2> allPassArray;
};
