#include "PitchDetector.h"

namespace humbucker
{

void PitchDetector::prepare (double newSampleRate, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = newSampleRate;
    analysisSize = juce::jlimit (1024, 4096, (int) std::round (sampleRate * 0.05));
    analysisBuffer.assign ((size_t) analysisSize, 0.0f);
    reset();
}

void PitchDetector::reset()
{
    lastFrequencyHz = 0.0f;
    confidence = 0.0f;
    voiced = false;
    std::fill (analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
}

float PitchDetector::yinDifference (const float* samples, int numSamples, int tau) const
{
    float sum = 0.0f;
    const int limit = numSamples - tau;
    for (int i = 0; i < limit; ++i)
    {
        const float delta = samples[i] - samples[i + tau];
        sum += delta * delta;
    }
    return sum;
}

float PitchDetector::parabolicInterpolation (const std::vector<float>& yinBuffer, int tau) const
{
    if (tau <= 0 || tau >= (int) yinBuffer.size() - 1)
        return (float) tau;

    const float s0 = yinBuffer[(size_t) tau - 1];
    const float s1 = yinBuffer[(size_t) tau];
    const float s2 = yinBuffer[(size_t) tau + 1];
    const float denom = 2.0f * (2.0f * s1 - s2 - s0);
    if (std::abs (denom) < 1.0e-6f)
        return (float) tau;

    return (float) tau + (s2 - s0) / denom;
}

float PitchDetector::process (const float* samples, int numSamples)
{
    if (numSamples < 64)
        return lastFrequencyHz;

    const int useSamples = juce::jmin (numSamples, analysisSize);
    std::copy (samples + numSamples - useSamples, samples + numSamples, analysisBuffer.begin());

    const int minTau = juce::jmax (2, (int) std::floor (sampleRate / (double) maxFrequencyHz));
    const int maxTau = juce::jmin (useSamples / 2, (int) std::ceil (sampleRate / (double) minFrequencyHz));

    if (maxTau <= minTau)
        return lastFrequencyHz;

    std::vector<float> yinBuffer ((size_t) maxTau + 1, 0.0f);
    float runningSum = 0.0f;

    for (int tau = 1; tau <= maxTau; ++tau)
    {
        const float d = yinDifference (analysisBuffer.data(), useSamples, tau);
        runningSum += d;
        yinBuffer[(size_t) tau] = runningSum > 0.0f ? d * (float) tau / runningSum : 1.0f;
    }

    int bestTau = -1;
    for (int tau = minTau; tau <= maxTau; ++tau)
    {
        if (yinBuffer[(size_t) tau] < voicingThreshold)
        {
            while (tau + 1 <= maxTau && yinBuffer[(size_t) tau + 1] < yinBuffer[(size_t) tau])
                ++tau;

            bestTau = tau;
            confidence = 1.0f - yinBuffer[(size_t) tau];
            break;
        }
    }

    if (bestTau < 0)
    {
        voiced = false;
        lastFrequencyHz = 0.0f;
        confidence = 0.0f;
        return 0.0f;
    }

    const float refinedTau = parabolicInterpolation (yinBuffer, bestTau);
    lastFrequencyHz = (float) (sampleRate / (double) refinedTau);
    voiced = lastFrequencyHz >= minFrequencyHz && lastFrequencyHz <= maxFrequencyHz;
    return voiced ? lastFrequencyHz : 0.0f;
}

} // namespace humbucker
