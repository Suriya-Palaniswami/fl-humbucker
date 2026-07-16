# FL Humbucker

Turn **your humming and beatboxing** into **patterns made from your chosen sample** — without wrestling with FL Studio routing.

## Core workflow

```
1. PERFORM     Hum a melody or beatbox a rhythm
2. PICK SOUND  Choose any WAV/sample from your packs
3. GET PATTERN Preview, export WAV, or export MIDI
```

You already know the musical idea. FL Humbucker captures **when** and **what pitch** you performed, then arranges **your sample** into that shape.

## Modes

| Mode | You perform | Plugin captures | Your sample becomes |
|------|-------------|-----------------|---------------------|
| **Melody (hum)** | A sung/hummed line | Pitch + timing | A melody shaped like your hum |
| **Rhythm (beatbox)** | A kick pattern, claps, etc. | Hit timing | Your sample on each hit |

### Options

- **Follow my pitch** — melody mode pitches your sample to match your hum
- **Quantize** — snap timing to 1/4, 1/8, 1/16, or 1/32
- **Preview Pattern** — hear the result inside the plugin
- **Export WAV Pattern** — drag into FL Playlist (easiest path)
- **Export MIDI** — import into piano roll / FPC if you prefer

## Build (Windows)

### Requirements

- Visual Studio 2022 (Desktop C++ workload) or Build Tools
- [CMake](https://cmake.org/download/) 3.22+
- Git

```bat
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Install the plugin:

```
build\fl_humbucker_artefacts\Release\VST3\FL Humbucker.vst3
  →  C:\Program Files\Common Files\VST3\
```

Rescan plugins in FL Studio.

## FL Studio quick start

See [docs/FL_STUDIO_SETUP.md](docs/FL_STUDIO_SETUP.md).

**Fastest path (no MIDI routing):**

1. Mic on a mixer track → insert **FL Humbucker**
2. **Record** your idea → **Stop**
3. **Choose Sample** (kick, snare, chop, pluck — anything)
4. **Preview Pattern** to check it
5. **Export WAV Pattern** → drag into the Playlist

Repeat for each layer (kick pass, snare pass, melody pass) — the way you'd naturally build a beat.

## Layering workflow (recommended)

This matches how arrangers think:

1. Beatbox a **kick rhythm** → pick your kick sample → export WAV → playlist
2. Beatbox a **snare rhythm** → pick snare sample → export WAV → layer
3. Hum a **bass or lead** → pick sample → enable **Follow my pitch** → export

No training, no MIDI ports, no drum classification required.

## Architecture

```
Mic performance
      │
      ▼
┌──────────────────┐
│ PerformanceCapture│  timed events (pitch or hits)
└────────┬─────────┘
         │
    ┌────┴────┐
    ▼         ▼
SampleSlot  PatternRenderer
 (your WAV)   (audio + MIDI)
    │
    ▼
PerformanceSampler (preview)
```

## Roadmap

- [x] Perform → pick sample → export pattern (core journey)
- [x] Melody + rhythm modes
- [x] Timeline preview
- [x] WAV + MIDI export
- [ ] Drag slice directly into FL Browser
- [ ] Multiple sample slots per pass
- [ ] Optional ML beatbox classifier

## License

MIT — see [LICENSE](LICENSE).
