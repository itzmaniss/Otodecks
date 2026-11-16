void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // My code starts here
    // Each deck is rendered on its own so the recorder can tap one in isolation;
    // summing them here is what a MixerAudioSource would otherwise do for us.
    bufferToFill.clearActiveBufferRegion();

    renderDeck(player1, bufferToFill);
    renderDeck(player2, bufferToFill);
