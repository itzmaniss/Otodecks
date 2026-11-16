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
