//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStringTable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Localization/BsStringTable.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs
{
	ScriptStringTable::ScriptStringTable(MonoObject* managedInstance, const ResourceHandle<StringTable>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptStringTable::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptStringTable::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_contains", (void*)&ScriptStringTable::Internal_contains);
		metaData.scriptClass->addInternalCall("Internal_getNumStrings", (void*)&ScriptStringTable::Internal_getNumStrings);
		metaData.scriptClass->addInternalCall("Internal_getIdentifiers", (void*)&ScriptStringTable::Internal_getIdentifiers);
		metaData.scriptClass->addInternalCall("Internal_setString", (void*)&ScriptStringTable::Internal_setString);
		metaData.scriptClass->addInternalCall("Internal_getString", (void*)&ScriptStringTable::Internal_getString);
		metaData.scriptClass->addInternalCall("Internal_removeString", (void*)&ScriptStringTable::Internal_removeString);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptStringTable::Internal_create);

	}

	 MonoObject*ScriptStringTable::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptStringTable::Internal_getRef(ScriptStringTable* thisPtr)
	{
		return thisPtr->getRRef();
	}

	bool ScriptStringTable::Internal_contains(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		bool tmp__output;
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		tmp__output = thisPtr->getHandle()->contains(tmpidentifier);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptStringTable::Internal_getNumStrings(ScriptStringTable* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getNumStrings();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptStringTable::Internal_getIdentifiers(ScriptStringTable* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->getHandle()->getIdentifiers();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptStringTable::Internal_setString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString* value)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		String tmpvalue;
		tmpvalue = MonoUtil::monoToString(value);
		thisPtr->getHandle()->setString(tmpidentifier, language, tmpvalue);
	}

	MonoString* ScriptStringTable::Internal_getString(ScriptStringTable* thisPtr, MonoString* identifier, Language language)
	{
		String tmp__output;
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		tmp__output = thisPtr->getHandle()->getString(tmpidentifier, language);

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	void ScriptStringTable::Internal_removeString(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		thisPtr->getHandle()->removeString(tmpidentifier);
	}

	void ScriptStringTable::Internal_create(MonoObject* managedInstance)
	{
		ResourceHandle<StringTable> instance = StringTable::create();
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}
}
