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
