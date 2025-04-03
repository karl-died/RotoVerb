/*
  ==============================================================================

    MatrixReverb.h
    Created: 15 Mar 2025 8:41:20pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once

#include <atomic>
#include <Eigen/Dense>
#include "SmoothedParameter.h"
#include <JuceHeader.h>
#include <algorithm>
#include <mutex>
#include "Data.h"


class MatrixReverb
{
public:
    MatrixReverb(std::atomic<float>* preDelayMs,
            std::atomic<float>* chorus,
            std::atomic<float>* decayS,
            std::atomic<float>* damp,
            std::atomic<float>* density,
            juce::AudioParameterBool* holdParameter);
    
    void setSamplerate(float);
    void processBlock(juce::AudioBuffer<float>&);
    
    void setMatrix(const char* name);
    std::map<std::string, const float*>& getMatrices();
    std::string getCurrentMatrixName();
    
    
private:
    static constexpr int numKernels = 12;

    
    void calculateDelays();
    float cubicLagrangeInterpolation(std::array<float, 4> y, float x);
    
    SmoothedParameter<float> distance;
    SmoothedParameter<float> chorus;
    SmoothedParameter<float> decayS;
    SmoothedParameter<float> damp;
    SmoothedParameter<float> density;
    juce::AudioParameterBool* holdParameter;

    const float smoothingMs = 10;
    float smoothingCoeff;
    
    float projectSampleRate = 44100;
    
    const float maxDelayMs = 400;
    const float minD0 = 16;
    const float maxD0 = 100;
    const float maxD1 = 120;
    
    std::mutex matrixChangeMutex;
    std::atomic<bool> matrixChanged;
    std::string matrixName{"Simple"};
    
    const float envelopeSmoothing = 0.95;
    float envelopeSample = 0;
    const float gateSmoothing = 0.99;
    const float gateThreshold = 0.1;
    float gateSample = 0;
    std::vector<float> gateBuffer;
    int clearSamples = 20;
    int clearCounter = 0;
    std::vector<float> clearBuffer;
    
    std::array<float, numKernels> targetDelays;
    std::array<float, numKernels> delays;
    std::array<float, numKernels> lowpassSamples;
    std::array<std::vector<float>, numKernels> delayBuffer;
    std::array<std::vector<float>, numKernels> outputBuffer;
    int delayBufferIndex = 0;
    int delayBufferSize;
    
    const float allpassDelayRatio = (0.1465 * 3.0) / 7;
    std::array<float, numKernels> targetAllpassDelays{0};
    std::array<float, numKernels> allpassDelays{0};
    std::array<std::vector<float>, numKernels> allpassBuffer;
    int allpassBufferIndex = 0;
    int allpassBufferSize;
    
    Eigen::Matrix<float, numKernels, 1> feedbackSamples;
    std::array<float, numKernels> targetDecays{0};
    Eigen::Matrix<float, numKernels, numKernels> decayMatrix;
    Eigen::Matrix<float, numKernels, numKernels> diffuseMatrix;

    const float modDepth = 100;
    const float modFrequency = 0.5;
    float modIncrement = 0;
    std::array<float, numKernels> modPhases{0};
};
