#include "SampleSlot.h"

namespace humbucker
{

bool SampleSlot::loadFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    const int numSamples = (int) reader->lengthInSamples;
    if (numSamples <= 0)
        return false;

    juce::AudioBuffer<float> buffer ((int) reader->numChannels, numSamples);
    reader->read (&buffer, 0, numSamples, 0, true, true);

    monoData.resize ((size_t) numSamples);
    sampleRate = reader->sampleRate;
    sampleFile = file;
    sampleName = file.getFileName();

    if (buffer.getNumChannels() == 1)
    {
        std::copy (buffer.getReadPointer (0),
                   buffer.getReadPointer (0) + numSamples,
                   monoData.begin());
    }
    else
    {
        const float* left = buffer.getReadPointer (0);
        const float* right = buffer.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
            monoData[(size_t) i] = 0.5f * (left[i] + right[i]);
    }

    return true;
}

void SampleSlot::clear()
{
    monoData.clear();
    sampleName.clear();
    sampleFile = juce::File();
    sampleRate = 44100.0;
}

} // namespace humbucker
