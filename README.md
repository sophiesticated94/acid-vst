# AcidLab303

AcidLab303 is an original JUCE/C++ VST3 instrument for Windows: a practical analog-style acid bassline synth with a 303-inspired workflow, internal step sequencer, accent/slide behavior, resonant low-pass filtering, saturation, compact FX, factory presets, MIDI export, and reactive distortion themes.

It is not an Arturia clone and does not use Arturia branding, presets, artwork, or copied interface assets.

## Build

From PowerShell:

```powershell
.\scripts\build.ps1
```

For a clean rebuild:

```powershell
.\scripts\clean-rebuild.ps1
```

The script uses the Visual Studio CMake bundled on this machine when `cmake` is not on `PATH`. The VST3 output is expected at:

```text
build\AcidLab303_artefacts\Release\VST3\AcidLab303.vst3
```

## Install For FL Studio

Run PowerShell as Administrator, then:

```powershell
.\scripts\install-vst3.ps1
```

This copies the plugin to:

```text
C:\Program Files\Common Files\VST3\AcidLab303.vst3
```

Then open FL Studio, rescan plugins, and add AcidLab303 as an instrument.

Copy the outer `AcidLab303.vst3` folder bundle, not the inner binary under `Contents\x86_64-win`. FL Studio is much more reliable with VST3 bundles in `C:\Program Files\Common Files\VST3` than in old VST2 folders like `C:\Program Files\Steinberg\VstPlugins`.

To create a local copyable package:

```powershell
.\scripts\package-local.ps1
```

To run VST3 validation when Steinberg's validator is installed:

```powershell
.\scripts\validate-vst3.ps1
```

## v0.7 Controls

- Layout: modern preset sidebar plus tabbed main editor. `Synth` is the default tab; deeper controls live under `Sequencer`, `Mod`, `Effects`, and `Analog`.
- Engine/theme: eleven distortion engines use the existing distortion mode parameter and recolor the global UI accent, step focus, buttons, knobs, and performance visual.
- Engines: Warm, Bite, Fold, Fuzz, Zap, Tube, Crush, Melt, Tape, Ring, and Shred. Each engine has three independently remembered `Character` controls with engine-specific labels and behavior.
- Synth: waveform, Clean/Acid filter model, tune, pulse width, cutoff, resonance, envelope amount, decay, accent, slide, sub level, volume.
- Sequencer: 16/32/64 steps, 1/8/1/16/1/32/triplet rates, A-H pattern slots, scale lock, mutation, copy/paste/clear/randomize, per-step note, octave, accent, slide, gate, rest.
- Mod: modulation-focused performance controls, including mutation, envelope amount, accent, slide, decay, and accent attack.
- Effects: compact engine selector, distortion drive/tone/mix, three adaptive Character controls, delay, modulation, reverb, FX tone, and an engine-specific reactive performance visual.
- Modulation: three LFOs with free/synced rates and seven shapes, two note-gated ADSR envelopes, and six bipolar matrix slots. Matrix targets include synth, FX, analog, volume, and active-engine Character A/B/C controls.
- Analog: off/2x/4x oversampling, filter tracking, bass boost, accent attack, clipping level, drift, circuit noise.
- Presets: factory/user source filtering, text search, category filter, file-based loading, `SAVE` for user presets, and `SAVE AS` for factory or unsaved sounds.
- Compatibility: preset format v3 preserves engine Character memory and modulation routing. v0.3-v0.5 presets and plugin state migrate legacy Warm/Bite/Fold/Fuzz/Zap choices to their unchanged algorithms.
- MIDI export: saves the active pattern as a `.mid` file; dragging the MIDI button exports a temporary MIDI file for hosts that accept file drops.
- UI: custom knob rendering and fixed-aspect resize limits.
- Pattern banks: save/load `.acidpattern.json` files for A-H pattern slots.
- User assets: optional Documents install via `scripts\install-user-assets.ps1`.

## Code Layout

- `src/dsp`: synth engine and DSP primitives.
- `src/model`: pattern bank model, MIDI pattern export, JSON helpers.
- `src/plugin`: JUCE processor and plugin state coordination.
- `src/ui`: editor and UI components.

## Verification

```powershell
.\scripts\package-local.ps1
.\scripts\verify-package.ps1
.\scripts\validate-vst3.ps1
```

## Next Iterations

- Calibrate filter and accent behavior against real 303-style reference audio.
- Add true MIDI drag/drop after file export works reliably in more DAWs.
- Add full pattern-only import/export browser polish.
- Add richer preset tags, author/category editing, and favorites.
- Add screenshot-based UI checks for the tabbed editor.
- Add Steinberg `vst3validator` to the toolchain once installed.
- Add installer packaging instead of requiring admin PowerShell copy.
- Add screenshots or standalone UI verification once the UI is visually final.
