juce::Array<int> MusicLibrary::findTracksMatching(const juce::String& searchTerm) const
{
	// EXTENSION: search over the library, backing the playlist search box.
	juce::Array<int> matches;
	auto term = searchTerm.trim();

	for (int i = 0; i < tracks.size(); ++i)
	{
		if (term.isEmpty() || tracks.getReference(i).displayName.containsIgnoreCase(term))
		{
			matches.add(i);
		}
	}

	return matches;
}
