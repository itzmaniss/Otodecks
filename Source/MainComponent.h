/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
// My code starts here
#include "MusicLibrary.h"
#include "PlaylistComponent.h"
#include "AudioRecorder.h"
#include "LoopSampler.h"
#include "BPMAnalyser.h"
// My code ends here


//==============================================================================
class MainComponent   : public AudioAppComponent,
// My code starts here
                        public Button::Listener
// My code ends here
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    // My code starts here
    void buttonClicked (Button* button) override;

    /** Clicking the status bar reveals the last recording in the OS file browser. */
    void mouseDown (const MouseEvent& event) override;
    // My code ends here

private:
    //==============================================================================
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbCache{100}; 

    // My code starts here
    /** Renders one deck into the scratch buffer, offers it to the recorder,
        then adds it to the output. Audio thread only. */
    void renderDeck(DJAudioPlayer& player, const AudioSourceChannelInfo& bufferToFill);

    void updateRecorderStatus();

    /** Takes a finished recording into both the library and the sampler panel,
        so it can be played on a deck or triggered as a loop. */
    void handleFinishedRecording (const File& file);

    /** What this platform calls its file browser, for the status bar wording. */
    static String fileBrowserName();

    MusicLibrary library{formatManager};
    AudioRecorder recorder;
    // My code ends here

    DJAudioPlayer player1{formatManager};
    // My code starts here
    // library and recorder added to the template's argument list.
    DeckGUI deckGUI1{&player1, formatManager, thumbCache, library, recorder};
    // My code ends here

    DJAudioPlayer player2{formatManager};
    // My code starts here
    // library and recorder added to the template's argument list.
    DeckGUI deckGUI2{&player2, formatManager, thumbCache, library, recorder};
    // My code ends here

    // My code starts here
    BPMAnalyser bpmAnalyser{formatManager};
    PlaylistComponent playlistComponent{library, bpmAnalyser};
    LoopSampler loopSampler{formatManager};

    TextButton micRecordButton{"RECORD MIC"};
    Label recorderStatusLabel;

    /** Scratch space for one deck's block; sized in prepareToPlay so the audio
        thread never allocates. Blocks larger than this are rendered in chunks. */
    AudioBuffer<float> deckBuffer;
    AudioBuffer<float> sampleBuffer;

    // Share of the window height given to the two decks; the playlist takes the rest.
    static constexpr float deckHeightProportion = 0.62f;
    static constexpr int recorderBarHeight = 28;

    /** Share of the bottom strip given to the playlist; the sampler takes the rest. */
    static constexpr float playlistWidthProportion = 0.65f;
    // My code ends here
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
