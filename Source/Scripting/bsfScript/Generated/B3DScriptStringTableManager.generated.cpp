//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTableManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTableManager.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace b3d
{
	ScriptStringTables::ScriptStringTables()
		:TScriptTypeDefinition()
	{
	}

	void ScriptStringTables::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActiveLanguage", (void*)&ScriptStringTables::InternalSetActiveLanguage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetActiveLanguage", (void*)&ScriptStringTables::InternalGetActiveLanguage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTable", (void*)&ScriptStringTables::InternalGetTable);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveTable", (void*)&ScriptStringTables::InternalRemoveTable);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTable", (void*)&ScriptStringTables::InternalSetTable);

	}

	void ScriptStringTables::InternalSetActiveLanguage(Language language)
	{
		StringTableManager::Instance().SetActiveLanguage(language);
	}

	Language ScriptStringTables::InternalGetActiveLanguage()
	{
		Language tmp__output;
		tmp__output = StringTableManager::Instance().GetActiveLanguage();

		Language __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptStringTables::InternalGetTable(uint32_t id)
	{
		TResourceHandle<StringTable> tmp__output;
		tmp__output = StringTableManager::Instance().GetTable(id);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptStringTables::InternalRemoveTable(uint32_t id)
	{
		StringTableManager::Instance().RemoveTable(id);
	}

	void ScriptStringTables::InternalSetTable(uint32_t id, MonoObject* table)
	{
		TResourceHandle<StringTable> tmptable;
		ScriptRRefBase* scriptObjectWrappertable;
		scriptObjectWrappertable = ScriptRRefBase::GetScriptObjectWrapper(table);
		if(scriptObjectWrappertable != nullptr)
			tmptable = B3DStaticResourceCast<StringTable>(scriptObjectWrappertable->GetNativeObject());
		StringTableManager::Instance().SetTable(id, tmptable);
	}
}
