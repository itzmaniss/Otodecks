void HotCueComponent::resized()
{
	auto area = getLocalBounds();

	// Two rows, so eight cue buttons stay wide enough to read their position and
	// to hit during a mix. Derived from the button count rather than hard-coded,
	// so changing Track::maxCuePoints does not need a layout change too.
	auto columns = juce::jmax(1, (cueButtons.size() + 1) / 2);

	// CLEAR ALL takes one column, full height, next to the grid.
	auto slotWidth = area.getWidth() / (columns + 1);
	clearAllButton.setBounds(area.removeFromRight(slotWidth).reduced(buttonGap, buttonGap));

	auto topRow = area.removeFromTop(area.getHeight() / 2);

	for (int i = 0; i < cueButtons.size(); ++i)
	{
		auto& row = i < columns ? topRow : area;

		cueButtons.getUnchecked(i)->setBounds(row.removeFromLeft(slotWidth)
		                                         .reduced(buttonGap, buttonGap));
	}
}
