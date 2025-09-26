/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RotoVerbAudioProcessorEditor::RotoVerbAudioProcessorEditor (RotoVerbAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    archivoFont = Font(FontOptions(Typeface::createSystemTypefaceFor(BinaryData::archivo_vf_ttf, BinaryData::archivo_vf_ttfSize)));
    archivoFont.setSizeAndStyle(18.0,
                                "Thin",
                                1.0,
                                0.0);
    
    getLookAndFeel().setDefaultSansSerifTypeface(archivoFont.getTypefacePtr());
    
    auto addLabel = [this] (Label& label, std::string name, int x, int y, int w, int h)
    {
        label.setText(name, dontSendNotification);
        label.setJustificationType(Justification::centred);
        label.setBounds(x, y, w, h);
        label.setFont(archivoFont);
        addAndMakeVisible(label);
    };
    
    auto addSlider = [this] (Slider& slider, std::unique_ptr<SliderAttachment>& attachment, std::string name, int x, int y, int w, int h)
    {
        slider.setBounds(x, y, w, h);
        slider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, 100, 15);
        addAndMakeVisible(slider);
        attachment.reset(new SliderAttachment(this->valueTreeState, name, slider));
    };
    
    int xInc = 120;
    int xO = 400;
    int bRad = 70;
    
    int labelY1 = 15;
    int sliderY1 = 45;
    int labelY2 = 185;
    int sliderY2 = 215;
    
    addLabel(distanceLabel,
             "Distance",
             xO - bRad,
             labelY1,
             2 * bRad,
             50);
    addSlider(distanceSlider,
              distanceAttachment,
              "distance",
              xO - bRad,
              sliderY1,
              2 * bRad,
              2 * bRad);
    
    addLabel(dampLabel,
             "Damp",
             xO - bRad,
             labelY2,
             2 * bRad,
             50);
    addSlider(dampSlider,
              dampAttachment,
              "damp",
              xO - bRad,
              sliderY2,
              2 * bRad,
              2 * bRad);
    
    xO += xInc;
    
    addLabel(rotationLabel,
             "Rotation",
             xO - bRad,
             labelY1,
             2 * bRad,
             50);
    addSlider(rotationSlider,
              rotationAttachment,
              "rotation",
              xO - bRad,
              sliderY1,
              2 * bRad,
              2 * bRad);
        
    addLabel(mixLabel,
             "Mix",
             xO - bRad,
             labelY2,
             2 * bRad,
             50);
    addSlider(mixSlider,
              mixAttachment,
              "mix",
              xO - bRad,
              sliderY2,
              2 * bRad,
              2 * bRad);
    
    xO += xInc;
    
    addLabel(dispersionLabel,
             "Dispersion",
             xO - bRad,
             labelY1,
             2 * bRad,
             50);
    addSlider(dispersionSlider,
              dispersionAttachment,
              "dispersion",
              xO - bRad,
              sliderY1,
              2 * bRad,
              2 * bRad);
    
    xO += xInc;
    
    addLabel(densityLabel,
             "Density",
             xO - bRad,
             labelY1,
             2 * bRad,
             50);
    addSlider(densitySlider,
              densityAttachment,
              "density",
              xO - bRad,
              sliderY1,
              2 * bRad,
              2 * bRad);
    
    xO += xInc;
    
    addLabel(decayLabel,
             "Decay",
             xO - bRad,
             labelY1,
             2 * bRad,
             50);
    addSlider(decaySlider,
              decayAttachment,
              "decayS",
              xO - bRad,
              sliderY1,
              2 * bRad,
              2 * bRad);
    
    xO += xInc;
    
    
    setSize (1000, 380);
    
    matrixMenu.setBounds(670, 250, 180, 30);
    
    const auto matrices = audioProcessor.getMatrices();
    int id = 1;
    for(const auto& matrix : matrices)
        matrixMenu.addItem(matrix.first, id++);
    
    matrixMenu.onChange = [this] {
        int currentId = matrixMenu.getSelectedId();
        int index = matrixMenu.indexOfItemId(currentId);
        String matrixName = matrixMenu.getItemText(index);
        audioProcessor.setMatrix(matrixName.getCharPointer());
    };
    
    matrixMenu.setText(audioProcessor.getCurrentMatrixName());
    addAndMakeVisible(matrixMenu);
    
    pointCloud.connectParameters(valueTreeState.getRawParameterValue("rotation"),
                                 valueTreeState.getRawParameterValue("dispersion"),
                                 valueTreeState.getRawParameterValue("distance"),
                                 valueTreeState.getRawParameterValue("density"));
    
    pointCloud.setBounds(50, 40, 300, 300);
    pointCloud.setScale(1.0);
    
    addAndMakeVisible(&pointCloud);
    startTimerHz(30);
    
    
    std::unique_ptr<PNGImageFormat> png = std::make_unique<PNGImageFormat>();
    std::unique_ptr<MemoryInputStream> imgStream = std::make_unique<MemoryInputStream>(BinaryData::background_l_png,
                                            BinaryData::background_l_pngSize,
                                            false);
    
    background = png->decodeImage(*imgStream);
   
    addAndMakeVisible(lens);
    lens.setBounds(43, 43, 300, 300);
    lens.setAlwaysOnTop(true);
    
    setLookAndFeel(&glassLookAndFeel);
    
    getLookAndFeel().setColour(Slider::thumbColourId, Colour(0x77f6eee1));
    getLookAndFeel().setColour(Slider::rotarySliderFillColourId, Colour(0x77f6eee1));
    getLookAndFeel().setColour(Slider::rotarySliderOutlineColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(ComboBox::backgroundColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(PopupMenu::backgroundColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(PopupMenu::highlightedBackgroundColourId, Colour(0x00f6eee1));
}

RotoVerbAudioProcessorEditor::~RotoVerbAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void RotoVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    //g.fillAll(juce::Colour::fromRGB(185, 200, 210));
    /*
    auto bounds = getLocalBounds();
    g.setGradientFill(ColourGradient(juce::Colour::fromHSV(0.65, 0.4, 0.5, 1.0),
                                     -100.0,
                                     -100.0,
                                     juce::Colour::fromHSV(0.70, 0.3, 0.8, 1.0),
                                     bounds.getWidth()  + 100,
                                     bounds.getHeight() + 100,
                                     false));
    
    //g.fillAll(juce::Colour::fromHSV(0.7, 0.3, 0.8, 1.0));
    g.fillAll();

    g.setGradientFill(ColourGradient(juce::Colour::fromHSV(0.7, 0.8, 0.1, 1.0),
                                     150.0,
                                     190.0,
                                     juce::Colour::fromHSV(0.7, 0.8, 0.1, 0.0),
                                     300.0,
                                     190.0,
                                     true));
    
    g.fillEllipse(0, 40, 300, 300);
    */
    g.fillAll(juce::Colours::black);
    const auto bgTransform = std::make_unique<AffineTransform>(0.5, 0.0, 0.0,
                                                               0.0, 0.5, 0.0);
    //g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImageTransformed(background, *bgTransform);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));

}

void RotoVerbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void RotoVerbAudioProcessorEditor::timerCallback()
{
    pointCloud.repaint();
}
