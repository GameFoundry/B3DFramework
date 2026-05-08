//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"

namespace b3d { struct ColorGradingSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradingSettings : public TScriptReflectableWrapper<ColorGradingSettings, ScriptColorGradingSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorGradingSettings")

		ScriptColorGradingSettings(const TShared<ColorGradingSettings>& nativeObject);
		~ScriptColorGradingSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetSaturation(ScriptColorGradingSettings* self, TVector3<float>* __output);
		static void InternalSetSaturation(ScriptColorGradingSettings* self, TVector3<float>* value);
		static void InternalGetContrast(ScriptColorGradingSettings* self, TVector3<float>* __output);
		static void InternalSetContrast(ScriptColorGradingSettings* self, TVector3<float>* value);
		static void InternalGetGain(ScriptColorGradingSettings* self, TVector3<float>* __output);
		static void InternalSetGain(ScriptColorGradingSettings* self, TVector3<float>* value);
		static void InternalGetOffset(ScriptColorGradingSettings* self, TVector3<float>* __output);
		static void InternalSetOffset(ScriptColorGradingSettings* self, TVector3<float>* value);
	};
}
