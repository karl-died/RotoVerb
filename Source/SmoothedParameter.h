/*
  ==============================================================================

    SmoothedParameter.h
    Created: 15 Mar 2025 8:41:07pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once

#include <atomic>

template <typename T>
class SmoothedParameter
{
public:
    SmoothedParameter(std::atomic<T>* target)
    {
        targetValue = target;
        value = *target;
        smoothing = 0;
    }
    
    void setSmoothing(float s) { smoothing = s; }
    float getSmoothing() { return smoothing; }
    
    void update()
    {
        value = smoothing * value + (1 - smoothing) * *targetValue;
    }
    
    void setTarget(std::atomic<T>* target) { targetValue = target; }
    
    T getTargetValue() { return *targetValue; }
    T getValue() { return value; }
    
    bool targetReached() { return abs(*targetValue - value) < 1e-8; }
    
private:
    std::atomic<T>* targetValue;
    T value;
    float smoothing;
};
