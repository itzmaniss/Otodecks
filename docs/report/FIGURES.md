# Figures to capture

The `.tex` compiles right now with every figure missing — each one renders as a
grey placeholder box naming the file it wants. Drop PNGs into `figures/` and they
replace the boxes on the next compile. No `.tex` edits needed.

All files go in `figures/` and must be named **exactly** as listed, `.png`.

---

## A. Code screenshots (CodeSnap)

Each snippet is already extracted to `snippets/` as its own file. Open one in
VS Code, select all (`Cmd+A`), then `Cmd+Shift+P` → **CodeSnap**, and save the
PNG under `figures/` with the same stem.

Suggested CodeSnap settings for a report: **no window controls**, **no line
numbers** (the snippets are fragments, so real line numbers would be wrong
anyway), container padding around 16px, and a light theme so it prints legibly.

| Save as | Snippet file | Shows |
|---|---|---|
| `01-deckgui-loadfile.png` | `01-deckgui-loadfile.cpp` | R1A — the single load path |
| `02-deck-defaults.png` | `02-deck-defaults.cpp` | R1D — slider defaults, speed skew |
| `03-position-nan-guard.png` | `03-position-nan-guard.cpp` | R1 — the NaN bug fix |
| `04-render-deck.png` | `04-render-deck.cpp` | R4A — per-deck render + recorder tap |
| `05-audio-block.png` | `05-audio-block.cpp` | R1B — two decks summed |
| `06-track-fromfile.png` | `06-track-fromfile.cpp` | R2A — reading duration |
| `07-library-search.png` | `07-library-search.cpp` | R2A — search in the library |
| `08-load-to-deck-callback.png` | `08-load-to-deck-callback.cpp` | R2B — decoupling via callback |
| `09-persistence-savejson.png` | `09-persistence-savejson.cpp` | R2C — atomic save |
| `10-library-prune.png` | `10-library-prune.cpp` | R2C — pruning dead rows |
| `11-hotcue-buttonclicked.png` | `11-hotcue-buttonclicked.cpp` | R3B — set / jump / alt-clear |
| `12-hotcue-setcue.png` | `12-hotcue-setcue.cpp` | R3A — setting a cue |
| `13-hotcue-grid.png` | `13-hotcue-grid.cpp` | R3 — the 8-slot grid layout |
| `14-hotcue-loadedtrack.png` | `14-hotcue-loadedtrack.cpp` | R3 — index, not pointer |
| `15-track-cue-persistence.png` | `15-track-cue-persistence.cpp` | R3D — defensive cue read |
| `16-recorder-arming.png` | `16-recorder-arming.cpp` | R4A — the arming-order bug fix |
| `17-recorder-capture.png` | `17-recorder-capture.cpp` | R4A — audio-thread try-lock |
| `18-recorder-stop.png` | `18-recorder-stop.cpp` | R4A — decide, tear down, announce |
| `19-recording-to-sampler.png` | `19-recording-to-sampler.cpp` | R4B — recording → library + sampler |
| `20-sampler-addrecorded.png` | `20-sampler-addrecorded.cpp` | R4B — sampler side of the same |
| `21-sampler-copyin.png` | `21-sampler-copyin.cpp` | R4C — copy into samples folder |
| `22-sampleplayer-play.png` | `22-sampleplayer-play.cpp` | R4B — retrigger from zero |
| `23-bpm-onset.png` | `23-bpm-onset.cpp` | R5D — onset strength |
| `24-bpm-autocorrelate.png` | `24-bpm-autocorrelate.cpp` | R5D — normalised autocorrelation |
| `25-bpm-parabolic.png` | `25-bpm-parabolic.cpp` | R5D — sub-frame interpolation |
| `26-bpm-octave-check.png` | `26-bpm-octave-check.cpp` | R5D — octave-error fix |
| `27-bpm-intervals.png` | `27-bpm-intervals.cpp` | R5D — median inter-onset estimate |
| `28-tap-tempo.png` | `28-tap-tempo.cpp` | R5D — tap tempo |
| `29-tap-finish.png` | `29-tap-finish.cpp` | R5D — deferred save |
| `30-recorder-threading-doc.png` | `30-recorder-threading-doc.h` | C2 — threading contract in the header |

---

## B. Application screenshots

Run the app (`cmake --build cmake-build-debug --target Otodecks`, then open
`cmake-build-debug/Otodecks_artefacts/Debug/Otodecks.app`).

On macOS, `Cmd+Shift+4` then `Space` captures a single window cleanly.

| Save as | What to capture |
|---|---|
| `app-overview.png` | The whole window, with tracks loaded on both decks and a populated library. This is the first figure a marker sees — make it look used, not empty. |
| `deck-controls.png` | One deck only. Transport, VOL, SPEED, POS, waveform with the playhead visible, the cue grid, REC. |
| `playlist.png` | The library table with several tracks, showing Title / Duration / BPM all populated. |
| `playlist-search.png` | The same library with a search term typed, showing it filtered down. Use a term that matches some but not all. |
| `hotcues-set.png` | The cue grid with roughly 4–5 slots set (orange, showing their seconds) and the rest empty — the contrast is the point. |
| `recording-active.png` | Mid-recording on deck one: REC red, deck two's REC greyed out, status bar naming the file. |
| `recording-saved.png` | Just after stopping: status bar showing the saved path in blue. |
| `sampler-panel.png` | The sampler with a few samples listed, **including one whose name starts with `deck`** — that shows R4B's automatic routing of recordings. |
| `bpm-column.png` | Close crop of the library table: some rows with BPM values, at least one showing `-`. |
| `tap-tempo.png` | The TAP button mid-count (showing `TAP 2`) or just after (showing the tempo). A crop of the button row is enough. |
| `data-folder.png` | Finder at `~/Library/Application Support/Otodecks/` showing `library.json`, `sampler.json`, `samples/`, `recordings/`. |
| `library-json.png` | `library.json` open in VS Code — CodeSnap this one too. Pick a track with a BPM and a couple of cue points set, so R2C and R3D are both evidenced in one image. |

---

## Compiling on Overleaf

Upload the whole `docs/report/` folder (or zip it). `Otodecks-Report.tex` is the
main document; pdfLaTeX, no special settings. `snippets/` is only source material
for the screenshots — it does not affect the build and does not need uploading.
