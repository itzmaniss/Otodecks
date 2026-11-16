juce::File LoopSampler::copyIntoSamplesFolder(const juce::File& file)
{
	// EXTENSION: added samples are copied in, so the set survives the original moving.
	PersistenceManager::ensureDirectoriesExist();

	auto samplesFolder = PersistenceManager::getSamplesDirectory();

	// A sample already living in the folder is left where it is, so re-adding
	// one does not pile up copies.
	if (file.isAChildOf(samplesFolder))
	{
		return file;
	}

	auto destination = samplesFolder.getChildFile(file.getFileName()).getNonexistentSibling();

	return file.copyFileTo(destination) ? destination : file;
}
