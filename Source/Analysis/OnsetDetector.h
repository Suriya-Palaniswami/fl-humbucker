#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

namespace humbucker
{

/** Energy + spectral-flux hybrid onset detector for beatbox transients. */
class OnsetDetector
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    /** Returns true when a new onset is detected in this block. */
    bool process (const float* samples, int numSamples);

    float getLastOnsetStrength() const noexcept { return lastStrength; }
    double getLastOnsetTimeSeconds() const noexcept { return lastOnsetTimeSec; }

    void setSensitivity (float s) noexcept { sensitivity = juce::jlimit (0.0f, 1.0f, s); }
    void setMinIntervalMs (float ms) noexcept { minIntervalSec = ms / 1000.0f; }

private:
    float computeEnergy (const float* samples, int numSamples) const;
    float computeSpectralFlux (const float* samples, int numSamples);

    double sampleRate = 44100.0;
    double streamTimeSec = 0.0;
    double lastOnsetTimeSec = -1.0;
    float sensitivity = 0.5f;
    float minIntervalSec = 0.08f;
    float lastStrength = 0.0f;
    float energyEma = 0.0f;
    float fluxEma = 0.0f;
    float prevMagnitude[256] {};
    int fftOrder = 8;
    juce::dsp::FFT fft { fftOrder };
    std::vector<float> fftData;
    std::vector<float> window;
};

} // namespace humbucker
