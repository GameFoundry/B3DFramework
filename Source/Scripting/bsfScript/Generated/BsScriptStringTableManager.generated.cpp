//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		metaData.ScriptClass->AddInternalCall("Internal_SetActiveLanguage", (void*)&ScriptStringTableManager::InternalSetActiveLanguage);
		metaData.ScriptClass->AddInternalCall("Internal_GetActiveLanguage", (void*)&ScriptStringTableManager::InternalGetActiveLanguage);
		metaData.ScriptClass->AddInternalCall("Internal_GetTable", (void*)&ScriptStringTableManager::InternalGetTable);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveTable", (void*)&ScriptStringTableManager::InternalRemoveTable);
		metaData.ScriptClass->AddInternalCall("Internal_SetTable", (void*)&ScriptStringTableManager::InternalSetTable);

	}

	void ScriptStringTableManager::InternalSetActiveLanguage(Language language)
	{
		StringTableManager::Instance().SetActiveLanguage(language);
	}

	Language ScriptStringTableManager::InternalGetActiveLanguage()
	{
		Language tmp__output;
		tmp__output = StringTableManager::Instance().GetActiveLanguage();

		Language __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptStringTableManager::InternalGetTable(uint32_t id)
	{
		ResourceHandle<StringTable> tmp__output;
		tmp__output = StringTableManager::Instance().GetTable(id);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptStringTableManager::InternalRemoveTable(uint32_t id)
	{
		StringTableManager::Instance().RemoveTable(id);
	}

	void ScriptStringTableManager::InternalSetTable(uint32_t id, MonoObject* table)
	{
		ResourceHandle<StringTable> tmptable;
		ScriptRRefBase* scripttable;
		scripttable = ScriptRRefBase::ToNative(table);
		if(scripttable != nullptr)
			tmptable = B3DStaticResourceCast<StringTable>(scripttable->GetHandle());
		StringTableManager::Instance().SetTable(id, tmptable);
	}
}
