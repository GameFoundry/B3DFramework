//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScriptCodeImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptScriptCodeImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptScriptCodeImportOptions::ScriptScriptCodeImportOptions(MonoObject* managedInstance, const SPtr<ScriptCodeImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptScriptCodeImportOptions>()) ScriptScriptCodeImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptScriptCodeImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<ScriptCodeImportOptions> nativeObject = ScriptCodeImportOptions::Create();
		new (B3DAllocate<ScriptScriptCodeImportOptions>())ScriptScriptCodeImportOptions(managedInstance, nativeObject);
	}
	bool ScriptScriptCodeImportOptions::InternalGetEditorScript(ScriptScriptCodeImportOptions* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EditorScript;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScriptCodeImportOptions::InternalSetEditorScript(ScriptScriptCodeImportOptions* self, bool value)
	{
		self->GetInternal()->EditorScript = value;
	}
#endif
}
