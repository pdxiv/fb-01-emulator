#include "midi.h"
#include "../emulator/fb01.h"
#include "RtMidi.h"
#include <cstdio>

namespace FB01 {

StandaloneMidi::~StandaloneMidi() {
    close();
}

bool StandaloneMidi::open(FB01Emulator* emulator) {
    m_emulator = emulator;

    try {
        m_midiIn = std::make_unique<RtMidiIn>();
    } catch (RtMidiError& e) {
        fprintf(stderr, "RtMidi error: %s\n", e.getMessage().c_str());
        return false;
    }

    unsigned int portCount = m_midiIn->getPortCount();
    if (portCount == 0) {
        fprintf(stderr, "No MIDI input ports available.\n");
        // Open a virtual port instead (Linux/macOS)
        try {
            m_midiIn->openVirtualPort("FB-01 MIDI In");
            m_portName = "FB-01 MIDI In (virtual)";
        } catch (RtMidiError& e) {
            fprintf(stderr, "Could not open virtual MIDI port: %s\n", e.getMessage().c_str());
            return false;
        }
    } else {
        // Prefer a hardware port over "Midi Through"
        unsigned int selectedPort = 0;
        for (unsigned int i = 0; i < portCount; i++) {
            std::string name = m_midiIn->getPortName(i);
            fprintf(stderr, "  MIDI port %u: %s\n", i, name.c_str());
            if (name.find("Midi Through") == std::string::npos && selectedPort == 0) {
                selectedPort = i;
            }
        }
        try {
            m_midiIn->openPort(selectedPort);
            m_portName = m_midiIn->getPortName(selectedPort);
            fprintf(stderr, "Opened MIDI port: %s\n", m_portName.c_str());
        } catch (RtMidiError& e) {
            fprintf(stderr, "Could not open MIDI port: %s\n", e.getMessage().c_str());
            return false;
        }
    }

    m_midiIn->setCallback(midiCallback, this);
    m_midiIn->ignoreTypes(false, false, false); // Accept SysEx, timing, active sensing

    return true;
}

void StandaloneMidi::close() {
    if (m_midiIn) {
        m_midiIn->closePort();
        m_midiIn.reset();
    }
}

void StandaloneMidi::midiCallback(double /*timeStamp*/,
                                  std::vector<unsigned char>* message,
                                  void* userData) {
    auto* self = static_cast<StandaloneMidi*>(userData);
    if (!message || message->empty())
        return;

    // Queue bytes into the ring buffer (RtMidi thread → audio thread)
    size_t wp = self->m_writePos.load(std::memory_order_relaxed);
    for (unsigned char byte : *message) {
        size_t next = (wp + 1) % kRingSize;
        if (next == self->m_readPos.load(std::memory_order_acquire))
            break; // Ring full, drop byte
        self->m_ring[wp] = byte;
        wp = next;
    }
    self->m_writePos.store(wp, std::memory_order_release);
}

void StandaloneMidi::processSample(double sampleRate) {
    if (!m_emulator)
        return;

    // Accumulate time and deliver bytes at 31250 baud rate
    m_byteAccum += kMidiBytesPerSec / sampleRate;
    while (m_byteAccum >= 1.0) {
        m_byteAccum -= 1.0;
        size_t rp = m_readPos.load(std::memory_order_relaxed);
        if (rp == m_writePos.load(std::memory_order_acquire))
            break; // Ring empty
        uint8_t byte = m_ring[rp];
        m_readPos.store((rp + 1) % kRingSize, std::memory_order_release);
        m_emulator->sendMidiByte(byte);
    }
}

} // namespace FB01
