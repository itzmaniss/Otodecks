#pragma once

#include <JuceHeader.h>

// My code starts here

/**
	Estimates a track's tempo from its audio.

	The method is the usual three stages: reduce the file to an onset strength
	envelope, autocorrelate that envelope over the lags a plausible tempo could
	occupy, and read the tempo off the strongest lag. Inter-onset intervals are
	computed too and used as a sanity check when autocorrelation is ambiguous.

	Analysis reads the file, so it must not run on the audio thread. It is
	message-thread work and takes a noticeable moment on a long track.
*/
class BPMAnalyser
{
public:
	explicit BPMAnalyser(juce::AudioFormatManager& formatManagerToUse);

	/** Returns 0 when the file cannot be read or no tempo stands out. */
	double analyse(const juce::File& file);

	/** Frame-by-frame energy of the file, one value per hop. */
	juce::Array<float> computeEnergyEnvelope(const juce::File& file);

	/** Frame indices where the onset strength rises above the local average. */
	juce::Array<int> findPeaks(const juce::Array<float>& onsetStrength) const;

	/** Correlation of the envelope with itself, one value per lag in the tempo range. */
	juce::Array<float> autocorrelate(const juce::Array<float>& onsetStrength) const;

	/** Median gap between peaks, expressed as a tempo. 0 when there are too few. */
	double estimateFromIntervals(const juce::Array<int>& peaks) const;

	/** Fallback for tracks the analysis cannot read: the user taps the beat. */
	void registerTap();
	void clearTaps();
	double getBPMFromUserTap() const;

	/** Tempo search bounds. Anything outside is folded into this range. */
	static constexpr double minBPM = 70.0;
	static constexpr double maxBPM = 180.0;

private:
	/** Converts a lag in frames to a tempo, and back. */
	double bpmForLag(double lagInFrames) const;
	int lagForBPM(double bpm) const;

	/** Halves or doubles a tempo until it sits inside the search bounds. */
	static double foldIntoRange(double bpm);

	/** Samples per envelope frame. Smaller means finer tempo resolution and more work. */
	static constexpr int hopSize = 256;

	/** Only the opening of a track is analysed; tempo rarely changes and long
	    files would otherwise make the user wait. */
	static constexpr double maxAnalysisSeconds = 60.0;

	/** Taps older than this start a new count rather than averaging with a stale one. */
	static constexpr int tapTimeoutMs = 2500;

	/** Below this many taps there is not enough to average. */
	static constexpr int minimumTaps = 3;

	juce::AudioFormatManager& formatManager;

	/** Sample rate of the file last analysed; used to turn lags into seconds. */
	double analysisSampleRate = 0.0;

	juce::Array<juce::int64> tapTimesMs;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BPMAnalyser)
};

// My code ends here
