#include "public.sdk/source/main/pluginfactory.h"
#include "plugids.h"
#include "processor.h"
#include "controller.h"

#define stringPluginName "FB-01"

BEGIN_FACTORY_DEF("FB01Emu", "https://github.com/fb01emu", "mailto:info@fb01emu.org")

DEF_CLASS2(INLINE_UID_FROM_FUID(FB01::kProcessorUID),
           PClassInfo::kManyInstances,
           kVstAudioEffectClass,
           stringPluginName,
           Vst::kDistributable,
           FB01::kSubCategories,
           "1.0.0",
           kVstVersionString,
           FB01::FB01Processor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(FB01::kControllerUID),
           PClassInfo::kManyInstances,
           kVstComponentControllerClass,
           stringPluginName "Controller",
           0,
           "",
           "1.0.0",
           kVstVersionString,
           FB01::FB01Controller::createInstance)

END_FACTORY
