#include "PluginProcessor.h"
#include "PluginEditor.h"

FlHumbuckerAudioProcessor::FlHumbuckerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::mono(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FlHumbuckerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "mode", "Mode",
        juce::StringArray { "Hum to MIDI", "Beatbox to Drums", "Capture Samples" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "inputGain", "Input Gain",
        juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f), 6.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "onsetSensitivity", "Onset Sensitivity",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "pitchConfidence", "Pitch Confidence",
        juce::NormalisableRange<float> (0.05f, 0.5f, 0.01f), 0.15f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "quantize", "Quantize", false));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "quantizeGrid", "Quantize Grid",
        juce::StringArray { "1/4", "1/8", "1/16", "1/32" }, 2));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        "recordPattern", "Record Pattern", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "monitorLevel", "Monitor Level",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    return { params.begin(), params.end() };
}

void FlHumbuckerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    pitchDetector.prepare (sampleRate, samplesPerBlock);
    onsetDetector.prepare (sampleRate, samplesPerBlock);
    beatboxClassifier.prepare (sampleRate);
    sampleCapture.prepare (sampleRate, samplesPerBlock);
    midiEmitter.prepare (sampleRate);
    patternRecorder.prepare (sampleRate);

    monoScratch.resize ((size_t) samplesPerBlock);
}

void FlHumbuckerAudioProcessor::releaseResources() {}

bool FlHumbuckerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void FlHumbuckerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const float inputGainDb = apvts.getRawParameterValue ("inputGain")->load();
    const float inputGain = juce::Decibels::decibelsToGain (inputGainDb);
    const float monitorLevel = apvts.getRawParameterValue ("monitorLevel")->load();

    outputMode = (humbucker::OutputMode) (int) apvts.getRawParameterValue ("mode")->load();
    onsetDetector.setSensitivity (apvts.getRawParameterValue ("onsetSensitivity")->load());
    pitchDetector.setVoicingThreshold (apvts.getRawParameterValue ("pitchConfidence")->load());

    const bool quantize = apvts.getRawParameterValue ("quantize")->load() > 0.5f;
    const int gridIndex = (int) apvts.getRawParameterValue ("quantizeGrid")->load();
    const int stepsPerBar[] { 4, 8, 16, 32 };

    midiEmitter.setOutputMode (outputMode);
    midiEmitter.setQuantizeEnabled (quantize);
    midiEmitter.setQuantizeGrid (stepsPerBar[juce::jlimit (0, 3, gridIndex)]);
    midiEmitter.setBpm (currentBpm);

    const bool shouldRecord = apvts.getRawParameterValue ("recordPattern")->load() > 0.5f;
    if (shouldRecord != patternRecorder.isRecording())
    {
        if (shouldRecord)
            patternRecorder.clear();
        patternRecorder.setRecording (shouldRecord);
    }

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBpm = *bpm;
            if (auto ppq = pos->getPpqPosition())
                currentPpq = *ppq;
            hostIsPlaying = pos->getIsPlaying();
        }
    }

    // Pass input to output (silent synth workaround for FL Studio VST3 MIDI routing)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain (ch, 0, numSamples, monitorLevel);

    const auto inputLayout = getBus (true, 0);
    if (! inputLayout.isEnabled() || buffer.getNumChannels() < 1)
        return;

    const float* input = buffer.getReadPointer (0);
    for (int i = 0; i < numSamples; ++i)
        monoScratch[(size_t) i] = input[i] * inputGain;

    sampleCapture.feedAudio (monoScratch.data(), numSamples);

    switch (outputMode)
    {
        case humbucker::OutputMode::humToMidi:
        {
            const float freq = pitchDetector.process (monoScratch.data(), numSamples);
            if (pitchDetector.isVoiced() && freq > 0.0f)
            {
                const float velocity = juce::jlimit (0.2f, 1.0f, pitchDetector.getConfidence());
                midiEmitter.emitHumNote (midiMessages, 0, freq, velocity, currentPpq);

                if (patternRecorder.isRecording())
                {
                    const int note = midiEmitter.frequencyToMidiNote (freq);
                    patternRecorder.addEvent (currentPpq, note, velocity, 480);
                }
            }
            else
            {
                midiEmitter.reset();
            }
            break;
        }

        case humbucker::OutputMode::beatboxToDrums:
        case humbucker::OutputMode::captureSamples:
        {
            if (onsetDetector.process (monoScratch.data(), numSamples))
            {
                const int lookback = juce::jmin (512, numSamples);
                lastDrumClass = beatboxClassifier.classify (monoScratch.data() + numSamples - lookback, lookback);
                const float velocity = juce::jlimit (0.3f, 1.0f, onsetDetector.getLastOnsetStrength() / 4.0f);
                const int note = humbucker::BeatboxClassifier::drumClassToMidiNote (lastDrumClass);

                if (outputMode == humbucker::OutputMode::beatboxToDrums)
                {
                    midiEmitter.emitDrumHit (midiMessages, 0, lastDrumClass, velocity, currentPpq);

                    if (patternRecorder.isRecording())
                        patternRecorder.addEvent (currentPpq, note, velocity, 240);
                }
                else
                {
                    sampleCapture.markOnset (onsetDetector.getLastOnsetTimeSeconds(), note);

                    if (patternRecorder.isRecording())
                        patternRecorder.addEvent (currentPpq, note, velocity, 120);
                }
            }
            break;
        }
    }

    juce::ignoreUnused (hostIsPlaying);
}

juce::AudioProcessorEditor* FlHumbuckerAudioProcessor::createEditor()
{
    return new FlHumbuckerAudioProcessorEditor (*this);
}

void FlHumbuckerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FlHumbuckerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

bool FlHumbuckerAudioProcessor::exportRecordedPattern (const juce::File& file) const
{
    const auto midi = patternRecorder.buildMidiFile (currentBpm, 960);
    juce::FileOutputStream stream (file);
    return stream.openedOk() && midi.writeTo (stream);
}

bool FlHumbuckerAudioProcessor::exportCapturedSlice (int sliceIndex, const juce::File& file) const
{
    return sampleCapture.exportSliceToWav (sliceIndex, file);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlHumbuckerAudioProcessor();
}
