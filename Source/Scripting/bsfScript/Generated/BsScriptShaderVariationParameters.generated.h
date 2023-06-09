//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"

namespace bs { class ShaderVariationParameters; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameters : public TScriptReflectable<ScriptShaderVariationParameters, ShaderVariationParameters>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderVariationParameters")

		ScriptShaderVariationParameters(MonoObject* managedInstance, const SPtr<ShaderVariationParameters>& value);

		static MonoObject* Create(const SPtr<ShaderVariationParameters>& value);

	private:
		static void InternalShaderVariationParameters(MonoObject* managedInstance);
		static int32_t InternalGetInt(ScriptShaderVariationParameters* thisPtr, MonoString* name);
		static uint32_t InternalGetUInt(ScriptShaderVariationParameters* thisPtr, MonoString* name);
		static float InternalGetFloat(ScriptShaderVariationParameters* thisPtr, MonoString* name);
		static bool InternalGetBool(ScriptShaderVariationParameters* thisPtr, MonoString* name);
		static void InternalSetInt(ScriptShaderVariationParameters* thisPtr, MonoString* name, int32_t value);
		static void InternalSetUInt(ScriptShaderVariationParameters* thisPtr, MonoString* name, uint32_t value);
		static void InternalSetFloat(ScriptShaderVariationParameters* thisPtr, MonoString* name, float value);
		static void InternalSetBool(ScriptShaderVariationParameters* thisPtr, MonoString* name, bool value);
		static void InternalRemoveParam(ScriptShaderVariationParameters* thisPtr, MonoString* paramName);
		static bool InternalHasParam(ScriptShaderVariationParameters* thisPtr, MonoString* paramName);
		static void InternalClearParams(ScriptShaderVariationParameters* thisPtr);
		static MonoArray* InternalGetParamNames(ScriptShaderVariationParameters* thisPtr);
	};
}
