//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	private:
		ScriptScene(MonoObject* instance);

		/** Triggered when the assembly refresh starts. */
		static void onRefreshStarted();

		/** Triggered when assembly domain is loaded during assembly refresh. */
		static void onRefreshDomainLoaded();

		static HEvent OnRefreshDomainLoadedConn;
		static HEvent OnRefreshStartedConn;

		static String ActiveSceneUUID;
		static WString ActiveSceneName;
		static bool IsGenericPrefab;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_LoadScene(MonoString* path);
		static MonoObject* internal_GetRoot();
		static void internal_ClearScene();
		static MonoObject* internal_GetMainCameraSO();
	};

	/** @} */
}