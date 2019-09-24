//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Resources/BsResourceManifest.h"
#include "Utility/BsUUID.h"

namespace bs { class ResourceManifest; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptResourceManifest : public TScriptReflectable<ScriptResourceManifest, ResourceManifest>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ResourceManifest")

		ScriptResourceManifest(MonoObject* managedInstance, const SPtr<ResourceManifest>& value);

		static MonoObject* create(const SPtr<ResourceManifest>& value);

	private:
		static MonoString* Internal_getName(ScriptResourceManifest* thisPtr);
		static void Internal_registerResource(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString* filePath);
		static void Internal_unregisterResource(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool Internal_uuidToFilePath(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString** filePath);
		static bool Internal_filePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID);
		static bool Internal_uuidExists(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool Internal_filePathExists(ScriptResourceManifest* thisPtr, MonoString* filePath);
		static void Internal_save(MonoObject* manifest, MonoString* path, MonoString* relativePath);
		static MonoObject* Internal_load(MonoString* path, MonoString* relativePath);
		static void Internal_create(MonoObject* managedInstance, MonoString* name);
	};
#endif
}
