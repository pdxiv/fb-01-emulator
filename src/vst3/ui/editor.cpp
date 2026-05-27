#include "editor.h"
#include "../controller.h"

#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/crect.h"
#include "vstgui/lib/cfont.h"

namespace FB01 {

FB01Editor::FB01Editor(Steinberg::Vst::EditController* controller)
    : VSTGUIEditor(controller)
{
    setRect({0, 0, kEditorWidth, kEditorHeight});
}

bool PLUGIN_API FB01Editor::open(void* parent, const VSTGUI::PlatformType& platformType) {
    if (frame)
        return false;

    VSTGUI::CRect size(0, 0, kEditorWidth, kEditorHeight);
    auto* newFrame = new VSTGUI::CFrame(size, this);
    newFrame->open(parent);
    newFrame->setBackgroundColor(VSTGUI::CColor(40, 40, 40, 255));

    frame = newFrame;
    createUI(newFrame);
    return true;
}

void PLUGIN_API FB01Editor::close() {
    if (frame) {
        frame->forget();
        frame = nullptr;
    }
    m_lcdLabel = nullptr;
}

void FB01Editor::createUI(VSTGUI::CFrame* frame) {
    using namespace VSTGUI;

    // LCD display area (dark red background, bright red text)
    CRect lcdRect(20, 20, 460, 60);
    m_lcdLabel = new CTextLabel(lcdRect, "                ");
    m_lcdLabel->setBackColor(CColor(30, 0, 0, 255));
    m_lcdLabel->setFontColor(CColor(180, 0, 0, 255));
    auto lcdFont = makeOwned<CFontDesc>("Courier New", 18, kBoldFace);
    m_lcdLabel->setFont(lcdFont);
    m_lcdLabel->setStyle(CTextLabel::kNoFrame);
    m_lcdLabel->setTextTruncateMode(CTextLabel::kTruncateNone);
    frame->addView(m_lcdLabel);

    // Front panel buttons
    const char* buttonNames[] = {
        "System\nSet Up", "Inst\nSelect", "Inst\nAssign", "Inst\nFunction",
        "Voice\nFunction", "Voice\nSelect", "-1/No", "+1/Yes"
    };
    Steinberg::Vst::ParamID buttonIDs[] = {
        FB01Controller::kButtonSystemSetUp,
        FB01Controller::kButtonInstSelect,
        FB01Controller::kButtonInstAssign,
        FB01Controller::kButtonInstFunction,
        FB01Controller::kButtonVoiceFunction,
        FB01Controller::kButtonVoiceSelect,
        FB01Controller::kButtonMinus,
        FB01Controller::kButtonPlus,
    };

    for (int i = 0; i < 8; i++) {
        CRect btnRect(20 + i * 56, 100, 20 + i * 56 + 50, 160);
        auto* btn = new CTextButton(btnRect, this, buttonIDs[i], buttonNames[i]);
        btn->setTag(buttonIDs[i]);
        frame->addView(btn);
    }
}

void FB01Editor::valueChanged(VSTGUI::CControl* pControl) {
    auto tag = pControl->getTag();
    auto value = pControl->getValue();

    if (auto* controller = getController()) {
        controller->beginEdit(tag);
        controller->performEdit(tag, value);
        controller->endEdit(tag);
    }
}

} // namespace FB01
