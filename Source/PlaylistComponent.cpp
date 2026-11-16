#include "PlaylistComponent.h"

// My code starts here

PlaylistComponent::PlaylistComponent(MusicLibrary& libraryToUse, BPMAnalyser& analyserToUse)
	: library(libraryToUse),
	  analyser(analyserToUse)
{
	tableComponent.setModel(this);
	tableComponent.setRowHeight(rowHeight);
	tableComponent.getHeader().addColumn("Title", titleColumn, 260);
	tableComponent.getHeader().addColumn("Duration", durationColumn, 80);
	tableComponent.getHeader().addColumn("BPM", bpmColumn, 60);

	searchBox.setTextToShowWhenEmpty("Search the library...",
	                                 juce::Colours::grey);
	searchBox.addListener(this);

	addAndMakeVisible(tableComponent);
	addAndMakeVisible(searchBox);
	addAndMakeVisible(addTracksButton);
	addAndMakeVisible(removeTrackButton);
	addAndMakeVisible(loadToDeckOneButton);
	addAndMakeVisible(loadToDeckTwoButton);
	addAndMakeVisible(analyseBPMButton);
	addAndMakeVisible(tapTempoButton);

	addTracksButton.addListener(this);
	removeTrackButton.addListener(this);
	loadToDeckOneButton.addListener(this);
	loadToDeckTwoButton.addListener(this);
	analyseBPMButton.addListener(this);
	tapTempoButton.addListener(this);

	library.addChangeListener(this);
	rebuildVisibleRows();
}

PlaylistComponent::~PlaylistComponent()
{
	stopTimer();
	library.removeChangeListener(this);
	tableComponent.setModel(nullptr);
}

void PlaylistComponent::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.setColour(juce::Colours::grey);
	g.drawRect(getLocalBounds(), 1);
}

void PlaylistComponent::resized()
{
	auto area = getLocalBounds().reduced(padding);

	searchBox.setBounds(area.removeFromTop(controlHeight));
	area.removeFromTop(padding);

	auto buttonRow = area.removeFromBottom(controlHeight);
	auto buttonWidth = buttonRow.getWidth() / buttonCount;

	addTracksButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(1, 0));
	removeTrackButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(1, 0));
	loadToDeckOneButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(1, 0));
	loadToDeckTwoButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(1, 0));
	analyseBPMButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(1, 0));
	tapTempoButton.setBounds(buttonRow.reduced(1, 0));

	area.removeFromBottom(padding);
	tableComponent.setBounds(area);
}

int PlaylistComponent::getNumRows()
{
	return visibleRows.size();
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int width,
                                           int height, bool rowIsSelected)
{
	juce::ignoreUnused(rowNumber, width, height);

	if (rowIsSelected)
	{
		g.fillAll(juce::Colours::orange);
	}
}

void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                                  int width, int height, bool rowIsSelected)
{
	auto libraryIndex = libraryIndexForRow(rowNumber);

	if (libraryIndex < 0)
	{
		return;
	}

	const auto& track = library.getTrack(libraryIndex);

	g.setColour(rowIsSelected ? juce::Colours::black : juce::Colours::white);
	g.setFont(static_cast<float>(height) * 0.7f);

	juce::String text;

	switch (columnId)
	{
		case titleColumn:
			text = track.displayName;
			break;

		case durationColumn:
			text = formatDuration(track.lengthInSeconds);
			break;

		case bpmColumn:
			text = track.bpm > Track::unknownBPM ? juce::String(track.bpm, 1) : "-";
			break;

		default:
			break;
	}

	g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

void PlaylistComponent::cellDoubleClicked(int rowNumber, int columnId,
                                          const juce::MouseEvent& event)
{
	juce::ignoreUnused(columnId, event);

	tableComponent.selectRow(rowNumber);
	loadSelectedTrackToDeck(1);
}

void PlaylistComponent::buttonClicked(juce::Button* button)
{
	if (button == &addTracksButton)
	{
		auto chooserFlags = juce::FileBrowserComponent::openMode
			| juce::FileBrowserComponent::canSelectFiles
			| juce::FileBrowserComponent::canSelectMultipleItems;

		fileChooser.launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
		{
			addFilesToLibrary(chooser.getResults());
		});
	}
	else if (button == &removeTrackButton)
	{
		auto libraryIndex = selectedLibraryIndex();

		if (libraryIndex >= 0)
		{
			library.removeTrack(libraryIndex);
			library.save();
		}
	}
	else if (button == &loadToDeckOneButton)
	{
		loadSelectedTrackToDeck(1);
	}
	else if (button == &loadToDeckTwoButton)
	{
		loadSelectedTrackToDeck(2);
	}
	else if (button == &analyseBPMButton)
	{
		analyseSelectedTrack();
	}
	else if (button == &tapTempoButton)
	{
		tapTempo();
	}
}

