//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs { class StringTableManager; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStringTableManager : public ScriptObject<ScriptStringTableManager>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "StringTables")

		ScriptStringTableManager(MonoObject* managedInstance);

	private:
		static void InternalSetActiveLanguage(Language language);
		static Language InternalGetActiveLanguage();
		static MonoObject* InternalGetTable(uint32_t id);
		static void InternalRemoveTable(uint32_t id);
		static void InternalSetTable(uint32_t id, MonoObject* table);
	};
}
