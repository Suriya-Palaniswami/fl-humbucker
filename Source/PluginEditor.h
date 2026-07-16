#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/PerformanceTimeline.h"

class FlHumbuckerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit FlHumbuckerAudioProcessorEditor (FlHumbuckerAudioProcessor&);
    ~FlHumbuckerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void toggleCapture();
    void browseForSample();
    void previewPattern();
    void exportAudio();
    void exportMidi();
    void refreshTimeline();

    FlHumbuckerAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Label step1Label;
    juce::Label step2Label;
    juce::Label step3Label;
    juce::Label sampleNameLabel;
    juce::Label eventCountLabel;
    juce::Label statusLabel;

    PerformanceTimeline timeline;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::ComboBox modeBox;
    juce::TextButton recordButton { "Record" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton browseButton { "Choose Sample..." };
    juce::TextButton previewButton { "Preview Pattern" };
    juce::TextButton exportAudioButton { "Export WAV Pattern" };
    juce::TextButton exportMidiButton { "Export MIDI" };

    juce::Slider inputGainSlider;
    juce::Slider onsetSlider;
    juce::Slider pitchConfSlider;
    juce::Slider monitorSlider;
    juce::Slider previewLevelSlider;
    juce::Slider bpmSlider;
    juce::ToggleButton quantizeButton;
    juce::ToggleButton followPitchButton;
    juce::ComboBox gridBox;

    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> onsetAttachment;
    std::unique_ptr<SliderAttachment> pitchConfAttachment;
    std::unique_ptr<SliderAttachment> monitorAttachment;
    std::unique_ptr<SliderAttachment> previewLevelAttachment;
    std::unique_ptr<SliderAttachment> bpmAttachment;
    std::unique_ptr<ButtonAttachment> quantizeAttachment;
    std::unique_ptr<ButtonAttachment> followPitchAttachment;
    std::unique_ptr<ComboAttachment> gridAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlHumbuckerAudioProcessorEditor)
};
