#include "AudioRecorder.h"
#include "DJAudioPlayer.h"
#include "PersistenceManager.h"

#include <memory>

// My code starts here

AudioRecorder::AudioRecorder()
{
	backgroundThread.startThread();
}

AudioRecorder::~AudioRecorder()
{
	// The owner may already be part-destroyed by now, so nothing is announced.
	onRecordingFinished = nullptr;
	stop();
}

bool AudioRecorder::startRecordingFromDeck(DJAudioPlayer* player, double sampleRate,
                                           int numChannels)
{
	if (player == nullptr || sampleRate <= 0.0)
	{
		return false;
	}

	// beginWriting stops any recording in progress, and stopping disarms the
	// deck - so the deck is armed only once the new writer is open.
	if (! beginWriting(PersistenceManager::makeRecordingFile("deck"), sampleRate,
	                   numChannels, Source::deck))
	{
		return false;
	}

	armedPlayer = player;

	return true;
}

bool AudioRecorder::startRecordingFromMic()
{
	if (deviceSampleRate <= 0.0)
	{
		// No device has started, so there is nothing to record from yet.
		return false;
	}

	return beginWriting(PersistenceManager::makeRecordingFile("mic"), deviceSampleRate,
	                    1, Source::microphone);
}

bool AudioRecorder::beginWriting(const juce::File& file, double sampleRate, int numChannels,
                                 Source newSource)
{
	stop();

	file.deleteFile();

	std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());

	if (stream == nullptr)
	{
		DBG("AudioRecorder: could not open " << file.getFullPathName());
		return false;
	}

	juce::WavAudioFormat wavFormat;
	std::unique_ptr<juce::AudioFormatWriter> writer(
		wavFormat.createWriterFor(stream.get(), sampleRate, static_cast<unsigned int>(numChannels),
		                          bitsPerSample, {}, 0));

	if (writer == nullptr)
	{
		DBG("AudioRecorder: could not create a writer for " << file.getFullPathName());
		return false;
	}

	// The writer now owns the stream, so the unique_ptr must not free it.
	stream.release();

	outputFile = file;

	auto newWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
		writer.release(), backgroundThread, writeBufferSize);

	source = newSource;
	droppedBlocks = 0;

	// Everything above allocates, so it happens before the audio thread can see
	// the writer. Only the pointer swap is done under the lock.
	{
		const juce::SpinLock::ScopedLockType lock(writerLock);
		threadedWriter = std::move(newWriter);
		activeWriter = threadedWriter.get();
	}

	return true;
}

void AudioRecorder::stop()
{
	// Whether this call is ending a real recording or is just a no-op, decided
	// before the writer is cleared so the callback fires exactly once.
	bool wasRecording = threadedWriter != nullptr;

	{
		const juce::SpinLock::ScopedLockType lock(writerLock);
		activeWriter = nullptr;
	}

	// Deleting the writer flushes and closes the file; the audio thread can no
	// longer reach it, so this is safe to do off the lock.
	threadedWriter.reset();

	armedPlayer = nullptr;
	source = Source::none;

	// Only now is the file complete on disk and safe to hand to anything else.
	if (wasRecording && onRecordingFinished != nullptr && outputFile.existsAsFile())
	{
		onRecordingFinished(outputFile);
	}
}

bool AudioRecorder::isRecording() const
{
	return source.load() != Source::none;
}

AudioRecorder::Source AudioRecorder::getSource() const
{
	return source.load();
}

juce::File AudioRecorder::getOutputFile() const
{
	return outputFile;
}

int AudioRecorder::getDroppedBlockCount() const
{
	// EXTENSION: reports blocks the writer could not keep up with, rather than hiding gaps.
	return droppedBlocks.load();
}

bool AudioRecorder::isRecordingDeck(const DJAudioPlayer* player) const
{
	return source.load() == Source::deck && armedPlayer.load() == player;
}

double AudioRecorder::getDeviceSampleRate() const
{
	return deviceSampleRate;
}

void AudioRecorder::captureDeckBlock(const DJAudioPlayer* player,
                                     const juce::AudioBuffer<float>& buffer,
                                     int startSample, int numSamples)
{
	if (source.load() != Source::deck || player != armedPlayer.load())
	{
		return;
	}

	const juce::SpinLock::ScopedTryLockType lock(writerLock);

	// A try-lock: if the message thread is swapping writers this block is
	// dropped rather than the audio thread stalling on disk work.
	if (! lock.isLocked() || activeWriter == nullptr)
	{
		++droppedBlocks;
		return;
	}

	const float* channels[2] = { nullptr, nullptr };
	auto numChannels = juce::jmin(buffer.getNumChannels(), 2);

	for (int i = 0; i < numChannels; ++i)
	{
		channels[i] = buffer.getReadPointer(i, startSample);
	}

	// write() refuses when its FIFO is full, which is how a disk that cannot
	// keep up shows itself. Counting it is all that is safe to do here.
	if (! activeWriter->write(channels, numSamples))
	{
		++droppedBlocks;
	}
}

void AudioRecorder::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
	deviceSampleRate = device != nullptr ? device->getCurrentSampleRate() : 0.0;
}

void AudioRecorder::audioDeviceStopped()
{
	deviceSampleRate = 0.0;
}

void AudioRecorder::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                     int numInputChannels,
                                                     float* const* outputChannelData,
                                                     int numOutputChannels,
                                                     int numSamples,
                                                     const juce::AudioIODeviceCallbackContext& context)
{
	juce::ignoreUnused(context);

	if (source.load() == Source::microphone && numInputChannels > 0)
	{
		const juce::SpinLock::ScopedTryLockType lock(writerLock);

		if (lock.isLocked() && activeWriter != nullptr)
		{
			if (! activeWriter->write(inputChannelData, numSamples))
			{
				++droppedBlocks;
			}
		}
		else
		{
			++droppedBlocks;
		}
	}

	// This recorder is a secondary device callback; the audio it is handed is
	// added to the device output, so it must leave silence behind.
	for (int i = 0; i < numOutputChannels; ++i)
	{
		if (outputChannelData[i] != nullptr)
		{
			juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
		}
	}

	juce::ignoreUnused(inputChannelData, numInputChannels);
}

// My code ends here
