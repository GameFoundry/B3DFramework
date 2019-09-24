//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct MotionBlurSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMotionBlurSettings : public TScriptReflectable<ScriptMotionBlurSettings, MotionBlurSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MotionBlurSettings")

		ScriptMotionBlurSettings(MonoObject* managedInstance, const SPtr<MotionBlurSettings>& value);

		static MonoObject* create(const SPtr<MotionBlurSettings>& value);

	private:
		static void Internal_MotionBlurSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptMotionBlurSettings* thisPtr);
		static void Internal_setenabled(ScriptMotionBlurSettings* thisPtr, bool value);
		static MotionBlurDomain Internal_getdomain(ScriptMotionBlurSettings* thisPtr);
		static void Internal_setdomain(ScriptMotionBlurSettings* thisPtr, MotionBlurDomain value);
		static MotionBlurFilter Internal_getfilter(ScriptMotionBlurSettings* thisPtr);
		static void Internal_setfilter(ScriptMotionBlurSettings* thisPtr, MotionBlurFilter value);
		static MotionBlurQuality Internal_getquality(ScriptMotionBlurSettings* thisPtr);
		static void Internal_setquality(ScriptMotionBlurSettings* thisPtr, MotionBlurQuality value);
		static float Internal_getmaximumRadius(ScriptMotionBlurSettings* thisPtr);
		static void Internal_setmaximumRadius(ScriptMotionBlurSettings* thisPtr, float value);
	};
}
