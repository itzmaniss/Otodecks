void DeckGUI::loadFile(const File& file)
{
    if (!file.existsAsFile())
    {
        return;
    }

    // The waveform reads the file itself, so both need the URL, not just the player.
    URL audioURL{file};
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);

    // Hot cues live on the library track, so a file dropped straight onto the
    // deck joins the library too - otherwise its cues would have nowhere to go.
    library.addTrack(file);
    hotCueComponent.setLoadedTrack(library.findTrackByPath(file.getFullPathName()));
    library.save();
}
