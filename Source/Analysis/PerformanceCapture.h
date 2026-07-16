#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace humbucker
{

enum class PerformanceMode
{
    melody = 0,
    rhythm
};

struct PerformanceEvent
{
    double timeSeconds = 0.0;
    int midiNote = 60;
    float pitchHz = 0.0f;
    float velocity = 0.8f;
};

/** Records one vocal performance pass as timed note/hit events. */
class PerformanceCapture
{
public:
    void clear();

    void start();
    void stop();
    bool isActive() const noexcept { return active; }

    void addMelodyEvent (double timeSeconds, float pitchHz, float velocity);
    void addRhythmEvent (double timeSeconds, float velocity);

    PerformanceMode getMode() const noexcept { return mode; }
    void setMode (PerformanceMode newMode) noexcept { mode = newMode; }

    const std::vector<PerformanceEvent>& getEvents() const noexcept { return events; }
    int getEventCount() const noexcept { return (int) events.size(); }
    double getDurationSeconds() const noexcept { return durationSeconds; }
    void setDurationSeconds (double seconds) noexcept { durationSeconds = juce::jmax (durationSeconds, seconds); }

    double quantizeTimeSeconds (double timeSeconds, double bpm, int stepsPerBar) const;

private:
    PerformanceMode mode = PerformanceMode::melody;
    bool active = false;
    double durationSeconds = 0.0;
    std::vector<PerformanceEvent> events;
};

} // namespace humbucker
