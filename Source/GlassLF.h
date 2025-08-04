/*
  ==============================================================================

    GlassLF.h
    Created: 14 Apr 2025 11:41:19am
    Author:  Karl Diedrichsen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class GlassLF : public LookAndFeel_V4
{
public:
    GlassLF()
    {
        using namespace juce;
        std::unique_ptr<PNGImageFormat> png = std::make_unique<PNGImageFormat>();
        std::unique_ptr<MemoryInputStream> bgStream = std::make_unique<MemoryInputStream>(BinaryData::knob_bg_l_png,
                                                BinaryData::knob_bg_l_pngSize,
                                                false);
        
        sliderBG = std::make_unique<Image>(png->decodeImage(*bgStream));
        
        std::unique_ptr<MemoryInputStream> thumbStream = std::make_unique<MemoryInputStream>(BinaryData::knob_hl_l_png,
                                                BinaryData::knob_hl_l_pngSize,
                                                false);
        
        sliderThumb = std::make_unique<Image>(png->decodeImage(*thumbStream));
        
    }
    void drawRotarySlider(Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          Slider& s)
    {
        const float x0 = -41;
        const float y0 = -35;
        const float r = 36;
        float x1 = x0 + (width  / 2) - r;
        float y1 = y0 + (height / 2) - r;
        
        const auto bgTransform = std::make_unique<AffineTransform>(0.5, 0.0, x1,
                                                                   0.0, 0.5, y1);
        
        g.drawImageTransformed(*sliderBG, *bgTransform);
        
        float angle = - (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));
        
        angle += 39 * M_PI / 48;
        
        float cx = width  / 2;
        float cy = y1 - y0 + r;
        const float r2 = 6;
        const float rr = 20;
        
        float x2 = cx - rr * sin(angle) - r2;
        float y2 = cy - rr * cos(angle) - r2;
        
        const auto thumbTransform = std::make_unique<AffineTransform>(0.5, 0.0, x2,
                                                                      0.0, 0.5, y2);
        
        g.drawImageTransformed(*sliderThumb, *thumbTransform);
    }
    
private:
    std::unique_ptr<juce::Image> sliderBG;
    std::unique_ptr<juce::Image> sliderThumb;
};
