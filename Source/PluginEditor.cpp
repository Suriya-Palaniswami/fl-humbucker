#include "PluginEditor.h"

namespace
{
void setupCompactSlider (juce::Slider& slider, const juce::String& name, const juce::String& suffix = {})
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 20);
    slider.setName (name);
    if (suffix.isNotEmpty())
        slider.setTextValueSuffix (suffix);
}
} // namespace

FlHumbuckerAudioProcessorEditor::FlHumbuckerAudioProcessorEditor (FlHumbuckerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (640, 560);

    titleLabel.setText ("FL Humbucker", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    statusLabel.setText ("Perform -> Pick Sound -> Get Pattern", juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.85f));
    addAndMakeVisible (statusLabel);

    for (auto* label : { &step1Label, &step2Label, &step3Label })
    {
        label->setFont (juce::FontOptions (15.0f, juce::Font::bold));
        label->setColour (juce::Label::textColourId, juce::Colour (0xff53d8fb));
        addAndMakeVisible (label);
    }

    step1Label.setText ("1. Perform", juce::dontSendNotification);
    step2Label.setText ("2. Pick Sound", juce::dontSendNotification);
    step3Label.setText ("3. Get Pattern", juce::dontSendNotification);

    sampleNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    sampleNameLabel.setText ("No sample loaded", juce::dontSendNotification);
    addAndMakeVisible (sampleNameLabel);

    eventCountLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
    eventCountLabel.setText ("0 events captured", juce::dontSendNotification);
    addAndMakeVisible (eventCountLabel);

    modeBox.addItemList ({ "Melody (hum)", "Rhythm (beatbox)" }, 1);
    addAndMakeVisible (modeBox);

    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe94560));
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff533483));
    for (auto* button : { &recordButton, &stopButton, &browseButton, &previewButton,
                          &exportAudioButton, &exportMidiButton })
        addAndMakeVisible (button);

    recordButton.onClick = [this] { toggleCapture(); };
    stopButton.onClick = [this]
    {
        if (processor.isPerformanceCapturing())
            toggleCapture();
        processor.stopPreview();
    };
    browseButton.onClick = [this] { browseForSample(); };
    previewButton.onClick = [this] { previewPattern(); };
    exportAudioButton.onClick = [this] { exportAudio(); };
    exportMidiButton.onClick = [this] { exportMidi(); };

    addAndMakeVisible (timeline);

    setupCompactSlider (inputGainSlider, "Input", " dB");
    setupCompactSlider (onsetSlider, "Sensitivity");
    setupCompactSlider (pitchConfSlider, "Pitch");
    setupCompactSlider (monitorSlider, "Monitor");
    setupCompactSlider (previewLevelSlider, "Preview");
    setupCompactSlider (bpmSlider, "BPM");

    for (auto* slider : { &inputGainSlider, &onsetSlider, &pitchConfSlider,
                          &monitorSlider, &previewLevelSlider, &bpmSlider })
        addAndMakeVisible (slider);

    quantizeButton.setButtonText ("Quantize");
    followPitchButton.setButtonText ("Follow my pitch");
    addAndMakeVisible (quantizeButton);
    addAndMakeVisible (followPitchButton);

    gridBox.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (gridBox);

    auto& apvts = processor.getApvts();
    modeAttachment = std::make_unique<ComboAttachment> (apvts, "mode", modeBox);
    inputGainAttachment = std::make_unique<SliderAttachment> (apvts, "inputGain", inputGainSlider);
    onsetAttachment = std::make_unique<SliderAttachment> (apvts, "onsetSensitivity", onsetSlider);
    pitchConfAttachment = std::make_unique<SliderAttachment> (apvts, "pitchConfidence", pitchConfSlider);
    monitorAttachment = std::make_unique<SliderAttachment> (apvts, "monitorLevel", monitorSlider);
    previewLevelAttachment = std::make_unique<SliderAttachment> (apvts, "previewLevel", previewLevelSlider);
    bpmAttachment = std::make_unique<SliderAttachment> (apvts, "exportBpm", bpmSlider);
    quantizeAttachment = std::make_unique<ButtonAttachment> (apvts, "quantize", quantizeButton);
    followPitchAttachment = std::make_unique<ButtonAttachment> (apvts, "followPitch", followPitchButton);
    gridAttachment = std::make_unique<ComboAttachment> (apvts, "quantizeGrid", gridBox);

    refreshTimeline();
    startTimerHz (20);
}

void FlHumbuckerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    auto drawSection = [&g] (juce::Rectangle<int> area)
    {
        g.setColour (juce::Colour (0xff16213e));
        g.fillRoundedRectangle (area.toFloat(), 8.0f);
        g.setColour (juce::Colour (0xff0f3460));
        g.drawRoundedRectangle (area.toFloat(), 8.0f, 1.0f);
    };

    auto bounds = getLocalBounds().reduced (14);
    bounds.removeFromTop (58);
    auto section1 = bounds.removeFromTop (150);
    bounds.removeFromTop (8);
    auto section2 = bounds.removeFromTop (88);
    bounds.removeFromTop (8);
    auto section3 = bounds.removeFromTop (108);

    drawSection (section1);
    drawSection (section2);
    drawSection (section3);
}

void FlHumbuckerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (14);
    auto header = bounds.removeFromTop (58);
    titleLabel.setBounds (header.removeFromTop (30));
    statusLabel.setBounds (header);

    auto section1 = bounds.removeFromTop (150);
    bounds.removeFromTop (8);
    auto section2 = bounds.removeFromTop (88);
    bounds.removeFromTop (8);
    auto section3 = bounds.removeFromTop (108);
    bounds.removeFromTop (8);
    auto tweaks = bounds;

    auto s1 = section1.reduced (12);
    step1Label.setBounds (s1.removeFromTop (22));
    auto row1 = s1.removeFromTop (34);
    modeBox.setBounds (row1.removeFromLeft (170));
    row1.removeFromLeft (8);
    recordButton.setBounds (row1.removeFromLeft (90));
    row1.removeFromLeft (8);
    stopButton.setBounds (row1.removeFromLeft (70));
    row1.removeFromLeft (12);
    eventCountLabel.setBounds (row1);
    s1.removeFromTop (6);
    timeline.setBounds (s1);

    auto s2 = section2.reduced (12);
    step2Label.setBounds (s2.removeFromTop (22));
    auto row2 = s2.removeFromTop (34);
    browseButton.setBounds (row2.removeFromLeft (150));
    row2.removeFromLeft (10);
    sampleNameLabel.setBounds (row2);
    auto row2b = s2.removeFromTop (28);
    followPitchButton.setBounds (row2b.removeFromLeft (160));

    auto s3 = section3.reduced (12);
    step3Label.setBounds (s3.removeFromTop (22));
    auto row3 = s3.removeFromTop (34);
    previewButton.setBounds (row3.removeFromLeft (130));
    row3.removeFromLeft (8);
    exportAudioButton.setBounds (row3.removeFromLeft (150));
    row3.removeFromLeft (8);
    exportMidiButton.setBounds (row3.removeFromLeft (110));
    auto row3b = s3.removeFromTop (30);
    quantizeButton.setBounds (row3b.removeFromLeft (90));
    row3b.removeFromLeft (8);
    gridBox.setBounds (row3b.removeFromLeft (72));
    row3b.removeFromLeft (8);
    bpmSlider.setBounds (row3b);

    auto placeSlider = [] (juce::Rectangle<int>& area, juce::Slider& slider, const juce::String& label)
    {
        auto row = area.removeFromTop (24);
        row.removeFromLeft (8);
        slider.setName (label);
        slider.setBounds (row);
    };

    placeSlider (tweaks, inputGainSlider, "Input");
    placeSlider (tweaks, onsetSlider, "Sensitivity");
    placeSlider (tweaks, pitchConfSlider, "Pitch");
    placeSlider (tweaks, monitorSlider, "Monitor");
    placeSlider (tweaks, previewLevelSlider, "Preview");
}

void FlHumbuckerAudioProcessorEditor::timerCallback()
{
    const auto& capture = processor.getPerformanceCapture();
    const int count = capture.getEventCount();
    eventCountLabel.setText (juce::String (count) + " events captured", juce::dontSendNotification);

    if (processor.isPerformanceCapturing())
    {
        recordButton.setButtonText ("Recording...");
        recordButton.setEnabled (false);
        statusLabel.setText ("Listening — hum or beatbox your idea", juce::dontSendNotification);
    }
    else
    {
        recordButton.setButtonText ("Record");
        recordButton.setEnabled (true);
        statusLabel.setText (count > 0 ? "Performance captured — choose a sample and create your pattern"
                                       : "Perform -> Pick Sound -> Get Pattern",
                             juce::dontSendNotification);
    }

    sampleNameLabel.setText (processor.hasLoadedSample()
                                 ? processor.getLoadedSampleName()
                                 : "No sample loaded — click Choose Sample",
                             juce::dontSendNotification);

    timeline.setPlayheadSeconds (processor.getPreviewPlayheadSeconds());
    refreshTimeline();
}

void FlHumbuckerAudioProcessorEditor::toggleCapture()
{
    if (processor.isPerformanceCapturing())
    {
        processor.stopPerformanceCapture();
        refreshTimeline();
    }
    else
    {
        processor.stopPreview();
        processor.startPerformanceCapture();
    }
}

void FlHumbuckerAudioProcessorEditor::browseForSample()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Choose a sample", juce::File(), "*.wav;*.aif;*.aiff;*.mp3;*.flac");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, chooser] (const juce::FileChooser& fc)
                          {
                              const auto file = fc.getResult();
                              if (file != juce::File())
                                  processor.loadSample (file);
                          });
}

void FlHumbuckerAudioProcessorEditor::previewPattern()
{
    if (! processor.hasLoadedSample())
    {
        browseForSample();
        return;
    }

    processor.startPreview();
}

void FlHumbuckerAudioProcessorEditor::exportAudio()
{
    if (! processor.hasLoadedSample())
    {
        browseForSample();
        return;
    }

    auto chooser = std::make_shared<juce::FileChooser> ("Export pattern audio", juce::File(), "*.wav");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, chooser] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file != juce::File())
            processor.exportPatternAudio (file.withFileExtension (".wav"));
    });
}

void FlHumbuckerAudioProcessorEditor::exportMidi()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Export MIDI pattern", juce::File(), "*.mid");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode, [this, chooser] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file != juce::File())
            processor.exportPatternMidi (file.withFileExtension (".mid"));
    });
}

void FlHumbuckerAudioProcessorEditor::refreshTimeline()
{
    const auto& capture = processor.getPerformanceCapture();
    timeline.setEvents (capture.getEvents(),
                        processor.getPerformanceMode(),
                        juce::jmax (0.5, capture.getDurationSeconds() + 0.35));
}
