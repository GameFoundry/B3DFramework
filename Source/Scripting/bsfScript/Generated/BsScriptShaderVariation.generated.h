//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"

namespace bs { class ShaderVariation; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariation : public TScriptReflectable<ScriptShaderVariation, ShaderVariation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderVariation")

		ScriptShaderVariation(MonoObject* managedInstance, const SPtr<ShaderVariation>& value);

		static MonoObject* Create(const SPtr<ShaderVariation>& value);

	private:
		static void InternalShaderVariation(MonoObject* managedInstance);
		static int32_t InternalGetInt(ScriptShaderVariation* thisPtr, MonoString* name);
		static uint32_t InternalGetUInt(ScriptShaderVariation* thisPtr, MonoString* name);
		static float InternalGetFloat(ScriptShaderVariation* thisPtr, MonoString* name);
		static bool InternalGetBool(ScriptShaderVariation* thisPtr, MonoString* name);
		static void InternalSetInt(ScriptShaderVariation* thisPtr, MonoString* name, int32_t value);
		static void InternalSetUInt(ScriptShaderVariation* thisPtr, MonoString* name, uint32_t value);
		static void InternalSetFloat(ScriptShaderVariation* thisPtr, MonoString* name, float value);
		static void InternalSetBool(ScriptShaderVariation* thisPtr, MonoString* name, bool value);
		static void InternalRemoveParam(ScriptShaderVariation* thisPtr, MonoString* paramName);
		static bool InternalHasParam(ScriptShaderVariation* thisPtr, MonoString* paramName);
		static void InternalClearParams(ScriptShaderVariation* thisPtr);
		static MonoArray* InternalGetParamNames(ScriptShaderVariation* thisPtr);
	};
}
