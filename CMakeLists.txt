cmake_minimum_required(VERSION 3.25.0)

project(fb-01-emulator
    VERSION 1.0.0
    DESCRIPTION "Yamaha FB-01 FM Sound Generator Emulator"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Disable SDK examples and hosting
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)

# Add VST3 SDK (suppress third-party warnings)
set(CMAKE_C_FLAGS_SAVED "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_SAVED "${CMAKE_CXX_FLAGS}")
if(MSVC OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    string(APPEND CMAKE_C_FLAGS " /w")
    string(APPEND CMAKE_CXX_FLAGS " /w")
else()
    string(APPEND CMAKE_C_FLAGS " -w")
    string(APPEND CMAKE_CXX_FLAGS " -w")
endif()
add_subdirectory(vst3sdk SYSTEM)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVED}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")

# Remove C++-only flag from SDK C targets (GCC/Clang warn about it on Linux)
if(TARGET vstgui_wayland_protocols)
    get_target_property(_opts vstgui_wayland_protocols COMPILE_OPTIONS)
    if(_opts)
        list(REMOVE_ITEM _opts "-Wsuggest-override")
        set_target_properties(vstgui_wayland_protocols PROPERTIES COMPILE_OPTIONS "${_opts}")
    endif()
endif()

# Z80 emulator library (C)
add_library(z80_lib STATIC z80/z80.c)
target_include_directories(z80_lib PUBLIC z80)
set_target_properties(z80_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(z80_lib PRIVATE /w)
else()
    target_compile_options(z80_lib PRIVATE -w)
endif()

# Nuked-OPM library (C)
add_library(opm_lib STATIC opm/opm.c)
target_include_directories(opm_lib PUBLIC opm)
set_target_properties(opm_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(opm_lib PRIVATE /w)
else()
    target_compile_options(opm_lib PRIVATE -w)
endif()

# Plugin sources
set(fb01_sources
    src/vst3/entry.cpp
    src/vst3/processor.h
    src/vst3/processor.cpp
    src/vst3/controller.h
    src/vst3/controller.cpp
    src/vst3/plugids.h
    src/vst3/ui/editor.h
    src/vst3/ui/editor.cpp
    src/emulator/fb01.h
    src/emulator/fb01.cpp
    src/emulator/scheduler.h
    src/emulator/scheduler.cpp
    src/emulator/i8251.h
    src/emulator/i8251.cpp
    src/emulator/hd44780.h
    src/emulator/hd44780.cpp
    src/emulator/resampler.h
)

smtg_add_vst3plugin(fb01 ${fb01_sources})

# Add platform-specific module entry point
if(SMTG_LINUX)
    target_sources(fb01 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/public.sdk/source/main/linuxmain.cpp
    )
elseif(SMTG_WIN)
    target_sources(fb01 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/public.sdk/source/main/dllmain.cpp
    )
elseif(SMTG_MAC)
    target_sources(fb01 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/public.sdk/source/main/macmain.cpp
    )
endif()

target_include_directories(fb01 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vst3
    ${CMAKE_CURRENT_SOURCE_DIR}/z80
    ${CMAKE_CURRENT_SOURCE_DIR}/opm
)

target_link_libraries(fb01 PRIVATE
    vstgui_support
    sdk
    z80_lib
    opm_lib
)

if(SMTG_MAC)
    smtg_target_set_bundle_id(fb01 "com.yamaha.fb01")
endif()

# --- Standalone SDL3 application (optional) ---
option(FB01_BUILD_STANDALONE "Build standalone SDL3 application" OFF)

if(FB01_BUILD_STANDALONE)
    # SDL3 (static)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    if(MSVC)
        set(CMAKE_C_FLAGS_SAVED2 "${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS_SAVED2 "${CMAKE_CXX_FLAGS}")
        string(APPEND CMAKE_C_FLAGS " /w")
        string(APPEND CMAKE_CXX_FLAGS " /w")
    else()
        set(CMAKE_C_FLAGS_SAVED2 "${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS_SAVED2 "${CMAKE_CXX_FLAGS}")
        string(APPEND CMAKE_C_FLAGS " -w")
        string(APPEND CMAKE_CXX_FLAGS " -w")
    endif()
    add_subdirectory(sdl3 SYSTEM EXCLUDE_FROM_ALL)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVED2}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED2}")

    # RtMidi (static)
    set(RTMIDI_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(RTMIDI_API_ALSA ON CACHE BOOL "" FORCE)
    if(MSVC)
        set(CMAKE_C_FLAGS_SAVED3 "${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS_SAVED3 "${CMAKE_CXX_FLAGS}")
        string(APPEND CMAKE_C_FLAGS " /w")
        string(APPEND CMAKE_CXX_FLAGS " /w")
    else()
        set(CMAKE_C_FLAGS_SAVED3 "${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS_SAVED3 "${CMAKE_CXX_FLAGS}")
        string(APPEND CMAKE_C_FLAGS " -w")
        string(APPEND CMAKE_CXX_FLAGS " -w")
    endif()
    add_subdirectory(rtmidi SYSTEM EXCLUDE_FROM_ALL)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVED3}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED3}")

    # Standalone executable
    add_executable(fb01-standalone
        src/standalone/main.cpp
        src/standalone/audio.h
        src/standalone/audio.cpp
        src/standalone/midi.h
        src/standalone/midi.cpp
        src/standalone/gui.h
        src/standalone/gui.cpp
        src/emulator/fb01.h
        src/emulator/fb01.cpp
        src/emulator/scheduler.h
        src/emulator/scheduler.cpp
        src/emulator/i8251.h
        src/emulator/i8251.cpp
        src/emulator/hd44780.h
        src/emulator/hd44780.cpp
    )

    target_include_directories(fb01-standalone PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rtmidi
    )

    target_link_libraries(fb01-standalone PRIVATE
        SDL3::SDL3-static
        rtmidi
        z80_lib
        opm_lib
    )

    set_target_properties(fb01-standalone PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
