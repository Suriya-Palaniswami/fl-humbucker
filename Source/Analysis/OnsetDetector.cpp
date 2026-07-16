#include "OnsetDetector.h"

namespace humbucker
{

void OnsetDetector::prepare (double newSampleRate, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = newSampleRate;
    const int fftSize = 1 << fftOrder;
    fftData.assign ((size_t) fftSize * 2, 0.0f);
    window.resize ((size_t) fftSize);

    for (int i = 0; i < fftSize; ++i)
        window[(size_t) i] = 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * (float) i / (float) (fftSize - 1)));

    std::fill (std::begin (prevMagnitude), std::end (prevMagnitude), 0.0f);
    reset();
}

void OnsetDetector::reset()
{
    streamTimeSec = 0.0;
    lastOnsetTimeSec = -1.0;
    lastStrength = 0.0f;
    energyEma = 0.0f;
    fluxEma = 0.0f;
    std::fill (std::begin (prevMagnitude), std::end (prevMagnitude), 0.0f);
}

float OnsetDetector::computeEnergy (const float* samples, int numSamples) const
{
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sum += samples[i] * samples[i];
    return sum / (float) juce::jmax (1, numSamples);
}

float OnsetDetector::computeSpectralFlux (const float* samples, int numSamples)
{
    const int fftSize = 1 << fftOrder;
    const int useSamples = juce::jmin (numSamples, fftSize);
    std::fill (fftData.begin(), fftData.end(), 0.0f);

    for (int i = 0; i < useSamples; ++i)
        fftData[(size_t) i] = samples[numSamples - useSamples + i] * window[(size_t) i];

    fft.performRealOnlyForwardTransform (fftData.data());

    float flux = 0.0f;
    const int bins = fftSize / 2;
    for (int i = 1; i < bins && i < 256; ++i)
    {
        const float re = fftData[(size_t) i * 2];
        const float im = fftData[(size_t) i * 2 + 1];
        const float mag = std::sqrt (re * re + im * im);
        const float diff = mag - prevMagnitude[i];
        if (diff > 0.0f)
            flux += diff;
        prevMagnitude[i] = mag;
    }

    return flux / (float) bins;
}

bool OnsetDetector::process (const float* samples, int numSamples)
{
    const float energy = computeEnergy (samples, numSamples);
    const float flux = computeSpectralFlux (samples, numSamples);

    const float energyAlpha = 0.08f;
    const float fluxAlpha = 0.12f;
    energyEma = (1.0f - energyAlpha) * energyEma + energyAlpha * energy;
    fluxEma = (1.0f - fluxAlpha) * fluxEma + fluxAlpha * flux;

    const float threshold = juce::jmap (sensitivity, 0.0f, 1.0f, 4.0f, 1.2f);
    const float energyRatio = energyEma > 1.0e-8f ? energy / energyEma : 0.0f;
    const float fluxRatio = fluxEma > 1.0e-8f ? flux / fluxEma : 0.0f;
    lastStrength = 0.6f * energyRatio + 0.4f * fluxRatio;

    const double blockDuration = (double) numSamples / sampleRate;
    streamTimeSec += blockDuration;

    const bool enoughTimePassed = (streamTimeSec - lastOnsetTimeSec) >= (double) minIntervalSec;
    const bool triggered = enoughTimePassed && lastStrength > threshold && energy > 1.0e-5f;

    if (triggered)
        lastOnsetTimeSec = streamTimeSec;

    return triggered;
}

} // namespace humbucker
