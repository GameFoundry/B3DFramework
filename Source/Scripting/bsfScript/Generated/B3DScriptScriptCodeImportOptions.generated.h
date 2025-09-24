//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"

namespace b3d { class ScriptCodeImportOptions; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptCodeImportOptions : public TScriptReflectableWrapper<ScriptCodeImportOptions, ScriptScriptCodeImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ScriptCodeImportOptions")

		ScriptScriptCodeImportOptions(const SPtr<ScriptCodeImportOptions>& nativeObject);
		~ScriptScriptCodeImportOptions();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalGetEditorScript(ScriptScriptCodeImportOptions* self);
		static void InternalSetEditorScript(ScriptScriptCodeImportOptions* self, bool value);
		static void InternalCreate(MonoObject* scriptObject);
	};
#endif
}
