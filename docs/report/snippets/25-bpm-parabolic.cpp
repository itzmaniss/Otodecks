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
