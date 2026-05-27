#pragma once

#include <SDL3/SDL.h>
#include "../emulator/fb01.h"

namespace FB01 {

class StandaloneMidi;

class StandaloneAudio {
public:
    StandaloneAudio() = default;
    ~StandaloneAudio();

    bool open(FB01Emulator* emulator, int sampleRate = 48000);
    void close();

    void setMidi(StandaloneMidi* midi) { m_midi = midi; }
    int getSampleRate() const { return m_sampleRate; }

private:
    static void audioCallback(void* userdata, SDL_AudioStream* stream,
                              int additional_amount, int total_amount);

    FB01Emulator* m_emulator = nullptr;
    StandaloneMidi* m_midi = nullptr;
    SDL_AudioStream* m_stream = nullptr;
    int m_sampleRate = 48000;
};

} // namespace FB01
