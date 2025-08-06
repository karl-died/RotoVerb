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
#include "GivensRotation.h"


class MatrixReverb
{
public:
    MatrixReverb(std::atomic<float>* preDelayMs,
            std::atomic<float>* rotation,
            std::atomic<float>* dispersion,
            std::atomic<float>* decayS,
            std::atomic<float>* damp,
            std::atomic<float>* density);
    
    void setSamplerate(float);
    void processBlock(juce::AudioBuffer<float>&);
    
    void setMatrix(const char* name);
    std::map<std::string, const float*>& getMatrices();
    std::string getCurrentMatrixName();
    
    
private:
    static constexpr int numKernels = 12;

    
    void calculateDelays();
    float cubicLagrangeInterpolation(const std::array<float, 4>& y, float x);
    
    SmoothedParameter<float> distance;
    SmoothedParameter<float> rotation;
    SmoothedParameter<float> dispersion;
    SmoothedParameter<float> decayS;
    SmoothedParameter<float> damp;
    SmoothedParameter<float> density;

    const float smoothingMs = 20;
    float smoothingCoeff;
    
    float projectSampleRate = 44100;
    
    const float maxDelayMs = 400;
    const float minD0 = 16;
    const float maxD0 = 100;
    const float maxD1 = 120;
    
    std::mutex matrixChangeMutex;
    std::atomic<bool> matrixChanged;
    std::string matrixName{"Simple"};
    
    std::array<float, numKernels> targetDelays{};
    std::array<float, numKernels> delays{};
    std::array<float, numKernels> lowpassSamples{};
    std::array<std::vector<float>, numKernels> delayBuffer{};
    std::array<std::vector<float>, numKernels> outputBuffer{};
    int delayBufferIndex = 0;
    int delayBufferSize;
    
    const float allpassDelayRatio = (0.1465 * 3.0) / 7;
    std::array<float, numKernels> targetAllpassDelays{0};
    std::array<float, numKernels> allpassDelays{0};
    std::array<std::vector<float>, numKernels> allpassBuffer{};
    int allpassBufferIndex = 0;
    int allpassBufferSize;
    
    Eigen::Matrix<float, numKernels, 1> feedbackSamples;
    std::array<float, numKernels> targetDecays{0};
    Eigen::Matrix<float, numKernels, numKernels> decayMatrix;
    Eigen::Matrix<float, numKernels, numKernels> diffuseMatrix;
    
    float maxRotation = 2 * M_PI / projectSampleRate;
    std::array<float, GivensRotation<numKernels>::numAngles> phases{};
    std::array<float, GivensRotation<numKernels>::numAngles> rotations{};
    GivensRotation<numKernels> rotationMatrix{};

};
