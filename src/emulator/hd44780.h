#pragma once

#include <cstdint>
#include <cstring>

namespace FB01 {

// Emulation of Hitachi HD44780A LCD controller.
// FB-01 uses 2x8 display as a single 1x16 line.
class HD44780 {
public:
    HD44780();

    void reset();

    // CPU interface (RS selects register: 0=instruction, 1=data)
    uint8_t read(uint8_t rs);
    void write(uint8_t rs, uint8_t val);

    // Get display content as null-terminated 16-char string
    const char* getDisplayText() const { return m_displayText; }

    // Get CGRAM (64 bytes for custom characters)
    const uint8_t* getCGRAM() const { return m_cgram; }

    // Get full DDRAM for rendering
    const uint8_t* getDDRAM() const { return m_ddram; }

    // Display enabled?
    bool isDisplayOn() const { return m_displayOn; }

private:
    void executeInstruction(uint8_t val);
    void updateDisplayText();

    // DDRAM: 128 bytes
    uint8_t m_ddram[128];
    // CGRAM: 64 bytes
    uint8_t m_cgram[64];

    // Address counter
    uint8_t m_addressCounter;
    bool m_cgramMode; // true = CGRAM selected, false = DDRAM

    // Display settings
    bool m_displayOn;
    bool m_cursorOn;
    bool m_blinkOn;
    bool m_increment;     // true = increment, false = decrement
    bool m_shiftDisplay;  // true = shift display on write
    bool m_twoLine;       // true = 2-line mode
    bool m_bigFont;       // true = 5x10, false = 5x8
    uint8_t m_displayShift; // current shift offset

    // Pre-rendered display text (16 chars + null)
    char m_displayText[17];
};

} // namespace FB01
