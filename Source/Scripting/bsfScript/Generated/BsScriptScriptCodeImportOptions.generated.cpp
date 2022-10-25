//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScriptCodeImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptScriptCodeImportOptions.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptScriptCodeImportOptions::ScriptScriptCodeImportOptions(MonoObject* managedInstance, const SPtr<ScriptCodeImportOptions>& value)
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptScriptCodeImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetEditorScript", (void*)&ScriptScriptCodeImportOptions::InternalGetEditorScript);
		metaData.ScriptClass->AddInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCodeImportOptions::InternalSetEditorScript);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptScriptCodeImportOptions::InternalCreate);
	}

	MonoObject* ScriptScriptCodeImportOptions::Create(const SPtr<ScriptCodeImportOptions>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptScriptCodeImportOptions>()) ScriptScriptCodeImportOptions(managedInstance, value);
		return managedInstance;
	}

	void ScriptScriptCodeImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<ScriptCodeImportOptions> instance = ScriptCodeImportOptions::Create();
		new(bs_alloc<ScriptScriptCodeImportOptions>()) ScriptScriptCodeImportOptions(managedInstance, instance);
	}

	bool ScriptScriptCodeImportOptions::InternalGetEditorScript(ScriptScriptCodeImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EditorScript;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScriptCodeImportOptions::InternalSetEditorScript(ScriptScriptCodeImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EditorScript = value;
	}
#endif
} // namespace bs
