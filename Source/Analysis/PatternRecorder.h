#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace humbucker
{

struct RecordedEvent
{
    double ppqPosition = 0.0;
    int midiNote = 60;
    float velocity = 0.8f;
    int durationSamples = 0;
};

/** Records detected MIDI events for export as an FL Studio pattern. */
class PatternRecorder
{
public:
    void prepare (double sampleRate);
    void clear();

    void setRecording (bool shouldRecord) noexcept { recording = shouldRecord; }
    bool isRecording() const noexcept { return recording; }

    void addEvent (double ppqPosition, int midiNote, float velocity, int durationSamples);

    juce::MidiFile buildMidiFile (double bpm, int ppq) const;
    juce::String toStepSequencerText() const;

    const std::vector<RecordedEvent>& getEvents() const noexcept { return events; }

private:
    double sampleRate = 44100.0;
    bool recording = false;
    std::vector<RecordedEvent> events;
};

} // namespace humbucker
