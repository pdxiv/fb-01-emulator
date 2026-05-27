# FB-01 Emulator — Project Directory Structure

```text
fb01/
├── .gitmodules                     # Declares all git submodules
├── CMakeLists.txt                  # Top-level build configuration
├── Makefile                        # Convenience targets (build, standalone, install, clean)
├── README.md
│
├── z80/                            # Git submodule: superzazu/z80 (C11, MIT)
│   ├── z80.h
│   ├── z80.c
│   └── ...
│
├── opm/                            # Git submodule: nukeykt/Nuked-OPM (C11, LGPL-2.1)
│   ├── opm.h
│   ├── opm.c
│   └── ...
│
├── vst3sdk/                        # Git submodule: steinbergmedia/vst3sdk (MIT)
│   └── ...
│
├── sdl3/                           # Git submodule: libsdl-org/SDL release-3.4.0
│   └── ...
│
├── rtmidi/                         # Git submodule: thestk/rtmidi
│   └── ...
│
├── docs/                           # Hardware and project documentation
│   ├── fb01-hardware-spec.md       # FB-01 hardware reference
│   ├── hd44780-spec.md             # LCD controller datasheet notes
│   ├── i8251-spec.md               # USART datasheet notes
│   ├── ymopm-spec.md               # YM2151/YM2164 OPM reference
│   └── project-structure.md        # This file
│
├── src/
│   ├── emulator/                   # Shared core (platform-independent)
│   │   ├── fb01.h                  # Top-level emulator interface
│   │   ├── fb01.cpp                # Memory map, I/O dispatch, interrupt logic
│   │   ├── scheduler.h            # Cycle-accurate interleaved scheduler
│   │   ├── scheduler.cpp          # Advances Z80/OPM per audio sample
│   │   ├── i8251.h                # USART emulation (MIDI serial interface)
│   │   ├── i8251.cpp
│   │   ├── hd44780.h              # LCD controller emulation
│   │   ├── hd44780.cpp
│   │   └── resampler.h            # OPM 62.5 kHz → host sample rate
│   │
│   ├── vst3/                       # VST3 plugin target
│   │   ├── entry.cpp              # VST3 factory: GetPluginFactory(), GUIDs
│   │   ├── plugids.h              # Plugin/parameter IDs, version constants
│   │   ├── processor.h            # FB01Processor class declaration
│   │   ├── processor.cpp          # Audio thread: process(), state save/load
│   │   ├── controller.h           # FB01Controller class declaration
│   │   ├── controller.cpp         # Parameters, setComponentState, createView
│   │   └── ui/
│   │       ├── editor.h
│   │       └── editor.cpp         # Renders LCD display and front-panel buttons
│   │
│   └── standalone/                 # Standalone SDL3 application
│       ├── main.cpp                # SDL3 entry point and main loop
│       ├── audio.h
│       ├── audio.cpp               # SDL3 audio device and callback
│       ├── gui.h
│       ├── gui.cpp                 # SDL3 renderer: LCD (bitmap font) and buttons
│       ├── midi.h
│       └── midi.cpp                # RtMidi MIDI input
│
├── rom/                            # ROM files (not distributed)
│   ├── .gitkeep
│   ├── nec__-011_xb712c0__8709ex700__d27c256c-15.ic11  # FB-01 firmware ROM
│   └── hd44780_a00.bin             # HD44780 CGROM font (A00 variant)
│
└── resource/                       # Plugin resources (icons, snapshots)
```

## Git Submodules

```bash
git submodule add https://github.com/superzazu/z80.git z80
git submodule add https://github.com/nukeykt/Nuked-OPM.git opm
git submodule add https://github.com/steinbergmedia/vst3sdk.git vst3sdk
git submodule add -b release-3.4.0 https://github.com/libsdl-org/SDL.git sdl3
git submodule add https://github.com/thestk/rtmidi.git rtmidi
```

Clone with all dependencies:

```bash
git clone --recursive <this-repo-url>
```
