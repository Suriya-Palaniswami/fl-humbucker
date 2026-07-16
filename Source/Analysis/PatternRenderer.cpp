#include "PatternRenderer.h"
#include <cmath>

namespace humbucker
{

int PatternRenderer::frequencyToMidiNote (float frequencyHz)
{
    if (frequencyHz <= 0.0f)
        return 60;

    const float midi = 69.0f + 12.0f * std::log2 (frequencyHz / 440.0f);
    return juce::jlimit (0, 127, (int) std::lround (midi));
}

float PatternRenderer::midiNoteToPlaybackRatio (int eventNote, int rootNote, bool followPitch)
{
    if (! followPitch)
        return 1.0f;

    return (float) std::pow (2.0, (eventNote - rootNote) / 12.0);
}

juce::AudioBuffer<float> PatternRenderer::renderAudio (const std::vector<PerformanceEvent>& events,
                                                      const SampleSlot& sample,
                                                      PerformanceMode mode,
                                                      const PatternRenderOptions& options) const
{
    juce::AudioBuffer<float> output (1, 1);

    if (events.empty() || ! sample.hasSample())
        return output;

    PerformanceCapture quantizer;
    double endTime = options.tailSeconds;

    for (const auto& event : events)
    {
        double time = event.timeSeconds;
        if (options.quantize)
            time = quantizer.quantizeTimeSeconds (time, options.bpm, options.stepsPerBar);

        const double sampleDuration = (double) sample.getMonoData().size() / sample.getSampleRate();
        endTime = juce::jmax (endTime, time + sampleDuration);
    }

    const int numSamples = (int) std::ceil (endTime * options.outputSampleRate);
    output.setSize (2, juce::jmax (1, numSamples), false, true, true);
    output.clear();

    const auto& sampleData = sample.getMonoData();
    const double sampleRateRatio = sample.getSampleRate() / options.outputSampleRate;

    for (const auto& event : events)
    {
        double time = event.timeSeconds;
        if (options.quantize)
            time = quantizer.quantizeTimeSeconds (time, options.bpm, options.stepsPerBar);

        const int startSample = (int) std::lround (time * options.outputSampleRate);
        const bool followPitch = mode == PerformanceMode::melody && options.followPitch;
        const float ratio = midiNoteToPlaybackRatio (event.midiNote, sample.getRootMidiNote(), followPitch);
        const float gain = event.velocity;
        double readPos = 0.0;

        while (true)
        {
            const int outIndex = startSample + (int) readPos;
            const int sampleIndex = (int) (readPos * (double) sampleRateRatio / (double) ratio);

            if (outIndex >= output.getNumSamples() || sampleIndex >= (int) sampleData.size())
                break;

            const float value = sampleData[(size_t) sampleIndex] * gain;
            output.addSample (0, outIndex, value);
            output.addSample (1, outIndex, value);
            readPos += 1.0;
        }
    }

    return output;
}

juce::MidiFile PatternRenderer::renderMidi (const std::vector<PerformanceEvent>& events,
                                            PerformanceMode mode,
                                            const PatternRenderOptions& options) const
{
    juce::MidiFile midiFile;
    const int ppq = 960;
    midiFile.setTicksPerQuarterNote (ppq);
    juce::MidiMessageSequence sequence;

    PerformanceCapture quantizer;
    const double secondsPerQuarter = 60.0 / juce::jmax (1.0, options.bpm);

    for (const auto& event : events)
    {
        double time = event.timeSeconds;
        if (options.quantize)
            time = quantizer.quantizeTimeSeconds (time, options.bpm, options.stepsPerBar);

        const int startTick = (int) std::lround ((time / secondsPerQuarter) * (double) ppq);
        const int note = mode == PerformanceMode::melody ? event.midiNote : 60;
        const int durationTicks = (int) std::lround ((mode == PerformanceMode::melody ? 0.35 : 0.15) / secondsPerQuarter * (double) ppq);
        const auto velocity = (juce::uint8) juce::jlimit (1, 127, (int) std::lround (event.velocity * 127.0f));

        sequence.addEvent (juce::MidiMessage::noteOn (1, note, velocity), startTick);
        sequence.addEvent (juce::MidiMessage::noteOff (1, note), startTick + juce::jmax (30, durationTicks));
    }

    sequence.updateMatchedPairs();
    midiFile.addTrack (sequence);
    return midiFile;
}

} // namespace humbucker
