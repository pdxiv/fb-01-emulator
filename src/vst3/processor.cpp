#include "processor.h"
#include "plugids.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/base/ibstream.h"
#include "base/source/fstreamer.h"

#include <cstring>
#include <fstream>
#include <filesystem>

namespace FB01 {

FB01Processor::FB01Processor()
    : m_sampleRate(44100.0)
    , m_romLoaded(false)
{
    setControllerClass(kControllerUID);
}

FB01Processor::~FB01Processor() = default;

Steinberg::tresult PLUGIN_API FB01Processor::initialize(Steinberg::FUnknown* context) {
    auto result = AudioEffect::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    // Stereo output
    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    // MIDI event input
    addEventInput(STR16("MIDI In"), 1);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API FB01Processor::terminate() {
    return AudioEffect::terminate();
}

Steinberg::tresult PLUGIN_API FB01Processor::setActive(Steinberg::TBool state) {
    if (state) {
        // Activate: reset emulator if ROM is loaded
        if (m_romLoaded) {
            m_emulator.reset();
            m_emulator.loadROM(m_romData.data(), m_romData.size());
        }
    }
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API FB01Processor::setupProcessing(Steinberg::Vst::ProcessSetup& setup) {
    m_sampleRate = setup.sampleRate;
    return AudioEffect::setupProcessing(setup);
}

Steinberg::tresult PLUGIN_API FB01Processor::canProcessSampleSize(Steinberg::int32 symbolicSampleSize) {
    if (symbolicSampleSize == Steinberg::Vst::kSample32)
        return Steinberg::kResultTrue;
    if (symbolicSampleSize == Steinberg::Vst::kSample64)
        return Steinberg::kResultTrue;
    return Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API FB01Processor::process(Steinberg::Vst::ProcessData& data) {
    // Process MIDI events
    if (data.inputEvents) {
        processEvents(data.inputEvents);
    }

    // Generate audio
    if (data.numOutputs == 0 || data.outputs[0].numChannels < 2)
        return Steinberg::kResultOk;

    Steinberg::int32 numSamples = data.numSamples;

    if (data.symbolicSampleSize == Steinberg::Vst::kSample32) {
        float* outL = data.outputs[0].channelBuffers32[0];
        float* outR = data.outputs[0].channelBuffers32[1];

        if (!m_romLoaded) {
            std::memset(outL, 0, numSamples * sizeof(float));
            std::memset(outR, 0, numSamples * sizeof(float));
            return Steinberg::kResultOk;
        }

        for (Steinberg::int32 i = 0; i < numSamples; i++) {
            m_emulator.generateSample(outL[i], outR[i], m_sampleRate);
        }
    } else {
        double* outL = data.outputs[0].channelBuffers64[0];
        double* outR = data.outputs[0].channelBuffers64[1];

        if (!m_romLoaded) {
            std::memset(outL, 0, numSamples * sizeof(double));
            std::memset(outR, 0, numSamples * sizeof(double));
            return Steinberg::kResultOk;
        }

        for (Steinberg::int32 i = 0; i < numSamples; i++) {
            float l, r;
            m_emulator.generateSample(l, r, m_sampleRate);
            outL[i] = static_cast<double>(l);
            outR[i] = static_cast<double>(r);
        }
    }

    return Steinberg::kResultOk;
}

void FB01Processor::processEvents(Steinberg::Vst::IEventList* events) {
    Steinberg::int32 count = events->getEventCount();
    for (Steinberg::int32 i = 0; i < count; i++) {
        Steinberg::Vst::Event event;
        if (events->getEvent(i, event) == Steinberg::kResultOk) {
            switch (event.type) {
                case Steinberg::Vst::Event::kNoteOnEvent:
                    // Convert to MIDI note on: 0x9n kk vv
                    m_emulator.sendMidiByte(0x90 | (event.noteOn.channel & 0x0F));
                    m_emulator.sendMidiByte(static_cast<uint8_t>(event.noteOn.pitch));
                    m_emulator.sendMidiByte(static_cast<uint8_t>(event.noteOn.velocity * 127.0f));
                    break;
                case Steinberg::Vst::Event::kNoteOffEvent:
                    // Convert to MIDI note off: 0x8n kk vv
                    m_emulator.sendMidiByte(0x80 | (event.noteOff.channel & 0x0F));
                    m_emulator.sendMidiByte(static_cast<uint8_t>(event.noteOff.pitch));
                    m_emulator.sendMidiByte(static_cast<uint8_t>(event.noteOff.velocity * 127.0f));
                    break;
                case Steinberg::Vst::Event::kDataEvent:
                    // SysEx or raw MIDI data
                    if (event.data.type == Steinberg::Vst::DataEvent::kMidiSysEx) {
                        // Send SysEx: F0, data, F7
                        m_emulator.sendMidiByte(0xF0);
                        for (Steinberg::uint32 j = 0; j < event.data.size; j++) {
                            m_emulator.sendMidiByte(event.data.bytes[j]);
                        }
                        m_emulator.sendMidiByte(0xF7);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

Steinberg::tresult PLUGIN_API FB01Processor::setState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kResultFalse;

    Steinberg::IBStreamer streamer(state, kLittleEndian);

    // Read ROM data size
    Steinberg::int32 romSize = 0;
    if (!streamer.readInt32(romSize))
        return Steinberg::kResultFalse;

    if (romSize == 32768) {
        m_romData.resize(romSize);
        Steinberg::int32 bytesRead = 0;
        state->read(m_romData.data(), romSize, &bytesRead);
        if (bytesRead == romSize) {
            m_romLoaded = m_emulator.loadROM(m_romData.data(), m_romData.size());
            if (m_romLoaded) {
                m_emulator.reset();
            }
        }
    }

    // Read RAM state if present
    Steinberg::int32 ramSize = 0;
    if (streamer.readInt32(ramSize) && ramSize == 16384) {
        std::vector<uint8_t> ramData(ramSize);
        Steinberg::int32 bytesRead = 0;
        state->read(ramData.data(), ramSize, &bytesRead);
        // RAM state would be restored into the emulator here
    }

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API FB01Processor::getState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kResultFalse;

    Steinberg::IBStreamer streamer(state, kLittleEndian);

    // Write ROM data
    Steinberg::int32 romSize = static_cast<Steinberg::int32>(m_romData.size());
    streamer.writeInt32(romSize);
    if (romSize > 0) {
        state->write(m_romData.data(), romSize, nullptr);
    }

    return Steinberg::kResultOk;
}

} // namespace FB01
