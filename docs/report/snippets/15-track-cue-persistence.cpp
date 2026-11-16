		if (auto* cues = fields->getProperty(cuesKey).getArray())
		{
			// Read defensively: a hand-edited or older library file may carry a
			// different number of cues than this build expects.
			for (int i = 0; i < track.cuePoints.size(); ++i)
			{
				track.cuePoints.set(i, i < cues->size() ? static_cast<double>((*cues)[i]) : noCue);
			}
		}
