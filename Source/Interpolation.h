/*
  ==============================================================================

    Interpolation.h
    Created: 25 Sep 2025 6:14:03pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once

#include <array>

namespace Interpolation {

inline float cubicLagrange(const std::array<float, 4>& y, const float x)
{
    if(x == 0) return y[1];
    else if(x == 1) return y[2];
    
    static const int x_0[4] = { -1, 0, 1, 2 };
    static const float w[4] = { 1.0 / -6, 1.0 / 2, 1.0 / -2, 1.0 / 6 };
    
    float s1 = 0;
    float s2 = 0;
    
    for(int i = 0; i < 4; i++)
    {
        float t = w[i] / (x - x_0[i]);
        s1 += t * y[i];
        s2 += t;
    }
    
    float res = s1 / s2;
    
    return res;
}

inline float lagrange8(const std::array<float, 8>& y, const float x)
{
    if(x == 0) return y[3];
    else if(x == 1) return y[4];
    
    static constexpr int x_0[8] = { -3, -2, -1, 0, 1, 2, 3, 4 };
    static constexpr float w[8] = {1.0 / -5040, 1.0 / 720, 1.0 / -240, 1.0 / 144, 1.0 / -144, 1.0 / 240, 1.0 / -720, 1.0 / 5040};
    
    float s1 = 0;
    float s2 = 0;
    
    for(int i = 0; i < 8; i++)
    {
        float t = w[i] / (x - x_0[i]);
        s1 += t * y[i];
        s2 += t;
    }
    
    float res = s1 / s2;
    
    return res;
}

}

