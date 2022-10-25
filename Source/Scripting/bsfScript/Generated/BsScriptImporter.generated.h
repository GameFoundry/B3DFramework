//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Utility/BsUUID.h"

namespace bs
{
	class Importer;
}

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptImporter : public ScriptObject<ScriptImporter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Importer")

		ScriptImporter(MonoObject* managedInstance);

	private:
		static MonoObject* InternalImport(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAsync(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAll(MonoString* inputFilePath, MonoObject* importOptions);
		static MonoObject* InternalImportAllAsync(MonoString* inputFilePath, MonoObject* importOptions);
		static bool InternalSupportsFileType(MonoString* extension);
	};
#endif
} // namespace bs
