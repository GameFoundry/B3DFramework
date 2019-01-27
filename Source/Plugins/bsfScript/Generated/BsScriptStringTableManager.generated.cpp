//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTableManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Localization/BsStringTableManager.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs
{
	ScriptStringTableManager::ScriptStringTableManager(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptStringTableManager::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setActiveLanguage", (void*)&ScriptStringTableManager::Internal_setActiveLanguage);
		metaData.scriptClass->addInternalCall("Internal_getActiveLanguage", (void*)&ScriptStringTableManager::Internal_getActiveLanguage);
		metaData.scriptClass->addInternalCall("Internal_getTable", (void*)&ScriptStringTableManager::Internal_getTable);
		metaData.scriptClass->addInternalCall("Internal_removeTable", (void*)&ScriptStringTableManager::Internal_removeTable);
		metaData.scriptClass->addInternalCall("Internal_setTable", (void*)&ScriptStringTableManager::Internal_setTable);

	}

	void ScriptStringTableManager::Internal_setActiveLanguage(Language language)
	{
		StringTableManager::instance().setActiveLanguage(language);
	}

	Language ScriptStringTableManager::Internal_getActiveLanguage()
	{
		Language tmp__output;
		tmp__output = StringTableManager::instance().getActiveLanguage();

		Language __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptStringTableManager::Internal_getTable(uint32_t id)
	{
		ResourceHandle<StringTable> tmp__output;
		tmp__output = StringTableManager::instance().getTable(id);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptStringTableManager::Internal_removeTable(uint32_t id)
	{
		StringTableManager::instance().removeTable(id);
	}

	void ScriptStringTableManager::Internal_setTable(uint32_t id, MonoObject* table)
	{
		ResourceHandle<StringTable> tmptable;
		ScriptRRefBase* scripttable;
		scripttable = ScriptRRefBase::toNative(table);
		if(scripttable != nullptr)
			tmptable = static_resource_cast<StringTable>(scripttable->getHandle());
		StringTableManager::instance().setTable(id, tmptable);
	}
}
