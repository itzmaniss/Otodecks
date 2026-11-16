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
