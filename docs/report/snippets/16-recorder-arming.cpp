bool AudioRecorder::startRecordingFromDeck(DJAudioPlayer* player, double sampleRate,
                                           int numChannels)
{
	if (player == nullptr || sampleRate <= 0.0)
	{
		return false;
	}

	// beginWriting stops any recording in progress, and stopping disarms the
	// deck - so the deck is armed only once the new writer is open.
	if (! beginWriting(PersistenceManager::makeRecordingFile("deck"), sampleRate,
	                   numChannels, Source::deck))
	{
		return false;
	}

	armedPlayer = player;

	return true;
}
