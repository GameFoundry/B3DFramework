//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"

namespace bs { class ColorGradientHDR; }
namespace bs { class ColorGradientHDREx; }
namespace bs { struct __ColorGradientKeyInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradientHDR : public ScriptObject<ScriptColorGradientHDR>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorGradientHDR")

		ScriptColorGradientHDR(MonoObject* managedInstance, const SPtr<ColorGradientHDR>& value);

		SPtr<ColorGradientHDR> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<ColorGradientHDR>& value);

	private:
		SPtr<ColorGradientHDR> mInternal;

		static void InternalColorGradientHDR(MonoObject* managedInstance);
		static void InternalColorGradientHDR0(MonoObject* managedInstance, Color* color);
		static void InternalColorGradientHDR1(MonoObject* managedInstance, MonoArray* keys);
		static void InternalSetKeys(ScriptColorGradientHDR* self, MonoArray* keys, float duration);
		static MonoArray* InternalGetKeys(ScriptColorGradientHDR* self);
		static uint32_t InternalGetNumKeys(ScriptColorGradientHDR* self);
		static void InternalGetKey(ScriptColorGradientHDR* self, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradientHDR* self, Color* color);
		static void InternalEvaluate(ScriptColorGradientHDR* self, float t, Color* __output);
	};
}
