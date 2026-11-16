void MainComponent::renderDeck (DJAudioPlayer& player, const AudioSourceChannelInfo& bufferToFill)
{
    auto capacity = deckBuffer.getNumSamples();

    if (capacity <= 0)
    {
        return;
    }

    // Work in chunks that fit the scratch buffer: a device may hand us a block
    // larger than it promised, and growing the buffer here would allocate on
    // the audio thread.
    for (int offset = 0; offset < bufferToFill.numSamples;)
    {
        auto samplesThisChunk = jmin(capacity, bufferToFill.numSamples - offset);

        AudioSourceChannelInfo deckInfo(&deckBuffer, 0, samplesThisChunk);
        player.getNextAudioBlock(deckInfo);

        recorder.captureDeckBlock(&player, deckBuffer, 0, samplesThisChunk);

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            bufferToFill.buffer->addFrom(channel,
                                         bufferToFill.startSample + offset,
                                         deckBuffer,
                                         channel % deckBuffer.getNumChannels(),
                                         0,
                                         samplesThisChunk);
        }

        offset += samplesThisChunk;
    }
