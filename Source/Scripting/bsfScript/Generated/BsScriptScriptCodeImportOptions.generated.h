//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"

namespace bs { class ScriptCodeImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptCodeImportOptions : public TScriptReflectableWrapper<ScriptCodeImportOptions, ScriptScriptCodeImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ScriptCodeImportOptions")

		ScriptScriptCodeImportOptions(const SPtr<ScriptCodeImportOptions>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalGetEditorScript(ScriptScriptCodeImportOptions* self);
		static void InternalSetEditorScript(ScriptScriptCodeImportOptions* self, bool value);
		static void InternalCreate(MonoObject* scriptObject);
	};
#endif
}
