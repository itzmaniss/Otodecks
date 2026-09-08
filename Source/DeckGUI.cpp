/*
  ==============================================================================

    DeckGUI.cpp
    Created: 13 Mar 2020 6:44:48pm
    Author:  matthew

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer* _player, 
                AudioFormatManager & 	formatManagerToUse,
                AudioThumbnailCache & 	cacheToUse,
// My code starts here
                MusicLibrary& libraryToUse,
                AudioRecorder& recorderToUse
// My code ends here
           ) : waveformDisplay(formatManagerToUse, cacheToUse),
               library(libraryToUse),
               recorder(recorderToUse),
               hotCueComponent(libraryToUse),
               player(_player)
{
    // My code starts here
    addAndMakeVisible(hotCueComponent);
    hotCueComponent.setPlayer(_player);

    addAndMakeVisible(recordButton);
    recordButton.addListener(this);
    updateRecordButton();

    // My code ends here
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
       
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);

    // My code starts here
    for (auto* label : { &volLabel, &speedLabel, &posLabel })
    {
        // attachToComponent places the label using its own width, so it needs a
        // size before it is attached or it lands zero pixels wide.
        label->setSize(sliderLabelWidth, sliderRowTextHeight);
        label->setJustificationType(Justification::centredRight);
        addAndMakeVisible(label);
    }

    volLabel.attachToComponent(&volSlider, true);
    speedLabel.attachToComponent(&speedSlider, true);
    posLabel.attachToComponent(&posSlider, true);

    // My code ends here
    addAndMakeVisible(waveformDisplay);


    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);


    volSlider.setRange(minGain, maxGain);
    speedSlider.setRange(minSpeed, maxSpeed);
    posSlider.setRange(0.0, 1.0);

    // My code starts here
    // Sliders otherwise sit at the bottom of their range, which would start the
    // deck silent and stopped. Set without notifying: the player already holds
    // these values, so there is nothing to push to it.
    volSlider.setValue(defaultGain, dontSendNotification);
    speedSlider.setValue(defaultSpeed, dontSendNotification);

    // Normal speed is the value the user returns to most, so put it in the
    // middle of the travel rather than a quarter of the way along.
    speedSlider.setSkewFactorFromMidPoint(defaultSpeed);

    for (auto* slider : { &volSlider, &speedSlider, &posSlider })
    {
        slider->setNumDecimalPlacesToDisplay(sliderDecimalPlaces);
        slider->setTextBoxStyle(Slider::TextBoxLeft, false, sliderTextBoxWidth, sliderRowTextHeight);
    }

    // My code ends here
    startTimer(playheadRefreshMs);


}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);
}

void DeckGUI::resized()
{
    double rowH = getHeight() / layoutRows;
    playButton.setBounds(0, 0, getWidth(), rowH);
    stopButton.setBounds(0, rowH, getWidth(), rowH);  
    // attachToComponent puts each label immediately to the left of its slider,
    // so the sliders are inset to leave room for it.
    volSlider.setBounds(sliderLabelWidth, rowH * 2, getWidth() - sliderLabelWidth, rowH);
    speedSlider.setBounds(sliderLabelWidth, rowH * 3, getWidth() - sliderLabelWidth, rowH);
    posSlider.setBounds(sliderLabelWidth, rowH * 4, getWidth() - sliderLabelWidth, rowH);
    waveformDisplay.setBounds(0, rowH * 5, getWidth(), rowH * 2);
    // My code starts here
    hotCueComponent.setBounds(0, rowH * 7, getWidth(), rowH);
    // My code ends here
    loadButton.setBounds(0, rowH * 8, getWidth(), rowH);
    // My code starts here
    recordButton.setBounds(0, rowH * 9, getWidth(), rowH);
    // My code ends here

}

void DeckGUI::buttonClicked(Button* button)
{
    if (button == &playButton)
    {
        std::cout << "Play button was clicked " << std::endl;
        player->start();
    }
     if (button == &stopButton)
    {
        std::cout << "Stop button was clicked " << std::endl;
        player->stop();

    }
    // My code starts here
    if (button == &recordButton)
    {
        if (recorder.isRecording())
        {
            recorder.stop();
        }
        else
        {
            // Two channels: the deck renders stereo even for a mono source.
            recorder.startRecordingFromDeck(player, recorder.getDeviceSampleRate(), 2);
        }

        updateRecordButton();

        if (onRecorderStateChanged != nullptr)
        {
            onRecorderStateChanged();
        }
    }
    // My code ends here
    if (button == &loadButton)
    {
       auto fileChooserFlags = 
        FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            File chosenFile = chooser.getResult();
            if (chosenFile.existsAsFile()){
                loadFile(chosenFile);
            }
        });
    }
}

void DeckGUI::sliderValueChanged (Slider *slider)
{
    if (slider == &volSlider)
    {
        player->setGain(slider->getValue());
    }

    if (slider == &speedSlider)
    {
        player->setSpeed(slider->getValue());
    }
    
    if (slider == &posSlider)
    {
        player->setPositionRelative(slider->getValue());
    }
    
}

bool DeckGUI::isInterestedInFileDrag (const StringArray &files)
{
  std::cout << "DeckGUI::isInterestedInFileDrag" << std::endl;
  return true; 
}

void DeckGUI::filesDropped (const StringArray &files, int x, int y)
{
  std::cout << "DeckGUI::filesDropped" << std::endl;
  if (files.size() == 1)
  {
    loadFile(File{files[0]});
  }
}

// My code starts here
void DeckGUI::updateRecordButton()
{
    // Red only when this deck is the thing being recorded, not the mic or the other deck.
    bool recordingThisDeck = recorder.getSource() == AudioRecorder::Source::deck
                             && recorder.isRecordingDeck(player);

    recordButton.setButtonText(recordingThisDeck ? "STOP REC" : "REC");
    recordButton.setColour(TextButton::buttonColourId,
                           recordingThisDeck ? Colours::red : Colours::darkgrey);
    recordButton.setEnabled(recordingThisDeck || !recorder.isRecording());
}

void DeckGUI::loadTrack(const Track& track)
{
    loadFile(track.getFile());
}

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
// My code ends here

void DeckGUI::timerCallback()
{
    // My code starts here
    auto position = player->getPositionRelative();

    waveformDisplay.setPositionRelative(position);

    // The position slider is a playhead as much as a control, so it follows the
    // audio - except while the user is dragging it, when they own it instead.
    if (!posSlider.isMouseButtonDown())
    {
        posSlider.setValue(position, dontSendNotification);
    }
    // My code ends here
}


    

