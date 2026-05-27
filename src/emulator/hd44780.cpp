#include "hd44780.h"

namespace FB01 {

HD44780::HD44780() {
    reset();
}

void HD44780::reset() {
    std::memset(m_ddram, 0x20, sizeof(m_ddram)); // Fill with spaces
    std::memset(m_cgram, 0x00, sizeof(m_cgram));

    m_addressCounter = 0;
    m_cgramMode = false;
    m_displayOn = false;
    m_cursorOn = false;
    m_blinkOn = false;
    m_increment = true;
    m_shiftDisplay = false;
    m_twoLine = true;  // FB-01 uses 2-line mode
    m_bigFont = false;
    m_displayShift = 0;

    updateDisplayText();
}

uint8_t HD44780::read(uint8_t rs) {
    if (rs == 0) {
        // Read busy flag + address counter
        // We don't emulate LCD clock timing, so always report ready (BF=0)
        uint8_t val = m_addressCounter & 0x7F;
        return val;
    } else {
        // Read data from current RAM
        uint8_t val;
        if (m_cgramMode) {
            val = m_cgram[m_addressCounter & 0x3F];
        } else {
            val = m_ddram[m_addressCounter & 0x7F];
        }
        // Advance address counter
        if (m_increment)
            m_addressCounter++;
        else
            m_addressCounter--;
        m_addressCounter &= 0x7F;
        return val;
    }
}

void HD44780::write(uint8_t rs, uint8_t val) {
    if (rs == 0) {
        // Instruction register
        executeInstruction(val);
    } else {
        // Data register: write to CGRAM or DDRAM
        if (m_cgramMode) {
            m_cgram[m_addressCounter & 0x3F] = val;
        } else {
            m_ddram[m_addressCounter & 0x7F] = val;
        }
        // Advance address counter
        if (m_increment)
            m_addressCounter++;
        else
            m_addressCounter--;

        if (m_cgramMode) {
            m_addressCounter &= 0x3F;
        } else {
            m_addressCounter &= 0x7F;
        }

        updateDisplayText();
    }
}

void HD44780::executeInstruction(uint8_t val) {
    if (val & 0x80) {
        // Set DDRAM address
        m_addressCounter = val & 0x7F;
        m_cgramMode = false;
    } else if (val & 0x40) {
        // Set CGRAM address
        m_addressCounter = val & 0x3F;
        m_cgramMode = true;
    } else if (val & 0x20) {
        // Function set
        m_twoLine = (val & 0x08) != 0;
        m_bigFont = (val & 0x04) != 0;
    } else if (val & 0x10) {
        // Cursor/display shift
        if (val & 0x08) {
            // Display shift
            if (val & 0x04)
                m_displayShift++;
            else
                m_displayShift--;
        }
    } else if (val & 0x08) {
        // Display on/off control
        m_displayOn = (val & 0x04) != 0;
        m_cursorOn  = (val & 0x02) != 0;
        m_blinkOn   = (val & 0x01) != 0;
        updateDisplayText();
    } else if (val & 0x04) {
        // Entry mode set
        m_increment    = (val & 0x02) != 0;
        m_shiftDisplay = (val & 0x01) != 0;
    } else if (val & 0x02) {
        // Return home
        m_addressCounter = 0;
        m_displayShift = 0;
        m_cgramMode = false;
        updateDisplayText();
    } else if (val & 0x01) {
        // Clear display
        std::memset(m_ddram, 0x20, sizeof(m_ddram));
        m_addressCounter = 0;
        m_displayShift = 0;
        m_cgramMode = false;
        updateDisplayText();
    }
}

void HD44780::updateDisplayText() {
    // FB-01 uses 2-line mode with 8 chars per line, displayed as 1x16.
    // Line 1: DDRAM 0x00-0x07, Line 2: DDRAM 0x40-0x47
    for (int i = 0; i < 8; i++) {
        uint8_t ch = m_ddram[(0x00 + i + m_displayShift) & 0x7F];
        m_displayText[i] = (ch >= 0x20 && ch < 0x7F) ? static_cast<char>(ch) : ' ';
    }
    for (int i = 0; i < 8; i++) {
        uint8_t ch = m_ddram[(0x40 + i + m_displayShift) & 0x7F];
        m_displayText[8 + i] = (ch >= 0x20 && ch < 0x7F) ? static_cast<char>(ch) : ' ';
    }
    m_displayText[16] = '\0';
}

} // namespace FB01
