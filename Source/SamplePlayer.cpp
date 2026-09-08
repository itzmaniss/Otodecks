#include "SamplePlayer.h"

#include <memory>

// My code starts here

SamplePlayer::SamplePlayer(juce::AudioFormatManager& formatManagerToUse)
	: formatManager(formatManagerToUse)
{
}

SamplePlayer::~SamplePlayer()
{
	transportSource.setSource(nullptr);
}

void SamplePlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	blockSize = samplesPerBlockExpected;
	currentSampleRate = sampleRate;

	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void SamplePlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	if (readerSource == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	transportSource.getNextAudioBlock(bufferToFill);
}

void SamplePlayer::releaseResources()
{
	transportSource.releaseResources();
}

bool SamplePlayer::load(const juce::File& file)
{
	auto* reader = formatManager.createReaderFor(file);

	if (reader == nullptr)
	{
		return false;
	}

	auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
	newSource->setLooping(looping);

	// Detach the old source before it is freed, so the audio thread never holds
	// a transport pointing at a deleted reader.
	transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
	readerSource = std::move(newSource);

	if (currentSampleRate > 0.0)
	{
		transportSource.prepareToPlay(blockSize, currentSampleRate);
	}

	return true;
}

void SamplePlayer::play()
{
	if (readerSource == nullptr)
	{
		return;
	}

	transportSource.setPosition(0.0);
	transportSource.start();
}

void SamplePlayer::stop()
{
	transportSource.stop();
}

bool SamplePlayer::isPlaying() const
{
	return transportSource.isPlaying();
}

void SamplePlayer::setLooping(bool shouldLoop)
{
	looping = shouldLoop;

	if (readerSource != nullptr)
	{
		readerSource->setLooping(shouldLoop);
	}
}

bool SamplePlayer::isLooping() const
{
	return looping;
}

void SamplePlayer::setGain(double gain)
{
	transportSource.setGain(static_cast<float>(juce::jlimit(0.0, 1.0, gain)));
}

// My code ends here
