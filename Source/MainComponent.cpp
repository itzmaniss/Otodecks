/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (1000, 720);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }  
    else
    {
        // My code starts here
        // Two inputs, so the recorder's microphone path has something to read.
        setAudioChannels (2, 2);
        // My code ends here
    }  

    addAndMakeVisible(deckGUI1); 
    addAndMakeVisible(deckGUI2);  
    // My code starts here
    addAndMakeVisible(playlistComponent);
    addAndMakeVisible(loopSampler);
    addAndMakeVisible(micRecordButton);
    addAndMakeVisible(recorderStatusLabel);

    micRecordButton.addListener(this);
    recorderStatusLabel.setJustificationType(Justification::centredLeft);

    // The label doubles as a link to the recording, so it has to hear clicks.
    recorderStatusLabel.addMouseListener(this, false);

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

    // My code ends here
    // Formats must be registered before the library reads durations from disk.
    formatManager.registerBasicFormats();
    // My code starts here
    library.load();

    loopSampler.load();
    loopSampler.onLoadSampleToDeck = [this](const File& file, int deckNumber)
    {
        if (deckNumber == 1)
        {
            deckGUI1.loadTrack(Track::fromFile(file, formatManager));
        }
        else
        {
            deckGUI2.loadTrack(Track::fromFile(file, formatManager));
        }
    };

    // The recorder rides along as a second device callback purely so it can see
    // the microphone input and learn the device sample rate.
    deviceManager.addAudioCallback(&recorder);

    recorder.onRecordingFinished = [this](const File& file)
    {
        handleFinishedRecording(file);
    };

    auto refreshRecorderUI = [this]
    {
        updateRecorderStatus();
        deckGUI1.updateRecordButton();
        deckGUI2.updateRecordButton();
    };

    deckGUI1.onRecorderStateChanged = refreshRecorderUI;
    deckGUI2.onRecorderStateChanged = refreshRecorderUI;

    updateRecorderStatus();
    // My code ends here
}

MainComponent::~MainComponent()
{
    // My code starts here
    // Drop the callback first: stopping here would otherwise reach back into a
    // component that is already going away.
    recorder.onRecordingFinished = nullptr;

    // Stop writing before the device goes away, or the writer outlives its input.
    recorder.stop();
    deviceManager.removeAudioCallback(&recorder);

    // My code ends here
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // My code starts here
    loopSampler.getPreviewPlayer().prepareToPlay(samplesPerBlockExpected, sampleRate);

    deckBuffer.setSize(2, samplesPerBlockExpected);
    sampleBuffer.setSize(2, samplesPerBlockExpected);
    // My code ends here
 }
void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // My code starts here
    // Each deck is rendered on its own so the recorder can tap one in isolation;
    // summing them here is what a MixerAudioSource would otherwise do for us.
    bufferToFill.clearActiveBufferRegion();

    renderDeck(player1, bufferToFill);
    renderDeck(player2, bufferToFill);

    // The sampler's preview is heard but never recorded - a deck recording
    // should capture the deck, not whatever was auditioned over the top of it.
    AudioSourceChannelInfo sampleInfo(&sampleBuffer, 0, bufferToFill.numSamples);

    if (sampleBuffer.getNumSamples() >= bufferToFill.numSamples)
    {
        loopSampler.getPreviewPlayer().getNextAudioBlock(sampleInfo);

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            bufferToFill.buffer->addFrom(channel, bufferToFill.startSample, sampleBuffer,
                                         channel % sampleBuffer.getNumChannels(), 0,
                                         bufferToFill.numSamples);
        }
    }
}

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
    // My code ends here
}

void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    // My code starts here
    loopSampler.getPreviewPlayer().releaseResources();
    // My code ends here
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    int deckHeight = roundToInt(static_cast<float>(getHeight()) * deckHeightProportion);

    deckGUI1.setBounds(0, 0, getWidth()/2, deckHeight);
    deckGUI2.setBounds(getWidth()/2, 0, getWidth()/2, deckHeight);

    // My code starts here
    int barTop = deckHeight;
    micRecordButton.setBounds(0, barTop, 140, recorderBarHeight);
    recorderStatusLabel.setBounds(140, barTop, getWidth() - 140, recorderBarHeight);

    int playlistTop = barTop + recorderBarHeight;
    int playlistWidth = roundToInt(static_cast<float>(getWidth()) * playlistWidthProportion);

    playlistComponent.setBounds(0, playlistTop, playlistWidth, getHeight() - playlistTop);
    loopSampler.setBounds(playlistWidth, playlistTop, getWidth() - playlistWidth,
                          getHeight() - playlistTop);
    // My code ends here
}

// My code starts here
void MainComponent::buttonClicked (Button* button)

{
    if (button != &micRecordButton)
    {
        return;
    }

    if (recorder.getSource() == AudioRecorder::Source::microphone)
    {
        recorder.stop();
    }
    else if (! recorder.startRecordingFromMic())
    {
        recorderStatusLabel.setText("Could not start the microphone - is an input device open?",
                                    dontSendNotification);
        return;
    }

    updateRecorderStatus();
    deckGUI1.updateRecordButton();
    deckGUI2.updateRecordButton();
}

String MainComponent::fileBrowserName()
{
    // EXTENSION: names the platform file browser so the status bar reads correctly.
   #if JUCE_MAC
    return "Finder";
   #elif JUCE_WINDOWS
    return "File Explorer";
   #else
    return "your file manager";
   #endif
}

void MainComponent::mouseDown (const MouseEvent& event)
{
    // EXTENSION: clicking the saved path reveals the file in the OS file browser.
    if (event.eventComponent != &recorderStatusLabel)
    {
        return;
    }

    auto file = recorder.getOutputFile();

    if (file.existsAsFile())
    {
        // Finder on macOS, Explorer on Windows, the desktop file manager on Linux.
        file.revealToUser();
    }
}

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

    updateRecorderStatus();
    deckGUI1.updateRecordButton();
    deckGUI2.updateRecordButton();
}

void MainComponent::updateRecorderStatus()
{
    bool micRecording = recorder.getSource() == AudioRecorder::Source::microphone;

    micRecordButton.setButtonText(micRecording ? "STOP MIC" : "RECORD MIC");
    micRecordButton.setColour(TextButton::buttonColourId,
                              micRecording ? Colours::red : Colours::darkgrey);

    if (recorder.isRecording())
    {
        recorderStatusLabel.setText("Recording to " + recorder.getOutputFile().getFileName(),
                                    dontSendNotification);
        recorderStatusLabel.setColour(Label::textColourId, Colours::red);
        recorderStatusLabel.setMouseCursor(MouseCursor::NormalCursor);
    }
    else if (recorder.getOutputFile().existsAsFile())
    {
        // Say it is clickable, since a label does not look like a link.
        recorderStatusLabel.setText("Saved to " + recorder.getOutputFile().getFullPathName()
                                        + "  (click to show in " + fileBrowserName() + ")",
                                    dontSendNotification);
        recorderStatusLabel.setColour(Label::textColourId, Colours::lightblue);
        recorderStatusLabel.setMouseCursor(MouseCursor::PointingHandCursor);
    }
    else
    {
        recorderStatusLabel.setText("Not recording", dontSendNotification);
        recorderStatusLabel.setColour(Label::textColourId,
                                      getLookAndFeel().findColour(Label::textColourId));
        recorderStatusLabel.setMouseCursor(MouseCursor::NormalCursor);
    }
}

// My code ends here
