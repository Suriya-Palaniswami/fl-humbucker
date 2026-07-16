#include "MidiEmitter.h"

namespace humbucker
{

void MidiEmitter::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

void MidiEmitter::reset()
{
    lastHumMidiNote = -1;
}

int MidiEmitter::frequencyToMidiNote (float frequencyHz) const
{
    if (frequencyHz <= 0.0f)
        return -1;

    const float midi = 69.0f + 12.0f * std::log2 (frequencyHz / 440.0f);
    return juce::jlimit (0, 127, (int) std::lround (midi));
}

int MidiEmitter::quantizeSampleOffset (int sampleOffset, double hostPpqPosition) const
{
    if (! quantizeEnabled || currentBpm <= 0.0 || sampleRate <= 0.0)
        return sampleOffset;

    const double samplesPerQuarter = sampleRate * 60.0 / currentBpm;
    const double ppqPerStep = 4.0 / (double) quantizeStepsPerBar;
    const double currentStep = std::floor (hostPpqPosition / ppqPerStep);
    const double targetPpq = currentStep * ppqPerStep;
    const double deltaPpq = targetPpq - hostPpqPosition;
    const int deltaSamples = (int) std::lround (deltaPpq * samplesPerQuarter);
    return juce::jmax (0, sampleOffset + deltaSamples);
}

void MidiEmitter::sendNoteOnOff (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity)
{
    const auto vel = (juce::uint8) juce::jlimit (1, 127, (int) std::lround (velocity * 127.0f));
    midi.addEvent (juce::MidiMessage::noteOn (1, note, vel), sampleOffset);
    midi.addEvent (juce::MidiMessage::noteOff (1, note), sampleOffset + 120);
}

void MidiEmitter::emitHumNote (juce::MidiBuffer& midi, int sampleOffset,
                               float frequencyHz, float velocity,
                               double hostPpqPosition)
{
    if (outputMode != OutputMode::humToMidi)
        return;

    const int note = frequencyToMidiNote (frequencyHz);
    if (note < 0)
        return;

    if (note == lastHumMidiNote)
        return;

  lastHumMidiNote = note;
    const int offset = quantizeSampleOffset (sampleOffset, hostPpqPosition);
    sendNoteOnOff (midi, offset, note, velocity);
}

void MidiEmitter::emitDrumHit (juce::MidiBuffer& midi, int sampleOffset,
                               DrumClass drum, float velocity,
                               double hostPpqPosition)
{
    if (outputMode != OutputMode::beatboxToDrums)
        return;

    const int note = BeatboxClassifier::drumClassToMidiNote (drum);
    const int offset = quantizeSampleOffset (sampleOffset, hostPpqPosition);
    sendNoteOnOff (midi, offset, note, velocity);
}

} // namespace humbucker
