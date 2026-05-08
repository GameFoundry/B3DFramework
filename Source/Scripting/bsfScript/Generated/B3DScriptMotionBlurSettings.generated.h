//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct MotionBlurSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMotionBlurSettings : public TScriptReflectableWrapper<MotionBlurSettings, ScriptMotionBlurSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MotionBlurSettings")

		ScriptMotionBlurSettings(const TShared<MotionBlurSettings>& nativeObject);
		~ScriptMotionBlurSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalMotionBlurSettings(MonoObject* scriptObject);
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
