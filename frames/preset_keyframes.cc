// Copyright 2026
//
// Default keyframe preset data for first boot.
//
// Replace this table with up to 64 keyframes. Timestamps must be sorted
// ascending. Values are 0-65535 per channel.

#include "frames/preset_keyframes.h"

namespace frames {

const Keyframe kPresetKeyframes[] = {
  // Example (replace or remove):
  // { 0, 0, { 0, 0, 0, 0 } },
  //{ 1024,  0, { 10000, 0, 0, 0 } },
  // ...
  {1040,1,{45287,48102,46181,48229}},
  {26006,2,{46440,45210,45220,48449}},
  {38489,3,{47192,45416,46168,46080}},
  {53052,4,{42662,42967,44263,43620}},
  {63455,5,{45902,44794,43700,43083}}
};

const uint16_t kPresetNumKeyframes =
    sizeof(kPresetKeyframes) / sizeof(kPresetKeyframes[0]);

}  // namespace frames
