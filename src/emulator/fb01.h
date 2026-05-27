#pragma once

#include <cstdint>
#include <vector>
#include <functional>

extern "C" {
#include "z80.h"
#include "opm.h"
}

namespace FB01 {

class I8251;
class HD44780;
class Scheduler;

// Top-level FB-01 emulator: ties together Z80, OPM, USART, LCD, panel switches.
class FB01Emulator {
public:
    FB01Emulator();
    ~FB01Emulator();

    // Load the 32KB ROM. Returns false if data is not exactly 32768 bytes.
    bool loadROM(const uint8_t* data, size_t size);

    // Reset all hardware to power-on state.
    void reset();

    // Feed a MIDI byte into the USART receive buffer.
    void sendMidiByte(uint8_t byte);

    // Get a MIDI byte from the USART transmit buffer. Returns false if empty.
    bool receiveMidiByte(uint8_t& byte);

    // Set front-panel button state (active-low, directly read from port 0x20).
    void setButtonState(uint8_t buttons);

    // Get LCD display state (16 characters).
    const char* getLCDText() const;

    // Get CGRAM data for custom characters.
    const uint8_t* getCGRAM() const;

    // Advance emulation by one audio sample at the given sample rate.
    // Writes stereo output to outL, outR.
    void generateSample(float& outL, float& outR, double sampleRate);

    // Access the scheduler for timing info
    Scheduler& getScheduler() { return *m_scheduler; }

private:
    friend class Scheduler;
    // Z80 memory callbacks
    static uint8_t z80ReadByte(void* userdata, uint16_t addr);
    static void z80WriteByte(void* userdata, uint16_t addr, uint8_t val);
    static uint8_t z80PortIn(struct z80* z, uint8_t port);
    static void z80PortOut(struct z80* z, uint8_t port, uint8_t val);

    void updateInterrupts();

    // Memory
    std::vector<uint8_t> m_rom;     // 32 KB
    std::vector<uint8_t> m_ram;     // 16 KB (mirrored at 0xC000-0xFFFF)

    // CPU
    z80* m_z80;

    // Peripherals
    opm_t* m_opm;
    I8251* m_usart;
    HD44780* m_lcd;
    Scheduler* m_scheduler;

    // Front panel buttons (active-low byte)
    uint8_t m_buttons;

    bool m_rom_loaded;
};

} // namespace FB01
