#include "PersistenceManager.h"

#include <memory>

// My code starts here

juce::File PersistenceManager::getDataDirectory()
{
	auto root = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

	// On macOS that special location is ~/Library itself, not ~/Library/Application Support.
   #if JUCE_MAC
	root = root.getChildFile("Application Support");
   #endif

	return root.getChildFile(dataFolderName);
}

bool PersistenceManager::ensureDirectoriesExist()
{
	// createDirectory makes parents too, and succeeds on a folder that is already there.
	return getDataDirectory().createDirectory().wasOk()
		&& getSamplesDirectory().createDirectory().wasOk()
		&& getRecordingsDirectory().createDirectory().wasOk();
}

juce::File PersistenceManager::getLibraryFile()
{
	return getDataDirectory().getChildFile(libraryFileName);
}

juce::File PersistenceManager::getSamplerFile()
{
	return getDataDirectory().getChildFile(samplerFileName);
}

juce::File PersistenceManager::getSamplesDirectory()
{
	return getDataDirectory().getChildFile(samplesFolderName);
}

juce::File PersistenceManager::getRecordingsDirectory()
{
	return getDataDirectory().getChildFile(recordingsFolderName);
}

juce::File PersistenceManager::makeRecordingFile(const juce::String& sourceName)
{
	ensureDirectoriesExist();

	auto stamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d %H-%M-%S");
	auto file = getRecordingsDirectory().getChildFile(sourceName + " " + stamp + ".wav");

	// Two recordings started inside the same second would otherwise collide.
	return file.getNonexistentSibling();
}

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

juce::var PersistenceManager::loadJSON(const juce::File& source)
{
	if (! source.existsAsFile())
	{
		return {};
	}

	juce::var parsed;

	if (juce::JSON::parse(source.loadFileAsString(), parsed).failed())
	{
		DBG("PersistenceManager: could not parse " << source.getFullPathName());
		return {};
	}

	return parsed;
}

// My code ends here
