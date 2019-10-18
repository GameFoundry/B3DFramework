//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"

namespace bs { class ShaderVariation; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptShaderVariation : public TScriptReflectable<ScriptShaderVariation, ShaderVariation>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ShaderVariation")

		ScriptShaderVariation(MonoObject* managedInstance, const SPtr<ShaderVariation>& value);

		static MonoObject* create(const SPtr<ShaderVariation>& value);

	private:
		static void Internal_ShaderVariation(MonoObject* managedInstance);
		static int32_t Internal_getInt(ScriptShaderVariation* thisPtr, MonoString* name);
		static uint32_t Internal_getUInt(ScriptShaderVariation* thisPtr, MonoString* name);
		static float Internal_getFloat(ScriptShaderVariation* thisPtr, MonoString* name);
		static bool Internal_getBool(ScriptShaderVariation* thisPtr, MonoString* name);
		static void Internal_setInt(ScriptShaderVariation* thisPtr, MonoString* name, int32_t value);
		static void Internal_setUInt(ScriptShaderVariation* thisPtr, MonoString* name, uint32_t value);
		static void Internal_setFloat(ScriptShaderVariation* thisPtr, MonoString* name, float value);
		static void Internal_setBool(ScriptShaderVariation* thisPtr, MonoString* name, bool value);
		static void Internal_removeParam(ScriptShaderVariation* thisPtr, MonoString* paramName);
		static bool Internal_hasParam(ScriptShaderVariation* thisPtr, MonoString* paramName);
		static void Internal_clearParams(ScriptShaderVariation* thisPtr);
		static MonoArray* Internal_getParamNames(ScriptShaderVariation* thisPtr);
	};
}
