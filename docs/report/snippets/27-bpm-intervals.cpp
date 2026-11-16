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
