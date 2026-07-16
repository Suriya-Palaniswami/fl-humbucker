#include "PerformanceTimeline.h"

void PerformanceTimeline::setEvents (const std::vector<humbucker::PerformanceEvent>& newEvents,
                                     humbucker::PerformanceMode mode,
                                     double durationSeconds)
{
    events = newEvents;
    performanceMode = mode;
    duration = juce::jmax (0.5, durationSeconds + 0.25);
    repaint();
}

void PerformanceTimeline::setPlayheadSeconds (double seconds)
{
    playhead = seconds;
    repaint();
}

void PerformanceTimeline::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (juce::Colour (0xff16213e));
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (juce::Colour (0xff0f3460));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    if (events.empty())
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (juce::FontOptions (13.0f));
        g.drawText ("Record a performance to see your pattern here",
                    bounds.toNearestInt(), juce::Justification::centred);
        return;
    }

    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    const float midY = bounds.getCentreY();

    for (const auto& event : events)
    {
        const float x = bounds.getX() + (float) (event.timeSeconds / duration) * width;
        const float radius = performanceMode == humbucker::PerformanceMode::melody ? 7.0f : 5.0f;

        if (performanceMode == humbucker::PerformanceMode::melody)
        {
            const float noteOffset = (float) (event.midiNote - 60) * 1.5f;
            g.setColour (juce::Colour (0xffe94560));
            g.fillEllipse (x - radius, midY - noteOffset - radius, radius * 2.0f, radius * 2.0f);
        }
        else
        {
            g.setColour (juce::Colour (0xff53d8fb));
            g.fillEllipse (x - radius, midY - radius, radius * 2.0f, radius * 2.0f);
        }
    }

    if (playhead > 0.0)
    {
        const float playX = bounds.getX() + (float) (playhead / duration) * width;
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawLine (playX, bounds.getY(), playX, bounds.getBottom(), 2.0f);
    }
}
