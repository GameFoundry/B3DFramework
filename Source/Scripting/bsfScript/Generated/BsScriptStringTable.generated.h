//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs { class StringTable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStringTable : public TScriptResource<ScriptStringTable, StringTable>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "StringTable")

		ScriptStringTable(MonoObject* managedInstance, const TResourceHandle<StringTable>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptStringTable* self);

		static bool InternalContains(ScriptStringTable* self, MonoString* identifier);
		static uint32_t InternalGetNumStrings(ScriptStringTable* self);
		static MonoArray* InternalGetIdentifiers(ScriptStringTable* self);
		static void InternalSetString(ScriptStringTable* self, MonoString* identifier, Language language, MonoString* value);
		static MonoString* InternalGetString(ScriptStringTable* self, MonoString* identifier, Language language);
		static void InternalRemoveString(ScriptStringTable* self, MonoString* identifier);
		static void InternalCreate(MonoObject* managedInstance);
	};
}
