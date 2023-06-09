//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"
#include "../../../Foundation/bsfCore/Material/BsShaderCompiler.h"

namespace bs { class ShaderImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderImportOptions : public TScriptReflectable<ScriptShaderImportOptions, ShaderImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderImportOptions")

		ScriptShaderImportOptions(MonoObject* managedInstance, const SPtr<ShaderImportOptions>& value);

		static MonoObject* Create(const SPtr<ShaderImportOptions>& value);

	private:
		static void InternalSetDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString* value);
		static bool InternalGetDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString** value);
		static bool InternalHasDefine(ScriptShaderImportOptions* thisPtr, MonoString* define);
		static void InternalRemoveDefine(ScriptShaderImportOptions* thisPtr, MonoString* define);
		static ShadingLanguageFlag InternalGetLanguages(ScriptShaderImportOptions* thisPtr);
		static void InternalSetLanguages(ScriptShaderImportOptions* thisPtr, ShadingLanguageFlag value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
