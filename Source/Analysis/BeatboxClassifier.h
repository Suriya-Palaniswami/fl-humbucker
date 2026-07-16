#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace humbucker
{

enum class DrumClass
{
  unknown = 0,
  kick,
  snare,
  hihat,
  clap,
  tom
};

/** Heuristic spectral classifier for common beatbox sounds. */
class BeatboxClassifier
{
public:
    void prepare (double sampleRate);
    DrumClass classify (const float* samples, int numSamples);

    static int drumClassToMidiNote (DrumClass drum);
    static const char* drumClassToName (DrumClass drum);

private:
    struct BandEnergy
    {
        float low = 0.0f;
        float mid = 0.0f;
        float high = 0.0f;
        float centroid = 0.0f;
    };

    BandEnergy analyseBands (const float* samples, int numSamples);

    double sampleRate = 44100.0;
    int fftOrder = 9;
    juce::dsp::FFT fft { fftOrder };
    std::vector<float> fftData;
    std::vector<float> window;
};

} // namespace humbucker
