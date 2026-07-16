#include "SampleCapture.h"

namespace humbucker
{

void SampleCapture::prepare (double newSampleRate, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = newSampleRate;
    ringSize = (int) std::round (sampleRate * 3.0);
    ringBuffer.assign ((size_t) ringSize, 0.0f);
    captureBuffer.reserve ((size_t) (preRollSamples + postRollSamples + 4096));
    reset();
}

void SampleCapture::reset()
{
    writeIndex = 0;
    ringWritePos = 0;
    capturing = false;
    samplesRemaining = 0;
    captureBuffer.clear();
    std::fill (ringBuffer.begin(), ringBuffer.end(), 0.0f);

    for (auto& slice : slices)
    {
        slice.audio.clear();
        slice.hasAudio = false;
    }
}

void SampleCapture::feedAudio (const float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        ringBuffer[(size_t) ringWritePos] = samples[i];
        ringWritePos = (ringWritePos + 1) % ringSize;

        if (capturing && samplesRemaining > 0)
        {
            captureBuffer.push_back (samples[i]);
            --samplesRemaining;

            if (samplesRemaining == 0)
                finalizePendingSlice();
        }
    }
}

void SampleCapture::markOnset (double onsetTimeSec, int midiNote)
{
    juce::ignoreUnused (onsetTimeSec);

    if (capturing)
        finalizePendingSlice();

    captureMidiNote = midiNote;
    captureBuffer.clear();
    captureBuffer.reserve ((size_t) (preRollSamples + postRollSamples));

    const int startPos = (ringWritePos - preRollSamples + ringSize) % ringSize;
    captureStartRingPos = startPos;

    for (int i = 0; i < preRollSamples; ++i)
    {
        const int pos = (startPos + i) % ringSize;
        captureBuffer.push_back (ringBuffer[(size_t) pos]);
    }

    capturing = true;
    samplesRemaining = postRollSamples;
}

void SampleCapture::finalizePendingSlice()
{
    if (captureBuffer.empty())
    {
        capturing = false;
        samplesRemaining = 0;
        return;
    }

    auto& slice = slices[(size_t) writeIndex % maxSlices];
    slice.audio = captureBuffer;
    slice.midiNote = captureMidiNote;
    slice.startTimeSec = 0.0;
    slice.hasAudio = true;

    writeIndex = (writeIndex + 1) % maxSlices;
    capturing = false;
    samplesRemaining = 0;
    captureBuffer.clear();
}

bool SampleCapture::exportSliceToWav (int index, const juce::File& file) const
{
    const auto& slice = getSlice (index);
    if (! slice.hasAudio || slice.audio.empty())
        return false;

    juce::WavAudioFormat format;
    std::unique_ptr<juce::FileOutputStream> stream (file.createOutputStream());
    if (stream == nullptr)
        return false;

    std::unique_ptr<juce::AudioFormatWriter> writer (
        format.createWriterFor (stream.get(), sampleRate, 1, 16, {}, 0));

    if (writer == nullptr)
        return false;

    stream.release();
    writer->writeFromFloatArrays (&slice.audio[0], 1, (int) slice.audio.size());
    return true;
}

} // namespace humbucker
