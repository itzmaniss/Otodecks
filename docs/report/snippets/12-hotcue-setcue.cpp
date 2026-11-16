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
