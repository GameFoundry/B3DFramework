//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs
{
	class StringTable;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptStringTable : public TScriptResource<ScriptStringTable, StringTable>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "StringTable")

		ScriptStringTable(MonoObject* managedInstance, const ResourceHandle<StringTable>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptStringTable* thisPtr);

		static bool InternalContains(ScriptStringTable* thisPtr, MonoString* identifier);
		static uint32_t InternalGetNumStrings(ScriptStringTable* thisPtr);
		static MonoArray* InternalGetIdentifiers(ScriptStringTable* thisPtr);
		static void InternalSetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString* value);
		static MonoString* InternalGetString(ScriptStringTable* thisPtr, MonoString* identifier, Language language);
		static void InternalRemoveString(ScriptStringTable* thisPtr, MonoString* identifier);
		static void InternalCreate(MonoObject* managedInstance);
	};
} // namespace bs
