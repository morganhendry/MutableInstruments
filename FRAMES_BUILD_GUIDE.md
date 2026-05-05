# Frames Firmware Beginner Guide

This document explains, step by step, how to set up the environment, modify the Frames firmware, build it, and generate the audio `.wav` file for updating the module. It reflects the exact workflow we used in this repo.

## What This Custom Firmware Does

1. Adds a **compiled-in 32-slot preset bank** in `frames/preset_keyframes.cc`.
2. Makes slot `0` a blank 0V preset and uses it on **first boot only**, when Frames storage is empty or invalid.
3. Adds a runtime preset-loader gesture:
   Hold `ADD`, then hold `DELETE` for about 3 seconds to enter patch selection.
4. Uses the 4 channel LEDs plus the keyframe LED as a **5-bit binary display** for slot `0-31`.
5. Keeps track of the last loaded slot in flash, so startup restores the last saved state from that slot.
6. Keeps the stock bootloader and calibration startup gestures unchanged:
   hold `ADD` at power-on for bootloader, hold `DELETE` at power-on for recalibration.

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
The audio encoder requires NumPy. On recent macOS/Homebrew setups, the safest
path is a local virtual environment in the repo:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install numpy
```

If you use the virtual environment, activate it before generating WAV files:

```bash
source .venv/bin/activate
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

## Step 2: Add Preset Bank Slots

We added new preset bank files:

- `frames/preset_keyframes.h`
- `frames/preset_keyframes.cc`

You can edit the compiled slot bank in:

```
frames/preset_keyframes.cc
```

Example keyframe entry:

```cpp
{ 1024, 0, { 10000, 0, 0, 0 } },
```

Meaning:
- `1024` = timeline timestamp (0–65535)
- `0` = keyframe ID (we overwrite this internally)
- `{ 10000, 0, 0, 0 }` = 4 channel values

Important notes:
- There are `32` selectable slots, numbered `0` to `31`.
- Slot `0` is intentionally a blank preset with 2 zero-valued keyframes, one at
  timestamp `0` and one at `65535`, so output stays at `0V` across the full
  FRAME sweep.
- Slot `1` currently contains the example musical preset that used to be the
  only compiled preset.
- Slots `2` through `31` are each declared as separate blank preset arrays, so
  the file is ready to accept pasted keyframe sequences slot by slot.
- Each populated slot supports up to `64` keyframes.
- The loader sorts keyframes by timestamp before using them, so the source array
  does not have to be perfectly ordered, though keeping it sorted is easier to
  read.
- Values are raw Frames channel levels, `0` to `65535`.

Each slot is described in the bank like this:

```cpp
const PresetDefinition kPresetBank[kNumPresetSlots] = {
  { kPresetSlot0Keyframes, sizeof(kPresetSlot0Keyframes) / sizeof(kPresetSlot0Keyframes[0]) },
  { kPresetSlot1Keyframes, sizeof(kPresetSlot1Keyframes) / sizeof(kPresetSlot1Keyframes[0]) },
  { kPresetSlot2Keyframes, sizeof(kPresetSlot2Keyframes) / sizeof(kPresetSlot2Keyframes[0]) },
  // ...
};
```

## Step 3: Firmware Seeding On First Boot

We modified `frames/keyframer.cc` so that:

- If no saved data exists in flash, slot `0` is loaded.
- That blank slot is immediately saved into flash.

After that, normal startup restores whatever saved state was last written to
flash, including the last loaded slot number.

### Runtime Preset Loader Gesture

The custom loader gesture lives in `frames/ui.cc`:

- Hold `ADD`
- While still holding `ADD`, hold `DELETE`
- Keep holding `DELETE` for about `3` seconds
- Release the buttons
- Turn the `FRAME` knob to choose slot `0-31`
- Press `ADD` to load the selected slot
- Press `DELETE` to cancel with no patch change

While selecting, the 4 channel LEDs plus the keyframe LED show the slot number
as a 5-bit binary value. With the current default bank, every slot is
populated, so the RGB LED should stay in the valid-slot state.

Confirming a populated slot calls `Keyframer::LoadPreset(slot, true)`, which
loads that compiled slot and saves it to flash immediately.

### Important Behavior Note

A very long `DELETE` press **without** `ADD` still clears the current in-memory
keyframes, but it does **not** save that state to flash by itself. After a
power cycle, the last saved state comes back unless you explicitly save again.

## Step 4: Build The Firmware And WAV (Recommended)

The easiest path is the `wav` target, which builds the `.bin` **and** produces
a **time-stamped** `.wav` file.

```bash
make -f frames/makefile wav \
  TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
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
make -f frames/makefile bin \
  TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

This generates:

- `build/frames/frames.elf`
- `build/frames/frames.hex`
- `build/frames/frames.bin`

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
Use the `wav` target, which already calls `python3`.

### “build/frames/frames.bin: No such file or directory”
The encoder needs the `.bin` file. Run either:

```bash
make -f frames/makefile bin TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

or just use:

```bash
make -f frames/makefile wav TOOLCHAIN_PATH=/Applications/ArmGNUToolchain/15.2.rel1/arm-none-eabi/
```

### “No module named numpy”
Activate the virtual environment and install NumPy there:

```bash
source .venv/bin/activate
python3 -m pip install numpy
```

## Files We Modified

- `frames/preset_keyframes.h`
- `frames/preset_keyframes.cc`
- `frames/keyframer.cc`
- `frames/ui.cc`
- `frames/makefile`
- `stmlib/linker_scripts/stm32f10x_flash_md_application.ld`
- `stm_audio_bootloader/qpsk/encoder.py`
- `stm_audio_bootloader/audio_stream_writer.py`

## Where The Preset Logic Lives

- `frames/preset_keyframes.cc`
  The compiled 32-slot preset bank itself.
- `frames/keyframer.cc`
  `LoadCompiledPreset()` copies one compiled slot into the working buffer,
  sorts by timestamp, resets IDs, and optionally saves to flash through
  `LoadPreset(slot, true)`.
- `frames/ui.cc`
  The `ADD` + very long `DELETE` gesture enters slot selection, and `ADD`
  confirms `LoadPreset(slot, true)`.
- `frames/makefile`
  The `wav` target now builds `frames.bin` first and emits a time-stamped WAV.

## Next Steps

If you want:
- A CSV-to-keyframe conversion helper
- A stricter preset validator

Just say the word and we can add it.
