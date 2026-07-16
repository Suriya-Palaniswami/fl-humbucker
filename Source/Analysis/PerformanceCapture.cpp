#include "PerformanceCapture.h"
#include <cmath>

namespace humbucker
{
namespace
{
int pitchHzToMidi (float frequencyHz)
{
    if (frequencyHz <= 0.0f)
        return 60;

    const float midi = 69.0f + 12.0f * std::log2 (frequencyHz / 440.0f);
    return juce::jlimit (0, 127, (int) std::lround (midi));
}
} // namespace

void PerformanceCapture::clear()
{
    events.clear();
    durationSeconds = 0.0;
    active = false;
}

void PerformanceCapture::start()
{
    events.clear();
    durationSeconds = 0.0;
    active = true;
}

void PerformanceCapture::stop()
{
    active = false;
}

void PerformanceCapture::addMelodyEvent (double timeSeconds, float pitchHz, float velocity)
{
    if (! active || pitchHz <= 0.0f)
        return;

    PerformanceEvent event;
    event.timeSeconds = timeSeconds;
    event.pitchHz = pitchHz;
    event.velocity = juce::jlimit (0.1f, 1.0f, velocity);
    event.midiNote = pitchHzToMidi (pitchHz);

    if (! events.empty())
    {
        const auto& last = events.back();
        if (std::abs (last.midiNote - event.midiNote) <= 0
            && (timeSeconds - last.timeSeconds) < 0.08)
            return;
    }

    events.push_back (event);
    durationSeconds = juce::jmax (durationSeconds, timeSeconds);
}

void PerformanceCapture::addRhythmEvent (double timeSeconds, float velocity)
{
    if (! active)
        return;

    PerformanceEvent event;
    event.timeSeconds = timeSeconds;
    event.midiNote = 60;
    event.velocity = juce::jlimit (0.1f, 1.0f, velocity);
    events.push_back (event);
    durationSeconds = juce::jmax (durationSeconds, timeSeconds);
}

double PerformanceCapture::quantizeTimeSeconds (double timeSeconds, double bpm, int stepsPerBar) const
{
    if (bpm <= 0.0 || stepsPerBar <= 0)
        return timeSeconds;

    const double stepDuration = (60.0 / bpm) * 4.0 / (double) stepsPerBar;
    return std::floor (timeSeconds / stepDuration + 0.5) * stepDuration;
}

} // namespace humbucker
