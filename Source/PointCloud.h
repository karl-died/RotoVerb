/*
  ==============================================================================

    PointCloud.h
    Created: 4 Apr 2025 3:35:08pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#ifndef GIVENS_ROTATION_INCLUDED
 #include "GivensRotation.h"
#endif

class PointCloud : public Component
{
public:
    PointCloud()
    {
       
        for(int i = 0; i < 12; i++)
        {
            vertices12d.emplace_back();
            for(int j = 0; j < 12; j++)
                vertices12d[2 * i](j) = 0;
            vertices12d[2 * i](i, 0) = 1 - (2 * (i % 2 == 0));
            
            vertices12d.emplace_back();
            for(int j = 0; j < 12; j++)
                vertices12d[2 * i + 1](j, 0) = 0;
            vertices12d[2 * i + 1](i, 0) = -sqrt(2);
            vertices12d[2 * i + 1]((i+5)%12, 0) = -sqrt(2);
        }
        
        
        //cube
        /*
        for(int i = 0; i < 8; i++)
            vertices12d.emplace_back();
        
        vertices12d[0] <<  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[1] <<  1,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[2] <<  1, -1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[3] <<  1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[4] << -1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[5] << -1,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[6] << -1, -1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
        vertices12d[7] << -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0;
         */
    }
    
    void connectParameters(std::atomic<float>* rotation,
                          std::atomic<float>* dispersion,
                          std::atomic<float>* distance,
                          std::atomic<float>* density)
    {
        rotationParam = rotation;
        dispersionParam = dispersion;
        distanceParam = distance;
        densityParam = density;
    }
    
    void paint(Graphics& g) override
    {
        float dAngle = 20 * M_PI * *rotationParam / 60;
        
        for(int i = 0; i < numAngles; i++)
        {
            float d = ((float) i) / numAngles;
            angles[i] = (1 - 2 * (i % 2 == 0)) * (dAngle / 2) - *dispersionParam * *rotationParam * d * dAngle;
            //angles[i] *= (angles[i] < 2 * M_PI);
        }

        for(auto& vertex : vertices12d)
            vertex = rotation.rotateSparser(angles, vertex);
        
        auto bounds = getLocalBounds();
        int cloudX = bounds.getWidth() / 2;
        int cloudY = bounds.getHeight() / 2;
        float pointSize = globalScale * (5 + 10 * *densityParam);
        float maxScale = globalScale * 210;
        float scale = globalScale * (40 + 80 * *distanceParam);
        
        if(scale < 0)
            scale = 0;
        
        int counter = 0;
        for(auto& vertex : vertices12d)
        {
            float x = vertex(0, 0);
            float y = vertex(1, 0);
            
            float dist2 = scale * (x * x + y * y) / maxScale;
            if(dist2 > 200) dist2 = 200;
            float r = tanh(dist2) / (dist2 + 0.000001);
            //if(r > 1.0) r = 1.0;
            x *= r;
            y *= r;
            
            float d = pointSize * (1 - 0.3 * vertex(2, 0));
            
            g.setColour(Colour::fromHSV((float) counter / 12,
                                        0.5f,
                                        1.0f,
                                        (float) 0.7 + 0.4 * vertex(2, 0)));
            g.fillEllipse(cloudX + scale * x - (d / 2),
                          cloudY + scale * y - (d / 2),
                          d,
                          d);
            counter++;
        }
    }
    
    void setScale(float s)
    {
        globalScale = s;
    }
private:
    
    float globalScale = 1;
    
    static constexpr int numAngles = GivensRotation<12>::numAngles;
    GivensRotation<12> rotation{};
    std::array<float, numAngles> angles{};
    std::atomic<float>* rotationParam;
    std::atomic<float>* dispersionParam;
    std::atomic<float>* distanceParam;
    std::atomic<float>* densityParam;
    std::vector<Eigen::Matrix<float, 12, 1>> vertices12d;
};
