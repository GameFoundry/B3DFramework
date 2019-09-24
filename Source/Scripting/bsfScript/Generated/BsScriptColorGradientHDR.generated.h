//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"

namespace bs { class ColorGradientHDR; }
namespace bs { class ColorGradientHDREx; }
namespace bs { struct __ColorGradientKeyInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptColorGradientHDR : public ScriptObject<ScriptColorGradientHDR>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorGradientHDR")

		ScriptColorGradientHDR(MonoObject* managedInstance, const SPtr<ColorGradientHDR>& value);

		SPtr<ColorGradientHDR> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ColorGradientHDR>& value);

	private:
		SPtr<ColorGradientHDR> mInternal;

		static void Internal_ColorGradientHDR(MonoObject* managedInstance);
		static void Internal_ColorGradientHDR0(MonoObject* managedInstance, Color* color);
		static void Internal_ColorGradientHDR1(MonoObject* managedInstance, MonoArray* keys);
		static void Internal_setKeys(ScriptColorGradientHDR* thisPtr, MonoArray* keys, float duration);
		static MonoArray* Internal_getKeys(ScriptColorGradientHDR* thisPtr);
		static uint32_t Internal_getNumKeys(ScriptColorGradientHDR* thisPtr);
		static void Internal_getKey(ScriptColorGradientHDR* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void Internal_setConstant(ScriptColorGradientHDR* thisPtr, Color* color);
		static void Internal_evaluate(ScriptColorGradientHDR* thisPtr, float t, Color* __output);
	};
}
