#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Analysis/PitchDetector.h"
#include "Analysis/OnsetDetector.h"
#include "Analysis/PerformanceCapture.h"
#include "Analysis/SampleSlot.h"
#include "Analysis/PatternRenderer.h"
#include "Analysis/PerformanceSampler.h"
#include <atomic>

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

    void startPerformanceCapture();
    void stopPerformanceCapture();
    bool isPerformanceCapturing() const noexcept { return captureActive.load(); }

    humbucker::PerformanceMode getPerformanceMode() const noexcept { return performanceMode; }
    const humbucker::PerformanceCapture& getPerformanceCapture() const noexcept { return performanceCapture; }

    bool loadSample (const juce::File& file);
    bool hasLoadedSample() const noexcept { return sampleSlot.hasSample(); }
    juce::String getLoadedSampleName() const { return sampleSlot.getName(); }

    void startPreview();
    void stopPreview();
    bool isPreviewing() const noexcept { return performanceSampler.isPreviewActive(); }
    double getPreviewPlayheadSeconds() const noexcept { return performanceSampler.getPreviewPlayheadSeconds(); }

    bool exportPatternAudio (const juce::File& file) const;
    bool exportPatternMidi (const juce::File& file) const;

    float getLastDetectedPitchHz() const noexcept { return pitchDetector.getLastFrequencyHz(); }
    float getLastOnsetStrength() const noexcept { return onsetDetector.getLastOnsetStrength(); }

    humbucker::PatternRenderOptions getRenderOptions() const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void processMelodyCapture (const float* samples, int numSamples, double blockStartTime);
    void processRhythmCapture (const float* samples, int numSamples, double blockStartTime);

    juce::AudioProcessorValueTreeState apvts;
    humbucker::PitchDetector pitchDetector;
    humbucker::OnsetDetector onsetDetector;
    humbucker::PerformanceCapture performanceCapture;
    humbucker::SampleSlot sampleSlot;
    humbucker::PatternRenderer patternRenderer;
    humbucker::PerformanceSampler performanceSampler;

    humbucker::PerformanceMode performanceMode = humbucker::PerformanceMode::melody;
    std::atomic<bool> captureActive { false };
    int64_t captureSampleCounter = 0;
    int lastCapturedMidiNote = -1;
    int unvoicedBlocks = 0;

    std::vector<float> monoScratch;
    double currentSampleRate = 44100.0;
    double currentBpm = 120.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlHumbuckerAudioProcessor)
};
