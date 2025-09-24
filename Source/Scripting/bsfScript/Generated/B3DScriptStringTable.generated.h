//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace b3d { class StringTable; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStringTable : public TScriptResourceWrapper<StringTable, ScriptStringTable>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "StringTable")

		ScriptStringTable(const TResourceHandle<StringTable>& nativeObject);
		~ScriptStringTable();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptStringTable* self);

		static bool InternalContains(ScriptStringTable* self, MonoString* identifier);
		static uint32_t InternalGetNumStrings(ScriptStringTable* self);
		static MonoArray* InternalGetIdentifiers(ScriptStringTable* self);
		static void InternalSetString(ScriptStringTable* self, MonoString* identifier, Language language, MonoString* value);
		static MonoString* InternalGetString(ScriptStringTable* self, MonoString* identifier, Language language);
		static void InternalRemoveString(ScriptStringTable* self, MonoString* identifier);
		static void InternalCreate(MonoObject* scriptObject);
	};
}
