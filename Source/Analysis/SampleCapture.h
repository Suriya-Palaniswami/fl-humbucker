#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <array>

namespace humbucker
{

struct CapturedSlice
{
    std::vector<float> audio;
    double startTimeSec = 0.0;
    int midiNote = 60;
    bool hasAudio = false;
};

/** Ring-buffer recorder that slices audio around detected onsets. */
class SampleCapture
{
public:
    static constexpr int maxSlices = 16;
    static constexpr int defaultSliceSamples = 44100; // ~1s at 44.1k

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setPreRollMs (float ms) noexcept { preRollSamples = (int) std::round (sampleRate * ms / 1000.0); }
    void setPostRollMs (float ms) noexcept { postRollSamples = (int) std::round (sampleRate * ms / 1000.0); }

    void feedAudio (const float* samples, int numSamples);
    void markOnset (double onsetTimeSec, int midiNote);

    const CapturedSlice& getSlice (int index) const noexcept { return slices[(size_t) index % maxSlices]; }
    int getWriteIndex() const noexcept { return writeIndex; }

    bool exportSliceToWav (int index, const juce::File& file) const;

private:
    void finalizePendingSlice();

    double sampleRate = 44100.0;
    int preRollSamples = 2205;
    int postRollSamples = 11025;
    int writeIndex = 0;

    std::vector<float> ringBuffer;
    int ringWritePos = 0;
    int ringSize = 0;

    bool capturing = false;
    int captureStartRingPos = 0;
    int captureMidiNote = 60;
    int samplesRemaining = 0;
    std::vector<float> captureBuffer;

    std::array<CapturedSlice, maxSlices> slices {};
};

} // namespace humbucker
