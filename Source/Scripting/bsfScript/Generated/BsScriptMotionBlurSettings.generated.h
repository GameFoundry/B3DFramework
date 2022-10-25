//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct MotionBlurSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMotionBlurSettings : public TScriptReflectable<ScriptMotionBlurSettings, MotionBlurSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MotionBlurSettings")

		ScriptMotionBlurSettings(MonoObject* managedInstance, const SPtr<MotionBlurSettings>& value);

		static MonoObject* Create(const SPtr<MotionBlurSettings>& value);

	private:
		static void InternalMotionBlurSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptMotionBlurSettings* thisPtr);
		static void InternalSetEnabled(ScriptMotionBlurSettings* thisPtr, bool value);
		static MotionBlurDomain InternalGetDomain(ScriptMotionBlurSettings* thisPtr);
		static void InternalSetDomain(ScriptMotionBlurSettings* thisPtr, MotionBlurDomain value);
		static MotionBlurFilter InternalGetFilter(ScriptMotionBlurSettings* thisPtr);
		static void InternalSetFilter(ScriptMotionBlurSettings* thisPtr, MotionBlurFilter value);
		static MotionBlurQuality InternalGetQuality(ScriptMotionBlurSettings* thisPtr);
		static void InternalSetQuality(ScriptMotionBlurSettings* thisPtr, MotionBlurQuality value);
		static float InternalGetMaximumRadius(ScriptMotionBlurSettings* thisPtr);
		static void InternalSetMaximumRadius(ScriptMotionBlurSettings* thisPtr, float value);
	};
} // namespace bs
