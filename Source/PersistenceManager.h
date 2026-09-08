#pragma once

#include <JuceHeader.h>

// My code starts here

/**
	Owns every decision about where application data lives on disk and what
	format it is written in. Nothing else in the app should build a data path
	or parse a data file itself.

	All methods are static - there is no state worth keeping, and callers reach
	for these from constructors before any manager instance would exist.
*/
class PersistenceManager
{
public:
	/** Application support folder for this app; not guaranteed to exist yet. */
	static juce::File getDataDirectory();

	/** Creates the data and samples folders. Returns false if the disk refused. */
	static bool ensureDirectoriesExist();

	static juce::File getLibraryFile();
	static juce::File getSamplerFile();
	static juce::File getSamplesDirectory();
	static juce::File getRecordingsDirectory();

	/** A dated, non-clashing wav path inside the recordings folder. */
	static juce::File makeRecordingFile(const juce::String& sourceName);

	/** Writes via a temporary file, so a crash mid-write cannot truncate the original. */
	static bool saveJSON(const juce::File& destination, const juce::var& contents);

	/** Returns a void var if the file is missing or unparseable - both are normal on first run. */
	static juce::var loadJSON(const juce::File& source);

private:
	static constexpr const char* dataFolderName = "Otodecks";
	static constexpr const char* samplesFolderName = "samples";
	static constexpr const char* recordingsFolderName = "recordings";
	static constexpr const char* libraryFileName = "library.json";
	static constexpr const char* samplerFileName = "sampler.json";
};

// My code ends here
