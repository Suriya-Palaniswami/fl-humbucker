#include "PatternRecorder.h"

namespace humbucker
{

void PatternRecorder::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
}

void PatternRecorder::clear()
{
    events.clear();
}

void PatternRecorder::addEvent (double ppqPosition, int midiNote, float velocity, int durationSamples)
{
    if (! recording)
        return;

    RecordedEvent event;
    event.ppqPosition = ppqPosition;
    event.midiNote = midiNote;
    event.velocity = velocity;
    event.durationSamples = durationSamples;
    events.push_back (event);
}

juce::MidiFile PatternRecorder::buildMidiFile (double bpm, int ppq) const
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote (ppq);

    juce::MidiMessageSequence sequence;

    for (const auto& event : events)
    {
        const double beats = event.ppqPosition;
        const int startTick = (int) std::lround (beats * (double) ppq);
        const int durationTicks = juce::jmax (1, (int) std::lround ((double) event.durationSamples / sampleRate * bpm / 60.0 * (double) ppq));

        sequence.addEvent (juce::MidiMessage::noteOn (1, event.midiNote, (juce::uint8) juce::jlimit (1, 127, (int) std::lround (event.velocity * 127.0f))), startTick);
        sequence.addEvent (juce::MidiMessage::noteOff (1, event.midiNote), startTick + durationTicks);
    }

    sequence.updateMatchedPairs();
    midiFile.addTrack (sequence);
    return midiFile;
}

juce::String PatternRecorder::toStepSequencerText() const
{
    juce::String text;
    text << "FL Humbucker recorded events (" << events.size() << ")\n";

    for (const auto& event : events)
        text << "PPQ " << juce::String (event.ppqPosition, 3)
             << " | Note " << event.midiNote
             << " | Vel " << juce::String (event.velocity, 2) << "\n";

    return text;
}

} // namespace humbucker
