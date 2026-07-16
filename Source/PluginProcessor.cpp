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
        "mode", "Mode", juce::StringArray { "Melody (hum)", "Rhythm (beatbox)" }, 0));

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
        "followPitch", "Follow Pitch", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "exportBpm", "Export BPM",
        juce::NormalisableRange<float> (40.0f, 200.0f, 0.1f), 120.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "monitorLevel", "Monitor Level",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "previewLevel", "Preview Level",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.85f));

    return { params.begin(), params.end() };
}

void FlHumbuckerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    pitchDetector.prepare (sampleRate, samplesPerBlock);
    onsetDetector.prepare (sampleRate, samplesPerBlock);
    performanceSampler.prepare (sampleRate, samplesPerBlock);
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

void FlHumbuckerAudioProcessor::startPerformanceCapture()
{
    performanceMode = (humbucker::PerformanceMode) (int) apvts.getRawParameterValue ("mode")->load();
    performanceCapture.setMode (performanceMode);
    performanceCapture.start();
    captureSampleCounter = 0;
    lastCapturedMidiNote = -1;
    unvoicedBlocks = 0;
    captureActive.store (true);
}

void FlHumbuckerAudioProcessor::stopPerformanceCapture()
{
    captureActive.store (false);
    performanceCapture.setDurationSeconds ((double) captureSampleCounter / juce::jmax (1.0, currentSampleRate));
    performanceCapture.stop();
}

void FlHumbuckerAudioProcessor::processMelodyCapture (const float* samples, int numSamples, double blockStartTime)
{
    const float freq = pitchDetector.process (samples, numSamples);

    if (pitchDetector.isVoiced() && freq > 0.0f)
    {
        unvoicedBlocks = 0;
        const int note = humbucker::PatternRenderer::frequencyToMidiNote (freq);
        const float velocity = juce::jlimit (0.2f, 1.0f, pitchDetector.getConfidence());

        if (lastCapturedMidiNote < 0 || std::abs (note - lastCapturedMidiNote) >= 1)
        {
            performanceCapture.addMelodyEvent (blockStartTime, freq, velocity);
            lastCapturedMidiNote = note;
        }
    }
    else
    {
        ++unvoicedBlocks;
        if (unvoicedBlocks > 8)
            lastCapturedMidiNote = -1;
    }
}

void FlHumbuckerAudioProcessor::processRhythmCapture (const float* samples, int numSamples, double blockStartTime)
{
    if (onsetDetector.process (samples, numSamples))
    {
        const float velocity = juce::jlimit (0.3f, 1.0f, onsetDetector.getLastOnsetStrength() / 4.0f);
        performanceCapture.addRhythmEvent (blockStartTime, velocity);
    }
}

humbucker::PatternRenderOptions FlHumbuckerAudioProcessor::getRenderOptions() const
{
    humbucker::PatternRenderOptions options;
    options.outputSampleRate = currentSampleRate;
    options.bpm = apvts.getRawParameterValue ("exportBpm")->load();
    options.quantize = apvts.getRawParameterValue ("quantize")->load() > 0.5f;
    options.followPitch = apvts.getRawParameterValue ("followPitch")->load() > 0.5f;

    const int gridIndex = (int) apvts.getRawParameterValue ("quantizeGrid")->load();
    const int stepsPerBar[] { 4, 8, 16, 32 };
    options.stepsPerBar = stepsPerBar[juce::jlimit (0, 3, gridIndex)];

    if (auto* playHead = getPlayHead())
        if (auto pos = playHead->getPosition())
            if (auto bpm = pos->getBpm())
                options.bpm = *bpm;

    return options;
}

void FlHumbuckerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    const int numSamples = buffer.getNumSamples();
    const float inputGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("inputGain")->load());
    const float monitorLevel = apvts.getRawParameterValue ("monitorLevel")->load();
    const float previewLevel = apvts.getRawParameterValue ("previewLevel")->load();

    performanceMode = (humbucker::PerformanceMode) (int) apvts.getRawParameterValue ("mode")->load();
    onsetDetector.setSensitivity (apvts.getRawParameterValue ("onsetSensitivity")->load());
    pitchDetector.setVoicingThreshold (apvts.getRawParameterValue ("pitchConfidence")->load());

    if (auto* playHead = getPlayHead())
        if (auto pos = playHead->getPosition())
            if (auto bpm = pos->getBpm())
                currentBpm = *bpm;

    const auto inputLayout = getBus (true, 0);
    const bool hasInput = inputLayout.isEnabled() && getBusBuffer (buffer, true, 0).getNumSamples() >= numSamples;

    if (hasInput)
    {
        const float* input = getBusBuffer (buffer, true, 0).getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
            monoScratch[(size_t) i] = input[i] * inputGain;
    }
    else
    {
        std::fill (monoScratch.begin(), monoScratch.begin() + numSamples, 0.0f);
    }

    buffer.clear();

    if (hasInput)
    {
        if (monitorLevel > 0.0f)
        {
            buffer.copyFrom (0, 0, monoScratch.data(), numSamples);
            if (buffer.getNumChannels() > 1)
                buffer.copyFrom (1, 0, monoScratch.data(), numSamples);
            buffer.applyGain (monitorLevel);
        }
    }

    if (captureActive.load())
    {
        const double blockStartTime = (double) captureSampleCounter / currentSampleRate;

        if (performanceMode == humbucker::PerformanceMode::melody)
            processMelodyCapture (monoScratch.data(), numSamples, blockStartTime);
        else
            processRhythmCapture (monoScratch.data(), numSamples, blockStartTime);

        captureSampleCounter += numSamples;
    }

    if (performanceSampler.isPreviewActive() && buffer.getNumChannels() >= 2)
    {
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);
        performanceSampler.process (left, right, numSamples);
        buffer.applyGain (previewLevel);
    }
}

bool FlHumbuckerAudioProcessor::loadSample (const juce::File& file)
{
    return sampleSlot.loadFromFile (file);
}

void FlHumbuckerAudioProcessor::startPreview()
{
    const auto options = getRenderOptions();
    performanceSampler.startPreview (performanceCapture.getEvents(),
                                       sampleSlot,
                                       performanceMode,
                                       options.followPitch);
}

void FlHumbuckerAudioProcessor::stopPreview()
{
    performanceSampler.stopPreview();
}

bool FlHumbuckerAudioProcessor::exportPatternAudio (const juce::File& file) const
{
    if (! sampleSlot.hasSample() || performanceCapture.getEvents().empty())
        return false;

    const auto options = getRenderOptions();
    const auto rendered = patternRenderer.renderAudio (performanceCapture.getEvents(),
                                                       sampleSlot,
                                                       performanceMode,
                                                       options);

    juce::WavAudioFormat format;
    std::unique_ptr<juce::FileOutputStream> stream (file.createOutputStream());
    if (stream == nullptr)
        return false;

    std::unique_ptr<juce::AudioFormatWriter> writer (
        format.createWriterFor (stream.get(), options.outputSampleRate,
                              (unsigned int) rendered.getNumChannels(), 16, {}, 0));

    if (writer == nullptr)
        return false;

    stream.release();
    writer->writeFromAudioSampleBuffer (rendered, 0, rendered.getNumSamples());
    return true;
}

bool FlHumbuckerAudioProcessor::exportPatternMidi (const juce::File& file) const
{
    if (performanceCapture.getEvents().empty())
        return false;

    const auto options = getRenderOptions();
    const auto midi = patternRenderer.renderMidi (performanceCapture.getEvents(),
                                                  performanceMode,
                                                  options);

    juce::FileOutputStream stream (file);
    return stream.openedOk() && midi.writeTo (stream);
}

juce::AudioProcessorEditor* FlHumbuckerAudioProcessor::createEditor()
{
    return new FlHumbuckerAudioProcessorEditor (*this);
}

void FlHumbuckerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("samplePath", sampleSlot.getFile().getFullPathName(), nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FlHumbuckerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml (*xml);
    apvts.replaceState (state);

    const auto samplePath = state.getProperty ("samplePath").toString();
    if (samplePath.isNotEmpty())
        loadSample (juce::File (samplePath));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlHumbuckerAudioProcessor();
}
