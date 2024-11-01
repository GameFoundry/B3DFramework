//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"
#include "BsScriptTypeDefinition.h"
#include "Utility/BsUUID.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneManager : public TScriptTypeDefinition<ScriptSceneManager>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneManager")

		ScriptSceneManager();

		static void SetupScriptBindings();

		static void StartUp();
		static void ShutDown();

	private:
		static void OnMainSceneLoaded(UUID p0);
		static void OnMainSceneUnloaded(UUID p0);

		typedef void(B3D_THUNKCALL *OnMainSceneLoadedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnMainSceneLoadedThunkDefinition OnMainSceneLoadedThunk;
		typedef void(B3D_THUNKCALL *OnMainSceneUnloadedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnMainSceneUnloadedThunkDefinition OnMainSceneUnloadedThunk;

		static HEvent OnMainSceneLoadedConnection;
		static HEvent OnMainSceneUnloadedConnection;

		static MonoObject* InternalGetMainScene();
		static void InternalClearMainScene(bool forceAll);
		static void InternalLoadMainScene(MonoObject* scene);
		static MonoObject* InternalGetMainCameraSceneObject();
	};
}
