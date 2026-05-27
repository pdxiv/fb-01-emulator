#include "i8251.h"

namespace FB01 {

I8251::I8251() {
    reset();
}

void I8251::reset() {
    m_state = STATE_MODE;
    m_mode = 0;
    m_charLength = 8;
    m_baudFactor = 16;

    m_txEnabled = false;
    m_rxEnabled = false;
    m_dtr = false;
    m_rts = false;

    m_txReady = true;
    m_txEmpty = true;
    m_rxReady = false;
    m_parityError = false;
    m_overrunError = false;
    m_framingError = false;

    m_rxData = 0;

    // Clear FIFOs
    while (!m_rxFifo.empty()) m_rxFifo.pop();
    while (!m_txFifo.empty()) m_txFifo.pop();
}

uint8_t I8251::readData() {
    uint8_t val = m_rxData;
    if (!m_rxFifo.empty()) {
        m_rxData = m_rxFifo.front();
        m_rxFifo.pop();
        if (m_rxFifo.empty()) {
            m_rxReady = false;
        }
    } else {
        m_rxReady = false;
    }
    return val;
}

uint8_t I8251::readStatus() {
    uint8_t status = 0;
    if (m_txReady && m_txEnabled)   status |= 0x01; // TXRDY
    if (m_rxReady && m_rxEnabled)   status |= 0x02; // RXRDY
    if (m_txEmpty)                  status |= 0x04; // TXEMPTY
    if (m_parityError)              status |= 0x08; // PE
    if (m_overrunError)             status |= 0x10; // OE
    if (m_framingError)             status |= 0x20; // FE
    // bit 6 = SYNDET (not used in async mode)
    // bit 7 = DSR (always ready for MIDI)
    status |= 0x80; // DSR active
    return status;
}

void I8251::writeData(uint8_t val) {
    m_txFifo.push(val);
    m_txReady = true;  // In emulation, we immediately "transmit"
    m_txEmpty = false;
    // In a real chip, TX_READY would deassert until the byte is shifted out.
    // For simplicity at MIDI speeds in emulation, we keep it ready.
    m_txEmpty = true;
    m_txReady = true;
}

void I8251::writeControl(uint8_t val) {
    if (m_state == STATE_MODE) {
        // Mode byte
        m_mode = val;
        uint8_t baudBits = val & 0x03;
        switch (baudBits) {
            case 0x01: m_baudFactor = 1; break;
            case 0x02: m_baudFactor = 16; break;
            case 0x03: m_baudFactor = 64; break;
            default:   m_baudFactor = 1; break; // Sync mode, not used
        }
        uint8_t charBits = (val >> 2) & 0x03;
        m_charLength = 5 + charBits;
        m_state = STATE_COMMAND;
    } else {
        // Command byte
        m_txEnabled = (val & 0x01) != 0;
        m_dtr       = (val & 0x02) != 0;
        m_rxEnabled = (val & 0x04) != 0;
        m_rts       = (val & 0x20) != 0;

        // Error reset
        if (val & 0x10) {
            m_parityError = false;
            m_overrunError = false;
            m_framingError = false;
        }

        // Internal reset
        if (val & 0x40) {
            m_state = STATE_MODE;
            m_txEnabled = false;
            m_rxEnabled = false;
            m_txReady = true;
            m_txEmpty = true;
            m_rxReady = false;
        }
    }
}

void I8251::receiveData(uint8_t byte) {
    if (m_rxReady && !m_rxFifo.empty()) {
        // Overrun: new data before previous was read
        m_overrunError = true;
    }
    m_rxFifo.push(byte);
    if (!m_rxReady) {
        m_rxData = m_rxFifo.front();
        m_rxFifo.pop();
    }
    m_rxReady = true;
}

bool I8251::transmitData(uint8_t& byte) {
    if (m_txFifo.empty())
        return false;
    byte = m_txFifo.front();
    m_txFifo.pop();
    return true;
}

} // namespace FB01
