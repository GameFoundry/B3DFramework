//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"

namespace bs { class ColorGradientHDREx; }
namespace bs { struct __ColorGradientKeyInterop; }
namespace bs
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
		static void InternalGetKey(ScriptColorGradientHDR* self, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradientHDR* self, Color* color);
		static void InternalEvaluate(ScriptColorGradientHDR* self, float t, Color* __output);
	};
}
