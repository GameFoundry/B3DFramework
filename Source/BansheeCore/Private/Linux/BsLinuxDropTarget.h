//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include <X11/X.h>
#include <X11/Xlib.h>

namespace bs
{
	/** Handles X11 drag and drop functionality. */
	class LinuxDragAndDrop
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
		 * Initializes the drag and drop system. Must be called before any other drag and drop methods are called.
		 *
		 * @note	Core thread only.
		 */
		static void startUp(::Display* xDisplay);

		/**
		 * Shuts down the drag and drop system. Should be called after no more calls to the system are expected.
		 *
		 * @note	Core thread only.
		 */
		static void shutDown();

		/**
		 * Triggers any drag and drop events.
		 *
		 * @note 	Sim thread only.
		 */
		static void update();

		/**
		 * Marks an X11 window as drag and drop aware (being able to accept and send drag and drop events).
		 *
		 * @note	Core thread only.
		 */
		static void makeDNDAware(::Window xWindow);

		/**
		 * Registers a new drop target Any further events processed will take this target into account, trigger its event
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

		/**
		 * Processes X11 ClientMessage event and handles any messages relating to drag and drop. Returns true if a message
		 * was handled, or false if it needs to be handled by the caller.
		 *
		 * @note 	Core thread only.
		 */
		static bool handleClientMessage(XClientMessageEvent& event);

		/**
		 * Processes X11 SelectionNotify event and handles it if it relates to drag and drop. Returns true if the event was
		 * handled, or false otherwise.
		 *
		 * @note 	Core thread only.
		 */
		static bool handleSelectionNotify(XSelectionEvent& event);

	private:
		static ::Display* sXDisplay;
		static bool sDragActive;
		static Vector<DropArea> sDropAreas;
		static Mutex sMutex;
		static INT32 sDNDVersion;
		static Atom sDNDType;
		static ::Window sDNDSource;
		static Vector2I sDragPosition;
		static Vector<DragAndDropOp> sQueuedOperations;
		static Vector<DropAreaOp> sQueuedAreaOperations;

		// Awareness
		static Atom sXdndAware;

		// Selection handling
		static Atom sXdndSelection;

		// Client messages
		static Atom sXdndEnter;
		static Atom sXdndLeave;
		static Atom sXdndPosition;
		static Atom sXdndStatus;
		static Atom sXdndDrop;
		static Atom sXdndFinished;

		// Actions
		static Atom sXdndActionCopy;

		// Type list
		static Atom sXdndTypeList;

		// Other
		static Atom sPRIMARY;
	};
}

