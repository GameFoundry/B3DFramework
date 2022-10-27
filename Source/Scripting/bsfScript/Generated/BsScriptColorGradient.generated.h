//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"

namespace bs
{
	struct __ColorGradientKeyInterop;
}

namespace bs
{
	class ColorGradient;
}

namespace bs
{
	class ColorGradientEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptColorGradient : public ScriptObject<ScriptColorGradient>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorGradient")

		ScriptColorGradient(MonoObject* managedInstance, const SPtr<ColorGradient>& value);

		SPtr<ColorGradient> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<ColorGradient>& value);

	private:
		SPtr<ColorGradient> mInternal;

		static void InternalColorGradient(MonoObject* managedInstance);
		static void InternalColorGradient0(MonoObject* managedInstance, Color* color);
		static void InternalColorGradient1(MonoObject* managedInstance, MonoArray* keys);
		static void InternalSetKeys(ScriptColorGradient* thisPtr, MonoArray* keys, float duration);
		static MonoArray* InternalGetKeys(ScriptColorGradient* thisPtr);
		static uint32_t InternalGetNumKeys(ScriptColorGradient* thisPtr);
		static void InternalGetKey(ScriptColorGradient* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradient* thisPtr, Color* color);
		static void InternalEvaluate(ScriptColorGradient* thisPtr, float t, Color* __output);
	};
} // namespace bs
