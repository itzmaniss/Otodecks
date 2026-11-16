	// Onset strength: only rises in energy matter. A falling edge is the tail of
	// a hit, not a new one, so it is discarded rather than counted twice.
	juce::Array<float> onsetStrength;
	onsetStrength.ensureStorageAllocated(envelope.size());
	onsetStrength.add(0.0f);

	for (int i = 1; i < envelope.size(); ++i)
	{
		onsetStrength.add(juce::jmax(0.0f, envelope[i] - envelope[i - 1]));
	}
