#pragma once

#include <JuceHeader.h>

#include <atomic>
#include <functional>
#include <memory>

// My code starts here

class DJAudioPlayer;

/**
	Writes audio to a .wav file, either from a deck's output or from the
	microphone.

	Disk writing happens on a background thread via
	`AudioFormatWriter::ThreadedWriter`; the audio thread only ever hands over a
	block. Writing straight to disk from the audio callback produces audible
	glitches, so nothing here opens, closes or flushes a file on that thread.

	Threading: `startRecordingFromDeck`, `startRecordingFromMic` and `stop` are
	message-thread only. `captureDeckBlock` and the device callback are audio
	thread only. The two are separated by `writerLock`, a spin lock held for the
	length of one block copy and never while allocating.
*/
class AudioRecorder : public juce::AudioIODeviceCallback
{
public:
	enum class Source
	{
		none,
		deck,
		microphone
	};

	AudioRecorder();
	~AudioRecorder() override;

	/** Arms the given deck. The deck's audio reaches the file via captureDeckBlock. */
	bool startRecordingFromDeck(DJAudioPlayer* player, double sampleRate, int numChannels);

	/** Records the device's input. Needs the app to have opened input channels. */
	bool startRecordingFromMic();

	void stop();

	/** Called on the message thread with the finished file, once per recording.
	    Set by the owner; used to fold recordings into the library. */
	std::function<void(const juce::File&)> onRecordingFinished;

	bool isRecording() const;
	Source getSource() const;

	/** The file being written, or the last one written; may not exist yet. */
	juce::File getOutputFile() const;

	/** Blocks the writer could not keep up with since the recording started.
	    Non-zero means the file has gaps - worth showing rather than hiding. */
	int getDroppedBlockCount() const;

	/** True when this exact deck is the armed source. */
	bool isRecordingDeck(const DJAudioPlayer* player) const;

	/** Rate the open device runs at; 0 when no device has started. */
	double getDeviceSampleRate() const;

	/** Called after the deck has rendered a block. Audio thread only. */
	void captureDeckBlock(const DJAudioPlayer* player, const juce::AudioBuffer<float>& buffer,
	                      int startSample, int numSamples);

	void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
	void audioDeviceStopped() override;
	void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
	                                      int numInputChannels,
	                                      float* const* outputChannelData,
	                                      int numOutputChannels,
	                                      int numSamples,
	                                      const juce::AudioIODeviceCallbackContext& context) override;

private:
	/** Opens the file and hands it to a background writer. Message thread only. */
	bool beginWriting(const juce::File& file, double sampleRate, int numChannels,
	                  Source newSource);

	static constexpr int bitsPerSample = 16;

	/** Samples the writer thread may fall behind by before blocks are dropped.
	    At 44.1kHz this is a little over a second of slack. */
	static constexpr int writeBufferSize = 65536;

	juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };
	std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;

	/** Read on the audio thread, set on the message thread, guarded by writerLock. */
	juce::AudioFormatWriter::ThreadedWriter* activeWriter = nullptr;
	juce::SpinLock writerLock;

	/** Which deck is armed; compared by pointer on the audio thread. */
	std::atomic<const DJAudioPlayer*> armedPlayer { nullptr };
	std::atomic<Source> source { Source::none };

	/** Set when the device starts, so the mic path knows what rate to write at. */
	double deviceSampleRate = 0.0;

	/** Incremented on the audio thread, read on the message thread. */
	std::atomic<int> droppedBlocks { 0 };

	juce::File outputFile;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorder)
};

// My code ends here
