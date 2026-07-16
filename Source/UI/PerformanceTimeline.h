#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Analysis/PerformanceCapture.h"

/** Simple timeline showing captured hits or melody events. */
class PerformanceTimeline : public juce::Component
{
public:
    void setEvents (const std::vector<humbucker::PerformanceEvent>& newEvents,
                    humbucker::PerformanceMode mode,
                    double durationSeconds);

    void setPlayheadSeconds (double seconds);

    void paint (juce::Graphics& g) override;

private:
    std::vector<humbucker::PerformanceEvent> events;
    humbucker::PerformanceMode performanceMode = humbucker::PerformanceMode::rhythm;
    double duration = 1.0;
    double playhead = 0.0;
};
