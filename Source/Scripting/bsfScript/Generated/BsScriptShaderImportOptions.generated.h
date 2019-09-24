//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"

namespace bs { class ShaderImportOptions; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptShaderImportOptions : public TScriptReflectable<ScriptShaderImportOptions, ShaderImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ShaderImportOptions")

		ScriptShaderImportOptions(MonoObject* managedInstance, const SPtr<ShaderImportOptions>& value);

		static MonoObject* create(const SPtr<ShaderImportOptions>& value);

	private:
		static void Internal_setDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString* value);
		static bool Internal_getDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString** value);
		static bool Internal_hasDefine(ScriptShaderImportOptions* thisPtr, MonoString* define);
		static void Internal_removeDefine(ScriptShaderImportOptions* thisPtr, MonoString* define);
		static ShadingLanguageFlag Internal_getlanguages(ScriptShaderImportOptions* thisPtr);
		static void Internal_setlanguages(ScriptShaderImportOptions* thisPtr, ShadingLanguageFlag value);
		static void Internal_create(MonoObject* managedInstance);
	};
#endif
}
