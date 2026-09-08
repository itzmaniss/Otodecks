#pragma once

#include <JuceHeader.h>
#include "MusicLibrary.h"
#include "BPMAnalyser.h"

#include <functional>

// My code starts here

/**
	The library's table view: add files, search them, remove them, and send one
	to either deck. Owns no track data - it reads the library and asks it to
	change, then repaints when the library says it has.
*/
class PlaylistComponent : public juce::Component,
                          public juce::TableListBoxModel,
                          public juce::Button::Listener,
                          public juce::TextEditor::Listener,
                          public juce::ChangeListener,
                          public juce::FileDragAndDropTarget
{
public:
	PlaylistComponent(MusicLibrary& libraryToUse, BPMAnalyser& analyserToUse);
	~PlaylistComponent() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	/** Set by the owner to route a chosen track to a deck. Deck numbers are 1 and 2. */
	std::function<void(const Track&, int deckNumber)> onLoadTrackToDeck;

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height,
	                        bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height,
	               bool rowIsSelected) override;
	void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;

	void buttonClicked(juce::Button* button) override;
	void textEditorTextChanged(juce::TextEditor& editor) override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	bool isInterestedInFileDrag(const juce::StringArray& files) override;
	void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
	/** Turns a table row into the library index behind it, or -1 if there is none. */
	int libraryIndexForRow(int rowNumber) const;

	int selectedLibraryIndex() const;
	void loadSelectedTrackToDeck(int deckNumber);
	void rebuildVisibleRows();
	void addFilesToLibrary(const juce::Array<juce::File>& files);

	/** Analyses the selected track and stores the result on it. */
	void analyseSelectedTrack();

	static juce::String formatDuration(double seconds);

	enum ColumnId
	{
		titleColumn = 1,
		durationColumn,
		bpmColumn
	};

	static constexpr int rowHeight = 22;
	static constexpr int controlHeight = 26;
	static constexpr int padding = 4;
	static constexpr int buttonCount = 5;

	MusicLibrary& library;
	BPMAnalyser& analyser;
	juce::TableListBox tableComponent;

	/** Library indices currently shown, in table order - the search filter lives here. */
	juce::Array<int> visibleRows;

	juce::TextEditor searchBox;
	juce::TextButton addTracksButton { "ADD TRACKS" };
	juce::TextButton removeTrackButton { "REMOVE" };
	juce::TextButton loadToDeckOneButton { "LOAD LEFT" };
	juce::TextButton loadToDeckTwoButton { "LOAD RIGHT" };
	juce::TextButton analyseBPMButton { "BPM" };

	juce::FileChooser fileChooser { "Select audio files to add to the library..." };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};

// My code ends here
