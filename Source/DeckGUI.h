/*
  ==============================================================================

    DeckGUI.h
    Created: 13 Mar 2020 6:44:48pm
    Author:  matthew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
// My code starts here
#include "Track.h"
#include "MusicLibrary.h"
#include "HotCueComponent.h"
#include "AudioRecorder.h"

#include <functional>
// My code ends here

//==============================================================================
class DeckGUI    : public Component,
                   public Button::Listener, 
                   public Slider::Listener, 
                   public FileDragAndDropTarget, 
                   public Timer
{
public:
    DeckGUI(DJAudioPlayer* player, 
           AudioFormatManager & 	formatManagerToUse,
           AudioThumbnailCache & 	cacheToUse,
// My code starts here
           MusicLibrary& libraryToUse,
           AudioRecorder& recorderToUse
// My code ends here
           );
    ~DeckGUI();

    // My code starts here
    /** Loads a library track onto this deck, waveform included. */
    void loadTrack(const Track& track);

    /** Repaints the REC button; call when the recorder is started or stopped elsewhere. */
    void updateRecordButton();

    /** Set by the owner so the other deck and the status bar can follow along. */
    std::function<void()> onRecorderStateChanged;
    // My code ends here

    void paint (Graphics&) override;
    void resized() override;

    void buttonClicked (Button *) override;

    void sliderValueChanged (Slider *slider) override;

    bool isInterestedInFileDrag (const StringArray &files) override;
    void filesDropped (const StringArray &files, int x, int y) override; 

    void timerCallback() override; 

private:
    // My code starts here
    /** The one path by which audio reaches this deck - chooser, drop and playlist all use it. */
    void loadFile(const juce::File& file);

    // My code ends here
    juce::FileChooser fChooser{"Select a file..."};

    TextButton playButton{"PLAY"};
    TextButton stopButton{"STOP"};
    TextButton loadButton{"LOAD"};
    // My code starts here
    TextButton recordButton{"REC"};
    // My code ends here
  
    Slider volSlider; 
    Slider speedSlider;
    Slider posSlider;

    // My code starts here
    Label volLabel{{}, "VOL"};
    Label speedLabel{{}, "SPEED"};
    Label posLabel{{}, "POS"};
    // My code ends here

    WaveformDisplay waveformDisplay;

    // My code starts here
    MusicLibrary& library;
    AudioRecorder& recorder;
    HotCueComponent hotCueComponent;
    // My code ends here

    DJAudioPlayer* player; 

    // Rows the deck is laid out in. The waveform takes two and the hot cue grid
    // takes two; the rest are one row each.
    static constexpr int layoutRows = 11;

    // Slider ranges and the values a fresh deck starts at. A deck that starts
    // at the bottom of each range is silent and stalled, which reads as broken.
    static constexpr double minGain = 0.0;
    static constexpr double maxGain = 1.0;
    static constexpr double defaultGain = 1.0;

    // Resampling ratio: 1.0 is the file's own pitch. Zero would stall the
    // resampler, so the range never reaches it.
    static constexpr double minSpeed = 0.25;
    static constexpr double maxSpeed = 3.0;
    static constexpr double defaultSpeed = 1.0;

    // How often the playhead display catches up with the audio.
    static constexpr int playheadRefreshMs = 100;

    // Two decimals is as fine as any of these controls is worth reading.
    static constexpr int sliderDecimalPlaces = 2;

    static constexpr int sliderLabelWidth = 64;
    static constexpr int sliderTextBoxWidth = 56;
    static constexpr int sliderRowTextHeight = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
