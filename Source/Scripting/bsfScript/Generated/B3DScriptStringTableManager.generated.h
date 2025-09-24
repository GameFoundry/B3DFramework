//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Localization/B3DStringTableManager.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Core/Localization/B3DStringTable.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStringTables : public TScriptTypeDefinition<ScriptStringTables>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "StringTables")

		ScriptStringTables();

		static void SetupScriptBindings();

	private:
		static void InternalSetActiveLanguage(Language language);
		static Language InternalGetActiveLanguage();
		static MonoObject* InternalGetTable(uint32_t id);
		static void InternalRemoveTable(uint32_t id);
		static void InternalSetTable(uint32_t id, MonoObject* table);
	};
}
