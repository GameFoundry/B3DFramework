//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs { class StringTable; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptStringTable : public TScriptResource<ScriptStringTable, StringTable>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "StringTable")

		ScriptStringTable(MonoObject* managedInstance, const ResourceHandle<StringTable>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptStringTable* thisPtr);

		static bool Internal_contains(ScriptStringTable* thisPtr, MonoString* identifier);
		static uint32_t Internal_getNumStrings(ScriptStringTable* thisPtr);
		static MonoArray* Internal_getIdentifiers(ScriptStringTable* thisPtr);
		static void Internal_setString(ScriptStringTable* thisPtr, MonoString* identifier, Language language, MonoString* value);
		static MonoString* Internal_getString(ScriptStringTable* thisPtr, MonoString* identifier, Language language);
		static void Internal_removeString(ScriptStringTable* thisPtr, MonoString* identifier);
		static void Internal_create(MonoObject* managedInstance);
	};
}
