#include "Track.h"

#include <memory>

// My code starts here

namespace
{
	// Keys used in library.json. Changing one breaks every saved library, so
	// they live here rather than being spelled out at each use site.
	const juce::Identifier pathKey { "path" };
	const juce::Identifier nameKey { "name" };
	const juce::Identifier lengthKey { "lengthInSeconds" };
	const juce::Identifier bpmKey { "bpm" };
	const juce::Identifier dateKey { "dateAdded" };
	const juce::Identifier cuesKey { "cuePoints" };
}

Track Track::fromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
	Track track;
	track.filePath = file.getFullPathName();
	track.displayName = file.getFileNameWithoutExtension();
	track.dateAdded = juce::Time::getCurrentTime();

	// A file the format manager cannot read still becomes a track, so the user
	// sees it in the table and can work out why it will not play.
	std::unique_ptr<juce::AudioFormatReader> reader { formatManager.createReaderFor(file) };

	if (reader != nullptr && reader->sampleRate > 0.0)
	{
		track.lengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
	}

	return track;
}

juce::var Track::toVar() const
{
	auto* fields = new juce::DynamicObject();

	fields->setProperty(pathKey, filePath);
	fields->setProperty(nameKey, displayName);
	fields->setProperty(lengthKey, lengthInSeconds);
	fields->setProperty(bpmKey, bpm);

	// Stored as milliseconds since the epoch so the file stays locale-independent.
	fields->setProperty(dateKey, dateAdded.toMilliseconds());

	juce::Array<juce::var> cues;

	for (const auto cue : cuePoints)
	{
		cues.add(cue);
	}

	fields->setProperty(cuesKey, cues);

	return juce::var(fields);
}

Track Track::fromVar(const juce::var& source)
{
	Track track;

	if (auto* fields = source.getDynamicObject())
	{
		track.filePath = fields->getProperty(pathKey).toString();
		track.displayName = fields->getProperty(nameKey).toString();
		track.lengthInSeconds = fields->getProperty(lengthKey);
		track.bpm = fields->getProperty(bpmKey);
		track.dateAdded = juce::Time(static_cast<juce::int64>(fields->getProperty(dateKey)));

		if (auto* cues = fields->getProperty(cuesKey).getArray())
		{
			// Read defensively: a hand-edited or older library file may carry a
			// different number of cues than this build expects.
			for (int i = 0; i < track.cuePoints.size(); ++i)
			{
				track.cuePoints.set(i, i < cues->size() ? static_cast<double>((*cues)[i]) : noCue);
			}
		}
	}

	if (track.displayName.isEmpty())
	{
		track.displayName = track.getFile().getFileNameWithoutExtension();
	}

	return track;
}

bool Track::hasCueAt(int index) const
{
	return juce::isPositiveAndBelow(index, cuePoints.size()) && cuePoints[index] >= 0.0;
}

juce::File Track::getFile() const
{
	return juce::File(filePath);
}

bool Track::exists() const
{
	return filePath.isNotEmpty() && getFile().existsAsFile();
}

// My code ends here
