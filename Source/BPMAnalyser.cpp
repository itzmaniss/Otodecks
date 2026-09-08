#include "BPMAnalyser.h"

#include <cmath>
#include <memory>

// My code starts here

BPMAnalyser::BPMAnalyser(juce::AudioFormatManager& formatManagerToUse)
	: formatManager(formatManagerToUse)
{
}

double BPMAnalyser::analyse(const juce::File& file)
{
	auto envelope = computeEnergyEnvelope(file);

	if (envelope.size() < 2 || analysisSampleRate <= 0.0)
	{
		return 0.0;
	}

	// Onset strength: only rises in energy matter. A falling edge is the tail of
	// a hit, not a new one, so it is discarded rather than counted twice.
	juce::Array<float> onsetStrength;
	onsetStrength.ensureStorageAllocated(envelope.size());
	onsetStrength.add(0.0f);

	for (int i = 1; i < envelope.size(); ++i)
	{
		onsetStrength.add(juce::jmax(0.0f, envelope[i] - envelope[i - 1]));
	}

	auto correlation = autocorrelate(onsetStrength);

	if (correlation.isEmpty())
	{
		return 0.0;
	}

	auto minLag = lagForBPM(maxBPM);
	int bestOffset = 0;

	for (int i = 1; i < correlation.size(); ++i)
	{
		if (correlation[i] > correlation[bestOffset])
		{
			bestOffset = i;
		}
	}

	if (correlation[bestOffset] <= 0.0f)
	{
		return 0.0;
	}

	double bestLag = bestOffset + minLag;

	// Parabolic interpolation across the peak and its neighbours: the true lag
	// rarely lands exactly on a frame boundary, and at these hop sizes one frame
	// is worth a couple of BPM.
	if (bestOffset > 0 && bestOffset < correlation.size() - 1)
	{
		double before = correlation[bestOffset - 1];
		double centre = correlation[bestOffset];
		double after = correlation[bestOffset + 1];
		double denominator = before - 2.0 * centre + after;

		if (std::abs(denominator) > 1.0e-9)
		{
			bestLag += 0.5 * (before - after) / denominator;
		}
	}

	auto bpm = foldIntoRange(bpmForLag(bestLag));

	// Cross-check against the spacing of actual peaks. Autocorrelation happily
	// locks onto half or double the real tempo; if the intervals disagree by
	// roughly a factor of two, they are the better witness.
	auto fromIntervals = estimateFromIntervals(findPeaks(onsetStrength));

	if (fromIntervals > 0.0)
	{
		auto ratio = bpm / fromIntervals;

		if (ratio > 1.8 && ratio < 2.2)
		{
			bpm = foldIntoRange(bpm / 2.0);
		}
		else if (ratio > 0.45 && ratio < 0.55)
		{
			bpm = foldIntoRange(bpm * 2.0);
		}
	}

	return bpm;
}

juce::Array<float> BPMAnalyser::computeEnergyEnvelope(const juce::File& file)
{
	juce::Array<float> envelope;
	analysisSampleRate = 0.0;

	std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

	if (reader == nullptr || reader->sampleRate <= 0.0 || reader->lengthInSamples <= 0)
	{
		return envelope;
	}

	analysisSampleRate = reader->sampleRate;

	auto samplesToRead = juce::jmin(reader->lengthInSamples,
	                                static_cast<juce::int64>(maxAnalysisSeconds * reader->sampleRate));
	auto numChannels = static_cast<int>(juce::jmin(reader->numChannels, 2u));

	juce::AudioBuffer<float> block(numChannels, hopSize);
	envelope.ensureStorageAllocated(static_cast<int>(samplesToRead / hopSize) + 1);

	for (juce::int64 position = 0; position + hopSize <= samplesToRead; position += hopSize)
	{
		reader->read(&block, 0, hopSize, position, true, numChannels > 1);

		// Mean square across channels: a mono sum is enough for onset detection
		// and costs less than treating the channels separately.
		double sum = 0.0;

		for (int channel = 0; channel < numChannels; ++channel)
		{
			const auto* samples = block.getReadPointer(channel);

			for (int i = 0; i < hopSize; ++i)
			{
				sum += static_cast<double>(samples[i]) * samples[i];
			}
		}

		envelope.add(static_cast<float>(std::sqrt(sum / (hopSize * numChannels))));
	}

	return envelope;
}

