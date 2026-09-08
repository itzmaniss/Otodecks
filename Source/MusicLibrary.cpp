#include "MusicLibrary.h"
#include "PersistenceManager.h"

// My code starts here

const juce::Identifier MusicLibrary::tracksKey { "tracks" };

MusicLibrary::MusicLibrary(juce::AudioFormatManager& formatManagerToUse)
	: formatManager(formatManagerToUse)
{
}

bool MusicLibrary::addTrack(const juce::File& file)
{
	if (findTrackByPath(file.getFullPathName()) >= 0)
	{
		return false;
	}

	tracks.add(Track::fromFile(file, formatManager));
	sendChangeMessage();

	return true;
}

int MusicLibrary::addTracks(const juce::Array<juce::File>& files)
{
	int added = 0;

	for (const auto& file : files)
	{
		// addTrack broadcasts per file; harmless here, the table coalesces repaints.
		if (addTrack(file))
		{
			++added;
		}
	}

	return added;
}

void MusicLibrary::removeTrack(int index)
{
	if (! juce::isPositiveAndBelow(index, tracks.size()))
	{
		return;
	}

	tracks.remove(index);
	sendChangeMessage();
}

int MusicLibrary::getNumTracks() const
{
	return tracks.size();
}

const Track& MusicLibrary::getTrack(int index) const
{
	jassert(juce::isPositiveAndBelow(index, tracks.size()));
	return tracks.getReference(index);
}

void MusicLibrary::updateTrack(int index, const Track& updated)
{
	if (! juce::isPositiveAndBelow(index, tracks.size()))
	{
		return;
	}

	tracks.set(index, updated);
	sendChangeMessage();
}

int MusicLibrary::findTrackByPath(const juce::String& path) const
{
	for (int i = 0; i < tracks.size(); ++i)
	{
		if (tracks.getReference(i).filePath == path)
		{
			return i;
		}
	}

	return -1;
}

juce::Array<int> MusicLibrary::findTracksMatching(const juce::String& searchTerm) const
{
	// EXTENSION: search over the library, backing the playlist search box.
	juce::Array<int> matches;
	auto term = searchTerm.trim();

	for (int i = 0; i < tracks.size(); ++i)
	{
		if (term.isEmpty() || tracks.getReference(i).displayName.containsIgnoreCase(term))
		{
			matches.add(i);
		}
	}

	return matches;
}

juce::var MusicLibrary::toVar() const
{
	juce::Array<juce::var> entries;

	for (const auto& track : tracks)
	{
		entries.add(track.toVar());
	}

	auto* root = new juce::DynamicObject();
	root->setProperty(tracksKey, entries);

	return juce::var(root);
}

void MusicLibrary::loadFromVar(const juce::var& source)
{
	tracks.clear();

	if (auto* root = source.getDynamicObject())
	{
		if (auto* entries = root->getProperty(tracksKey).getArray())
		{
			for (const auto& entry : *entries)
			{
				auto track = Track::fromVar(entry);

				// A track whose file has since been moved or deleted is dropped
				// rather than shown as a row that can never play.
				if (track.exists())
				{
					tracks.add(track);
				}
			}
		}
	}

	sendChangeMessage();
}

bool MusicLibrary::save() const
{
	return PersistenceManager::saveJSON(PersistenceManager::getLibraryFile(), toVar());
}

void MusicLibrary::load()
{
	loadFromVar(PersistenceManager::loadJSON(PersistenceManager::getLibraryFile()));
}

// My code ends here
