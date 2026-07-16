#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace humbucker
{

/** Holds one user-selected sample for pattern playback and export. */
class SampleSlot
{
public:
    bool loadFromFile (const juce::File& file);
    void clear();

    bool hasSample() const noexcept { return ! monoData.empty(); }
    const juce::String& getName() const noexcept { return sampleName; }
    const juce::File& getFile() const noexcept { return sampleFile; }
    const std::vector<float>& getMonoData() const noexcept { return monoData; }
    double getSampleRate() const noexcept { return sampleRate; }

    void setRootMidiNote (int note) noexcept { rootMidiNote = juce::jlimit (0, 127, note); }
    int getRootMidiNote() const noexcept { return rootMidiNote; }

private:
    juce::File sampleFile;
    juce::String sampleName;
    std::vector<float> monoData;
    double sampleRate = 44100.0;
    int rootMidiNote = 60;
};

} // namespace humbucker
