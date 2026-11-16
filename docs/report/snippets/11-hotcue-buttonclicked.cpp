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
