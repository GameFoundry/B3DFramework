//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"

namespace b3d { class ShaderVariationParameters; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameters : public TScriptReflectableWrapper<ShaderVariationParameters, ScriptShaderVariationParameters>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderVariationParameters")

		ScriptShaderVariationParameters(const SPtr<ShaderVariationParameters>& nativeObject);
		~ScriptShaderVariationParameters();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalShaderVariationParameters(MonoObject* scriptObject);
		static int32_t InternalGetInt(ScriptShaderVariationParameters* self, MonoString* name);
		static uint32_t InternalGetUInt(ScriptShaderVariationParameters* self, MonoString* name);
		static float InternalGetFloat(ScriptShaderVariationParameters* self, MonoString* name);
		static bool InternalGetBool(ScriptShaderVariationParameters* self, MonoString* name);
		static void InternalSetInt(ScriptShaderVariationParameters* self, MonoString* name, int32_t value);
		static void InternalSetUInt(ScriptShaderVariationParameters* self, MonoString* name, uint32_t value);
		static void InternalSetFloat(ScriptShaderVariationParameters* self, MonoString* name, float value);
		static void InternalSetBool(ScriptShaderVariationParameters* self, MonoString* name, bool value);
		static void InternalRemoveParam(ScriptShaderVariationParameters* self, MonoString* paramName);
		static bool InternalHasParam(ScriptShaderVariationParameters* self, MonoString* paramName);
		static void InternalClearParams(ScriptShaderVariationParameters* self);
		static MonoArray* InternalGetParamNames(ScriptShaderVariationParameters* self);
	};
}
