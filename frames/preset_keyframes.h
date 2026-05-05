// Copyright 2026
//
// Default keyframe preset data bank.
//
// Replace the slot arrays and kPresetBank entries with your own data.

#ifndef FRAMES_PRESET_KEYFRAMES_H_
#define FRAMES_PRESET_KEYFRAMES_H_

#include "frames/keyframer.h"

namespace frames {

struct PresetDefinition {
  const Keyframe* keyframes;
  uint16_t num_keyframes;
};

extern const PresetDefinition kPresetBank[kNumPresetSlots];

}  // namespace frames

#endif  // FRAMES_PRESET_KEYFRAMES_H_
