//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptStringTable.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "BsStringTableManager.h"

namespace bs
{
	ScriptStringTable::ScriptStringTable(MonoObject* instance, const HStringTable& table)
		:TScriptResource(instance, table)
	{ }

	void ScriptStringTable::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptStringTable::internal_CreateInstance);

		metaData.scriptClass->addInternalCall("Internal_GetNumStrings", &ScriptStringTable::internal_GetNumStrings);
		metaData.scriptClass->addInternalCall("Internal_GetIdentifiers", &ScriptStringTable::internal_GetIdentifiers);
		metaData.scriptClass->addInternalCall("Internal_Contains", &ScriptStringTable::internal_Contains);

		metaData.scriptClass->addInternalCall("Internal_SetString", &ScriptStringTable::internal_SetString);
		metaData.scriptClass->addInternalCall("Internal_SetStringDefault", &ScriptStringTable::internal_SetStringDefault);
		metaData.scriptClass->addInternalCall("Internal_RemoveString", &ScriptStringTable::internal_RemoveString);
		metaData.scriptClass->addInternalCall("Internal_GetString", &ScriptStringTable::internal_GetString);
		metaData.scriptClass->addInternalCall("Internal_GetStringDefault", &ScriptStringTable::internal_GetStringDefault);
	}

	void ScriptStringTable::internal_CreateInstance(MonoObject* instance)
	{
		HStringTable stringTable = StringTable::create();
		ScriptResourceBase* scriptInstance = ScriptResourceManager::instance().createBuiltinScriptResource(stringTable, instance);
	}

	bool ScriptStringTable::internal_Contains(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);

		auto& identifiers = thisPtr->getHandle()->getIdentifiers();
		return identifiers.find(nativeIdentifier) != identifiers.end();
	}

	UINT32 ScriptStringTable::internal_GetNumStrings(ScriptStringTable* thisPtr)
	{
		return (UINT32)thisPtr->getHandle()->getIdentifiers().size();
	}

	MonoArray* ScriptStringTable::internal_GetIdentifiers(ScriptStringTable* thisPtr)
	{
		const UnorderedSet<WString>& identifiers = thisPtr->getHandle()->getIdentifiers();
		UINT32 numIdentifiers = (UINT32)identifiers.size();

		ScriptArray outArray = ScriptArray::create<WString>(numIdentifiers);
		UINT32 idx = 0;
		for (auto identifier : identifiers)
		{
			outArray.set(idx, identifier);

			idx++;
		}

		return outArray.getInternal();
	}

	void ScriptStringTable::internal_SetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString* value)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);
		WString nativeValue = MonoUtil::monoToWString(value);

		thisPtr->getHandle()->setString(nativeIdentifier, language, nativeValue);
	}

	void ScriptStringTable::internal_SetStringDefault(ScriptStringTable* thisPtr, MonoString* identifier, MonoString* value)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);
		WString nativeValue = MonoUtil::monoToWString(value);

		thisPtr->getHandle()->setString(nativeIdentifier, StringTableManager::instance().getActiveLanguage(), nativeValue);
	}

	void ScriptStringTable::internal_RemoveString(ScriptStringTable* thisPtr, MonoString* identifier)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);
		thisPtr->getHandle()->removeString(nativeIdentifier);
	}

	void ScriptStringTable::internal_GetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString** value)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);
		WString nativeValue = thisPtr->getHandle()->getString(nativeIdentifier, language);

		*value = MonoUtil::wstringToMono(nativeValue);
	}

	void ScriptStringTable::internal_GetStringDefault(ScriptStringTable* thisPtr, MonoString* identifier, MonoString** value)
	{
		WString nativeIdentifier = MonoUtil::monoToWString(identifier);
		WString nativeValue = thisPtr->getHandle()->getString(nativeIdentifier, StringTableManager::instance().getActiveLanguage());

		*value = MonoUtil::wstringToMono(nativeValue);
	}

	MonoObject* ScriptStringTable::createInstance()
	{
		bool dummy = false;

		void* params[1];
		params[0] = &dummy;

		return metaData.scriptClass->createInstance("bool", params);
	}

	MonoObject* ScriptStringTable::_createManagedInstance(bool construct)
	{
		if (construct)
			return createInstance();

		return metaData.scriptClass->createInstance(false);
	}
}