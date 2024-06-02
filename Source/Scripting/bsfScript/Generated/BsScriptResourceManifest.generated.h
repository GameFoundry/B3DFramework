//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Resources/BsResourceManifest.h"
#include "Utility/BsUUID.h"

namespace bs { class ResourceManifest; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceManifest : public TScriptReflectable<ScriptResourceManifest, ResourceManifest>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ResourceManifest")

		ScriptResourceManifest(MonoObject* managedInstance, const SPtr<ResourceManifest>& value);

		static MonoObject* Create(const SPtr<ResourceManifest>& value);

	private:
		static MonoString* InternalGetName(ScriptResourceManifest* thisPtr);
		static void InternalRegisterResource(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString* filePath);
		static void InternalUnregisterResource(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool InternalUUIDToPhysicalFilePath(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString** filePath);
		static bool InternalPhysicalFilePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID);
		static bool InternalVirtualFilePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID);
		static bool InternalUuidExists(ScriptResourceManifest* thisPtr, UUID* uuid);
		static bool InternalFilePathExists(ScriptResourceManifest* thisPtr, MonoString* filePath);
		static bool InternalVirtualToPhysicalPath(ScriptResourceManifest* thisPtr, MonoString* virtualPath, MonoString** outPhysicalPath);
		static void InternalSave(MonoObject* manifest, MonoString* path, MonoString* physicalPathPrefix);
		static MonoObject* InternalLoad(MonoString* path, MonoString* physicalPathPrefix, MonoString* virtualPathPrefix);
		static void InternalCreate(MonoObject* managedInstance, MonoString* name);
	};
#endif
}
