//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptImportOptions.generated.h"

namespace bs
{
	class ScriptCodeImportOptions;

#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptScriptCodeImportOptions : public ScriptObject<ScriptScriptCodeImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ScriptCodeImportOptions")

		ScriptScriptCodeImportOptions(MonoObject* managedInstance, const SPtr<ScriptCodeImportOptions>& value);

		SPtr<ScriptCodeImportOptions> getInternal() const;
		static MonoObject* create(const SPtr<ScriptCodeImportOptions>& value);

	private:
		static bool Internal_geteditorScript(ScriptScriptCodeImportOptions* thisPtr);
		static void Internal_seteditorScript(ScriptScriptCodeImportOptions* thisPtr, bool value);
		static void Internal_create(MonoObject* managedInstance);
	};
#endif
}
