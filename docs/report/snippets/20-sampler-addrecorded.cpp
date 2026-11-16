bool LoopSampler::addRecordedSample(const juce::File& file)
{
	// EXTENSION: a finished recording becomes a sample, so a loop taken off a
	// deck can be triggered straight back over the mix.
	// The recording itself stays in the recordings folder; the sampler works
	// from its own copy, exactly as it does for any other added file.
	if (! addSample(copyIntoSamplesFolder(file)))
	{
		return false;
	}

	save();

	return true;
}
