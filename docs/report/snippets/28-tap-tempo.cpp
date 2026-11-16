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
