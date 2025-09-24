//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Importer/B3DImporter.h"
#include "B3DScriptTypeDefinition.h"
#include "Utility/B3DUUID.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptImporter : public TScriptTypeDefinition<ScriptImporter>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Importer")

		ScriptImporter();

		static void SetupScriptBindings();

	private:
		static MonoObject* InternalImport(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAsync(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID);
		static MonoObject* InternalImportAll(MonoString* inputFilePath, MonoObject* importOptions);
		static MonoObject* InternalImportAllAsync(MonoString* inputFilePath, MonoObject* importOptions);
		static bool InternalSupportsFileType(MonoString* extension);
	};
#endif
}
