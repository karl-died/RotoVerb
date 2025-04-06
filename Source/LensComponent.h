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
        std::unique_ptr<MemoryInputStream> imgStream = std::make_unique<MemoryInputStream>(BinaryData::lens_png,
                                                                                           BinaryData::lens_pngSize,
                                                                                           false);
        image = std::make_unique<Image>(png->decodeImage(*imgStream));
    }
    ~LensComponent()
    {
        
    }
    
    
    void paint(Graphics& g) override
    {
        g.drawImageAt(*image, 0, 0);
    }
    
private:
    std::unique_ptr<Image> image;
};