void PlaylistComponent::textEditorTextChanged(juce::TextEditor& editor)
{
	juce::ignoreUnused(editor);
	rebuildVisibleRows();
}

void PlaylistComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	juce::ignoreUnused(source);
	rebuildVisibleRows();
}

bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
	juce::ignoreUnused(files);
	return true;
}

void PlaylistComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
	juce::ignoreUnused(x, y);

	juce::Array<juce::File> dropped;

	for (const auto& path : files)
	{
		dropped.add(juce::File(path));
	}

	addFilesToLibrary(dropped);
}

int PlaylistComponent::libraryIndexForRow(int rowNumber) const
{
	if (! juce::isPositiveAndBelow(rowNumber, visibleRows.size()))
	{
		return -1;
	}

	auto libraryIndex = visibleRows[rowNumber];

	// The filter is rebuilt on every library change, but a repaint can still
	// arrive with a stale row number in flight.
	return libraryIndex < library.getNumTracks() ? libraryIndex : -1;
}

int PlaylistComponent::selectedLibraryIndex() const
{
	return libraryIndexForRow(tableComponent.getSelectedRow());
}

void PlaylistComponent::loadSelectedTrackToDeck(int deckNumber)
{
	auto libraryIndex = selectedLibraryIndex();

	if (libraryIndex < 0 || onLoadTrackToDeck == nullptr)
	{
		return;
	}

	onLoadTrackToDeck(library.getTrack(libraryIndex), deckNumber);
}

void PlaylistComponent::rebuildVisibleRows()
{
	visibleRows = library.findTracksMatching(searchBox.getText());
	tableComponent.updateContent();
	tableComponent.repaint();
}

void PlaylistComponent::addFilesToLibrary(const juce::Array<juce::File>& files)
{
	if (files.isEmpty())
	{
		return;
	}

	if (library.addTracks(files) > 0)
	{
		library.save();
	}
}

void PlaylistComponent::analyseSelectedTrack()
{
	auto libraryIndex = selectedLibraryIndex();

	if (libraryIndex < 0)
	{
		return;
	}

	auto track = library.getTrack(libraryIndex);

	// Analysis reads the file, so the interface stops responding for a moment on
	// a long track. The mouse cursor is the only warning the user gets.
	juce::MouseCursor::showWaitCursor();
	auto bpm = analyser.analyse(track.getFile());
	juce::MouseCursor::hideWaitCursor();

	if (bpm <= 0.0)
	{
		return;
	}

	track.bpm = bpm;
	library.updateTrack(libraryIndex, track);
	library.save();
}

void PlaylistComponent::tapTempo()
{
	analyser.registerTap();

	// The analyser drops a stale count by itself, but only when the next tap
	// arrives. Matching its timeout here means the button stops advertising a
	// count that has already expired.
	startTimer(BPMAnalyser::tapTimeoutMs);

	auto bpm = analyser.getBPMFromUserTap();

	if (bpm <= 0.0)
	{
		// Not enough taps to average yet. Showing the count stops the first two
		// clicks looking like a dead button.
		tapTempoButton.setButtonText("TAP " + juce::String(analyser.getTapCount()));
		return;
	}

	tapTempoButton.setButtonText(juce::String(bpm, 1));

	auto libraryIndex = selectedLibraryIndex();

	if (libraryIndex < 0)
	{
		// Nothing to write to, but the reading is still worth showing.
		return;
	}

	auto track = library.getTrack(libraryIndex);
	track.bpm = bpm;

	// Updated on every tap so the BPM column tightens as the user keeps tapping,
	// but written to disk only once the sequence ends - see finishTapTempo.
	library.updateTrack(libraryIndex, track);
	tapResultNeedsSaving = true;
}

void PlaylistComponent::finishTapTempo()
{
	stopTimer();

	if (tapResultNeedsSaving)
	{
		library.save();
		tapResultNeedsSaving = false;
	}

	analyser.clearTaps();
	tapTempoButton.setButtonText("TAP");
}

void PlaylistComponent::timerCallback()
{
	finishTapTempo();
}

juce::String PlaylistComponent::formatDuration(double seconds)
{
	if (seconds <= 0.0)
	{
		return "-";
	}

	auto totalSeconds = juce::roundToInt(seconds);

	return juce::String(totalSeconds / 60) + ":"
		+ juce::String(totalSeconds % 60).paddedLeft('0', 2);
}

// My code ends here
