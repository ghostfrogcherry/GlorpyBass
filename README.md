# Glorpy Bass

A minimal mono bass synth plugin for Ableton, aimed at a rubbery wet sound.

## Sound Design

- Sine plus slightly detuned saw core
- Square sub oscillator for weight
- Glide for slippery note transitions
- Low-pass filter with a fast envelope punch
- Wobble LFO for moving cutoff motion
- Saturation stage that adds the gooey `glorp`

## Main Controls

- `Wave`: blend sine to saw
- `Sub`: amount of sub oscillator
- `Cutoff`: base filter tone
- `Reso`: resonance
- `Glorp`: filter envelope and wetness character
- `Rate` / `Depth`: wobble movement
- `Glide`: note slide time
- `Drive`: saturation amount
- `Decay`: envelope release length
- `Level`: output level

## Build

Prerequisite: clone JUCE to `/home/gfc/JUCE`

```bash
cmake -S . -B build
cmake --build build
```

On Windows, generate for Visual Studio and build the `GlorpyBass_VST3` target for Ableton.
