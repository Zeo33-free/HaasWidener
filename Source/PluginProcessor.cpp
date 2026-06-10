#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>

HaasWidenerAudioProcessor::HaasWidenerAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

HaasWidenerAudioProcessor::~HaasWidenerAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HaasWidenerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gain",      "Gain (dB)",    -150.0f, 12.0f,  0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay",     "Delay (ms)",   0.0f,    40.0f, 10.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dry",       "Dry (dB)",     -60.0f,  0.0f,  0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("wet",       "Wet (dB)",     -60.0f,  0.0f, -6.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  ("dry_off",   "Dry Off",      false));
    params.push_back (std::make_unique<juce::AudioParameterBool>  ("bypass",    "Bypass",       false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("low_cut",   "Low Cut (Hz)", 20.0f,  1000.0f, 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("high_cut",  "High Cut (Hz)",1000.0f,20000.0f,20000.0f));
    return { params.begin(), params.end() };
}

void HaasWidenerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto maxDelaySamples = static_cast<int> (maxDelaySeconds * sampleRate);
    delayBufferL.setSize (1, maxDelaySamples + 1);
    delayBufferR.setSize (1, maxDelaySamples + 1);
    delayBufferL.clear();
    delayBufferR.clear();
    writePosition = 0;

    smoothedGain.setCurrentAndTargetValue (1.0f);
    smoothedDry.setCurrentAndTargetValue  (1.0f);
    smoothedWet.setCurrentAndTargetValue  (juce::Decibels::decibelsToGain (-6.0f));

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
    hpFilterL.prepare (spec); hpFilterR.prepare (spec);
    lpFilterL.prepare (spec); lpFilterR.prepare (spec);

    auto defaultHP = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 20.0f, 0.7071f);
    hpFilterL.coefficients = std::move (defaultHP);
    hpFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 20.0f, 0.7071f);

    auto defaultLP = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f, 0.7071f);
    lpFilterL.coefficients = std::move (defaultLP);
    lpFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f, 0.7071f);

    lastLowCut  = 20.0f;
    lastHighCut = 20000.0f;
}


void HaasWidenerAudioProcessor::releaseResources() {}

void HaasWidenerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (totalNumInputChannels < 2)
        return;

    if (apvts.getRawParameterValue ("bypass")->load() > 0.5f)
        return;

    // Raw parameters
    auto gainDb   = apvts.getRawParameterValue ("gain")     ->load();
    auto delayMs  = apvts.getRawParameterValue ("delay")    ->load();
    auto dryDb    = apvts.getRawParameterValue ("dry")      ->load();
    auto wetDb    = apvts.getRawParameterValue ("wet")      ->load();
    auto dryOff   = apvts.getRawParameterValue ("dry_off")  ->load();
    auto lowCut   = apvts.getRawParameterValue ("low_cut")  ->load();
    auto highCut  = apvts.getRawParameterValue ("high_cut") ->load();

    // Smoothing targets
    smoothedGain.setTargetValue (juce::Decibels::decibelsToGain (gainDb));
    smoothedDry.setTargetValue  (juce::Decibels::decibelsToGain (dryDb));
    smoothedWet.setTargetValue  (juce::Decibels::decibelsToGain (wetDb));

    // Update filter coefficients only when frequency changes
    if (lowCut != lastLowCut)
    {
        auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), lowCut, 0.7071f);
        hpFilterL.coefficients = hpCoeffs;
        hpFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), lowCut, 0.7071f);
        lastLowCut = lowCut;
    }
    if (highCut != lastHighCut)
    {
        auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (getSampleRate(), highCut, 0.7071f);
        lpFilterL.coefficients = lpCoeffs;
        lpFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (getSampleRate(), highCut, 0.7071f);
        lastHighCut = highCut;
    }

    // Delay in samples
    auto delaySamples = static_cast<int> ((delayMs / 1000.0) * getSampleRate());
    delaySamples = juce::jlimit (0, delayBufferL.getNumSamples() - 1, delaySamples);

    auto* channelL = buffer.getWritePointer (0);
    auto* channelR = buffer.getWritePointer (1);
    auto* delayL   = delayBufferL.getWritePointer (0);
    auto* delayR   = delayBufferR.getWritePointer (0);
    const auto delayBufSize = delayBufferL.getNumSamples();

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Write to delay line
        delayL[writePosition] = channelL[sample];
        delayR[writePosition] = channelR[sample];

        int readPos = writePosition - delaySamples;
        if (readPos < 0) readPos += delayBufSize;

        auto delayedL = delayL[readPos];
        auto delayedR = delayR[readPos];

        const float gain = smoothedGain.getNextValue();
        const float dry  = dryOff > 0.5f ? 0.0f : smoothedDry.getNextValue();
        const float wet  = smoothedWet.getNextValue();
        // Sum delayed L and R to mono
        float monoDelayed = (delayedL + delayedR) * 0.5f;

        // Filter the mono signal (one filter chain)
        float filtered = lpFilterL.processSample (hpFilterL.processSample (monoDelayed));

        // Create wet signals (perfectly out of phase)
        float wetL_raw =  filtered * wet;
        float wetR_raw = -filtered * wet;

        // Mix
        auto outL = channelL[sample] * dry + wetL_raw;
        auto outR = channelR[sample] * dry + wetR_raw;

        channelL[sample] = outL * gain;
        channelR[sample] = outR * gain;

        writePosition = (writePosition + 1) % delayBufSize;
    }
}

void HaasWidenerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HaasWidenerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* HaasWidenerAudioProcessor::createEditor()
{
    return new HaasWidenerAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HaasWidenerAudioProcessor();
}