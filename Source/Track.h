#pragma once

#include <JuceHeader.h>

// My code starts here

/**
	One entry in the music library. Plain data - it owns no audio resources and
	is safe to copy around between the library, the playlist table and the decks.
*/
class Track
{
public:
	Track() = default;

	/** Builds a track from a file on disk, reading its duration via the reader. */
	static Track fromFile(const juce::File& file, juce::AudioFormatManager& formatManager);

	juce::var toVar() const;
	static Track fromVar(const juce::var& source);

	bool hasCueAt(int index) const;

	juce::File getFile() const;
	bool exists() const;

	/** How many hot cues a track can hold. The GUI lays out one button per cue. */
	static constexpr int maxCuePoints = 4;

	/** Sentinel stored in cuePoints for a slot that has not been set. */
	static constexpr double noCue = -1.0;

	/** Stored in bpm until an analysis or a tap has produced a real value. */
	static constexpr double unknownBPM = 0.0;

	juce::String filePath;
	juce::String displayName;
	double lengthInSeconds = 0.0;
	double bpm = unknownBPM;
	juce::Time dateAdded;

	/** Positions in seconds from the start of the file; noCue means unset. */
	juce::Array<double> cuePoints { noCue, noCue, noCue, noCue };
};

// My code ends here
