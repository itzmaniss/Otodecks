				// A track whose file has since been moved or deleted is dropped
				// rather than shown as a row that can never play.
				if (track.exists())
				{
					tracks.add(track);
				}
