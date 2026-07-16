#include "BeatboxClassifier.h"

namespace humbucker
{

void BeatboxClassifier::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    const int fftSize = 1 << fftOrder;
    fftData.assign ((size_t) fftSize * 2, 0.0f);
    window.resize ((size_t) fftSize);

    for (int i = 0; i < fftSize; ++i)
        window[(size_t) i] = 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * (float) i / (float) (fftSize - 1)));
}

BeatboxClassifier::BandEnergy BeatboxClassifier::analyseBands (const float* samples, int numSamples)
{
    BandEnergy bands;
    const int fftSize = 1 << fftOrder;
    const int useSamples = juce::jmin (numSamples, fftSize);
    std::fill (fftData.begin(), fftData.end(), 0.0f);

    for (int i = 0; i < useSamples; ++i)
        fftData[(size_t) i] = samples[i] * window[(size_t) i];

    fft.performRealOnlyForwardTransform (fftData.data());

    const int bins = fftSize / 2;
    float weightedSum = 0.0f;
    float totalMag = 0.0f;

    for (int i = 1; i < bins; ++i)
    {
        const float re = fftData[(size_t) i * 2];
        const float im = fftData[(size_t) i * 2 + 1];
        const float mag = std::sqrt (re * re + im * im);
        const float freq = (float) i * (float) sampleRate / (float) fftSize;

        totalMag += mag;
        weightedSum += freq * mag;

        if (freq < 180.0f)
            bands.low += mag;
        else if (freq < 2000.0f)
            bands.mid += mag;
        else
            bands.high += mag;
    }

    if (totalMag > 1.0e-8f)
    {
        bands.low /= totalMag;
        bands.mid /= totalMag;
        bands.high /= totalMag;
        bands.centroid = weightedSum / totalMag;
    }

    return bands;
}

DrumClass BeatboxClassifier::classify (const float* samples, int numSamples)
{
    const auto bands = analyseBands (samples, numSamples);

    if (bands.low > 0.55f && bands.centroid < 250.0f)
        return DrumClass::kick;

    if (bands.high > 0.45f && bands.centroid > 3500.0f)
        return DrumClass::hihat;

    if (bands.mid > 0.35f && bands.high > 0.2f && bands.centroid > 1200.0f && bands.centroid < 4500.0f)
        return DrumClass::snare;

    if (bands.mid > 0.4f && bands.centroid > 500.0f && bands.centroid < 1800.0f)
        return DrumClass::tom;

    if (bands.mid > 0.35f && bands.high > 0.25f)
        return DrumClass::clap;

    return DrumClass::unknown;
}

int BeatboxClassifier::drumClassToMidiNote (DrumClass drum)
{
    switch (drum)
    {
        case DrumClass::kick:   return 36;
        case DrumClass::snare:  return 38;
        case DrumClass::hihat:  return 42;
        case DrumClass::clap:   return 39;
        case DrumClass::tom:    return 45;
        default:                return 37;
    }
}

const char* BeatboxClassifier::drumClassToName (DrumClass drum)
{
    switch (drum)
    {
        case DrumClass::kick:   return "Kick";
        case DrumClass::snare:  return "Snare";
        case DrumClass::hihat:  return "Hi-Hat";
        case DrumClass::clap:   return "Clap";
        case DrumClass::tom:    return "Tom";
        default:                return "Unknown";
    }
}

} // namespace humbucker
