#include "LoopSampler.h"
#include "PersistenceManager.h"

#include <memory>

// My code starts here

namespace
{
	const juce::Identifier samplesKey { "samples" };
	const juce::Identifier pathKey { "path" };
}

LoopSampler::LoopSampler(juce::AudioFormatManager& formatManagerToUse)
	: formatManager(formatManagerToUse),
	  previewPlayer(formatManagerToUse)
{
	tableComponent.setModel(this);
	tableComponent.setRowHeight(rowHeight);
	tableComponent.getHeader().addColumn("Sample", nameColumn, 200);

	gainSlider.setRange(0.0, 1.0);
	gainSlider.setValue(defaultGain, juce::dontSendNotification);
	gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
	previewPlayer.setGain(defaultGain);

	addAndMakeVisible(tableComponent);
	addAndMakeVisible(addButton);
	addAndMakeVisible(removeButton);
	addAndMakeVisible(playButton);
	addAndMakeVisible(stopButton);
	addAndMakeVisible(toDeckOneButton);
	addAndMakeVisible(toDeckTwoButton);
	addAndMakeVisible(loopButton);
	addAndMakeVisible(gainSlider);

	for (auto* button : { &addButton, &removeButton, &playButton, &stopButton,
	                      &toDeckOneButton, &toDeckTwoButton })
	{
		button->addListener(this);
	}

	loopButton.addListener(this);
	gainSlider.addListener(this);
}

LoopSampler::~LoopSampler()
{
	tableComponent.setModel(nullptr);
}

void LoopSampler::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.setColour(juce::Colours::grey);
	g.drawRect(getLocalBounds(), 1);
}

void LoopSampler::resized()
{
	auto area = getLocalBounds().reduced(padding);

	auto topRow = area.removeFromTop(controlHeight);
	auto topWidth = topRow.getWidth() / 4;
	addButton.setBounds(topRow.removeFromLeft(topWidth).reduced(1, 0));
	removeButton.setBounds(topRow.removeFromLeft(topWidth).reduced(1, 0));
	toDeckOneButton.setBounds(topRow.removeFromLeft(topWidth).reduced(1, 0));
	toDeckTwoButton.setBounds(topRow.reduced(1, 0));

	auto bottomRow = area.removeFromBottom(controlHeight);
	auto bottomWidth = bottomRow.getWidth() / 4;
	playButton.setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(1, 0));
	stopButton.setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(1, 0));
	loopButton.setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(1, 0));
	gainSlider.setBounds(bottomRow.reduced(1, 0));

	area.removeFromTop(padding);
	area.removeFromBottom(padding);
	tableComponent.setBounds(area);
}

SamplePlayer& LoopSampler::getPreviewPlayer()
{
	return previewPlayer;
}

bool LoopSampler::addSample(const juce::File& file)
{
	if (! file.existsAsFile() || samples.contains(file))
	{
		return false;
	}

	// Anything the format manager cannot read would be a row that never plays.
	std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

	if (reader == nullptr)
	{
		return false;
	}

	samples.add(file);
	refreshTable();

	return true;
}

bool LoopSampler::addRecordedSample(const juce::File& file)
{
	// EXTENSION: a finished recording becomes a sample, so a loop taken off a
	// deck can be triggered straight back over the mix.
	// The recording itself stays in the recordings folder; the sampler works
	// from its own copy, exactly as it does for any other added file.
	if (! addSample(copyIntoSamplesFolder(file)))
	{
		return false;
	}

	save();

	return true;
}

void LoopSampler::removeSample(int index)
{
	if (! juce::isPositiveAndBelow(index, samples.size()))
	{
		return;
	}

	// The file itself stays on disk; only this list forgets it.
	samples.remove(index);
	refreshTable();
}

void LoopSampler::previewSample(int index)
{
	if (! juce::isPositiveAndBelow(index, samples.size()))
	{
		return;
	}

	if (previewPlayer.load(samples[index]))
	{
		previewPlayer.play();
	}
}

void LoopSampler::loadSampleToDeck(int index, int deckNumber)
{
	if (! juce::isPositiveAndBelow(index, samples.size()) || onLoadSampleToDeck == nullptr)
	{
		return;
	}

	onLoadSampleToDeck(samples[index], deckNumber);
}

void LoopSampler::scanSamplesFolder()
{
	PersistenceManager::ensureDirectoriesExist();

	for (const auto& entry : juce::RangedDirectoryIterator(
			PersistenceManager::getSamplesDirectory(), false, "*", juce::File::findFiles))
	{
		addSample(entry.getFile());
	}
}

