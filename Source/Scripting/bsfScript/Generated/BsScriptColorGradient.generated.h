//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"

namespace bs { struct __ColorGradientKeyInterop; }
namespace bs { class ColorGradient; }
namespace bs { class ColorGradientEx; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptColorGradient : public ScriptObject<ScriptColorGradient>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorGradient")

		ScriptColorGradient(MonoObject* managedInstance, const SPtr<ColorGradient>& value);

		SPtr<ColorGradient> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ColorGradient>& value);

	private:
		SPtr<ColorGradient> mInternal;

		static void Internal_ColorGradient(MonoObject* managedInstance);
		static void Internal_ColorGradient0(MonoObject* managedInstance, Color* color);
		static void Internal_ColorGradient1(MonoObject* managedInstance, MonoArray* keys);
		static void Internal_setKeys(ScriptColorGradient* thisPtr, MonoArray* keys, float duration);
		static MonoArray* Internal_getKeys(ScriptColorGradient* thisPtr);
		static uint32_t Internal_getNumKeys(ScriptColorGradient* thisPtr);
		static void Internal_getKey(ScriptColorGradient* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void Internal_setConstant(ScriptColorGradient* thisPtr, Color* color);
		static void Internal_evaluate(ScriptColorGradient* thisPtr, float t, Color* __output);
	};
}
