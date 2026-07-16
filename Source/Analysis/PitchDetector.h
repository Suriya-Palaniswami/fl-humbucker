#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace humbucker
{

/** Real-time monophonic pitch tracker using a YIN-style autocorrelation algorithm. */
class PitchDetector
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    /** Process one block; returns detected frequency in Hz, or 0 if unvoiced. */
    float process (const float* samples, int numSamples);

    float getLastFrequencyHz() const noexcept { return lastFrequencyHz; }
    float getConfidence() const noexcept { return confidence; }
    bool isVoiced() const noexcept { return voiced; }

    void setMinFrequencyHz (float hz) noexcept { minFrequencyHz = hz; }
    void setMaxFrequencyHz (float hz) noexcept { maxFrequencyHz = hz; }
    void setVoicingThreshold (float t) noexcept { voicingThreshold = t; }

private:
    float yinDifference (const float* samples, int numSamples, int tau) const;
    float parabolicInterpolation (const std::vector<float>& yinBuffer, int tau) const;

    double sampleRate = 44100.0;
    float minFrequencyHz = 80.0f;
    float maxFrequencyHz = 1000.0f;
    float voicingThreshold = 0.15f;
    float lastFrequencyHz = 0.0f;
    float confidence = 0.0f;
    bool voiced = false;

    std::vector<float> analysisBuffer;
    int analysisSize = 2048;
};

} // namespace humbucker
