//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Utility/BsUUID.h"

namespace bs { class Importer; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptImporter : public ScriptObject<ScriptImporter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Importer")

		ScriptImporter(MonoObject* managedInstance);

	private:
		static MonoObject* Internal_import(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* Internal_importAsync(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* Internal_importAll(MonoString* inputFilePath, MonoObject* importOptions);
		static MonoObject* Internal_importAllAsync(MonoString* inputFilePath, MonoObject* importOptions);
		static bool Internal_supportsFileType(MonoString* extension);
	};
#endif
}
