#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "audio.h"
#include "midi.h"
#include "gui.h"
#include "../emulator/fb01.h"

static std::vector<uint8_t> loadRomFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return {};
    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

int main(int argc, char* argv[]) {
    // Find ROM file: command line argument or default path
    const char* romPath = nullptr;
    if (argc > 1) {
        romPath = argv[1];
    } else {
        // Try default location
        romPath = "rom/fb01.bin";
    }

    // Load ROM
    auto romData = loadRomFile(romPath);
    if (romData.size() != 32768) {
        fprintf(stderr, "Error: Could not load 32KB ROM from '%s'\n", romPath);
        fprintf(stderr, "Usage: %s [path-to-rom]\n", argv[0]);
        return 1;
    }

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create emulator and load ROM
    FB01::FB01Emulator emulator;
    if (!emulator.loadROM(romData.data(), romData.size())) {
        fprintf(stderr, "Failed to load ROM into emulator.\n");
        SDL_Quit();
        return 1;
    }
    emulator.reset();

    // Open GUI
    FB01::StandaloneGui gui;
    if (!gui.open(&emulator)) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Open audio
    FB01::StandaloneAudio audio;
    if (!audio.open(&emulator, 48000)) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Open MIDI
    FB01::StandaloneMidi midi;
    if (!midi.open(&emulator)) {
        fprintf(stderr, "Warning: MIDI input not available.\n");
    }
    audio.setMidi(&midi);

    fprintf(stderr, "FB-01 running. Audio: %d Hz. Press ESC or close window to quit.\n",
            audio.getSampleRate());

    // Main loop
    bool running = true;

    while (running) {
        running = gui.processEvents();
        gui.render();
        SDL_Delay(16); // ~60 fps
    }

    // Cleanup
    midi.close();
    audio.close();
    gui.close();
    SDL_Quit();

    return 0;
}
