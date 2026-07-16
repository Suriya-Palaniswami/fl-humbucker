#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Analysis/PitchDetector.h"
#include "Analysis/OnsetDetector.h"
#include "Analysis/BeatboxClassifier.h"
#include "Analysis/SampleCapture.h"
#include "Analysis/MidiEmitter.h"
#include "Analysis/PatternRecorder.h"

class FlHumbuckerAudioProcessor : public juce::AudioProcessor
{
public:
    FlHumbuckerAudioProcessor();
    ~FlHumbuckerAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getApvts() noexcept { return apvts; }

    humbucker::OutputMode getOutputMode() const noexcept { return outputMode; }
    bool isPatternRecording() const noexcept { return patternRecorder.isRecording(); }
    const humbucker::PatternRecorder& getPatternRecorder() const noexcept { return patternRecorder; }
    const humbucker::SampleCapture& getSampleCapture() const noexcept { return sampleCapture; }

    float getLastDetectedPitchHz() const noexcept { return pitchDetector.getLastFrequencyHz(); }
    float getLastOnsetStrength() const noexcept { return onsetDetector.getLastOnsetStrength(); }
    humbucker::DrumClass getLastDrumClass() const noexcept { return lastDrumClass; }

    bool exportRecordedPattern (const juce::File& file) const;
    bool exportCapturedSlice (int sliceIndex, const juce::File& file) const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    humbucker::PitchDetector pitchDetector;
    humbucker::OnsetDetector onsetDetector;
    humbucker::BeatboxClassifier beatboxClassifier;
    humbucker::SampleCapture sampleCapture;
    humbucker::MidiEmitter midiEmitter;
    humbucker::PatternRecorder patternRecorder;

    humbucker::OutputMode outputMode = humbucker::OutputMode::humToMidi;
    humbucker::DrumClass lastDrumClass = humbucker::DrumClass::unknown;

    std::vector<float> monoScratch;
    double currentBpm = 120.0;
    double currentPpq = 0.0;
    bool hostIsPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlHumbuckerAudioProcessor)
};
