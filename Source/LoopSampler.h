#pragma once

#include <JuceHeader.h>
#include "SamplePlayer.h"

#include <functional>

// My code starts here

/**
	A table of short samples the user can preview or send to a deck.

	The sample list is its own file (sampler.json), separate from the music
	library: samples are loops and one-shots, not tracks, and they are not
	meant to appear in the playlist.

	Owns the preview player; the owner must render it, since this component
	sits outside the audio graph.
*/
class LoopSampler : public juce::Component,
                    public juce::TableListBoxModel,
                    public juce::Button::Listener,
                    public juce::Slider::Listener,
                    public juce::FileDragAndDropTarget
{
public:
	explicit LoopSampler(juce::AudioFormatManager& formatManagerToUse);
	~LoopSampler() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	/** Set by the owner to send a sample to a deck. Deck numbers are 1 and 2. */
	std::function<void(const juce::File&, int deckNumber)> onLoadSampleToDeck;

	/** The owner renders this alongside the decks. */
	SamplePlayer& getPreviewPlayer();

	bool addSample(const juce::File& file);
	void removeSample(int index);

	void previewSample(int index);
	void loadSampleToDeck(int index, int deckNumber);

	/** Adds every readable audio file sitting in the samples folder. */
	void scanSamplesFolder();

	juce::var toVar() const;
	void loadFromVar(const juce::var& source);

	bool save() const;
	void load();

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height,
	                        bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height,
	               bool rowIsSelected) override;
	void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;

	void buttonClicked(juce::Button* button) override;
	void sliderValueChanged(juce::Slider* slider) override;

	bool isInterestedInFileDrag(const juce::StringArray& files) override;
	void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
	int selectedIndex() const;

	/** Copies a dropped file into the samples folder so the set survives a restart. */
	juce::File copyIntoSamplesFolder(const juce::File& file);

	void refreshTable();

	enum ColumnId
	{
		nameColumn = 1
	};

	static constexpr int rowHeight = 20;
	static constexpr int controlHeight = 24;
	static constexpr int padding = 4;
	static constexpr double defaultGain = 0.8;

	juce::AudioFormatManager& formatManager;
	SamplePlayer previewPlayer;

	juce::Array<juce::File> samples;

	juce::TableListBox tableComponent;
	juce::TextButton addButton { "ADD" };
	juce::TextButton removeButton { "REMOVE" };
	juce::TextButton playButton { "PLAY" };
	juce::TextButton stopButton { "STOP" };
	juce::TextButton toDeckOneButton { "> L" };
	juce::TextButton toDeckTwoButton { "> R" };
	juce::ToggleButton loopButton { "Loop" };
	juce::Slider gainSlider;

	juce::FileChooser fileChooser { "Select samples to add..." };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopSampler)
};

// My code ends here
