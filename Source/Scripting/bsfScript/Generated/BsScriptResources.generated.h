//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "BsScriptObject.h"
#include "Utility/BsUUID.h"

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

		static void OnResourceLoaded(const TResourceHandle<Resource>& p0);
		static void OnResourceDestroyed(const UUID& p0);
		static void OnResourceModified(const TResourceHandle<Resource>& p0);

		typedef void(B3D_THUNKCALL *OnResourceLoadedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnResourceLoadedThunkDefinition OnResourceLoadedThunk;
		typedef void(B3D_THUNKCALL *OnResourceDestroyedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnResourceDestroyedThunkDefinition OnResourceDestroyedThunk;
		typedef void(B3D_THUNKCALL *OnResourceModifiedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnResourceModifiedThunkDefinition OnResourceModifiedThunk;

		static HEvent OnResourceLoadedConn;
		static HEvent OnResourceDestroyedConn;
		static HEvent OnResourceModifiedConn;

		static void InternalReleaseInternalReference(MonoObject* resource);
		static void InternalUnloadAllUnused();
		static void InternalUnloadAll();
		static bool InternalIsLoaded(UUID* uuid, bool checkInProgress);
		static float InternalGetLoadProgress(MonoObject* resource);
	};
#endif
}
