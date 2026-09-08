#pragma once

#include <JuceHeader.h>
#include "Track.h"

// My code starts here

/**
	The set of tracks the user has added, and the only thing that reads or
	writes library.json. Broadcasts a change whenever the track list moves, so
	the playlist table can repaint without being told directly.

	The library owns its tracks; a deck given one holds a copy, not a pointer.
*/
class MusicLibrary : public juce::ChangeBroadcaster
{
public:
	explicit MusicLibrary(juce::AudioFormatManager& formatManagerToUse);

	/** Skips files already in the library. Returns false if it was a duplicate. */
	bool addTrack(const juce::File& file);
	int addTracks(const juce::Array<juce::File>& files);
	void removeTrack(int index);

	int getNumTracks() const;

	/** Index must be in range - callers get it from the table, which asks first. */
	const Track& getTrack(int index) const;
	void updateTrack(int index, const Track& updated);

	/** Returns -1 when the path is not in the library. */
	int findTrackByPath(const juce::String& path) const;

	/** Row indices whose display name contains the term; empty term means all rows. */
	juce::Array<int> findTracksMatching(const juce::String& searchTerm) const;

	juce::var toVar() const;
	void loadFromVar(const juce::var& source);

	bool save() const;
	void load();

private:
	juce::AudioFormatManager& formatManager;
	juce::Array<Track> tracks;

	static const juce::Identifier tracksKey;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicLibrary)
};

// My code ends here
