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

Prerequisite: this project currently expects JUCE at `/home/gfc/JUCE` because `CMakeLists.txt` uses a fixed path.

## Windows Ableton Build

Install:

- Visual Studio 2022 with Desktop development for C++
- CMake
- Git

JUCE resources:

- JUCE GitHub: `https://github.com/juce-framework/JUCE`
- JUCE website: `https://juce.com/`
- JUCE CMake docs: `https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md`
- JUCE getting started docs: `https://juce.com/learn/documentation/`

A simple Windows folder layout is:

```text
C:\dev\JUCE
C:\dev\GlorpyBass
```

Clone JUCE:

```powershell
git clone https://github.com/juce-framework/JUCE.git C:\dev\JUCE
```

Clone this project:

```powershell
git clone https://github.com/ghostfrogcherry/GlorpyBass.git C:\dev\GlorpyBass
cd C:\dev\GlorpyBass
```

Clone JUCE somewhere on your Windows machine, then either:

- edit `CMakeLists.txt` and replace `add_subdirectory("/home/gfc/JUCE" JUCE)` with your Windows JUCE path
- or move JUCE and the project so the path matches your local setup

Example JUCE path change:

```cmake
add_subdirectory("C:/dev/JUCE" JUCE)
```

If CMake cannot find the Visual Studio compiler, open `x64 Native Tools Command Prompt for VS 2022` or `Developer PowerShell for VS 2022` and run the commands there.

Then build from a Developer PowerShell or normal PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release --target GlorpyBass_VST3
```

The built plugin will end up under the build artefacts folder as a `.vst3` bundle.

Typical output path:

```text
C:\dev\GlorpyBass\build\GlorpyBass_artefacts\VST3\Glorpy Bass.vst3
```

Install it for Ableton by copying the resulting `Glorpy Bass.vst3` into:

```text
C:\Program Files\Common Files\VST3\
```

Then in Ableton Live:

1. Open `Options > Settings > Plug-Ins`
2. Make sure VST3 plug-ins are enabled
3. Rescan plug-ins
4. Load `Glorpy Bass` on a MIDI track

If Ableton still does not see it:

1. Confirm the file is inside `C:\Program Files\Common Files\VST3\`
2. Make sure you built the `Release` configuration
3. Rescan plug-ins again from Ableton settings
4. Restart Ableton after copying the plugin

## Linux Test Build

If JUCE is available at `/home/gfc/JUCE`, you can build and test locally with:

```bash
cmake -S . -B build
cmake --build build
./build/GlorpyBass_artefacts/Standalone/Glorpy\ Bass
```
