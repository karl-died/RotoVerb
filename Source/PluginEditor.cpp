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
    
    int xCounter = 0;
    int xIncrement = 120;
    int xOffset = 330;
    
    int labelY1 = 15;
    int sliderY1 = 55;
    int labelY2 = 185;
    int sliderY2 = 225;
    
    addLabel(distanceLabel,
             "Distance", xOffset + xCounter * xIncrement,
             labelY1,
             180,
             50);
    addSlider(distanceSlider,
              distanceAttachment,
              "distance", xOffset + xCounter * xIncrement,
              sliderY1,
              180,
              115);
    
    addLabel(dampLabel,
             "Damp",
             xOffset + xCounter * xIncrement,
             labelY2,
             180,
             50);
    addSlider(dampSlider,
              dampAttachment,
              "damp",
              xOffset + xCounter * xIncrement,
              sliderY2,
              180,
              115);
    
    xCounter++;
    
    addLabel(rotationLabel,
             "Rotation",
             xOffset + xCounter * xIncrement,
             labelY1,
             180,
             50);
    addSlider(rotationSlider,
              rotationAttachment,
              "rotation",
              xOffset + xCounter * xIncrement,
              sliderY1,
              180,
              115);
        
    addLabel(mixLabel,
             "Mix",
             xOffset + xCounter * xIncrement,
             labelY2,
             180,
             50);
    addSlider(mixSlider,
              mixAttachment,
              "mix",
              xOffset + xCounter * xIncrement,
              sliderY2,
              180,
              115);
    
    xCounter++;
    
    addLabel(dispersionLabel,
             "Dispersion",
             xOffset + xCounter * xIncrement,
             labelY1,
             180,
             50);
    addSlider(dispersionSlider,
              dispersionAttachment,
              "dispersion",
              xOffset + xCounter * xIncrement,
              55,
              180,
              115);
    
    xCounter++;
    
    addLabel(densityLabel,
             "Density",
             xOffset + xCounter * xIncrement,
             labelY1,
             180,
             50);
    addSlider(densitySlider,
              densityAttachment,
              "density",
              xOffset + xCounter * xIncrement,
              sliderY1,
              180,
              115);
    
    xCounter++;
    
    addLabel(decayLabel,
             "Decay",
             xOffset + xCounter * xIncrement,
             labelY1,
             180,
             50);
    addSlider(decaySlider,
              decayAttachment,
              "decayS",
              xOffset + xCounter * xIncrement,
              sliderY1,
              180,
              115);
    
    xCounter++;
    
    
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
    std::unique_ptr<MemoryInputStream> imgStream = std::make_unique<MemoryInputStream>(BinaryData::background_png,
                                                                                       BinaryData::background_pngSize,
                                                                                       false);
    
    background = png->decodeImage(*imgStream);
   
    addAndMakeVisible(lens);
    lens.setBounds(43, 43, 300, 300);
    lens.setAlwaysOnTop(true);
    
    getLookAndFeel().setColour(Slider::thumbColourId, Colour(0x77f6eee1));
    getLookAndFeel().setColour(Slider::rotarySliderFillColourId, Colour(0x77f6eee1));
    getLookAndFeel().setColour(Slider::rotarySliderOutlineColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(ComboBox::backgroundColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(PopupMenu::backgroundColourId, Colour(0x22f6eee1));
    getLookAndFeel().setColour(PopupMenu::highlightedBackgroundColourId, Colour(0x00f6eee1));
}

RotoVerbAudioProcessorEditor::~RotoVerbAudioProcessorEditor()
{
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
    g.drawImageAt(background, 0, 0);
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
