//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsDragAndDropManager.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	class ScriptSceneObjectDragDropData;
	class ScriptResourceDragDropData;

	/** Types of drag and drop operations supported by the managed drag and drop system. */
	enum class ScriptDragDropType // Note: Must be equal to C# DragDropType enum
	{
		Resource,
		SceneObject,
		None
	};

	/**	Interop class between C++ & CLR for DragAndDropManager. */
	class BS_SCR_BED_EXPORT ScriptDragDrop : public ScriptObject<ScriptDragDrop>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "DragDrop");

	private:
		ScriptDragDrop(MonoObject* instance);

		/**
		 * Triggered when the scene object drag and drop operation ends.
		 *
		 * @param[in]	processed	True if the drop operations was accepted by some system.
		 */
		static void sceneObjectDragDropFinalize(bool processed);

		/**
		 * Triggered when the resource drag and drop operation ends.
		 *
		 * @param[in]	processed	True if the drop operations was accepted by some system.
		 */
		static void resourceDragDropFinalize(bool processed);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_IsDragInProgress();
		static bool internal_IsDropInProgress();
		static MonoObject* internal_GetData();
		static ScriptDragDropType internal_GetDragType();
		static void internal_StartSceneObjectDrag(ScriptSceneObjectDragDropData* dragData);
		static void internal_StartResourceDrag(ScriptResourceDragDropData* dragData);
	};

	/**
	 * Interop class between C++ & CLR for SceneObjectDragDropData. Contains a set of scene objects used during managed drag
	 * and drop operations.
	 */
	class BS_SCR_BED_EXPORT ScriptSceneObjectDragDropData : public ScriptObject<ScriptSceneObjectDragDropData>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SceneObjectDragDropData");

		/** Creates a new managed instance of SceneObjectDragDropData containing the specified scene objects. */
		static MonoObject* create(const Vector<HSceneObject>& sceneObjects);

		/**	Returns the scene objects referenced by this object. */
		const Vector<HSceneObject>& getSceneObjects() const { return mSceneObjects; }

	private:
		ScriptSceneObjectDragDropData(MonoObject* instance, const Vector<HSceneObject>& sceneObjects);

		Vector<HSceneObject> mSceneObjects;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoArray* objects);
		static MonoArray* internal_GetObjects(ScriptSceneObjectDragDropData* instance);
	};

	/**
	 * Interop class between C++ & CLR for ResourceDragDropData. Contains a set of resource paths used during managed drag
	 * and drop operations.
	 */
	class BS_SCR_BED_EXPORT ScriptResourceDragDropData : public ScriptObject < ScriptResourceDragDropData >
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ResourceDragDropData");

		/**	Creates a new managed instance of ResourceDragDropData containing the specified resource paths. */
		static MonoObject* create(const Vector<Path>& paths);

		/**	Returns the resource paths referenced by this object. */
		const Vector<Path>& getPaths() const { return mPaths; }

	private:
		ScriptResourceDragDropData(MonoObject* instance, const Vector<Path>& paths);

		Vector<Path> mPaths;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoArray* paths);
		static MonoArray* internal_GetPaths(ScriptResourceDragDropData* instance);
	};

	/** @} */
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**
	 * Handles managed drag and drop operations. Wraps the existing functionality of DragAndDropManager. Essentially
	 * converts the callback nature of DragAndDropManager into a polling based system.
	 */
	class BS_SCR_BED_EXPORT ScriptDragDropManager : public Module<ScriptDragDropManager>
	{
	public:
		ScriptDragDropManager();
		~ScriptDragDropManager();

		/**
		 * Called every frame. Checks for changes in drag and drop operations.
		 *
		 * @note	Internal method.
		 */
		void update();

		/**	Checks has the user performed a drop operation this frame.  */
		bool isDropInProgress() const { return mIsDropInProgress; }

		/**
		 * Returns the managed representation of currently dragged data (for example SceneObjectDragDropData). This will be
		 * null if drag or drop is not in progress or of unsupported type.
		 */
		MonoObject* getDropData() const;

		/**	Checks the type of the current drag or drop operation. */
		ScriptDragDropType getDragType() const;

	private:
		/**
		 * Triggered when a native drag and drop operation ends.
		 *
		 * @param[in]	evt				Pointer data regarding where the drop operation occurred.
		 * @param[in]	callbackInfo	Data whether the drop was processed or not.
		 */
		void onMouseDragEnded(const PointerEvent& evt, DragCallbackInfo& callbackInfo);

		HEvent mDragEndedConn;

		bool mIsDropInProgress;
		UINT64 mDroppedFrameIdx;
		ScriptDragDropType mDropType;
		Vector<Path> mDroppedPaths;
		Vector<HSceneObject> mDroppedSceneObjects;
	};

	/** @} */
}