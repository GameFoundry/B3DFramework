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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Scene")

		/** Registers internal callbacks. Must be called on scripting system load. */
		static void startUp();

		/** Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void shutDown();

		/** Handles per-frame operations. Needs to be called every frame. */
		static void update();

	private:
		ScriptScene(MonoObject* instance);

		/** Triggered when the assembly refresh starts. */
		static void onRefreshStarted();

		/** Triggered when assembly domain is loaded during assembly refresh. */
		static void onRefreshDomainLoaded();

		/** Makes the provided prefab the currently active scene. */
		static void setActiveScene(const HPrefab& prefab);

		static HEvent OnRefreshDomainLoadedConn;
		static HEvent OnRefreshStartedConn;

		static UUID sActiveSceneUUID;
		static String sActiveSceneName;
		static bool sIsGenericPrefab;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetActiveScene(ScriptPrefab* scriptPrefab);
		static MonoObject* internal_GetRoot();
		static void internal_ClearScene();
		static MonoObject* internal_GetMainCameraSO();

		typedef void(BS_THUNKCALL *OnUpdateThunkDef)(MonoException**);
		static OnUpdateThunkDef onUpdateThunk;
	};

	/** @} */
}