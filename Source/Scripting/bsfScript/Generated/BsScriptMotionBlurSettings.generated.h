//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMotionBlurSettings : public TScriptReflectable<ScriptMotionBlurSettings, MotionBlurSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MotionBlurSettings")

		ScriptMotionBlurSettings(MonoObject* managedInstance, const SPtr<MotionBlurSettings>& value);

		static MonoObject* Create(const SPtr<MotionBlurSettings>& value);

	private:
		static void InternalMotionBlurSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptMotionBlurSettings* self);
		static void InternalSetEnabled(ScriptMotionBlurSettings* self, bool value);
		static MotionBlurDomain InternalGetDomain(ScriptMotionBlurSettings* self);
		static void InternalSetDomain(ScriptMotionBlurSettings* self, MotionBlurDomain value);
		static MotionBlurFilter InternalGetFilter(ScriptMotionBlurSettings* self);
		static void InternalSetFilter(ScriptMotionBlurSettings* self, MotionBlurFilter value);
		static MotionBlurQuality InternalGetQuality(ScriptMotionBlurSettings* self);
		static void InternalSetQuality(ScriptMotionBlurSettings* self, MotionBlurQuality value);
		static float InternalGetMaximumRadius(ScriptMotionBlurSettings* self);
		static void InternalSetMaximumRadius(ScriptMotionBlurSettings* self, float value);
	};
}
