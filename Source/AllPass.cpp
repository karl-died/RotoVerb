/*
  ==============================================================================

    AllPass.cpp
    Created: 9 Feb 2025 9:00:32am
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#include "AllPass.h"
#include "Interpolation.h"

AllPass::AllPass()
{
    AllPass(5, 0.1, 44100);
}

AllPass::AllPass(float delayMs, float feedbackPercent, int sampleRate)
{
    if(delayMs > maxDelayMs)
    {
        targetDelayMs = maxDelayMs;
        currentDelayMs = maxDelayMs;
    }
    else if(delayMs < 0)
    {
        targetDelayMs = 0;
        currentDelayMs = 0;
    }
    else
    {
        targetDelayMs = delayMs;
        currentDelayMs = delayMs;
    }
    
    if(feedbackPercent > 100)
    {
        targetFeedback = 100;
        currentFeedback = 100;
    }
    else if(feedbackPercent < 0)
    {
        targetFeedback = 0;
        currentFeedback = 0;
    }
    else
    {
        targetFeedback = feedbackPercent;
        currentFeedback = feedbackPercent;
    }
    
    this->sampleRate = sampleRate;
    smoothingCoeff = pow(0.1, 1000.0 / (parameterSmoothingMs * sampleRate));
    
    int backBufferSize = maxDelayMs * ((float) sampleRate / 1000) + 1;
    backBuffer = std::vector<float> (backBufferSize, 0);
}

void AllPass::setDelayMs(float delayMs)
{
    targetDelayMs = std::max(delayMs, minDelayMs);
}

void AllPass::setFeedbackPercent(float feedbackPercent)
{
    targetFeedback = feedbackPercent;
}

void AllPass::setSampleRate(int sampleRate)
{
    this->sampleRate = sampleRate;
    smoothingCoeff = pow(0.1, 1000.0 / (parameterSmoothingMs * sampleRate));
    
    backBufferSize = maxDelayMs * ((float) sampleRate / 1000) + 1;
    backBuffer = std::vector<float> (backBufferSize, 0);
    backBufferIndex = 0;
}

float AllPass::process(float sample)
{
    backBufferIndex = (backBufferIndex + 1) % backBufferSize;
    
    currentDelayMs = smoothingCoeff * currentDelayMs + (1.0 - smoothingCoeff) * targetDelayMs;
    currentFeedback = smoothingCoeff * currentFeedback + (1.0 - smoothingCoeff) * targetFeedback;
     
    
    float delaySamples = currentDelayMs * ((float) sampleRate / 1000);
    float delaySampleOffset = delaySamples - (int) delaySamples;
    
    for(int i = 0; i < 4; i++)
        interpolationSamples[i] = backBuffer[(backBufferSize + backBufferIndex - (int) delaySamples - 2 + i) % backBufferSize];
    
    
    float delayedSample = Interpolation::cubicLagrange(interpolationSamples, 1.0f - delaySampleOffset);
    float feedback = currentFeedback / 100.0f;
    
    backBuffer[backBufferIndex] = sample + feedback * delayedSample;
    return ((1.0 / feedback) - feedback) * backBuffer[backBufferIndex] - (1.0f / feedback) * sample;
}


void AllPass::processBlock(float* buffer, int numSamples)
{
    for(int i = 0; i < numSamples; i++)
        buffer[i] = process(buffer[i]);
}

