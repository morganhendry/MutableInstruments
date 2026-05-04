# Frames Firmware Overview

This document is a high-level guide to the Frames firmware. It focuses on how the module boots, how keyframes are stored and interpolated, how the UI works, and where to start if you want to modify behavior.

## Boot Flow And Main Loop

### Boot Flow (Startup)
1. **Hardware initialization** happens in `frames/frames.cc` in `Init()`.
   - Initializes system timers, DAC, trigger output, keyframer, poly LFO, and UI.
2. **Splash screen** runs until the UI exits splash mode.
3. **Calibration check** is performed via `ui.TryCalibration()`.
4. **Main loop** begins and runs forever.

### Main Loop (Runtime)
The main loop in `frames/frames.cc` does three things repeatedly:
- **Process UI events** (`ui.DoEvents()`), which reacts to user input.
- **Render outputs** on each DAC refresh.
- **Switch between two core modes**:
  - **Poly LFO mode**: the poly LFO drives outputs.
  - **Keyframer mode**: the keyframer interpolates and outputs values.

Key input signals:
- **FRAME** knob sets position through keyframes.
- **FRAME MOD** input offsets the FRAME position.
- **Trigger output** fires on keyframe changes.

## Keyframer Data Model And Interpolation

### Data Model
Defined in `frames/keyframer.h`:
- `Keyframe` is a timestamp plus four channel values.
- Keyframes are sorted by `timestamp`.
- `kMaxNumKeyframe` is 64.
- Per-channel settings include:
  - `easing_curve`
  - `response`

### Interpolation
Implemented in `frames/keyframer.cc`:
- `Evaluate(timestamp)` computes the current output levels for all channels.
- If the timestamp is before the first keyframe or after the last, it holds the nearest keyframe.
- If the timestamp lies between two keyframes, it interpolates using the selected **easing curve**.

### Output Conversion
- `ConvertToDacCode()` shapes linear values into a response curve for the 2164 VCA.
- This uses lookup tables in `frames/resources.cc` and `frames/resources.h`.

## UI Event Handling And Modes

### Event Loop
`frames/ui.cc` handles:
- Switch presses and releases.
- Pot movements.
- Mode switching (normal, edit, save, erase, etc.).

### Key Modes
- **UI_MODE_NORMAL**: standard operation (keyframer or poly LFO).
- **UI_MODE_EDIT_EASING**: edit easing curve per channel.
- **UI_MODE_EDIT_RESPONSE**: edit VCA response curve per channel.
- **UI_MODE_SAVE_CONFIRMATION**: feedback for saving.
- **UI_MODE_ERASE_CONFIRMATION**: feedback for erasing.
- **UI_MODE_FACTORY_TESTING**: hardware test mode.

### Important Actions
- **Short press ADD**: create or edit keyframes.
- **Long press ADD**: enter easing edit.
- **Very long press ADD**: save to storage.
- **Short press DELETE**: remove nearest keyframe.
- **Long press DELETE**: enter response edit.
- **Very long press DELETE**: clear all keyframes.

## Drivers (ADC, DAC, LEDs, Switches, Trigger)

Drivers live in `frames/drivers/`:
- **ADC** (`adc.cc`): reads the pots and CV inputs.
- **DAC** (`dac.cc`): writes output voltages.
- **LEDs**:
  - Channel LEDs (`channel_leds.cc`)
  - RGB LED (`rgb_led.cc`)
  - Keyframe indicator LED (`keyframe_led.cc`)
- **Switches** (`switches.cc`): debounced button input.
- **Trigger output** (`trigger_output.cc`): fires when moving between keyframes.

These drivers are initialized in `Init()` in `frames/frames.cc` and updated in the main loop or timer interrupt.

## Storage (Saving And Loading Keyframes)

Storage is handled by `frames/keyframer.cc` using `stmlib::Storage`.

- **Load**: `Keyframer::Init()` reads stored keyframes and settings from flash.
- **Save**: `Keyframer::Save()` writes the current keyframes/settings to flash.
- **Erase**: `Keyframer::Clear()` deletes the in-memory set.

If storage is empty or invalid, `Keyframer::Init()` defaults to an empty set.

## Where To Start If You Want To Modify Behavior

Here are the most common starting points:

1. **Keyframe behavior or interpolation**
   - `frames/keyframer.cc`
   - Look at `Evaluate()`, `AddKeyframe()`, and `ConvertToDacCode()`.

2. **UI interactions**
   - `frames/ui.cc`
   - Look at `OnSwitchReleased()`, `OnPotChanged()`, and `DoEvents()`.

3. **Main runtime behavior**
   - `frames/frames.cc`
   - Look at the main loop and `TIM1_UP_IRQHandler()`.

4. **Storage and presets**
   - `frames/keyframer.cc`
   - Look at `Init()` and `Save()`.

If you want, I can expand any of these sections with deeper callouts or diagrams.
