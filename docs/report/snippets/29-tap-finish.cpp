void PlaylistComponent::finishTapTempo()
{
	stopTimer();

	if (tapResultNeedsSaving)
	{
		library.save();
		tapResultNeedsSaving = false;
	}

	analyser.clearTaps();
	tapTempoButton.setButtonText("TAP");
}
