//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptImportOptions.generated.h"
#include "../../../Engine/Core/Importer/B3DShaderImportOptions.h"
#include "../../../Engine/Core/Material/B3DShaderCompiler.h"

namespace b3d { class ShaderImportOptions; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderImportOptions : public TScriptReflectableWrapper<ShaderImportOptions, ScriptShaderImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderImportOptions")

		ScriptShaderImportOptions(const TShared<ShaderImportOptions>& nativeObject);
		~ScriptShaderImportOptions();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetDefine(ScriptShaderImportOptions* self, MonoString* define, MonoString* value);
		static bool InternalGetDefine(ScriptShaderImportOptions* self, MonoString* define, MonoString** outValue);
		static bool InternalHasDefine(ScriptShaderImportOptions* self, MonoString* define);
		static void InternalRemoveDefine(ScriptShaderImportOptions* self, MonoString* define);
		static ShadingLanguageFlag InternalGetLanguages(ScriptShaderImportOptions* self);
		static void InternalSetLanguages(ScriptShaderImportOptions* self, ShadingLanguageFlag value);
		static void InternalCreate(MonoObject* scriptObject);
	};
#endif
}
