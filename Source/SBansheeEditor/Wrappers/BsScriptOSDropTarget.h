//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsRect2I.h"

namespace bs
{
	class DropTarget;

	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**
	 * Interop class between C++ & CLR for OSDropTarget. Managed drop target is always associated with a managed
	 * EditorWindow.
	 */
	class BS_SCR_BED_EXPORT ScriptOSDropTarget : public ScriptObject <ScriptOSDropTarget>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "OSDropTarget")

	private:
		ScriptOSDropTarget(MonoObject* instance, ScriptEditorWindow* parent);
		~ScriptOSDropTarget();

		/**	Destroys the internal native drop target. */
		void destroy();

		/**
		 * Creates an internal native drop target over the specified window. Any previous drop target is overwritten.
		 * 
		 * @param[in]	parentWindow	Window the drop target is located on.
		 * @param[in]	x				X position of the drop target, relative to window, in pixels.
		 * @param[in]	y				Y position of the drop target, relative to window, in pixels.
		 * @param[in]	width			Width of the drop target in pixels.
		 * @param[in]	height			Height of the drop target in pixels.
		 */
		void setDropTarget(const SPtr<RenderWindow>& parentWindow, INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**
		 * Updates bounds of an existing drop target.
		 *
		 * @param[in]	bounds	Area of the drop target relative to the editor widget (EditorWindow in managed terms).
		 */
		void setBounds(const Rect2I& bounds);

		/**
		 * Triggered when editor widget (EditorWindow in managed terms) parent changes. This might mean we need to re-create
		 * the drop target as the parent render window might have changed.
		 */
		void widgetParentChanged(EditorWidgetContainer* parent);

		/**	Triggered when the parent editor widget (EditorWindow in managed terms) is resized. */
		void widgetResized(UINT32 width, UINT32 height);

		/**	Triggered when the parent editor widget (EditorWindow in managed terms) is moved. */
		void widgetMoved(INT32 x, INT32 y);

		/**	Returns the editor widget (EditorWindow in managed terms) this drop target belongs to. */
		EditorWidgetBase* getParentWidget() const;

		/**
		 * Returns the bounds of the drop target, relative to the parent window. This depends of set bounds using 
		 * setBounds() and the current position and size of the editor widget.
		 */
		Rect2I getDropTargetArea() const;

		/**
		 * Triggered when the drag and drop operation has entered the area over an OS drop target.
		 *
		 * @param[in]	thisPtr		C++/CLR interop object that contains the native OSDropTarget that triggered the event.
		 * @param[in]	x			X coordinate of the pointer, relative to parent window, in pixels.
		 * @param[in]	y			Y coordinate of the pointer, relative to parent window, in pixels.
		 */
		static void dropTargetDragEnter(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y);

		/**
		 * Triggered every frame that pointer moves while over the area over an OS drop target.
		 *
		 * @param[in]	thisPtr		C++/CLR interop object that contains the native OSDropTarget that triggered the event.
		 * @param[in]	x			X coordinate of the pointer, relative to parent window, in pixels.
		 * @param[in]	y			Y coordinate of the pointer, relative to parent window, in pixels.
		 */
		static void dropTargetDragMove(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y);

		/**
		 * Triggered when the drag and drop operation has left the area over an OS drop target.
		 *
		 * @param[in]	thisPtr		C++/CLR interop object that contains the native OSDropTarget that triggered the event.
		 */
		static void dropTargetDragLeave(ScriptOSDropTarget* thisPtr);

		/**
		 * Triggered when the drag and drop operation has finished over an OS drop target.
		 *
		 * @param[in]	thisPtr		C++/CLR interop object that contains the native OSDropTarget that triggered the event.
		 * @param[in]	x			X coordinate of the pointer, relative to parent window, in pixels.
		 * @param[in]	y			Y coordinate of the pointer, relative to parent window, in pixels.
		 */
		static void dropTargetDragDropped(ScriptOSDropTarget* thisPtr, INT32 x, INT32 y);

		ScriptEditorWindow* mParent;
		SPtr<DropTarget> mDropTarget;
		Rect2I mParentArea;
		Rect2I mArea;
		bool mIsDestroyed;

		HEvent mDropTargetEnterConn;
		HEvent mDropTargetMoveConn;
		HEvent mDropTargetLeaveConn;
		HEvent mDropTargetDroppedConn;
		HEvent mWidgetParentChangedConn;
		HEvent mWidgetMovedConn;
		HEvent mWidgetResizedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(BS_THUNKCALL *OnEnterThunkDef) (MonoObject*, INT32, INT32, MonoException**);
		typedef void(BS_THUNKCALL *OnMoveDef) (MonoObject*, INT32, INT32, MonoException**);
		typedef void(BS_THUNKCALL *OnLeaveDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnDropThunkDef) (MonoObject*, INT32, INT32, MonoException**);

		static OnEnterThunkDef onEnterThunk;
		static OnMoveDef onMoveThunk;
		static OnLeaveDef onLeaveThunk;
		static OnDropThunkDef onDropThunk;

		static void internal_CreateInstance(MonoObject* instance, ScriptEditorWindow* editorWindow);
		static void internal_Destroy(ScriptOSDropTarget* nativeInstance);
		static void internal_SetBounds(ScriptOSDropTarget* nativeInstance, Rect2I* bounds);
		static MonoArray* internal_GetFilePaths(ScriptOSDropTarget* nativeInstance);
	};

	/** @} */
}