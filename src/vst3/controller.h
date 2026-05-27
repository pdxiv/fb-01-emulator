#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace FB01 {

class FB01Controller : public Steinberg::Vst::EditControllerEx1 {
public:
    FB01Controller() = default;
    ~FB01Controller() override = default;

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IEditController*>(new FB01Controller());
    }

    // EditController overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) override;

    // Button parameter IDs for front panel
    enum ParamIDs : Steinberg::Vst::ParamID {
        kButtonSystemSetUp = 100,
        kButtonInstSelect,
        kButtonInstAssign,
        kButtonInstFunction,
        kButtonVoiceFunction,
        kButtonVoiceSelect,
        kButtonMinus,
        kButtonPlus,
        kParamRomLoaded = 200,
    };
};

} // namespace FB01
