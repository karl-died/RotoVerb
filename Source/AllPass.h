/*
  ==============================================================================

    AllPass.h
    Created: 28 Mar 2025 6:40:46pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once
#include <array>
#include <vector>

class AllPass
{
public:
    AllPass();
    AllPass(float delayMs, float feedbackPercent, int sampleRate);
    
    void setDelayMs(float);
    void setFeedbackPercent(float);
    void setSampleRate(int);
    
    float process(float);
    void processBlock(float*, int);
private:
    
    std::vector<float> backBuffer;
    int backBufferIndex = 0;
    int backBufferSize = 0;
    
    int sampleRate;
    const float parameterSmoothingMs = 10;
    float smoothingCoeff;
    
    const float minDelayMs = 0.1;
    const float maxDelayMs = 20;
    float targetDelayMs;
    float currentDelayMs;
    
    float targetFeedback;
    float currentFeedback;
    
    std::array<float, 4> interpolationSamples{0.0};
};
