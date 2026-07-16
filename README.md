# FL Humbucker

Turn beatboxing and humming into **MIDI patterns** and **audio samples** inside FL Studio.

FL Humbucker is a VST3/AU audio plugin built with [JUCE](https://juce.com/). It listens to your microphone, analyzes what you perform in real time, and outputs either MIDI notes or captured WAV slices you can drop into the playlist.

## What it does

| Mode | Input | Output |
|------|-------|--------|
| **Hum to MIDI** | Sung/hummed melody | Monophonic MIDI notes (pitch-tracked) |
| **Beatbox to Drums** | Kick, snare, hi-hat, etc. | General MIDI drum hits |
| **Capture Samples** | Any vocal percussion | 16-slot rolling buffer of WAV slices |

All modes support optional **grid quantization** and **pattern recording** with MIDI export.

## Architecture

```
Microphone
    │
    ▼
┌─────────────────────────────────────────┐
│  JUCE Plugin (VST3 / AU / Standalone)   │
│                                         │
│  ┌─────────────┐   ┌─────────────────┐  │
│  │ Pitch (YIN) │   │ Onset Detector  │  │
│  └──────┬──────┘   └────────┬────────┘  │
│         │                   │           │
│         ▼                   ▼           │
│  ┌─────────────┐   ┌─────────────────┐  │
│  │ Hum → MIDI  │   │ Beatbox Classify│  │
│  └──────┬──────┘   └────────┬────────┘  │
│         │                   │           │
│         └────────┬──────────┘           │
│                  ▼                      │
│         ┌─────────────────┐             │
│         │  MIDI Emitter   │             │
│         │ Pattern Recorder│             │
│         │  Sample Capture │             │
│         └────────┬────────┘             │
└──────────────────┼──────────────────────┘
                   ▼
         FL Studio / other DAW
```

### Analysis modules

- **PitchDetector** — YIN-style autocorrelation for monophonic humming
- **OnsetDetector** — energy + spectral-flux transient detection
- **BeatboxClassifier** — heuristic spectral bands (kick/snare/hat/tom/clap)
- **SampleCapture** — ring-buffer slicer with pre/post roll
- **MidiEmitter** — note output with optional quantization
- **PatternRecorder** — records events for `.mid` export

## Build

### Requirements

- CMake 3.22+
- C++17 compiler (Clang, GCC, or MSVC)
- Git (JUCE is fetched automatically)

### macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 is copied to:
`~/Library/Audio/Plug-Ins/VST3/FL Humbucker.vst3`

### Windows

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Install the built `.vst3` from `build/fl_humbucker_artefacts/Release/VST3/` into:
`C:\Program Files\Common Files\VST3\`

### Use a local JUCE checkout

```bash
cmake -B build -DJUCE_PATH=/path/to/JUCE
cmake --build build
```

## FL Studio setup

See [docs/FL_STUDIO_SETUP.md](docs/FL_STUDIO_SETUP.md) for routing MIDI to other channels.

Quick version:

1. Rescan plugins in FL Studio (Options → Manage plugins)
2. Add **FL Humbucker** on an **audio input** mixer track (enable your mic)
3. Open the plugin **gear menu** → set **MIDI output** to a free port
4. On your drum/synth channel, set **MIDI input** to the same port
5. Arm record + enable **Record to piano roll** to capture patterns

> **Note:** VST3 has no native "MIDI effect" type. This plugin is registered as a synth with silent audio output so FL Studio can route its MIDI out to other instruments.

## Roadmap

### v0.1 (current) — MVP
- [x] Real-time pitch tracking (hum → MIDI)
- [x] Onset detection + heuristic drum classification
- [x] Sample capture on transients
- [x] Pattern record + MIDI export
- [x] Basic plugin UI

### v0.2 — Better recognition
- [ ] Train a small CNN classifier (see [Deepbox](https://github.com/eupston/Deepbox) for reference)
- [ ] User calibration ("this sound = kick")
- [ ] Velocity curves and note length from envelope

### v0.3 — DAW integration
- [ ] Drag-and-drop slices into FL Browser
- [ ] Direct FLP pattern export
- [ ] FL Studio "Burn MIDI" helper workflow

### v0.4 — Polish
- [ ] Latency compensation display
- [ ] Noise gate / input auto-gain
- [ ] Presets per beatbox style

## Improving accuracy

**Humming**
- Use a quiet room; hum clearly into the mic
- Lower **Pitch Confidence** if notes are missed; raise it if you get false triggers
- Enable **Quantize** when laying ideas over a beat

**Beatboxing**
- Exaggerate the difference between kick (deep chest), snare (sharp "ka"), and hat (tight "ts")
- Increase **Onset Sensitivity** for soft hits; decrease for noisy environments
- For best results, plan to train a custom model on *your* sounds (v0.2)

## License

MIT — see [LICENSE](LICENSE).

Analysis code is original. JUCE is used under its own license when you build the project.

## Related projects

- [Deepbox](https://github.com/eupston/Deepbox) — beatbox CNN → MIDI (great inspiration for ML path)
- [aubio](https://aubio.org/) — mature onset/pitch library (GPL; optional future integration)
- [rt-slicer](https://github.com/dnewcome/rt-slicer) — live transient slicing reference
