#pragma once

// Resampling from OPM native rate (62.5 kHz) to host sample rate
// is handled directly in the scheduler by accumulating OPM clock outputs
// and averaging per host sample. The Nuked-OPM library produces sample-accurate
// output at its native rate, and we use simple averaging (box filter) to
// downsample/upsample to the host rate. This is sufficient because:
// 1. The OPM output is already band-limited by the chip's DAC emulation
// 2. For typical host rates (44.1/48/96 kHz), we're downsampling from 62.5 kHz
//    which requires minimal filtering.

namespace FB01 {

// No separate resampler class needed — logic is inline in Scheduler.
// This header exists as a documentation placeholder per the project structure spec.

} // namespace FB01
