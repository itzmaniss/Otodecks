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
