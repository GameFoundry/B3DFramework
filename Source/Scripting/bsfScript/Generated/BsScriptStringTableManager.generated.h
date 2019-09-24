//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Localization/BsStringTable.h"

namespace bs { class StringTableManager; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptStringTableManager : public ScriptObject<ScriptStringTableManager>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "StringTables")

		ScriptStringTableManager(MonoObject* managedInstance);

	private:
		static void Internal_setActiveLanguage(Language language);
		static Language Internal_getActiveLanguage();
		static MonoObject* Internal_getTable(uint32_t id);
		static void Internal_removeTable(uint32_t id);
		static void Internal_setTable(uint32_t id, MonoObject* table);
	};
}
