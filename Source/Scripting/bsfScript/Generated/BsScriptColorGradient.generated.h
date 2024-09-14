//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"

namespace bs { class ColorGradientEx; }
namespace bs { struct __ColorGradientKeyInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradient : public ScriptObject<ScriptColorGradient>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ColorGradient")

		ScriptColorGradient(MonoObject* managedInstance, const SPtr<ColorGradient>& value);

		static void SetupScriptBindings();

		SPtr<ColorGradient> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<ColorGradient>& value);

	private:
		SPtr<ColorGradient> mInternal;

		static void InternalColorGradient(MonoObject* managedInstance);
		static void InternalColorGradient0(MonoObject* managedInstance, Color* color);
		static void InternalColorGradient1(MonoObject* managedInstance, MonoArray* keys);
		static void InternalSetKeys(ScriptColorGradient* self, MonoArray* keys, float duration);
		static MonoArray* InternalGetKeys(ScriptColorGradient* self);
		static uint32_t InternalGetNumKeys(ScriptColorGradient* self);
		static void InternalGetKey(ScriptColorGradient* self, uint32_t idx, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradient* self, Color* color);
		static void InternalEvaluate(ScriptColorGradient* self, float t, Color* __output);
	};
}
