#include "PluginEditor.h"

namespace
{
juce::Slider makeRotary (const juce::String& name)
{
    juce::Slider slider;
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    slider.setName (name);
    return slider;
}
} // namespace

FlHumbuckerAudioProcessorEditor::FlHumbuckerAudioProcessorEditor (FlHumbuckerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (520, 420);

    titleLabel.setText ("FL Humbucker", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    statusLabel.setText ("Mic in -> MIDI / samples", juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    for (auto* label : { &pitchLabel, &onsetLabel, &drumLabel })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
    }

    modeBox.addItemList ({ "Hum to MIDI", "Beatbox to Drums", "Capture Samples" }, 1);
    addAndMakeVisible (modeBox);

    inputGainSlider = makeRotary ("Input Gain");
    onsetSlider = makeRotary ("Onset");
    pitchConfSlider = makeRotary ("Pitch Conf");
    monitorSlider = makeRotary ("Monitor");

    for (auto* slider : { &inputGainSlider, &onsetSlider, &pitchConfSlider, &monitorSlider })
        addAndMakeVisible (slider);

    quantizeButton.setButtonText ("Quantize");
    recordButton.setButtonText ("Record Pattern");
    addAndMakeVisible (quantizeButton);
    addAndMakeVisible (recordButton);

    gridBox.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (gridBox);

    exportPatternButton.onClick = [this] { exportPattern(); };
    exportSliceButton.onClick = [this] { exportLastSlice(); };
    addAndMakeVisible (exportPatternButton);
    addAndMakeVisible (exportSliceButton);

    auto& apvts = processor.getApvts();
    modeAttachment = std::make_unique<ComboAttachment> (apvts, "mode", modeBox);
    inputGainAttachment = std::make_unique<SliderAttachment> (apvts, "inputGain", inputGainSlider);
    onsetAttachment = std::make_unique<SliderAttachment> (apvts, "onsetSensitivity", onsetSlider);
    pitchConfAttachment = std::make_unique<SliderAttachment> (apvts, "pitchConfidence", pitchConfSlider);
    monitorAttachment = std::make_unique<SliderAttachment> (apvts, "monitorLevel", monitorSlider);
    quantizeAttachment = std::make_unique<ButtonAttachment> (apvts, "quantize", quantizeButton);
    gridAttachment = std::make_unique<ComboAttachment> (apvts, "quantizeGrid", gridBox);
    recordAttachment = std::make_unique<ButtonAttachment> (apvts, "recordPattern", recordButton);

    startTimerHz (15);
}

void FlHumbuckerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    g.setColour (juce::Colour (0xffe94560));
    g.fillRoundedRectangle (10.0f, 10.0f, (float) getWidth() - 20.0f, 56.0f, 8.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Turn beatboxing and humming into MIDI patterns and samples",
                  24, 34, getWidth() - 40, 24, juce::Justification::centredLeft);
}

void FlHumbuckerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    auto header = area.removeFromTop (72);
    titleLabel.setBounds (header.removeFromTop (28).reduced (8, 0));
    statusLabel.setBounds (header.reduced (8, 0));

    auto controls = area.removeFromTop (220);
    auto topRow = controls.removeFromTop (40);
    modeBox.setBounds (topRow.removeFromLeft (180));
    topRow.removeFromLeft (8);
    quantizeButton.setBounds (topRow.removeFromLeft (90));
    topRow.removeFromLeft (8);
    gridBox.setBounds (topRow.removeFromLeft (80));
    topRow.removeFromLeft (8);
    recordButton.setBounds (topRow);

    auto sliderRow = controls.removeFromTop (150);
    const int w = sliderRow.getWidth() / 4;
    inputGainSlider.setBounds (sliderRow.removeFromLeft (w).reduced (6));
    onsetSlider.setBounds (sliderRow.removeFromLeft (w).reduced (6));
    pitchConfSlider.setBounds (sliderRow.removeFromLeft (w).reduced (6));
    monitorSlider.setBounds (sliderRow.reduced (6));

    auto bottom = area;
    pitchLabel.setBounds (bottom.removeFromTop (22));
    onsetLabel.setBounds (bottom.removeFromTop (22));
    drumLabel.setBounds (bottom.removeFromTop (22));
    bottom.removeFromTop (8);

    auto buttons = bottom.removeFromTop (32);
    exportPatternButton.setBounds (buttons.removeFromLeft (140));
    buttons.removeFromLeft (8);
    exportSliceButton.setBounds (buttons.removeFromLeft (160));
}

void FlHumbuckerAudioProcessorEditor::timerCallback()
{
    pitchLabel.setText ("Pitch: " + juce::String (processor.getLastDetectedPitchHz(), 1) + " Hz",
                        juce::dontSendNotification);
    onsetLabel.setText ("Onset strength: " + juce::String (processor.getLastOnsetStrength(), 2),
                        juce::dontSendNotification);
    drumLabel.setText ("Last drum: " + juce::String (humbucker::BeatboxClassifier::drumClassToName (processor.getLastDrumClass())),
                       juce::dontSendNotification);
}

void FlHumbuckerAudioProcessorEditor::exportPattern()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Export MIDI pattern", juce::File(), "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, chooser] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file != juce::File())
            processor.exportRecordedPattern (file.withFileExtension (".mid"));
    });
}

void FlHumbuckerAudioProcessorEditor::exportLastSlice()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Export captured slice", juce::File(), "*.wav");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, chooser] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file != juce::File())
        {
            const int idx = (processor.getSampleCapture().getWriteIndex() + humbucker::SampleCapture::maxSlices - 1)
                          % humbucker::SampleCapture::maxSlices;
            processor.exportCapturedSlice (idx, file.withFileExtension (".wav"));
        }
    });
}
