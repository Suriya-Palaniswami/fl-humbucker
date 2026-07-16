#pragma once

#include "PerformanceCapture.h"
#include "SampleSlot.h"
#include <vector>

namespace humbucker
{

/** Real-time preview of a captured pattern using the loaded sample. */
class PerformanceSampler
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void startPreview (const std::vector<PerformanceEvent>& events,
                       const SampleSlot& sample,
                       PerformanceMode mode,
                       bool followPitch);

    void stopPreview();
    bool isPreviewActive() const noexcept { return previewActive; }
    double getPreviewPlayheadSeconds() const noexcept;

    void process (float* left, float* right, int numSamples);

private:
    struct Voice
    {
        int64_t startSample = 0;
        double readPosition = 0.0;
        double playbackRatio = 1.0;
        float velocity = 1.0f;
        bool active = false;
    };

    void spawnVoice (int64_t startSample, int midiNote, float velocity,
                     const SampleSlot& sample, bool followPitch);

    double sampleRate = 44100.0;
    bool previewActive = false;
    int64_t transportSample = 0;
    std::vector<Voice> voices;
    std::vector<float> sampleCache;
    double cachedSampleRate = 44100.0;
};

} // namespace humbucker
