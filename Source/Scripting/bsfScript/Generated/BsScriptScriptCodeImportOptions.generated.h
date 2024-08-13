//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"

namespace bs { class ScriptCodeImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptCodeImportOptions : public TScriptReflectable<ScriptScriptCodeImportOptions, ScriptCodeImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ScriptCodeImportOptions")

		ScriptScriptCodeImportOptions(MonoObject* managedInstance, const SPtr<ScriptCodeImportOptions>& value);

		static MonoObject* Create(const SPtr<ScriptCodeImportOptions>& value);

	private:
		static bool InternalGetEditorScript(ScriptScriptCodeImportOptions* self);
		static void InternalSetEditorScript(ScriptScriptCodeImportOptions* self, bool value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
