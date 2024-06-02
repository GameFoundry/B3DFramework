//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs
{
	ScriptStringTable::ScriptStringTable(MonoObject* managedInstance, const TResourceHandle<StringTable>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptStringTable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptStringTable::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_Contains", (void*)&ScriptStringTable::InternalContains);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumStrings", (void*)&ScriptStringTable::InternalGetNumStrings);
		metaData.ScriptClass->AddInternalCall("Internal_GetIdentifiers", (void*)&ScriptStringTable::InternalGetIdentifiers);
		metaData.ScriptClass->AddInternalCall("Internal_SetString", (void*)&ScriptStringTable::InternalSetString);
		metaData.ScriptClass->AddInternalCall("Internal_GetString", (void*)&ScriptStringTable::InternalGetString);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveString", (void*)&ScriptStringTable::InternalRemoveString);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptStringTable::InternalCreate);

	}

	 MonoObject*ScriptStringTable::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptStringTable::InternalGetRef(ScriptStringTable* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	bool ScriptStringTable::InternalContains(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		bool tmp__output;
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		tmp__output = thisPtr->GetHandle()->Contains(tmpidentifier);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptStringTable::InternalGetNumStrings(ScriptStringTable* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNumStrings();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptStringTable::InternalGetIdentifiers(ScriptStringTable* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->GetHandle()->GetIdentifiers();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptStringTable::InternalSetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString* value)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		thisPtr->GetHandle()->SetString(tmpidentifier, language, tmpvalue);
	}

	MonoString* ScriptStringTable::InternalGetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language)
	{
		String tmp__output;
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		tmp__output = thisPtr->GetHandle()->GetString(tmpidentifier, language);

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptStringTable::InternalRemoveString(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		thisPtr->GetHandle()->RemoveString(tmpidentifier);
	}

	void ScriptStringTable::InternalCreate(MonoObject* managedInstance)
	{
		TResourceHandle<StringTable> instance = StringTable::Create();
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
