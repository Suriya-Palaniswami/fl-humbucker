# FL Studio Setup Guide

This guide walks through using **FL Humbucker** in Image-Line FL Studio.

## 1. Install the plugin

After building, ensure FL Studio can see the VST3:

- **macOS:** `~/Library/Audio/Plug-Ins/VST3/FL Humbucker.vst3`
- **Windows:** `C:\Program Files\Common Files\VST3\FL Humbucker.vst3`

In FL Studio: **Options → Manage plugins → Find plugins** (or rescan).

## 2. Create an input track

1. Add a new **Mixer** track
2. Set its input to your microphone (e.g. `Mic 1`)
3. Insert **FL Humbucker** as an effect on that track
4. Turn up **Input Gain** in the plugin until your performance peaks without clipping

## 3. Route MIDI to a drum/synth channel

FL Studio routes VST3 MIDI through each plugin's settings panel:

1. Click the **gear icon** on the FL Humbucker wrapper
2. Under **Output**, pick a MIDI port (e.g. `Port 1`)
3. On your target instrument channel (FPC, Drum Rack, synth, etc.):
   - Open its wrapper gear menu
   - Set **Input** to the same port

Alternative: use **Patcher** and cable FL Humbucker's MIDI out to your instrument.

## 4. Modes

### Hum to MIDI

- Select **Hum to MIDI**
- Route MIDI to any melodic instrument (3x Osc, Serum, etc.)
- Optional: enable **Quantize** + grid (1/16 works well for melodies)

### Beatbox to Drums

- Select **Beatbox to Drums**
- Route MIDI to **FPC**, **Slicex**, or a GM drum map
- Default mapping:
  - Kick → 36
  - Snare → 38
  - Hi-hat → 42
  - Clap → 39
  - Tom → 45

### Capture Samples

- Select **Capture Samples**
- Each detected hit saves a slice (16 slots, rolling)
- Click **Export Last Slice** to write a `.wav` for the Playlist or Browser

## 5. Record a pattern

**Live recording into piano roll:**

1. Enable **Record Pattern** in the plugin
2. Press FL Studio's global record button
3. Perform your beatbox/hum
4. Stop recording — notes appear on the target instrument's piano roll

**Export MIDI file:**

1. Record with **Record Pattern** enabled
2. Click **Export MIDI**
3. Drag the `.mid` into FL Studio or import via **File → Import → MIDI file**

FL Studio also offers **Tools → Burn MIDI** on some plugin wrappers — useful if live routing is flaky.

## 6. Troubleshooting

| Problem | Fix |
|---------|-----|
| No MIDI reaching synth | Check gear-menu port routing on both plugin and instrument |
| Plugin mutes track | Keep **Monitor** at 0; ensure input bus is enabled |
| Wrong drum sounds | Target instrument must use GM drum mapping (FPC bank A) |
| Missed hits | Raise **Onset Sensitivity** or **Input Gain** |
| False hum notes | Raise **Pitch Confidence** threshold |
| Latency | Lower FL buffer size; expect ~20–50 ms analysis latency |

## 7. Suggested workflow

1. Loop a beat in FL Studio
2. Hum a bassline idea → **Hum to MIDI** → 3x Osc
3. Switch to **Beatbox to Drums**, quantize to 1/16, lay a kick/snare pattern
4. Switch to **Capture Samples** for one-shot vocal FX
5. Export MIDI + WAV slices into your project folder
