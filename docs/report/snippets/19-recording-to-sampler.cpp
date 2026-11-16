void MainComponent::handleFinishedRecording (const File& file)
{
    // EXTENSION: a finished recording joins the library so it can be played straight back.
    if (library.addTrack(file))
    {
        library.save();
    }

    // ...and the sampler panel, so a loop recorded off a deck can be triggered
    // over the mix without the user having to go and find the file.
    loopSampler.addRecordedSample(file);
