//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTableManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTableManager.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs
{
	ScriptStringTableManager::ScriptStringTableManager(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptStringTableManager::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setActiveLanguage", (void*)&ScriptStringTableManager::Internal_setActiveLanguage);
		metaData.scriptClass->AddInternalCall("Internal_getActiveLanguage", (void*)&ScriptStringTableManager::Internal_getActiveLanguage);
		metaData.scriptClass->AddInternalCall("Internal_getTable", (void*)&ScriptStringTableManager::Internal_getTable);
		metaData.scriptClass->AddInternalCall("Internal_removeTable", (void*)&ScriptStringTableManager::Internal_removeTable);
		metaData.scriptClass->AddInternalCall("Internal_setTable", (void*)&ScriptStringTableManager::Internal_setTable);

	}

	void ScriptStringTableManager::Internal_setActiveLanguage(Language language)
	{
		StringTableManager::instance().SetActiveLanguage(language);
	}

	Language ScriptStringTableManager::Internal_getActiveLanguage()
	{
		Language tmp__output;
		tmp__output = StringTableManager::instance().GetActiveLanguage();

		Language __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptStringTableManager::Internal_getTable(uint32_t id)
	{
		ResourceHandle<StringTable> tmp__output;
		tmp__output = StringTableManager::instance().GetTable(id);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptStringTableManager::Internal_removeTable(uint32_t id)
	{
		StringTableManager::instance().RemoveTable(id);
	}

	void ScriptStringTableManager::Internal_setTable(uint32_t id, MonoObject* table)
	{
		ResourceHandle<StringTable> tmptable;
		ScriptRRefBase* scripttable;
		scripttable = ScriptRRefBase::toNative(table);
		if(scripttable != nullptr)
			tmptable = static_resource_cast<StringTable>(scripttable->GetHandle());
		StringTableManager::instance().SetTable(id, tmptable);
	}
}
