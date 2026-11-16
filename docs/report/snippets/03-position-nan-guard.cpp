double DJAudioPlayer::getPositionRelative()
{
    // My code starts here
    // Guard the empty-deck case: dividing by a zero length gives a NaN that
    // then propagates into the waveform and the position slider.
    if (transportSource.getLengthInSeconds() <= 0.0)
    {
        return 0.0;
    }

    // My code ends here
    return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}
