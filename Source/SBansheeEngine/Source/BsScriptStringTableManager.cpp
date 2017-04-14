//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptStringTableManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "BsScriptStringTable.h"

namespace bs
{
	ScriptStringTableManager::ScriptStringTableManager(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptStringTableManager::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetActiveLanguage", &ScriptStringTableManager::internal_GetActiveLanguage);
		metaData.scriptClass->addInternalCall("Internal_SetActiveLanguage", &ScriptStringTableManager::internal_SetActiveLanguage);

		metaData.scriptClass->addInternalCall("Internal_GetTable", &ScriptStringTableManager::internal_GetTable);
		metaData.scriptClass->addInternalCall("Internal_SetTable", &ScriptStringTableManager::internal_SetTable);
		metaData.scriptClass->addInternalCall("Internal_RemoveTable", &ScriptStringTableManager::internal_RemoveTable);
	}

	void ScriptStringTableManager::internal_GetActiveLanguage(Language* value)
	{
		*value = StringTableManager::instance().getActiveLanguage();
	}

	void ScriptStringTableManager::internal_SetActiveLanguage(Language value)
	{
		StringTableManager::instance().setActiveLanguage(value);
	}

	MonoObject* ScriptStringTableManager::internal_GetTable(UINT32 id)
	{
		HStringTable table = StringTableManager::instance().getTable(id);

		ScriptResourceBase* scriptStringTable = ScriptResourceManager::instance().getScriptResource(table, true);
		return scriptStringTable->getManagedInstance();
	}

	void ScriptStringTableManager::internal_SetTable(UINT32 id, MonoObject* table)
	{
		HStringTable nativeTable;
		if (table != nullptr)
			nativeTable = ScriptStringTable::toNative(table)->getHandle();

		StringTableManager::instance().setTable(id, nativeTable);
	}

	void ScriptStringTableManager::internal_RemoveTable(UINT32 id)
	{
		StringTableManager::instance().removeTable(id);
	}
}