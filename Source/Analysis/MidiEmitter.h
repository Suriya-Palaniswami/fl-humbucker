#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "BeatboxClassifier.h"

namespace humbucker
{

enum class OutputMode
{
    humToMidi = 0,
    beatboxToDrums,
    captureSamples
};

/** Converts analysis results into MIDI note events with optional quantization. */
class MidiEmitter
{
public:
    void prepare (double sampleRate);
    void reset();

    void setOutputMode (OutputMode mode) noexcept { outputMode = mode; }
    void setQuantizeEnabled (bool enabled) noexcept { quantizeEnabled = enabled; }
    void setQuantizeGrid (int stepsPerBar) noexcept { quantizeStepsPerBar = juce::jmax (1, stepsPerBar); }
    void setBpm (double bpm) noexcept { currentBpm = bpm; }

    void emitHumNote (juce::MidiBuffer& midi, int sampleOffset,
                      float frequencyHz, float velocity,
                      double hostPpqPosition);

    void emitDrumHit (juce::MidiBuffer& midi, int sampleOffset,
                      DrumClass drum, float velocity,
                      double hostPpqPosition);

    int frequencyToMidiNote (float frequencyHz) const;

private:
    int quantizeSampleOffset (int sampleOffset, double hostPpqPosition) const;
    void sendNoteOnOff (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity);

    OutputMode outputMode = OutputMode::humToMidi;
    bool quantizeEnabled = false;
    int quantizeStepsPerBar = 16;
    double sampleRate = 44100.0;
    double currentBpm = 120.0;
    int lastHumMidiNote = -1;
};

} // namespace humbucker
