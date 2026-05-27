#include "controller.h"
#include "plugids.h"
#include "ui/editor.h"
#include "base/source/fstreamer.h"

namespace FB01 {

Steinberg::tresult PLUGIN_API FB01Controller::initialize(Steinberg::FUnknown* context) {
    auto result = EditControllerEx1::initialize(context);
    if (result != Steinberg::kResultOk)
        return result;

    // Front panel button parameters (momentary, 0 or 1)
    parameters.addParameter(STR16("System Set Up"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonSystemSetUp);
    parameters.addParameter(STR16("Inst Select"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonInstSelect);
    parameters.addParameter(STR16("Inst Assign"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonInstAssign);
    parameters.addParameter(STR16("Inst Function"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonInstFunction);
    parameters.addParameter(STR16("Voice Function"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonVoiceFunction);
    parameters.addParameter(STR16("Voice Select"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonVoiceSelect);
    parameters.addParameter(STR16("-1 / No"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonMinus);
    parameters.addParameter(STR16("+1 / Yes"), nullptr, 1, 0,
        Steinberg::Vst::ParameterInfo::kCanAutomate, kButtonPlus);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API FB01Controller::terminate() {
    return EditControllerEx1::terminate();
}

Steinberg::tresult PLUGIN_API FB01Controller::setComponentState(Steinberg::IBStream* state) {
    if (!state)
        return Steinberg::kResultFalse;
    // Nothing to restore from component state for the controller
    return Steinberg::kResultOk;
}

Steinberg::IPlugView* PLUGIN_API FB01Controller::createView(Steinberg::FIDString name) {
    if (Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor)) {
        return new FB01Editor(this);
    }
    return nullptr;
}

} // namespace FB01
