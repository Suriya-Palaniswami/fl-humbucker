#pragma once

#include "PerformanceCapture.h"
#include "SampleSlot.h"

namespace humbucker
{

struct PatternRenderOptions
{
    double outputSampleRate = 44100.0;
    double bpm = 120.0;
    bool quantize = false;
    int stepsPerBar = 16;
    bool followPitch = true;
    float tailSeconds = 1.5f;
};

/** Renders a captured performance with a chosen sample to audio or MIDI. */
class PatternRenderer
{
public:
    juce::AudioBuffer<float> renderAudio (const std::vector<PerformanceEvent>& events,
                                          const SampleSlot& sample,
                                          PerformanceMode mode,
                                          const PatternRenderOptions& options) const;

    juce::MidiFile renderMidi (const std::vector<PerformanceEvent>& events,
                               PerformanceMode mode,
                               const PatternRenderOptions& options) const;

    static int frequencyToMidiNote (float frequencyHz);
    static float midiNoteToPlaybackRatio (int eventNote, int rootNote, bool followPitch);
};

} // namespace humbucker