juce::Array<int> BPMAnalyser::findPeaks(const juce::Array<float>& onsetStrength) const
{
	juce::Array<int> peaks;

	if (onsetStrength.size() < 3)
	{
		return peaks;
	}

	double total = 0.0;

	for (auto value : onsetStrength)
	{
		total += value;
	}

	// A peak has to beat both its neighbours and the track's average rise,
	// otherwise every ripple in a quiet passage counts as a beat.
	auto threshold = static_cast<float>(total / onsetStrength.size()) * 1.5f;

	for (int i = 1; i < onsetStrength.size() - 1; ++i)
	{
		if (onsetStrength[i] > threshold
			&& onsetStrength[i] >= onsetStrength[i - 1]
			&& onsetStrength[i] > onsetStrength[i + 1])
		{
			peaks.add(i);
		}
	}

	return peaks;
}

juce::Array<float> BPMAnalyser::autocorrelate(const juce::Array<float>& onsetStrength) const
{
	juce::Array<float> correlation;

	auto minLag = lagForBPM(maxBPM);
	auto maxLag = lagForBPM(minBPM);

	if (minLag < 1 || maxLag <= minLag || onsetStrength.size() <= maxLag * 2)
	{
		return correlation;
	}

	correlation.ensureStorageAllocated(maxLag - minLag + 1);

	for (int lag = minLag; lag <= maxLag; ++lag)
	{
		double sum = 0.0;

		for (int i = 0; i + lag < onsetStrength.size(); ++i)
		{
			sum += static_cast<double>(onsetStrength[i]) * onsetStrength[i + lag];
		}

		// Divide by the overlap length, or long lags are penalised simply for
		// having fewer samples to add up.
		correlation.add(static_cast<float>(sum / (onsetStrength.size() - lag)));
	}

	return correlation;
}

double BPMAnalyser::estimateFromIntervals(const juce::Array<int>& peaks) const
{
	if (peaks.size() < minimumTaps || analysisSampleRate <= 0.0)
	{
		return 0.0;
	}

	juce::Array<int> intervals;

	for (int i = 1; i < peaks.size(); ++i)
	{
		intervals.add(peaks[i] - peaks[i - 1]);
	}

	// The median shrugs off the occasional missed or doubled onset in a way a
	// mean does not.
	intervals.sort();
	auto medianFrames = intervals[intervals.size() / 2];

	if (medianFrames <= 0)
	{
		return 0.0;
	}

	return foldIntoRange(bpmForLag(medianFrames));
}

void BPMAnalyser::registerTap()
{
	// EXTENSION: manual tap tempo, the fallback for tracks the analysis cannot read.
	auto now = juce::Time::currentTimeMillis();

	if (! tapTimesMs.isEmpty() && now - tapTimesMs.getLast() > tapTimeoutMs)
	{
		// Too long since the last tap to be the same count.
		tapTimesMs.clear();
	}

	tapTimesMs.add(now);
}

void BPMAnalyser::clearTaps()
{
	tapTimesMs.clear();
}

double BPMAnalyser::getBPMFromUserTap() const
{
	// EXTENSION: manual tap tempo, the fallback for tracks the analysis cannot read.
	if (tapTimesMs.size() < minimumTaps)
	{
		return 0.0;
	}

	auto span = tapTimesMs.getLast() - tapTimesMs.getFirst();

	if (span <= 0)
	{
		return 0.0;
	}

	auto averageGapMs = static_cast<double>(span) / (tapTimesMs.size() - 1);

	return foldIntoRange(60000.0 / averageGapMs);
}

double BPMAnalyser::bpmForLag(double lagInFrames) const
{
	if (lagInFrames <= 0.0 || analysisSampleRate <= 0.0)
	{
		return 0.0;
	}

	auto secondsPerFrame = hopSize / analysisSampleRate;

	return 60.0 / (lagInFrames * secondsPerFrame);
}

int BPMAnalyser::lagForBPM(double bpm) const
{
	if (bpm <= 0.0 || analysisSampleRate <= 0.0)
	{
		return 0;
	}

	auto secondsPerFrame = hopSize / analysisSampleRate;

	return juce::roundToInt(60.0 / (bpm * secondsPerFrame));
}

double BPMAnalyser::foldIntoRange(double bpm)
{
	if (bpm <= 0.0)
	{
		return 0.0;
	}

	while (bpm < minBPM)
	{
		bpm *= 2.0;
	}

	while (bpm > maxBPM)
	{
		bpm /= 2.0;
	}

	return bpm;
}

// My code ends here
