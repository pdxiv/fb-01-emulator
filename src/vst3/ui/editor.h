#pragma once

#include "public.sdk/source/vst/vstguieditor.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/ctextlabel.h"

namespace FB01 {

class FB01Editor : public Steinberg::Vst::VSTGUIEditor, public VSTGUI::IControlListener {
public:
    FB01Editor(Steinberg::Vst::EditController* controller);
    ~FB01Editor() override = default;

    // VSTGUIEditor overrides
    bool PLUGIN_API open(void* parent, const VSTGUI::PlatformType& platformType) override;
    void PLUGIN_API close() override;

    // IControlListener
    void valueChanged(VSTGUI::CControl* pControl) override;

    DELEGATE_REFCOUNT(Steinberg::Vst::VSTGUIEditor)

private:
    void createUI(VSTGUI::CFrame* frame);

    VSTGUI::CTextLabel* m_lcdLabel = nullptr;

    static constexpr int kEditorWidth = 480;
    static constexpr int kEditorHeight = 200;
};

} // namespace FB01
