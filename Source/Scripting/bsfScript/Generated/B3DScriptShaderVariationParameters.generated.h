//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Material/B3DShaderVariation.h"

namespace b3d { class ShaderVariationParameters; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameters : public TScriptReflectableWrapper<ShaderVariationParameters, ScriptShaderVariationParameters>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderVariationParameters")

		ScriptShaderVariationParameters(const TShared<ShaderVariationParameters>& nativeObject);
		~ScriptShaderVariationParameters();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalShaderVariationParameters(MonoObject* scriptObject);
		static int32_t InternalGetI32(ScriptShaderVariationParameters* self, MonoString* name);
		static uint32_t InternalGetUI32(ScriptShaderVariationParameters* self, MonoString* name);
		static float InternalGetFloat(ScriptShaderVariationParameters* self, MonoString* name);
		static bool InternalGetBool(ScriptShaderVariationParameters* self, MonoString* name);
		static void InternalSetI32(ScriptShaderVariationParameters* self, MonoString* name, int32_t value);
		static void InternalSetU32(ScriptShaderVariationParameters* self, MonoString* name, uint32_t value);
		static void InternalSetFloat(ScriptShaderVariationParameters* self, MonoString* name, float value);
		static void InternalSetBool(ScriptShaderVariationParameters* self, MonoString* name, bool value);
		static void InternalRemoveParameter(ScriptShaderVariationParameters* self, MonoString* parameter);
		static bool InternalHasParameter(ScriptShaderVariationParameters* self, MonoString* paramName);
		static void InternalClearParameters(ScriptShaderVariationParameters* self);
		static MonoArray* InternalGetParameters(ScriptShaderVariationParameters* self);
	};
}
