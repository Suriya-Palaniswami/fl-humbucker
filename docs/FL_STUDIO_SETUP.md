# FL Studio Setup Guide

FL Humbucker is designed so you **don't need MIDI routing** for the core workflow.

## Recommended workflow (easiest)

### 1. Set up the mic

1. Open FL Studio
2. Add a **Mixer** track
3. Set input to your microphone
4. Insert **FL Humbucker** on that track
5. Raise **Input** gain until you see events when you perform (optional: raise **Monitor** to hear yourself)

### 2. Perform your idea

1. Choose **Melody (hum)** or **Rhythm (beatbox)**
2. Click **Record**
3. Hum or beatbox your pattern once
4. Click **Stop**
5. Check the timeline — dots should show your captured pattern

### 3. Pick your sound

1. Click **Choose Sample...**
2. Browse to any WAV from your free packs (kick, snare, vocal chop, synth pluck, etc.)
3. For melodies, keep **Follow my pitch** on so the sample follows your hum

### 4. Get your pattern

| Button | What you get | Best for |
|--------|--------------|----------|
| **Preview Pattern** | Hear it through the plugin | Quick check |
| **Export WAV Pattern** | One audio file with your sample placed on the pattern | **Drag into Playlist** |
| **Export MIDI** | Standard `.mid` file | Piano roll / FPC workflow |

**Playlist method (recommended):**

1. Export WAV
2. Drag the file into the FL **Playlist**
3. Line it up with your beat
4. Repeat for the next layer (new performance, new sample)

## Layering example

Build a beat the way you hear it in your head:

| Pass | Mode | You do | Sample | Output |
|------|------|--------|--------|--------|
| 1 | Rhythm | Beatbox "boots boots boots" | `kick.wav` | Kick layer WAV |
| 2 | Rhythm | Beatbox snare pattern | `snare.wav` | Snare layer WAV |
| 3 | Melody | Hum bass line | `bass_pluck.wav` | Bass melody WAV |

Stack the WAVs on separate playlist tracks. You arrange — the plugin executes.

## Optional: MIDI workflow

If you prefer piano roll editing:

1. Export MIDI
2. **File → Import → MIDI file** (or drag into a channel)
2. Load your sample in **FPC**, **Slicex**, or **DirectWave**
3. All notes are on one pitch (C4 / note 60) for rhythm mode
4. Melody mode exports the pitches you hummed

## Settings tips

| Control | When to adjust |
|---------|----------------|
| **Sensitivity** | Missed beatbox hits → raise. False triggers → lower |
| **Pitch** | Missed hum notes → lower. Wrong notes → raise |
| **Quantize** | Turn on when performing over a beat in FL |
| **BPM** | Match your project tempo for quantize + MIDI export |
| **Monitor** | Hear your mic while recording |
| **Preview** | Volume of pattern preview playback |

## Troubleshooting

| Problem | Fix |
|---------|-----|
| No events captured | Check mic input on mixer track; raise Input gain |
| Timeline empty after stop | Perform louder/clearer; lower Pitch threshold for hum |
| Export WAV is silent | Choose a sample first; make sure events were captured |
| Melody sounds wrong pitch | Toggle **Follow my pitch**; try a shorter sample |
| Pattern feels off-time | Enable **Quantize**, set grid to 1/16, match **BPM** to project |
| Can't find plugin | Rescan VST3 folder; confirm install path |

## Why this is easier than routing MIDI

Old approach: mic → plugin → MIDI port → another channel → record → piano roll.

**New approach:** mic → plugin → **WAV file** → playlist.

You skip the parts of FL Studio that block your ideas. You still layer and arrange — you just start from audio patterns you can hear immediately.
