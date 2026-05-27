#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace FB01 {

// Processor GUID
static const Steinberg::FUID kProcessorUID(0x12345678, 0xABCD0001, 0xFB010000, 0x00000001);
// Controller GUID
static const Steinberg::FUID kControllerUID(0x12345678, 0xABCD0001, 0xFB010000, 0x00000002);

static const char* kPluginName = "FB-01";
static const char* kVendorName = "FB01Emu";
static const char* kSubCategories = "Instrument|Synth";

} // namespace FB01
