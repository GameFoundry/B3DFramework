//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DVector2.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	class CocoaWindow;

	/** Handles Cocoa drag and drop functionality. */
	class CocoaDragAndDrop
	{
		/** Possible states of the DND manager. */
		enum class State
		{
			Inactive,
			Entered,
			Active
		};

		/** Type of drag and drop operation. */
		enum class DragAndDropOpType
		{
			Enter,
			DragOver,
			Drop,
			Leave
		};

		/**	Structure describing a drag and drop operation. */
		struct DragAndDropOp
		{
			DragAndDropOp(DragAndDropOpType type, DropTarget* target)
				: Type(type), Target(target)
			{}

			DragAndDropOp(DragAndDropOpType type, DropTarget* target, const Vector2I& position)
				: Type(type), Target(target), Position(position)
			{}

			DragAndDropOp(DragAndDropOpType type, DropTarget* target, const Vector2I& position, const Vector<Path>& fileList)
				: Type(type), Target(target), Position(position), FileList(fileList)
			{}

			DragAndDropOpType Type;
			DropTarget* Target;
			Vector2I Position;
			Vector<Path> FileList;
		};

		/** Represents a single registered drop area. */
		struct DropArea
		{
			DropArea(DropTarget* target, const Area2I& area)
				: Target(target), Area(area)
			{}

			DropTarget* Target;
			Area2I Area;
		};

		/** Type of operations that can happen to a DropArea. */
		enum class DropAreaOpType
		{
			Register, /**< New DropArea is being registered. */
			Unregister, /**< DropArea is being unregistered. */
			Update /**< DropArea was updated. */
		};

		/** Operation that in some way modifies a DropArea. */
		struct DropAreaOp
		{
			DropAreaOp(DropTarget* target, DropAreaOpType type, const Area2I& area = Area2I::kEmpty)
				: Target(target), Area(area), Type(type)
			{}

			DropTarget* Target;
			Area2I Area;
			DropAreaOpType Type;
		};

	public:
		/**
		 * Triggers any drag and drop events.
		 *
		 * @note 	Main thread only.
		 */
		static void Update();

		/**
		 * Registers a new drop target. Any further events processed will take this target into account, trigger its events
		 * and populate its data if a drop occurs.
		 *
		 * @note 	Thread safe.
		 */
		static void RegisterDropTarget(DropTarget* target);

		/**
		 * Updates information about previous registered DropTarget. Call this when drop target area changes.
		 *
		 * @note	Thread safe.
		 */
		static void UpdateDropTarget(DropTarget* target);

		/**
		 * Unregisters a drop target. Its events will no longer be triggered.
		 *
		 * @note	Thread safe.
		 */
		static void UnregisterDropTarget(DropTarget* target);

		/** Triggered by Cocoa window when mouse cursor enters its content area while dragging. */
		static bool NotifyDragEnteredInternal(u32 windowId, const Vector2I& position);

		/** Triggered by Cocoa window when mouse cursor moves within its content area while dragging. */
		static bool NotifyDragMovedInternal(u32 windowId, const Vector2I& position);

		/** Triggered by Cocoa window when mouse cursor leaves its content area while dragging.  */
		static void NotifyDragLeftInternal(u32 windowId);

		/** Triggered by Cocoa window when the user stops dragging (drops the items) within the window's content area. */
		static bool NotifyDragDroppedInternal(u32 windowId, const Vector2I& position, const Vector<Path>& paths);

	private:
		static Vector<DropArea> sDropAreas;
		static Mutex sMutex;
		static Vector<DragAndDropOp> sQueuedOperations;
		static Vector<DropAreaOp> sQueuedAreaOperations;
	};
} // namespace b3d
