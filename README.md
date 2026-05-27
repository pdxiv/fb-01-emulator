# FB-01 VST3

A cycle-accurate emulator of the **Yamaha FB-01 FM Sound Generator** (1986), packaged as a VST3 plugin.

## Overview

The FB-01 is a rack-mount FM synthesizer built around a Z80 CPU and Yamaha YM2164 (OPM) sound chip. This project emulates the entire hardware at the cycle level, running the original firmware ROM to produce authentic output.

**Architecture:**
- Z80 CPU at 6 MHz — [superzazu/z80](https://github.com/superzazu/z80)
- YM2164 (OPM) at 4 MHz — [Nuked-OPM](https://github.com/nukeykt/Nuked-OPM)
- i8251-compatible USART (MIDI at 31.25 kbaud)
- HD44780 LCD controller (2×8 character display)
- Front-panel button emulation via GUI

## Building

### Prerequisites

**Linux:**
```bash
sudo apt install build-essential cmake libx11-dev libxcb-util-dev \
    libxcb-cursor-dev libxcb-keysyms1-dev libxcb-xkb-dev \
    libxkbcommon-dev libxkbcommon-x11-dev libcairo2-dev \
    libpango1.0-dev libgtkmm-3.0-dev libfreetype-dev libsqlite3-dev
```

**Windows (clang-cl):**
```cmd
winget install Kitware.CMake LLVM.LLVM Ninja-build.Ninja
```
Also install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) for the Windows SDK headers.

### Clone

```bash
git clone --recursive https://github.com/your-username/fb01-vst.git
cd fb01-vst
```

If you already cloned without `--recursive`:
```bash
make submodules
```

### Build (Linux)

```bash
make
```

Or manually:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target fb01 -j$(nproc)
```

### Build (Windows)

```cmd
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release
cmake --build . --target fb01
```

### Standalone Application

A standalone SDL3 application (no DAW required) with audio output, MIDI input, and GUI:

```bash
make standalone
./build/bin/fb01-standalone rom/fb01.bin
```

On Windows:
```cmd
cmake .. -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release -DFB01_BUILD_STANDALONE=ON
cmake --build . --target fb01-standalone
```

The standalone app uses SDL3 for audio/window and RtMidi for MIDI input. It opens the first available MIDI port (or creates a virtual port on Linux/macOS).

### Install

```bash
make install    # copies to ~/.vst3/
```

## ROM

The plugin requires the original FB-01 firmware ROM (32 KB) to operate. It is **not included** in this repository.

| Property | Value |
|----------|-------|
| Size | 32,768 bytes |
| CRC32 | `0x7357E9A4` |
| SHA1 | `049c482d6c91b7e2846757dd0f5138e0d8b687f0` |

Place the ROM file in the `rom/` directory. It is loaded via the plugin's saved state.

## Usage

1. Load the **FB-01** plugin as a VST3 instrument in your DAW
2. Load the ROM via the plugin state mechanism
3. Send MIDI notes, CC, and SysEx — the emulator processes them exactly as the original hardware would

All FB-01 parameters are accessible through standard MIDI (CC, SysEx, Program Change). No additional parameter mapping layer is needed.

## Project Structure

```
├── CMakeLists.txt          Top-level build
├── Makefile                Convenience wrapper
├── src/
│   ├── entry.cpp           VST3 factory
│   ├── processor.cpp       Audio thread
│   ├── controller.cpp      Parameter controller
│   ├── emulator/
│   │   ├── fb01.cpp        Memory map, I/O dispatch, interrupts
│   │   ├── scheduler.cpp   Cycle-accurate Z80/OPM scheduling
│   │   ├── i8251.cpp       USART (MIDI serial)
│   │   └── hd44780.cpp     LCD controller
│   ├── standalone/
│   │   ├── main.cpp        SDL3 entry point and main loop
│   │   ├── audio.cpp       SDL3 audio device callback
│   │   ├── midi.cpp        RtMidi input handling
│   │   └── gui.cpp         SDL3 window, LCD and button rendering
│   └── ui/
│       └── editor.cpp      VSTGUI editor (LCD + buttons)
├── z80/                    Git submodule: superzazu/z80
├── opm/                    Git submodule: Nuked-OPM
├── vst3sdk/                Git submodule: Steinberg VST3 SDK
├── sdl3/                   Git submodule: SDL3
└── rtmidi/                 Git submodule: RtMidi
```

## License

- Emulator source code: MIT
- [Nuked-OPM](https://github.com/nukeykt/Nuked-OPM): LGPL-2.1
- [superzazu/z80](https://github.com/superzazu/z80): MIT
- [VST3 SDK](https://github.com/steinbergmedia/vst3sdk): Dual licensed (proprietary / GPLv3)
- [SDL3](https://github.com/libsdl-org/SDL): zlib
- [RtMidi](https://github.com/thestk/rtmidi): MIT

## Acknowledgements

- Nuke.YKT for the Nuked-OPM emulator
- superzazu for the Z80 emulator
- Steinberg for the VST3 SDK
