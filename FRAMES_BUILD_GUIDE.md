# Frames Firmware Beginner Guide

This document explains, step by step, how to set up the environment, modify the Frames firmware, build it, and generate the audio `.wav` file for updating the module. It reflects the exact workflow we used in this repo.

## What We Achieved

1. Added a **preset keyframe table** that can be compiled into the firmware.
2. Modified the firmware so the preset is **saved into flash on first boot**.
3. Set up the **ARM toolchain** to build the firmware.
4. Generated the **`.wav` file** for the audio bootloader.

## Repo Layout (Relevant Parts)

- `frames/` — Frames firmware code.
- `stmlib/` — Shared STM32 support libraries (submodule).
- `stm_audio_bootloader/` — Audio encoder used to make the `.wav` update file.
- `build/frames/` — Build output directory (contains `.elf`, `.hex`, `.bin`, `.wav`).

## Prerequisites (Mac)

### 1. Git (to pull submodules)
If Git is not installed, install the Xcode Command Line Tools:

```bash
xcode-select --install
```

### 2. Homebrew
If you don’t already have Homebrew:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Python 3 + pip
macOS ships with Python 3, but you can check:

```bash
python3 --version
```

You also need `pip` for Python packages:

```bash
python3 -m pip --version
```

### 4. Install ARM GCC Toolchain
The firmware requires the ARM embedded compiler.

We used the Arm GNU Toolchain installed by Homebrew:

```bash
brew install gcc-arm-embedded
```

This installs under:

```
/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

### 5. Python Dependency: NumPy
The audio encoder requires NumPy.

```bash
python3 -m pip install numpy
```

## VS Code Setup (Recommended)

You can use any editor, but VS Code is friendly for beginners.

### Install VS Code
Download from: https://code.visualstudio.com

### Recommended Extensions
These are optional but helpful:

- **C/C++** (Microsoft) — syntax highlighting, IntelliSense.
- **C/C++ Extension Pack** (Microsoft) — bundle of C/C++ tools.
- **Makefile Tools** (Microsoft) — run `make` targets easily.
- **Python** (Microsoft) — for the encoder scripts.

You can install these from the Extensions panel in VS Code.

## Step 1: Initialize Submodules

The repository uses git submodules for `stmlib` and the audio bootloader.

From the repo root:

```bash
git submodule update --init --recursive
```

This ensures that:

- `stmlib/makefile.inc` exists
- the bootloader encoder scripts are present

## Step 2: Add Preset Keyframes

We added new preset files:

- `frames/preset_keyframes.h`
- `frames/preset_keyframes.cc`

You can edit your preset in:

```
frames/preset_keyframes.cc
```

Example entry:

```cpp
{ 1024, 0, { 10000, 0, 0, 0 } },
```

Meaning:
- `1024` = timeline timestamp (0–65535)
- `0` = keyframe ID (we overwrite this internally)
- `{ 10000, 0, 0, 0 }` = 4 channel values

Set:

```cpp
const uint16_t kPresetNumKeyframes = 64;
```

To enable your full preset table.

## Step 3: Firmware Seeding On First Boot

We modified `frames/keyframer.cc` so that:

- If no saved data exists in flash, the preset table is loaded.
- If the preset table is non‑empty, it is immediately saved into flash.

This makes your preset the new “factory” state.

## Step 4: Build The Firmware And WAV (Recommended)

The easiest path is the `wav` target, which builds the `.bin` **and** produces
a **time-stamped** `.wav` file.

```bash
make -f frames/makefile wav TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

This prints the WAV path, for example:

```
WAV generated: build/frames/frames_20260215_143012.wav
```

If you want to control the name, you can pass:

```bash
make -f frames/makefile wav \
  TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/ \
  WAV_FILE=build/frames/frames_custom.wav
```

## Step 5: Build Only (Optional)

If you only want to build the firmware binary without making a WAV:

```bash
make -f frames/makefile bin TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

## Step 6: Play The WAV Into Frames

Use the Frames update procedure:

1. Put the module into bootloader mode.
2. Play the `.wav` file at full volume from a clean source.
3. Wait for the update LED sequence to confirm success.

(Refer to the Frames manual for exact bootloader instructions.)

## Common Problems And Fixes

### “stmlib/makefile.inc: No such file or directory”
Submodules not initialized.

```bash
git submodule update --init --recursive
```

### “arm-none-eabi-g++ not found”
Toolchain missing.

```bash
brew install gcc-arm-embedded
```

### “python: No such file or directory”
Use Python 3 explicitly (the `wav` target already does this):

### “No module named numpy”
Install NumPy:

```bash
python3 -m pip install numpy
```

## Files We Modified

- `frames/preset_keyframes.h`
- `frames/preset_keyframes.cc`
- `frames/keyframer.cc`
- `stmlib/linker_scripts/stm32f10x_flash_md_application.ld`
- `stm_audio_bootloader/qpsk/encoder.py`
- `stm_audio_bootloader/audio_stream_writer.py`

## Optional Quality Of Life

You can add this to your shell to make the encoder command shorter:

```bash
export PYTHONPATH=.
```

Then run:

```bash
python3 stm_audio_bootloader/qpsk/encoder.py -s 48000 -b 12000 -c 6000 -p 256 build/frames/frames.bin
```

## Next Steps

If you want:
- A “hold a button on boot to restore preset” feature
- Automatic preset validation (sorted timestamps)
- A helper script to convert CSV presets into C++ tables

Just say the word and we can add it.
