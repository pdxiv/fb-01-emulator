#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "emulator/fb01.h"
#include <vector>
#include <mutex>

namespace FB01 {

class FB01Processor : public Steinberg::Vst::AudioEffect {
public:
    FB01Processor();
    ~FB01Processor() override;

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new FB01Processor());
    }

    // AudioEffect overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

    // State
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;

private:
    void processEvents(Steinberg::Vst::IEventList* events);

    FB01Emulator m_emulator;
    double m_sampleRate;
    bool m_romLoaded;
    std::vector<uint8_t> m_romData;
};

} // namespace FB01
