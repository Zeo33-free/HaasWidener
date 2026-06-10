#pragma once

#include <JuceHeader.h>

class HaasWidenerAudioProcessor : public juce::AudioProcessor
{
public:
    HaasWidenerAudioProcessor();
    ~HaasWidenerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Haas Widener"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter state management
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Delay buffers
    juce::AudioBuffer<float> delayBufferL, delayBufferR;
    int writePosition = 0;
    const float maxDelaySeconds = 0.04f;

    // Smoothing
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDry  { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedWet  { 0.0f };

    // Wet signal filters (12 dB/oct, Butterworth)
    juce::dsp::IIR::Filter<float> hpFilterL, hpFilterR, lpFilterL, lpFilterR;
    float lastLowCut  = -1.0f;
    float lastHighCut = -1.0f;
    juce::AudioParameterBool* dryOffParam = nullptr;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HaasWidenerAudioProcessor)
};