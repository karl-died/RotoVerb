/*
  ==============================================================================

    MatrixReverb.cpp
    Created: 15 Mar 2025 8:41:26pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#include "MatrixReverb.h"
#include "Data.h"
#include "Interpolation.h"

MatrixReverb::MatrixReverb(std::atomic<float>* distance,
                           std::atomic<float>* rotation,
                           std::atomic<float>* dispersion,
                           std::atomic<float>* decayS,
                           std::atomic<float>* damp,
                           std::atomic<float>* density) :
                distance(distance),
                rotation(rotation),
                dispersion(dispersion),
                decayS(decayS),
                damp(damp),
                density(density)
{
    delays = std::array<float, numKernels>{0};
    lowpassSamples = std::array<float, numKernels>{0};
    
    //delayBuffer = std::array<std::vector<float>, numKernels>{std::vector<float>{0}};
    //outputBuffer = std::array<std::vector<float>, numKernels>{std::vector<float>{0}};
    delayBuffer = std::vector<std::array<float, numKernels>>{std::array<float, numKernels>{0}};
    outputBuffer = std::vector<std::array<float, numKernels>>{std::array<float, numKernels>{0}};
    
    allpassBuffer = std::array<std::vector<float>, numKernels>{std::vector<float>{0}};
    
    feedbackSamples = Eigen::MatrixXf::Zero(numKernels, 1);
    decayMatrix = Eigen::MatrixXf::Zero(numKernels, numKernels);
    diffuseMatrix = Eigen::MatrixXf::Zero(numKernels, numKernels);
    
    const float* initialMatrix = Data::matrices[matrixName];
    
    for(int i = 0; i < numKernels; i++)
        for(int j = 0; j < numKernels; j++)
            diffuseMatrix(i, j) = initialMatrix[i + j * numKernels];
}
                

void MatrixReverb::setSamplerate(float sampleRate, int maxNumSamples)
{
    projectSampleRate = sampleRate;
    
    delayBufferSize   = std::ceil(maxDelayMs * projectSampleRate / 1000);
    allpassBufferSize = std::ceil(delayBufferSize * allpassDelayRatio);
    outputBufferSize = maxNumSamples;
    
    lowpassSamples = std::array<float, numKernels>{0};
    
    
    for(int i = 0; i < numKernels; i++)
    {
        //delayBuffer[i]   = std::vector<float>(delayBufferSize, 0);
        //outputBuffer[i]  = std::vector<float>(delayBufferSize, 0);
        allpassBuffer[i] = std::vector<float>(allpassBufferSize, 0);
    }
    
    delayBuffer = std::vector<std::array<float, numKernels>>(delayBufferSize, std::array<float, numKernels>{0});
    outputBuffer = std::vector<std::array<float, numKernels>>(outputBufferSize, std::array<float, numKernels>{0});

    smoothingCoeff = pow(0.1, 1000.0 / (smoothingMs * projectSampleRate));
    
    maxRotation = 20 * M_PI / projectSampleRate;
    
    rotation.setSmoothing(smoothingCoeff);
    decayS.setSmoothing(smoothingCoeff);
    damp.setSmoothing(smoothingCoeff);
    density.setSmoothing(smoothingCoeff);
}

void MatrixReverb::setMatrix(const char* name)
{
    std::lock_guard<std::mutex> lock(matrixChangeMutex);
    matrixName = name;
    matrixChanged = true;
}

std::map<std::string, const float*>& MatrixReverb::getMatrices()
{
    return Data::matrices;
}

std::string MatrixReverb::getCurrentMatrixName()
{
    std::lock_guard<std::mutex> lock(matrixChangeMutex);
    return matrixName;
}


void MatrixReverb::calculateDelays()
{
    float d0 = minD0 + (maxD0 - minD0) * distance.getValue();
    
    //float delayScale = pow((d1 + d0) / d0, 1.0 / numKernels);
    float delayScale = pow(3.0, 1.0 / numKernels);//pow(2.0 + 1.0 * size.getValue(), 1.0 / numKernels);
    
    targetDelays[0] = d0 * projectSampleRate / 1000;
    for(int i = 1; i < numKernels; i++)
        targetDelays[i] = targetDelays[i - 1] * delayScale; 
    
    float decaySamples = decayS.getValue() * projectSampleRate;
    for(int i = 0; i < numKernels; i++)
        targetDecays[i] = pow(0.001, targetDelays[i] / decaySamples);
    
    for(int i = 0; i < numKernels; i++)
        targetAllpassDelays[i] = allpassDelayRatio * targetDelays[i];
}

void MatrixReverb::processBlock(juce::AudioBuffer<float>& buffer)
{
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    
    std::vector<float> inputBuffer(numSamples, 0);
    
    if(matrixChanged)
    {
        std::lock_guard<std::mutex> lock(matrixChangeMutex);
        
        const float* newMatrix = Data::matrices[matrixName];
        diffuseMatrix = Eigen::MatrixXf::Zero(numKernels, numKernels);
        
        if(newMatrix != nullptr)
            for(int i = 0; i < numKernels; i++)
                for(int j = 0; j < numKernels; j++)
                    diffuseMatrix(i, j) = newMatrix[i + j * numKernels];
        
        matrixChanged = false;
    }
    
    calculateDelays();
    
    for(int sample = 0; sample < numSamples; sample++)
    {
        for(int buffer = 0; buffer < numKernels; buffer++)
        {
            int writeIndex = (delayBufferIndex + sample) % delayBufferSize;
            delayBuffer[writeIndex][buffer] = 0;
        }
    }
    
    for(int channel = 0; channel < numChannels; channel++)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for(int sample = 0; sample < numSamples; sample++)
        {
            inputBuffer[sample] += channelData[sample];
        }
    }
    
    for(int buffer = 0; buffer < numKernels; buffer++)
    {
        float d0 = delays[buffer];
        for(int sample = 0; sample < numSamples; sample++)
        {
            d0 = smoothingCoeff * d0 + (1 - smoothingCoeff) * targetDelays[buffer];
            int delaySamples = 0.37293 * d0;
            int dWriteIndex = (delayBufferSize + delayBufferIndex + sample - delaySamples) % delayBufferSize;
            int writeIndex = (delayBufferIndex + sample) % delayBufferSize;
            //delayBuffer[dWriteIndex][buffer] += inputBuffer[sample] / (2 * numKernels);
            delayBuffer[writeIndex][buffer] -= inputBuffer[sample] / (2 * numKernels);
        }
    }
    
    for(int sample = 0; sample < numSamples; sample++)
    {
        
        for(int i = 0; i < numKernels; i++)
            decays[i] = smoothingCoeff * decays[i] + (1 - smoothingCoeff) * targetDecays[i];
        
        float lowPassCoeff = (5000 * damp.getValue() / projectSampleRate);
        
        for(int i = 0; i < numKernels; i++)
        {
            allpassDelays[i] = smoothingCoeff * allpassDelays[i] + (1 - smoothingCoeff) * targetAllpassDelays[i];
            delays[i] = smoothingCoeff * delays[i] + (1 - smoothingCoeff) * targetDelays[i];
            
            float delay = delays[i];
            
            int bufferIndex = (delayBufferSize + delayBufferIndex - (int) delay) % delayBufferSize;
            
            std::array<float, 8> interpolationSamples{0};
            
            for(int j = 0; j < 8; j++)
            {
                int index = (delayBufferSize + bufferIndex - 4 + j) % delayBufferSize;
                interpolationSamples[j] = delayBuffer[index][i];
            }
            
            float modulatedSample = Interpolation::lagrange8(interpolationSamples, 1.0 - (delay - floor(delay)));
            
            float f = (maxDampF - minDampF) * (1 - damp.getValue()) + minDampF;
            float l = exp(-2.0 * M_PI * f / projectSampleRate);
            lowpassSamples[i] = (1 - l) * modulatedSample + l * lowpassSamples[i];
            float m = 0.5;
            modulatedSample = (1 - m) * modulatedSample + m * lowpassSamples[i];
            
            allpassBuffer[i][allpassBufferIndex] = modulatedSample;
            
            for(int j = 0; j < 8; j++)
            {
                int index = (allpassBufferSize + allpassBufferIndex - (int) allpassDelays[i] - 4 + j) % allpassBufferSize;
                interpolationSamples[j] = allpassBuffer[i][index];
            }
            
            float allpassSample = Interpolation::lagrange8(interpolationSamples, 1.0 - (allpassDelays[i] - floor(allpassDelays[i])));
            
            float in = allpassBuffer[i][allpassBufferIndex];
            float diffusion = 0.001 + 0.7 * density.getValue();
            allpassBuffer[i][allpassBufferIndex] += diffusion * allpassSample;
 
            allpassSample = ((1.0 / diffusion) - diffusion) * allpassBuffer[i][allpassBufferIndex] - (1.0 / diffusion) * in;
            
            feedbackSamples(i, 0) = allpassSample;
        }
        
        feedbackSamples = feedbackSamples.transpose() * diffuseMatrix;
        
        const int numAngles = GivensRotation<12>::numAngles;
        float r = rotation.getValue();
        float d = dispersion.getValue();
        
        for(int i = 0; i < numAngles; i++)
        {
            float del = ((float) i) / numAngles;
            
            phases[i] += (r / 2 - (del * d * r)) * maxRotation;
            phases[i] *= (phases[i] < 2 * M_PI);
            
            rotations[i] = 0.5 * sin(phases[i]);
        }
        
        feedbackSamples = rotationMatrix.rotateSparser(rotations, feedbackSamples);
        
        for(int i = 0; i < numKernels; i++)
            outputBuffer[sample][i] = feedbackSamples(i, 0);
        
        for(int i = 0; i < numKernels; i++)
            feedbackSamples(i, 0) *= decays[i];
        
        for(int i = 0; i < numKernels; i++)
            delayBuffer[delayBufferIndex][i] += feedbackSamples(i, 0);
       
        distance.update();
        rotation.update();
        dispersion.update();
        decayS.update();
        damp.update();
        density.update();
        
        delayBufferIndex += 1;
        delayBufferIndex %= delayBufferSize;
        
        allpassBufferIndex += 1;
        allpassBufferIndex %= allpassBufferSize;
    }
    
    buffer.applyGain(0);
    for(int channel = 0; channel < numChannels; channel++)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for(int sample = 0; sample < numSamples; sample++)
            for(int kernel = 0; kernel < numKernels; kernel += 2)
                channelData[sample] += outputBuffer[sample][kernel + channel];
    }
    
}
