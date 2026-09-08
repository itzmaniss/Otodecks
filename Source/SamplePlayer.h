#pragma once

#include <JuceHeader.h>

#include <memory>

// My code starts here

/**
	Plays one short sample, optionally looping. Lighter than a DJAudioPlayer:
	no speed control, no seeking, just fire and stop.

	`load` is message-thread only; the transport source it swaps is what the
	audio thread reads, so nothing else here loads a file.
*/
class SamplePlayer : public juce::AudioSource
{
public:
	explicit SamplePlayer(juce::AudioFormatManager& formatManagerToUse);
	~SamplePlayer() override;

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	bool load(const juce::File& file);

	/** Restarts from the beginning, so retriggering a sample behaves like a pad. */
	void play();
	void stop();

	bool isPlaying() const;

	void setLooping(bool shouldLoop);
	bool isLooping() const;

	/** Linear gain, not dB; clamped to 0-1. */
	void setGain(double gain);

private:
	juce::AudioFormatManager& formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;

	bool looping = false;

	int blockSize = 0;
	double currentSampleRate = 0.0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplePlayer)
};

// My code ends here
