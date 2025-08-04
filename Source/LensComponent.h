/*
  ==============================================================================

    LensComponent.h
    Created: 6 Apr 2025 7:35:08pm
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LensComponent : public Component
{
public:
    LensComponent()
    {
        std::unique_ptr<PNGImageFormat> png = std::make_unique<PNGImageFormat>();
        std::unique_ptr<MemoryInputStream> imgStream = std::make_unique<MemoryInputStream>(BinaryData::lens_l_png,
                                                                                           BinaryData::lens_l_pngSize,
                                                                                           false);
        image = std::make_unique<Image>(png->decodeImage(*imgStream));
    }
    ~LensComponent()
    {
        
    }
    
    
    void paint(Graphics& g) override
    {
        const std::shared_ptr<AffineTransform> transform = std::make_unique<AffineTransform>(0.5, 0.0, 0.0,
                                              0.0, 0.5, 0.0);
        
        g.drawImageTransformed(*image, *transform);
    }
    
private:
    std::unique_ptr<Image> image;
};
