#include "audio.h"
#include "midi.h"
#include <cstring>

namespace FB01 {

StandaloneAudio::~StandaloneAudio() {
    close();
}

bool StandaloneAudio::open(FB01Emulator* emulator, int sampleRate) {
    m_emulator = emulator;
    m_sampleRate = sampleRate;

    SDL_AudioSpec spec;
    spec.freq = sampleRate;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;

    m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                         &spec, audioCallback, this);
    if (!m_stream)
        return false;

    SDL_ResumeAudioStreamDevice(m_stream);
    return true;
}

void StandaloneAudio::close() {
    if (m_stream) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
}

void StandaloneAudio::audioCallback(void* userdata, SDL_AudioStream* stream,
                                    int additional_amount, int /*total_amount*/) {
    auto* self = static_cast<StandaloneAudio*>(userdata);
    int numSamples = additional_amount / (2 * sizeof(float)); // stereo float

    // Use a stack buffer for small blocks, heap for large
    float buf[2048];
    float* out = (numSamples * 2 <= 2048) ? buf : new float[numSamples * 2];

    for (int i = 0; i < numSamples; i++) {
        if (self->m_midi)
            self->m_midi->processSample(static_cast<double>(self->m_sampleRate));
        self->m_emulator->generateSample(out[i * 2], out[i * 2 + 1],
                                         static_cast<double>(self->m_sampleRate));
    }

    SDL_PutAudioStreamData(stream, out, numSamples * 2 * sizeof(float));

    if (out != buf)
        delete[] out;
}

} // namespace FB01
