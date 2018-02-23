//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsVector2I.h"
#include "Math/BsRect2I.h"

namespace bs
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
				:type(type), target(target)
			{ }

			DragAndDropOp(DragAndDropOpType type, DropTarget* target, const Vector2I& pos)
				:type(type), target(target), position(pos)
			{ }

			DragAndDropOp(DragAndDropOpType type, DropTarget* target, const Vector2I& pos,
				const Vector<Path>& fileList)
				:type(type), target(target), position(pos), fileList(fileList)
			{ }

			DragAndDropOpType type;
			DropTarget* target;
			Vector2I position;
			Vector<Path> fileList;
		};

		/** Represents a single registered drop area. */
		struct DropArea
		{
			DropArea(DropTarget* target, const Rect2I& area)
				:target(target), area(area)
			{ }

			DropTarget* target;
			Rect2I area;
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
			DropAreaOp(DropTarget* target, DropAreaOpType type, const Rect2I& area = Rect2I::EMPTY)
				:target(target), area(area), type(type)
			{ }

			DropTarget* target;
			Rect2I area;
			DropAreaOpType type;
		};

	public:
		/**
		 * Triggers any drag and drop events.
		 *
		 * @note 	Sim thread only.
		 */
		static void update();

		/**
		 * Registers a new drop target. Any further events processed will take this target into account, trigger its events
		 * and populate its data if a drop occurs.
		 *
		 * @note 	Thread safe.
		 */
		static void registerDropTarget(DropTarget* target);

		/**
		 * Updates information about previous registered DropTarget. Call this when drop target area changes.
		 *
		 * @note	Thread safe.
		 */
		static void updateDropTarget(DropTarget* target);

		/**
		 * Unregisters a drop target. Its events will no longer be triggered.
		 *
		 * @note	Thread safe.
		 */
		static void unregisterDropTarget(DropTarget* target);

		/** Triggered by Cocoa window when mouse cursor enters its content area while dragging. */
		static bool _notifyDragEntered(UINT32 windowId, const Vector2I& position);

		/** Triggered by Cocoa window when mouse cursor moves within its content area while dragging. */
		static bool _notifyDragMoved(UINT32 windowId, const Vector2I& position);

		/** Triggered by Cocoa window when mouse cursor leaves its content area while dragging.  */
		static void _notifyDragLeft(UINT32 windowId);

		/** Triggered by Cocoa window when the user stops dragging (drops the items) within the window's content area. */
		static bool _notifyDragDropped(UINT32 windowId, const Vector2I& position, const Vector<Path>& paths);

	private:
		static Vector<DropArea> sDropAreas;
		static Mutex sMutex;
		static Vector<DragAndDropOp> sQueuedOperations;
		static Vector<DropAreaOp> sQueuedAreaOperations;
	};
}

