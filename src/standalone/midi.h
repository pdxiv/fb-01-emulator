#pragma once

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include "RtMidi.h"

namespace FB01 {

class FB01Emulator;

class StandaloneMidi {
public:
    StandaloneMidi() = default;
    ~StandaloneMidi();

    bool open(FB01Emulator* emulator);
    void close();

    // Called from the audio thread: feed buffered MIDI bytes to the emulator
    // at the correct baud rate. Call once per audio sample.
    void processSample(double sampleRate);

    const char* getPortName() const { return m_portName.c_str(); }

private:
    static void midiCallback(double timeStamp, std::vector<unsigned char>* message,
                             void* userData);

    FB01Emulator* m_emulator = nullptr;
    std::unique_ptr<RtMidiIn> m_midiIn;
    std::string m_portName;

    // Lock-free ring buffer for cross-thread MIDI byte delivery
    static constexpr size_t kRingSize = 1024;
    uint8_t m_ring[kRingSize] = {};
    std::atomic<size_t> m_writePos{0};
    std::atomic<size_t> m_readPos{0};

    // Baud rate timing: 31250 baud, 10 bits per byte (1 start + 8 data + 1 stop)
    // = 3125 bytes/sec → one byte every 320µs
    double m_byteAccum = 0.0;
    static constexpr double kMidiBytesPerSec = 3125.0;
};

} // namespace FB01
