//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Utility/Image/B3DColorGradient.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"
#include "../../../Foundation/Utility/Image/B3DColorGradient.h"

namespace b3d { class ColorGradientHDREx; }
namespace b3d { struct __ColorGradientKeyInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradientHDR : public TScriptNonReflectableWrapper<ColorGradientHDR, ScriptColorGradientHDR>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorGradientHDR")

		ScriptColorGradientHDR(const SPtr<ColorGradientHDR>& nativeObject);
		~ScriptColorGradientHDR();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalColorGradientHDR(MonoObject* scriptObject);
		static void InternalColorGradientHDR0(MonoObject* scriptObject, Color* color);
		static void InternalColorGradientHDR1(MonoObject* scriptObject, MonoArray* keys);
		static void InternalSetKeys(ScriptColorGradientHDR* self, MonoArray* keys, float duration);
		static MonoArray* InternalGetKeys(ScriptColorGradientHDR* self);
		static uint32_t InternalGetNumKeys(ScriptColorGradientHDR* self);
		static void InternalGetKey(ScriptColorGradientHDR* self, uint32_t index, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradientHDR* self, Color* color);
		static void InternalEvaluate(ScriptColorGradientHDR* self, float t, Color* __output);
	};
}
