#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "MusicLibrary.h"

// My code starts here

/**
	A row of hot cue buttons for one deck.

	An empty slot stores the playhead when clicked; a set slot jumps to its
	position. Alt-clicking a set slot clears it, and CLEAR ALL empties the row.

	Cue positions belong to the library track, not to the deck, so this writes
	them straight back through the library and saves. It refers to the track by
	library index rather than holding a `Track*`: the library keeps its tracks in
	an Array, which reallocates when tracks are added, and a stored pointer would
	dangle the moment the user added a file.
*/
class HotCueComponent : public juce::Component,
                        public juce::Button::Listener
{
public:
	HotCueComponent(MusicLibrary& libraryToUse);

	void resized() override;
	void buttonClicked(juce::Button* button) override;

	/** Borrowed; the deck owns the player and outlives this component. */
	void setPlayer(DJAudioPlayer* playerToUse);

	/** Pass -1 when the deck holds no library track; the buttons go inactive. */
	void setLoadedTrack(int libraryIndex);

	void setCue(int cueIndex);
	void jumpToCue(int cueIndex);
	void clearCue(int cueIndex);
	void clearAllCues();

	void updateButtonStates();

private:
	/** Reads the deck's track from the library, or nullptr when none is loaded. */
	const Track* loadedTrack() const;

	/** Applies a changed cue set to the library and writes library.json. */
	void commitCues(const juce::Array<double>& cues);

	static constexpr int buttonGap = 2;

	MusicLibrary& library;
	DJAudioPlayer* player = nullptr;

	/** Index into the library of the track on this deck; -1 when there is none. */
	int trackIndex = -1;

	juce::OwnedArray<juce::TextButton> cueButtons;
	juce::TextButton clearAllButton { "CLEAR ALL" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HotCueComponent)
};

// My code ends here
