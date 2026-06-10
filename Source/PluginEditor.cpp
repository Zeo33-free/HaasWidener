#include "PluginProcessor.h"
#include "PluginEditor.h"

HaasWidenerAudioProcessorEditor::HaasWidenerAudioProcessorEditor (HaasWidenerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), resizer (this, &constrainer)
{
    // Custom colour scheme
    customLookAndFeel.setColour (juce::Slider::backgroundColourId,          juce::Colour (0xff2a2a2a));
    customLookAndFeel.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffcccccc));
    customLookAndFeel.setColour (juce::Slider::trackColourId,               juce::Colour (0xff444444));
    customLookAndFeel.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xff888888));
    customLookAndFeel.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    customLookAndFeel.setColour (juce::TextButton::buttonColourId,          juce::Colour (0xff444444));
    customLookAndFeel.setColour (juce::TextButton::buttonOnColourId,        juce::Colour (0xffff5a5a));
    customLookAndFeel.setColour (juce::TextButton::textColourOffId,         juce::Colour (0xffcccccc));
    customLookAndFeel.setColour (juce::TextButton::textColourOnId,          juce::Colour (0xffffffff));
    setLookAndFeel (&customLookAndFeel);

    // Helper for vertical sliders (gain, delay, dry, wet)
    auto makeVerticalSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::LinearVertical);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 16);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setColour (juce::Slider::textBoxTextColourId,    juce::Colour (0xffaaaaaa));
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1e1e1e));
        addAndMakeVisible (s);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
        l.attachToComponent (&s, false);
        l.setFont (sliderLabelFont);
        addAndMakeVisible (l);
    };

    // Helper for rotary sliders (low cut, high cut)
    auto makeRotarySlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setColour (juce::Slider::textBoxTextColourId,    juce::Colour (0xffaaaaaa));
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1e1e1e));
        addAndMakeVisible (s);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
        l.attachToComponent (&s, false);
        l.setFont (sliderLabelFont);
        addAndMakeVisible (l);
    };

    makeVerticalSlider (gainSlider,  gainLabel,  "Gain");
    makeVerticalSlider (delaySlider, delayLabel, "Delay");
    makeVerticalSlider (drySlider,   dryLabel,   "Dry");
    makeVerticalSlider (wetSlider,   wetLabel,   "Wet");

    makeRotarySlider (lowCutSlider,  lowCutLabel,  "Low Cut");
    makeRotarySlider (highCutSlider, highCutLabel, "High Cut");

    // Bypass button
    bypassButton.setButtonText ("BYPASS");
    bypassButton.setClickingTogglesState (true);
    bypassButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3a3a));
    bypassButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffff5a5a));
    bypassButton.setSize (68, 24);
    addAndMakeVisible (bypassButton);
    dryOffButton.setButtonText ("DRY OFF");
    dryOffButton.setClickingTogglesState (true);
    dryOffButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3a3a));
    dryOffButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffff5a5a));
    dryOffButton.setSize (68, 24);
    addAndMakeVisible (dryOffButton);

    dryOffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "dry_off", dryOffButton);

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dryOffAttachment;
    gainAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "gain",     gainSlider);
    delayAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "delay",    delaySlider);
    dryAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "dry",      drySlider);
    wetAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "wet",      wetSlider);
    lowCutAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "low_cut",  lowCutSlider);
    highCutAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "high_cut", highCutSlider);
    bypassAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "bypass",   bypassButton);

    constrainer.setMinimumSize (400, 450);
    constrainer.setMaximumSize (1200, 1360);
    addAndMakeVisible (resizer);
    resizer.setAlwaysOnTop (true);
    setSize (400, 450);
}

HaasWidenerAudioProcessorEditor::~HaasWidenerAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void HaasWidenerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1c1c1c));
    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawRect (getLocalBounds().toFloat(), 1.5f);

    g.setColour (juce::Colour (0xffcccccc));
    g.setFont (juce::Font (22.0f * currentScale, juce::Font::bold));
    g.drawText ("Haas Widener",
                juce::roundToInt (24 * currentScale),
                juce::roundToInt (13 * currentScale),
                juce::roundToInt (200 * currentScale),
                juce::roundToInt (28 * currentScale),
                juce::Justification::left, false);
    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::Font(9.0f * currentScale));
    g.drawText("made by R1C1N",
        juce::roundToInt(24 * currentScale),
        getHeight() - juce::roundToInt(30 * currentScale),
        juce::roundToInt(200 * currentScale),
        juce::roundToInt(14 * currentScale),
        juce::Justification::left, false);
    
}

void HaasWidenerAudioProcessorEditor::resized()
{
    const float scaleX = getWidth()  / refWidth;
    const float scaleY = getHeight() / refHeight;
    const float scale  = juce::jmin (scaleX, scaleY); // keep proportions uniform
    currentScale = scale;
    sliderLabelFont = juce::Font (12.0f * currentScale);
    // Helper to scale an int value
    auto sc = [&](int val) { return juce::roundToInt (val * scale); };

    auto area = getLocalBounds().reduced (sc (20));

    // Top row: title and buttons
    auto topArea = area.removeFromTop (sc (40));
    // Bypass button (right)
    bypassButton.setBounds (topArea.removeFromRight (sc (68))
                                    .withTrimmedTop (sc (-7))
                                    .withTrimmedBottom (sc (19)));
    dryOffButton.setBounds (bypassButton.getX() - sc (73),
                            bypassButton.getY(),
                            bypassButton.getWidth(),
                            bypassButton.getHeight());

    auto sliderArea = area.removeFromTop (juce::roundToInt (area.getHeight() * 0.60f));
    const int numSliders = 4;
    const int spacing    = sc (20);
    const int sliderWidth = (sliderArea.getWidth() - (numSliders - 1) * spacing) / numSliders;
    int xPos = sliderArea.getX();

    gainSlider.setBounds  (xPos, sliderArea.getY(), sliderWidth, sliderArea.getHeight());
    xPos += sliderWidth + spacing;
    delaySlider.setBounds (xPos, sliderArea.getY(), sliderWidth, sliderArea.getHeight());
    xPos += sliderWidth + spacing;
    drySlider.setBounds   (xPos, sliderArea.getY(), sliderWidth, sliderArea.getHeight());
    xPos += sliderWidth + spacing;
    wetSlider.setBounds   (xPos, sliderArea.getY(), sliderWidth, sliderArea.getHeight());

    // Gap and rotary knobs
    area.removeFromTop (sc (10));
    auto rotaryArea = area;
    const int rotaryWidth  = sc (110);
    const int rotaryHeight = rotaryArea.getHeight();
    const int rotaryCenterX = rotaryArea.getCentreX();
    const int rotaryY       = rotaryArea.getY() + sc (15);

    lowCutSlider.setBounds (rotaryCenterX - rotaryWidth - sc (20),
                            rotaryY, rotaryWidth, rotaryHeight);
    highCutSlider.setBounds (rotaryCenterX + sc (20),
                             rotaryY, rotaryWidth, rotaryHeight);

    // Resize corner
    const int resizerSize = sc (16);
    resizer.setBounds (getWidth() - resizerSize, getHeight() - resizerSize, resizerSize, resizerSize);
    for (auto* label : { &gainLabel, &delayLabel, &dryLabel, &wetLabel, &lowCutLabel, &highCutLabel })
    label->setFont (sliderLabelFont);
}