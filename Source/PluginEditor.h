#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class HaasWidenerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    HaasWidenerAudioProcessorEditor (HaasWidenerAudioProcessor&);
    ~HaasWidenerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HaasWidenerAudioProcessor& audioProcessor;
    float currentScale = 1.0f;
    const float refWidth = 400.0f;
    const float refHeight = 450.0f;
    juce::Font sliderLabelFont { 12.0f };
    juce::Slider gainSlider, delaySlider, drySlider, wetSlider;
    juce::Label  gainLabel,  delayLabel,  dryLabel,  wetLabel;
    juce::Slider lowCutSlider, highCutSlider;
    juce::Label  lowCutLabel,  highCutLabel;
    juce::TextButton bypassButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
        juce::TextButton dryOffButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dryOffAttachment;

    juce::LookAndFeel_V4 customLookAndFeel;

    juce::ComponentBoundsConstrainer constrainer;
    juce::ResizableCornerComponent resizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HaasWidenerAudioProcessorEditor)
};