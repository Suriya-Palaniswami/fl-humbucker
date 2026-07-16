#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

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
    void exportPattern();
    void exportLastSlice();

    FlHumbuckerAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label pitchLabel;
    juce::Label onsetLabel;
    juce::Label drumLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::ComboBox modeBox;
    juce::Slider inputGainSlider;
    juce::Slider onsetSlider;
    juce::Slider pitchConfSlider;
    juce::Slider monitorSlider;
    juce::ToggleButton quantizeButton;
    juce::ComboBox gridBox;
    juce::ToggleButton recordButton;
    juce::TextButton exportPatternButton { "Export MIDI" };
    juce::TextButton exportSliceButton { "Export Last Slice" };

    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> onsetAttachment;
    std::unique_ptr<SliderAttachment> pitchConfAttachment;
    std::unique_ptr<SliderAttachment> monitorAttachment;
    std::unique_ptr<ButtonAttachment> quantizeAttachment;
    std::unique_ptr<ComboAttachment> gridAttachment;
    std::unique_ptr<ButtonAttachment> recordAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlHumbuckerAudioProcessorEditor)
};
