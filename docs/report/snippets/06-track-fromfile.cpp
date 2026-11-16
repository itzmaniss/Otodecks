Track Track::fromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
	Track track;
	track.filePath = file.getFullPathName();
	track.displayName = file.getFileNameWithoutExtension();
	track.dateAdded = juce::Time::getCurrentTime();

	// A file the format manager cannot read still becomes a track, so the user
	// sees it in the table and can work out why it will not play.
	std::unique_ptr<juce::AudioFormatReader> reader { formatManager.createReaderFor(file) };

	if (reader != nullptr && reader->sampleRate > 0.0)
	{
		track.lengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
	}

	return track;
}
