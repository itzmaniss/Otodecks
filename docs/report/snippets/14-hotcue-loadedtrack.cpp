const Track* HotCueComponent::loadedTrack() const
{
	if (! juce::isPositiveAndBelow(trackIndex, library.getNumTracks()))
	{
		return nullptr;
	}

	return &library.getTrack(trackIndex);
}
