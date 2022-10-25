//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Resources/BsResourceManifest.h"
#include "Utility/BsUUID.h"

namespace bs
{
	class ResourceManifest;
}

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptResourceManifest : public TScriptReflectable<ScriptResourceManifest, ResourceManifest>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ResourceManifest")

		ScriptResourceManifest(MonoObject* managedInstance, const SPtr<ResourceManifest>& value);

		static MonoObject* Create(const SPtr<ResourceManifest>& value);

	private:
		static MonoString* InternalGetName(ScriptResourceManifest* thisPtr);
		static void InternalRegisterResource(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString* filePath);
		static void InternalUnregisterResource(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool InternalUuidToFilePath(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString** filePath);
		static bool InternalFilePathToUuid(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID);
		static bool InternalUuidExists(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool InternalFilePathExists(ScriptResourceManifest* thisPtr, MonoString* filePath);
		static void InternalSave(MonoObject* manifest, MonoString* path, MonoString* relativePath);
		static MonoObject* InternalLoad(MonoString* path, MonoString* relativePath);
		static void InternalCreate(MonoObject* managedInstance, MonoString* name);
	};
#endif
} // namespace bs