juce::var LoopSampler::toVar() const
{
	juce::Array<juce::var> entries;

	for (const auto& file : samples)
	{
		auto* entry = new juce::DynamicObject();
		entry->setProperty(pathKey, file.getFullPathName());
		entries.add(juce::var(entry));
	}

	auto* root = new juce::DynamicObject();
	root->setProperty(samplesKey, entries);

	return juce::var(root);
}

void LoopSampler::loadFromVar(const juce::var& source)
{
	samples.clear();

	if (auto* root = source.getDynamicObject())
	{
		if (auto* entries = root->getProperty(samplesKey).getArray())
		{
			for (const auto& entry : *entries)
			{
				if (auto* fields = entry.getDynamicObject())
				{
					juce::File file(fields->getProperty(pathKey).toString());

					if (file.existsAsFile())
					{
						samples.addIfNotAlreadyThere(file);
					}
				}
			}
		}
	}

	refreshTable();
}

bool LoopSampler::save() const
{
	return PersistenceManager::saveJSON(PersistenceManager::getSamplerFile(), toVar());
}

void LoopSampler::load()
{
	loadFromVar(PersistenceManager::loadJSON(PersistenceManager::getSamplerFile()));
	scanSamplesFolder();
}

int LoopSampler::getNumRows()
{
	return samples.size();
}

void LoopSampler::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height,
                                     bool rowIsSelected)
{
	juce::ignoreUnused(rowNumber, width, height);

	if (rowIsSelected)
	{
		g.fillAll(juce::Colours::orange);
	}
}

void LoopSampler::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width,
                            int height, bool rowIsSelected)
{
	if (! juce::isPositiveAndBelow(rowNumber, samples.size()) || columnId != nameColumn)
	{
		return;
	}

	g.setColour(rowIsSelected ? juce::Colours::black : juce::Colours::white);
	g.setFont(static_cast<float>(height) * 0.7f);
	g.drawText(samples[rowNumber].getFileNameWithoutExtension(), 2, 0, width - 4, height,
	           juce::Justification::centredLeft, true);
}

void LoopSampler::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	juce::ignoreUnused(columnId, event);

	tableComponent.selectRow(rowNumber);
	previewSample(rowNumber);
}

void LoopSampler::buttonClicked(juce::Button* button)
{
	if (button == &addButton)
	{
		auto chooserFlags = juce::FileBrowserComponent::openMode
			| juce::FileBrowserComponent::canSelectFiles
			| juce::FileBrowserComponent::canSelectMultipleItems;

		fileChooser.launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
		{
			bool anyAdded = false;

			for (const auto& file : chooser.getResults())
			{
				anyAdded = addSample(copyIntoSamplesFolder(file)) || anyAdded;
			}

			if (anyAdded)
			{
				save();
			}
		});
	}
	else if (button == &removeButton)
	{
		removeSample(selectedIndex());
		save();
	}
	else if (button == &playButton)
	{
		previewSample(selectedIndex());
	}
	else if (button == &stopButton)
	{
		previewPlayer.stop();
	}
	else if (button == &toDeckOneButton)
	{
		loadSampleToDeck(selectedIndex(), 1);
	}
	else if (button == &toDeckTwoButton)
	{
		loadSampleToDeck(selectedIndex(), 2);
	}
	else if (button == &loopButton)
	{
		previewPlayer.setLooping(loopButton.getToggleState());
	}
}

void LoopSampler::sliderValueChanged(juce::Slider* slider)
{
	if (slider == &gainSlider)
	{
		previewPlayer.setGain(slider->getValue());
	}
}

bool LoopSampler::isInterestedInFileDrag(const juce::StringArray& files)
{
	juce::ignoreUnused(files);
	return true;
}

void LoopSampler::filesDropped(const juce::StringArray& files, int x, int y)
{
	juce::ignoreUnused(x, y);

	bool anyAdded = false;

	for (const auto& path : files)
	{
		anyAdded = addSample(copyIntoSamplesFolder(juce::File(path))) || anyAdded;
	}

	if (anyAdded)
	{
		save();
	}
}

int LoopSampler::selectedIndex() const
{
	auto row = tableComponent.getSelectedRow();

	return juce::isPositiveAndBelow(row, samples.size()) ? row : -1;
}

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

void LoopSampler::refreshTable()
{
	tableComponent.updateContent();
	tableComponent.repaint();
}

// My code ends here
