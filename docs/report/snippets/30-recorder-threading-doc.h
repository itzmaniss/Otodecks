/**
	Writes audio to a .wav file, either from a deck's output or from the
	microphone.

	Disk writing happens on a background thread via
	`AudioFormatWriter::ThreadedWriter`; the audio thread only ever hands over a
	block. Writing straight to disk from the audio callback produces audible
	glitches, so nothing here opens, closes or flushes a file on that thread.

	Threading: `startRecordingFromDeck`, `startRecordingFromMic` and `stop` are
	message-thread only. `captureDeckBlock` and the device callback are audio
	thread only. The two are separated by `writerLock`, a spin lock held for the
	length of one block copy and never while allocating.
*/
