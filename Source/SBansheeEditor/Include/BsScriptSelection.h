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

	/**	Interop class between C++ & CLR for Selection. */
	class BS_SCR_BED_EXPORT ScriptSelection : public ScriptObject<ScriptSelection>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "Selection");

		/**	Hooks up selection callbacks. Must be called on library load. */
		static void startUp();

		/**	Destroys selection callbacks. Must be called before library is unloaded. */
		static void shutDown();

	private:
		ScriptSelection(MonoObject* instance);

		/**
		 * Triggered when selection has changed.
		 *
		 * @param[in]	sceneObjects	Newly selected scene objects. This will be empty if no scene objects are selected
		 *								or if selection hasn't changed.
		 * @param[in]	resPaths		Paths to newly selected resources. This will be empty if no resources are selected
		 *								or if selection hasn't changed.
		 */
		static void onSelectionChanged(const Vector<HSceneObject>& sceneObjects, const Vector<Path>& resPaths);

		/**	Triggered when ping action is requested for the resource at the specified path. */
		static void onResourcePing(const Path& resPath);

		/**	Triggered when ping action is requested for the specified scene object. */
		static void onSceneObjectPing(const HSceneObject& sceneObject);

		static HEvent OnSelectionChangedConn;
		static HEvent OnPingResourceConn;
		static HEvent OnPingSceneObjectConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *OnSelectionChangedThunkDef) (MonoArray*, MonoArray*, MonoException**);
		typedef void(__stdcall *OnPingResourceThunkDef) (MonoString*, MonoException**);
		typedef void(__stdcall *OnPingSceneObjectThunkDef) (MonoObject*, MonoException**);

		static OnSelectionChangedThunkDef OnSelectionChangedThunk;
		static OnPingResourceThunkDef OnPingResourceThunk;
		static OnPingSceneObjectThunkDef OnPingSceneObjectThunk;

		static void internal_GetSceneObjectSelection(MonoArray** selection);
		static void internal_SetSceneObjectSelection(MonoArray* selection);

		static void internal_GetResourceUUIDSelection(MonoArray** selection);
		static void internal_SetResourceUUIDSelection(MonoArray* selection);

		static void internal_GetResourcePathSelection(MonoArray** selection);
		static void internal_SetResourcePathSelection(MonoArray* selection);

		static void internal_PingResource(MonoString* resourcePath);
		static void internal_PingSceneObject(MonoObject* so);
	};

	/** @} */
}