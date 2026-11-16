    playlistComponent.onLoadTrackToDeck = [this](const Track& track, int deckNumber)
    {
        if (deckNumber == 1)
        {
            deckGUI1.loadTrack(track);
        }
        else
        {
            deckGUI2.loadTrack(track);
        }
    };
