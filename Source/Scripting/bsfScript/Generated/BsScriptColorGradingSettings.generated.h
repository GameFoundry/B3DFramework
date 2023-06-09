//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Math/BsVector3.h"

namespace bs { struct ColorGradingSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradingSettings : public TScriptReflectable<ScriptColorGradingSettings, ColorGradingSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorGradingSettings")

		ScriptColorGradingSettings(MonoObject* managedInstance, const SPtr<ColorGradingSettings>& value);

		static MonoObject* Create(const SPtr<ColorGradingSettings>& value);

	private:
		static void InternalGetSaturation(ScriptColorGradingSettings* thisPtr, TVector3<float>* __output);
		static void InternalSetSaturation(ScriptColorGradingSettings* thisPtr, TVector3<float>* value);
		static void InternalGetContrast(ScriptColorGradingSettings* thisPtr, TVector3<float>* __output);
		static void InternalSetContrast(ScriptColorGradingSettings* thisPtr, TVector3<float>* value);
		static void InternalGetGain(ScriptColorGradingSettings* thisPtr, TVector3<float>* __output);
		static void InternalSetGain(ScriptColorGradingSettings* thisPtr, TVector3<float>* value);
		static void InternalGetOffset(ScriptColorGradingSettings* thisPtr, TVector3<float>* __output);
		static void InternalSetOffset(ScriptColorGradingSettings* thisPtr, TVector3<float>* value);
	};
}
