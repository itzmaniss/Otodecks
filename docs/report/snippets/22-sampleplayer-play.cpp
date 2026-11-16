void SamplePlayer::play()
{
	if (readerSource == nullptr)
	{
		return;
	}

	transportSource.setPosition(0.0);
	transportSource.start();
}
