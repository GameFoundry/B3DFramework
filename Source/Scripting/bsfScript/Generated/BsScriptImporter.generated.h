//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptObject.h"
#include "Utility/BsUUID.h"

namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptImporter : public ScriptObject<ScriptImporter>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Importer")

		ScriptImporter(MonoObject* managedInstance);

	private:
		static MonoObject* InternalImport(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAsync(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAll(MonoString* inputFilePath, MonoObject* importOptions);
		static MonoObject* InternalImportAllAsync(MonoString* inputFilePath, MonoObject* importOptions);
		static bool InternalSupportsFileType(MonoString* extension);
	};
#endif
}
