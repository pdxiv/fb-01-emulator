#pragma once

#include <cstdint>

namespace FB01 {

class FB01Emulator;

// Cycle-accurate scheduler: advances Z80, OPM, USART per audio sample.
// Z80 runs at 6 MHz, OPM at 4 MHz (outputs at 4MHz/64 = 62.5 kHz).
class Scheduler {
public:
    explicit Scheduler(FB01Emulator& emu);

    void reset();

    // Advance all components to produce one audio sample at the given host rate.
    void advanceOneSample(float& outL, float& outR, double hostSampleRate);

private:
    FB01Emulator& m_emu;

    // Fractional cycle accumulators
    double m_z80CycleAccum;   // Z80 cycles to run per sample
    double m_opmCycleAccum;   // OPM clocks per sample
    double m_opmSubCycleAccum = 0.0; // fractional OPM clocks within Z80 stepping

    // OPM accumulator for resampling (sum of OPM outputs per host sample)
    double m_opmOutL;
    double m_opmOutR;
    int m_opmSampleCount;

    static constexpr double kZ80Clock = 6000000.0;   // 6 MHz
    static constexpr double kOPMClock = 4000000.0;   // 4 MHz
    static constexpr double kOPMPerZ80 = kOPMClock / kZ80Clock; // ~0.6667
    static constexpr double kOPMSamplesPerClock = 1.0 / 64.0; // OPM outputs every 64 clocks
};

} // namespace FB01
