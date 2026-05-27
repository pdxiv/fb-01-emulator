#pragma once

#include <SDL3/SDL.h>
#include "../emulator/fb01.h"

namespace FB01 {

class StandaloneGui {
public:
    StandaloneGui() = default;
    ~StandaloneGui();

    bool open(FB01Emulator* emulator);
    void close();

    // Process SDL events. Returns false if user requested quit.
    bool processEvents();

    // Render current frame.
    void render();

private:
    void renderLCD();
    void renderButtons();
    void handleButtonClick(int x, int y, bool pressed);
    void updateButtonHold();

    FB01Emulator* m_emulator = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    // Button state tracking
    bool m_buttonPressed[8] = {};
    uint32_t m_buttonPressTime[8] = {}; // SDL tick when pressed

    // Minimum hold time in ms to survive firmware debounce scan
    static constexpr uint32_t kMinHoldMs = 100;

    static constexpr int kWindowWidth = 480;
    static constexpr int kWindowHeight = 200;
    static constexpr int kLCDX = 20;
    static constexpr int kLCDY = 20;
    static constexpr int kLCDWidth = 440;
    static constexpr int kLCDHeight = 40;
    static constexpr int kButtonY = 100;
    static constexpr int kButtonWidth = 50;
    static constexpr int kButtonHeight = 60;
    static constexpr int kButtonSpacing = 56;
    static constexpr int kButtonStartX = 20;
};

} // namespace FB01
