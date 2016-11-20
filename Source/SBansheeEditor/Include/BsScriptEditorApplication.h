//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for EditorApplication. */
	class BS_SCR_BED_EXPORT ScriptEditorApplication : public ScriptObject <ScriptEditorApplication>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "EditorApplication")

		/**	Registers internal callbacks. Must be called on scripting system load. */
		static void startUp();

		/**	Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void shutDown();

		/**	Called every frame. Triggers delayed project load. */
		 static void update();

	private:
		ScriptEditorApplication(MonoObject* instance);

		/**	Triggered when the user clicks on the editor's status bar. */
		static void onStatusBarClicked();

		static bool mRequestProjectLoad;
		static bool mRequestAssemblyReload;
		static Path mProjectLoadPath;
		static HEvent OnStatusBarClickedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetStatusScene(MonoString* name, bool modified);
		static void internal_SetStatusProject(bool modified);
		static void internal_SetStatusCompiling(bool compiling);
		static MonoString* internal_GetProjectPath();
		static MonoString* internal_GetProjectName();
		static bool internal_GetProjectLoaded();
		static MonoString* internal_GetCompilerPath();
		static MonoString* internal_GetBuiltinReleaseAssemblyPath();
		static MonoString* internal_GetBuiltinDebugAssemblyPath();
		static MonoString* internal_GetScriptAssemblyPath();
		static MonoString* internal_GetFrameworkAssemblyPath();
		static MonoString* internal_GetEngineAssemblyName();
		static MonoString* internal_GetEditorAssemblyName();
		static MonoString* internal_GetScriptGameAssemblyName();
		static MonoString* internal_GetScriptEditorAssemblyName();
		static MonoObject* internal_SaveScene(MonoString* path);
		static bool internal_IsValidProject(MonoString* path);
		static void internal_SaveProject();
		static void internal_LoadProject(MonoString* path);
		static void internal_UnloadProject();
		static void internal_CreateProject(MonoString* path);
		static void internal_ReloadAssemblies();
		static void internal_OpenExternally(MonoString* path);
		static void internal_RunUnitTests();
		static void internal_Quit();
		static void internal_ToggleToolbarItem(MonoString* name, bool on);
		static bool internal_GetIsPlaying();
		static void internal_SetIsPlaying(bool value);
		static bool internal_GetIsPaused();
		static void internal_SetIsPaused(bool value);
		static void internal_FrameStep();
		static void internal_SetMainRenderTarget(ScriptRenderTarget* renderTarget);
		static bool internal_HasFocus();

		typedef void(__stdcall *OnProjectLoadedThunkDef)(MonoException**);
		typedef void(__stdcall *OnStatusBarClickedThunkDef) (MonoException**);

		static OnProjectLoadedThunkDef onProjectLoadedThunk;
		static OnStatusBarClickedThunkDef onStatusBarClickedThunk;
	};

	/** @} */
}