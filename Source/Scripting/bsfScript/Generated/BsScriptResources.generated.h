//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "Utility/BsUUID.h"

namespace bs { class Resources; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptResources : public ScriptObject<ScriptResources>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Resources")

		ScriptResources(MonoObject* managedInstance);

		static void startUp();
		static void shutDown();

	private:
		uint32_t mGCHandle = 0;

		static void onResourceLoaded(const ResourceHandle<Resource>& p0);
		static void onResourceDestroyed(const UUID& p0);
		static void onResourceModified(const ResourceHandle<Resource>& p0);

		typedef void(BS_THUNKCALL *onResourceLoadedThunkDef) (MonoObject* p0, MonoException**);
		static onResourceLoadedThunkDef onResourceLoadedThunk;
		typedef void(BS_THUNKCALL *onResourceDestroyedThunkDef) (MonoObject* p0, MonoException**);
		static onResourceDestroyedThunkDef onResourceDestroyedThunk;
		typedef void(BS_THUNKCALL *onResourceModifiedThunkDef) (MonoObject* p0, MonoException**);
		static onResourceModifiedThunkDef onResourceModifiedThunk;

		static HEvent onResourceLoadedConn;
		static HEvent onResourceDestroyedConn;
		static HEvent onResourceModifiedConn;

		static MonoObject* Internal_load(MonoString* filePath, ResourceLoadFlag loadFlags);
		static MonoObject* Internal_loadAsync(MonoString* filePath, ResourceLoadFlag loadFlags);
		static MonoObject* Internal_loadFromUUID(UUID* uuid, bool async, ResourceLoadFlag loadFlags);
		static void Internal_release(MonoObject* resource);
		static void Internal_unloadAllUnused();
		static void Internal_unloadAll();
		static void Internal_save(MonoObject* resource, MonoString* filePath, bool overwrite, bool compress);
		static void Internal_save0(MonoObject* resource, bool compress);
		static MonoArray* Internal_getDependencies(MonoString* filePath);
		static bool Internal_isLoaded(UUID* uuid, bool checkInProgress);
		static float Internal_getLoadProgress(MonoObject* resource, bool includeDependencies);
		static void Internal_registerResourceManifest(MonoObject* manifest);
		static void Internal_unregisterResourceManifest(MonoObject* manifest);
		static MonoObject* Internal_getResourceManifest(MonoString* name);
		static bool Internal_getFilePathFromUUID(UUID* uuid, MonoString** filePath);
		static bool Internal_getUUIDFromFilePath(MonoString* path, UUID* uuid);
	};
#endif
}
