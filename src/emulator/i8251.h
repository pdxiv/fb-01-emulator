#pragma once

#include <cstdint>
#include <queue>

namespace FB01 {

// Emulation of Intel 8251 / NEC µPD71051C USART (MIDI serial interface).
// Simplified for MIDI: async, 8-bit, no parity, 1 stop bit, x16 baud rate.
class I8251 {
public:
    I8251();

    void reset();

    // CPU-side interface
    uint8_t readData();
    uint8_t readStatus();
    void writeData(uint8_t val);
    void writeControl(uint8_t val);

    // External interface (MIDI)
    void receiveData(uint8_t byte);        // Feed MIDI IN byte
    bool transmitData(uint8_t& byte);      // Get MIDI OUT byte

    // Interrupt signals
    bool getTxReady() const { return m_txReady && m_txEnabled; }
    bool getRxReady() const { return m_rxReady && m_rxEnabled; }

private:
    enum State {
        STATE_MODE,     // Expecting mode byte
        STATE_COMMAND   // Expecting command bytes
    };

    State m_state;

    // Mode settings
    uint8_t m_mode;
    uint8_t m_charLength;
    uint8_t m_baudFactor;

    // Command register
    bool m_txEnabled;
    bool m_rxEnabled;
    bool m_dtr;
    bool m_rts;

    // Status flags
    bool m_txReady;
    bool m_txEmpty;
    bool m_rxReady;
    bool m_parityError;
    bool m_overrunError;
    bool m_framingError;

    // Data buffers
    std::queue<uint8_t> m_rxFifo;    // Incoming MIDI data
    std::queue<uint8_t> m_txFifo;    // Outgoing MIDI data
    uint8_t m_rxData;                // Last read data byte
};

} // namespace FB01
