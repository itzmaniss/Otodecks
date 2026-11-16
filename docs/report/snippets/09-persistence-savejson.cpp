bool PersistenceManager::saveJSON(const juce::File& destination, const juce::var& contents)
{
	// EXTENSION: writes through a temporary file so a crash mid-write cannot truncate the original.
	if (! ensureDirectoriesExist())
	{
		DBG("PersistenceManager: could not create " << getDataDirectory().getFullPathName());
		return false;
	}

	juce::TemporaryFile temporary(destination);

	{
		juce::FileOutputStream output(temporary.getFile());

		if (! output.openedOk())
		{
			DBG("PersistenceManager: could not open " << temporary.getFile().getFullPathName());
			return false;
		}

		output.setPosition(0);
		output.truncate();
		output.writeText(juce::JSON::toString(contents), false, false, "\n");
	}

	// Only now does the real file get replaced, and only if the write survived.
	return temporary.overwriteTargetFileWithTemporary();
}
