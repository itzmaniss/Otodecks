#include "HotCueComponent.h"

// My code starts here

HotCueComponent::HotCueComponent(MusicLibrary& libraryToUse)
	: library(libraryToUse)
{
	for (int i = 0; i < Track::maxCuePoints; ++i)
	{
		auto* button = cueButtons.add(new juce::TextButton("CUE " + juce::String(i + 1)));
		button->addListener(this);
		addAndMakeVisible(button);
	}

	clearAllButton.addListener(this);
	addAndMakeVisible(clearAllButton);

	updateButtonStates();
}

void HotCueComponent::resized()
{
	auto area = getLocalBounds();

	// The clear-all button takes the same share as one cue button.
	auto slotWidth = area.getWidth() / (cueButtons.size() + 1);

	for (auto* button : cueButtons)
	{
		button->setBounds(area.removeFromLeft(slotWidth).reduced(buttonGap, 0));
	}

	clearAllButton.setBounds(area.reduced(buttonGap, 0));
}

void HotCueComponent::buttonClicked(juce::Button* button)
{
	if (button == &clearAllButton)
	{
		clearAllCues();
		return;
	}

	auto cueIndex = cueButtons.indexOf(dynamic_cast<juce::TextButton*>(button));

	if (cueIndex < 0)
	{
		return;
	}

	const auto* track = loadedTrack();

	if (track == nullptr)
	{
		return;
	}

	if (! track->hasCueAt(cueIndex))
	{
		setCue(cueIndex);
	}
	else if (juce::ModifierKeys::getCurrentModifiers().isAltDown())
	{
		clearCue(cueIndex);
	}
	else
	{
		jumpToCue(cueIndex);
	}
}

void HotCueComponent::setPlayer(DJAudioPlayer* playerToUse)
{
	player = playerToUse;
	updateButtonStates();
}

void HotCueComponent::setLoadedTrack(int libraryIndex)
{
	trackIndex = libraryIndex;
	updateButtonStates();
}

void HotCueComponent::setCue(int cueIndex)
{
	const auto* track = loadedTrack();

	if (track == nullptr || player == nullptr
		|| ! juce::isPositiveAndBelow(cueIndex, Track::maxCuePoints))
	{
		return;
	}

	// A cue past the end of the file would be unreachable, so refuse it rather
	// than storing a position the deck can never seek to.
	auto position = player->getPositionInSeconds();

	if (position < 0.0 || position > player->getLengthInSeconds())
	{
		return;
	}

	auto cues = track->cuePoints;
	cues.set(cueIndex, position);
	commitCues(cues);
}

void HotCueComponent::jumpToCue(int cueIndex)
{
	const auto* track = loadedTrack();

	if (track == nullptr || player == nullptr || ! track->hasCueAt(cueIndex))
	{
		return;
	}

	player->setPosition(track->cuePoints[cueIndex]);
}

void HotCueComponent::clearCue(int cueIndex)
{
	const auto* track = loadedTrack();

	if (track == nullptr || ! juce::isPositiveAndBelow(cueIndex, Track::maxCuePoints))
	{
		return;
	}

	auto cues = track->cuePoints;
	cues.set(cueIndex, Track::noCue);
	commitCues(cues);
}

void HotCueComponent::clearAllCues()
{
	const auto* track = loadedTrack();

	if (track == nullptr)
	{
		return;
	}

	juce::Array<double> cues;

	for (int i = 0; i < Track::maxCuePoints; ++i)
	{
		cues.add(Track::noCue);
	}

	commitCues(cues);
}

void HotCueComponent::updateButtonStates()
{
	const auto* track = loadedTrack();

	for (int i = 0; i < cueButtons.size(); ++i)
	{
		auto* button = cueButtons.getUnchecked(i);
		bool isSet = track != nullptr && track->hasCueAt(i);

		button->setEnabled(track != nullptr);
		button->setColour(juce::TextButton::buttonColourId,
		                  isSet ? juce::Colours::orange : juce::Colours::darkgrey);

		// The label carries the cue position, so the user can see where a slot
		// points without having to jump to it.
		button->setButtonText(isSet
			? "CUE " + juce::String(i + 1) + "\n" + juce::String(track->cuePoints[i], 1) + "s"
			: "CUE " + juce::String(i + 1));
	}

	clearAllButton.setEnabled(track != nullptr);
}

const Track* HotCueComponent::loadedTrack() const
{
	if (! juce::isPositiveAndBelow(trackIndex, library.getNumTracks()))
	{
		return nullptr;
	}

	return &library.getTrack(trackIndex);
}

void HotCueComponent::commitCues(const juce::Array<double>& cues)
{
	const auto* track = loadedTrack();

	if (track == nullptr)
	{
		return;
	}

	auto updated = *track;
	updated.cuePoints = cues;

	library.updateTrack(trackIndex, updated);
	library.save();

	updateButtonStates();
}

// My code ends here
