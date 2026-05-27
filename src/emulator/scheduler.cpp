#include "scheduler.h"
#include "fb01.h"

namespace FB01 {

Scheduler::Scheduler(FB01Emulator& emu)
    : m_emu(emu)
    , m_z80CycleAccum(0.0)
    , m_opmCycleAccum(0.0)
    , m_opmOutL(0.0)
    , m_opmOutR(0.0)
    , m_opmSampleCount(0)
{
}

void Scheduler::reset() {
    m_z80CycleAccum = 0.0;
    m_opmCycleAccum = 0.0;
    m_opmSubCycleAccum = 0.0;
    m_opmOutL = 0.0;
    m_opmOutR = 0.0;
    m_opmSampleCount = 0;
}

void Scheduler::advanceOneSample(float& outL, float& outR, double hostSampleRate) {
    // Calculate how many Z80 and OPM cycles to run per host sample
    double z80CyclesPerSample = kZ80Clock / hostSampleRate;
    double opmClocksPerSample = kOPMClock / hostSampleRate;

    m_z80CycleAccum += z80CyclesPerSample;
    m_opmCycleAccum += opmClocksPerSample;

    int z80CyclesToRun = static_cast<int>(m_z80CycleAccum);
    int opmClocksToRun = static_cast<int>(m_opmCycleAccum);
    m_z80CycleAccum -= z80CyclesToRun;
    m_opmCycleAccum -= opmClocksToRun;

    // Interleave Z80 and OPM execution proportionally.
    // Ratio: for every 3 Z80 cycles, run 2 OPM clocks (6MHz:4MHz = 3:2)
    unsigned long z80Target = m_emu.m_z80->cyc + z80CyclesToRun;
    int opmDone = 0;

    while (m_emu.m_z80->cyc < z80Target) {
        // Step Z80 one instruction
        unsigned long before = m_emu.m_z80->cyc;
        z80_step(m_emu.m_z80);
        unsigned long elapsed = m_emu.m_z80->cyc - before;

        // Run proportional OPM clocks (4/6 = 2/3 OPM clocks per Z80 cycle)
        m_opmSubCycleAccum += elapsed * kOPMPerZ80;
        int opmNow = static_cast<int>(m_opmSubCycleAccum);
        m_opmSubCycleAccum -= opmNow;

        for (int i = 0; i < opmNow && opmDone < opmClocksToRun; i++, opmDone++) {
            int32_t output[2];
            uint8_t sh1, sh2, so;
            OPM_Clock(m_emu.m_opm, output, &sh1, &sh2, &so);

            if (sh1) {
                m_opmOutL += static_cast<double>(output[0]);
                m_opmOutR += static_cast<double>(output[1]);
                m_opmSampleCount++;
            }
        }
    }

    // Run any remaining OPM clocks for this sample
    while (opmDone < opmClocksToRun) {
        int32_t output[2];
        uint8_t sh1, sh2, so;
        OPM_Clock(m_emu.m_opm, output, &sh1, &sh2, &so);
        if (sh1) {
            m_opmOutL += static_cast<double>(output[0]);
            m_opmOutR += static_cast<double>(output[1]);
            m_opmSampleCount++;
        }
        opmDone++;
    }

    // Average the OPM outputs accumulated during this host sample period
    if (m_opmSampleCount > 0) {
        constexpr double kNormalize = 1.0 / 16384.0;
        outL = static_cast<float>(m_opmOutL / m_opmSampleCount * kNormalize);
        outR = static_cast<float>(m_opmOutR / m_opmSampleCount * kNormalize);
    } else {
        outL = 0.0f;
        outR = 0.0f;
    }

    m_opmOutL = 0.0;
    m_opmOutR = 0.0;
    m_opmSampleCount = 0;

    // Update interrupts after processing
    m_emu.updateInterrupts();
}

} // namespace FB01
