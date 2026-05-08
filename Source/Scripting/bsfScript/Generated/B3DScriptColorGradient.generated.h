//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Utility/Image/B3DColorGradient.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Utility/Image/B3DColor.h"
#include "../../../Engine/Utility/Image/B3DColorGradient.h"

namespace b3d { class ColorGradientEx; }
namespace b3d { struct __ColorGradientKeyInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradient : public TScriptNonReflectableWrapper<ColorGradient, ScriptColorGradient>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorGradient")

		ScriptColorGradient(const TShared<ColorGradient>& nativeObject);
		~ScriptColorGradient();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalColorGradient(MonoObject* scriptObject);
		static void InternalColorGradient0(MonoObject* scriptObject, Color* color);
		static void InternalColorGradient1(MonoObject* scriptObject, MonoArray* keys);
		static void InternalSetKeys(ScriptColorGradient* self, MonoArray* keys, float duration);
		static MonoArray* InternalGetKeys(ScriptColorGradient* self);
		static uint32_t InternalGetNumKeys(ScriptColorGradient* self);
		static void InternalGetKey(ScriptColorGradient* self, uint32_t index, __ColorGradientKeyInterop* __output);
		static void InternalSetConstant(ScriptColorGradient* self, Color* color);
		static void InternalEvaluate(ScriptColorGradient* self, float t, Color* __output);
	};
}
