#include "PerformanceSampler.h"
#include "PatternRenderer.h"
#include <cmath>

namespace humbucker
{

void PerformanceSampler::prepare (double newSampleRate, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = newSampleRate;
    voices.assign (32, Voice {});
    reset();
}

void PerformanceSampler::reset()
{
    previewActive = false;
    transportSample = 0;

    for (auto& voice : voices)
        voice.active = false;
}

void PerformanceSampler::spawnVoice (int64_t startSample, int midiNote, float velocity,
                                     const SampleSlot& sample, bool followPitch)
{
    for (auto& voice : voices)
    {
        if (voice.active)
            continue;

        voice.active = true;
        voice.startSample = startSample;
        voice.readPosition = 0.0;
        voice.velocity = velocity;
        voice.playbackRatio = PatternRenderer::midiNoteToPlaybackRatio (
            midiNote, sample.getRootMidiNote(), followPitch);
        return;
    }
}

void PerformanceSampler::startPreview (const std::vector<PerformanceEvent>& events,
                                       const SampleSlot& sample,
                                       PerformanceMode mode,
                                       bool followPitch)
{
    reset();
    sampleCache = sample.getMonoData();
    cachedSampleRate = sample.getSampleRate();
    previewActive = ! events.empty() && sample.hasSample();

    if (! previewActive)
        return;

    const bool usePitch = mode == PerformanceMode::melody && followPitch;

    for (const auto& event : events)
    {
        const int64_t startSample = (int64_t) std::lround (event.timeSeconds * sampleRate);
        spawnVoice (startSample, event.midiNote, event.velocity, sample, usePitch);
    }
}

void PerformanceSampler::stopPreview()
{
    previewActive = false;

    for (auto& voice : voices)
        voice.active = false;
}

double PerformanceSampler::getPreviewPlayheadSeconds() const noexcept
{
    return sampleRate > 0.0 ? (double) transportSample / sampleRate : 0.0;
}

void PerformanceSampler::process (float* left, float* right, int numSamples)
{
    if (! previewActive || sampleCache.empty())
        return;

    const double sampleRateRatio = cachedSampleRate / sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        const int64_t currentSample = transportSample + i;
        float mix = 0.0f;

        for (auto& voice : voices)
        {
            if (! voice.active)
                continue;

            if (currentSample < voice.startSample)
                continue;

            const int sampleIndex = (int) (voice.readPosition * sampleRateRatio / (double) voice.playbackRatio);

            if (sampleIndex >= (int) sampleCache.size())
            {
                voice.active = false;
                continue;
            }

            mix += sampleCache[(size_t) sampleIndex] * voice.velocity;
            voice.readPosition += 1.0;
        }

        left[i] += mix;
        right[i] += mix;
    }

    transportSample += numSamples;

    bool anyActive = false;
    for (const auto& voice : voices)
        anyActive = anyActive || voice.active;

    if (! anyActive)
        previewActive = false;
}

} // namespace humbucker
