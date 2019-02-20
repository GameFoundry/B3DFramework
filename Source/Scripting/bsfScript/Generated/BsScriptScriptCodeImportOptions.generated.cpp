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
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptScriptCodeImportOptions::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_geteditorScript", (void*)&ScriptScriptCodeImportOptions::Internal_geteditorScript);
		metaData.scriptClass->addInternalCall("Internal_seteditorScript", (void*)&ScriptScriptCodeImportOptions::Internal_seteditorScript);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptScriptCodeImportOptions::Internal_create);

	}

	MonoObject* ScriptScriptCodeImportOptions::create(const SPtr<ScriptCodeImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptScriptCodeImportOptions>()) ScriptScriptCodeImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptScriptCodeImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<ScriptCodeImportOptions> instance = ScriptCodeImportOptions::create();
		new (bs_alloc<ScriptScriptCodeImportOptions>())ScriptScriptCodeImportOptions(managedInstance, instance);
	}
	bool ScriptScriptCodeImportOptions::Internal_geteditorScript(ScriptScriptCodeImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->editorScript;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScriptCodeImportOptions::Internal_seteditorScript(ScriptScriptCodeImportOptions* thisPtr, bool value)
	{
		thisPtr->getInternal()->editorScript = value;
	}
#endif
}
