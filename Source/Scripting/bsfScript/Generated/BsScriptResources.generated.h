//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "Utility/BsUUID.h"

namespace bs { class Resources; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResources : public ScriptObject<ScriptResources>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Resources")

		ScriptResources(MonoObject* managedInstance);

		static void StartUp();
		static void ShutDown();

	private:
		uint32_t mGCHandle = 0;

		static void OnResourceLoaded(const ResourceHandle<Resource>& p0);
		static void OnResourceDestroyed(const UUID& p0);
		static void OnResourceModified(const ResourceHandle<Resource>& p0);

		typedef void(B3D_THUNKCALL *OnResourceLoadedThunkDef) (MonoObject* p0, MonoException**);
		static OnResourceLoadedThunkDef OnResourceLoadedThunk;
		typedef void(B3D_THUNKCALL *OnResourceDestroyedThunkDef) (MonoObject* p0, MonoException**);
		static OnResourceDestroyedThunkDef OnResourceDestroyedThunk;
		typedef void(B3D_THUNKCALL *OnResourceModifiedThunkDef) (MonoObject* p0, MonoException**);
		static OnResourceModifiedThunkDef OnResourceModifiedThunk;

		static HEvent OnResourceLoadedConn;
		static HEvent OnResourceDestroyedConn;
		static HEvent OnResourceModifiedConn;

		static MonoObject* InternalLoad(MonoString* filePath, ResourceLoadFlag loadFlags);
		static MonoObject* InternalLoadAsync(MonoString* filePath, ResourceLoadFlag loadFlags);
		static MonoObject* InternalLoadFromUuid(UUID* uuid, bool async, ResourceLoadFlag loadFlags);
		static void InternalRelease(MonoObject* resource);
		static void InternalUnloadAllUnused();
		static void InternalUnloadAll();
		static void InternalSave(MonoObject* resource, MonoString* filePath, bool overwrite, bool compress);
		static void InternalSave0(MonoObject* resource, bool compress);
		static MonoArray* InternalGetDependencies(MonoString* filePath);
		static bool InternalIsLoaded(UUID* uuid, bool checkInProgress);
		static float InternalGetLoadProgress(MonoObject* resource, bool includeDependencies);
		static void InternalRegisterResourceManifest(MonoObject* manifest);
		static void InternalUnregisterResourceManifest(MonoObject* manifest);
		static MonoObject* InternalGetResourceManifest(MonoString* name);
		static bool InternalGetFilePathFromUuid(UUID* uuid, MonoString** filePath);
		static bool InternalGetUUUIDAndPhysicalPathFromFilePath(MonoString* path, UUID* outUUID, MonoString** outPhysicalFilePath);
	};
#endif
}
