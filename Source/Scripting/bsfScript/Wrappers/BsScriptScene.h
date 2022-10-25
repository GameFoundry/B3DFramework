//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for SceneManager. */
	class BS_SCR_BE_EXPORT ScriptScene : public ScriptObject<ScriptScene>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Scene")

		/** Registers internal callbacks. Must be called on scripting system load. */
		static void StartUp();

		/** Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void ShutDown();

		/** Handles per-frame operations. Needs to be called every frame. */
		static void Update();

	private:
		ScriptScene(MonoObject* instance);

		/** Triggered when the assembly refresh starts. */
		static void OnRefreshStarted();

		/** Triggered when assembly domain is loaded during assembly refresh. */
		static void OnRefreshDomainLoaded();

		/** Makes the provided prefab the currently active scene. */
		static void SetActiveScene(const HPrefab& prefab);

		static HEvent OnRefreshDomainLoadedConn;
		static HEvent OnRefreshStartedConn;

		static UUID sActiveSceneUUID;
		static String sActiveSceneName;
		static bool sIsGenericPrefab;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalGetRoot();
		static MonoObject* InternalGetMainCameraSo();

#if BS_IS_BANSHEE3D
		static void InternalSetActiveScene(ScriptPrefab* scriptPrefab);
		static void InternalClearScene();

		typedef void(BS_THUNKCALL* OnUpdateThunkDef)(MonoException**);
		static OnUpdateThunkDef onUpdateThunk;
#endif
	};

	/** @} */
} // namespace bs
