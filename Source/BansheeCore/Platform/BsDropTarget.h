//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsEvent.h"
#include "Math/BsRect2I.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Type of drop event type. This is used when dragging items over drop targets. */
	enum class DropTargetType
	{
		FileList,
		None
	};

	/**
	 * Drop targets allow you to register a certain portion of a window as a drop target that accepts certain drop types
	 * from the OS (platform) specific drag and drop system. Accepted drop types are provided by the OS and include things
	 * like file and item dragging.
	 *
	 * You will receive events with the specified drop area as long as it is active.
	 */
	class BS_CORE_EXPORT DropTarget
	{
	public:
		~DropTarget();

		/**	Sets the drop target area, in local window coordinates. */
		void setArea(const Rect2I& area);

		/** Returns the drop target area, in local window coordinates. */
		const Rect2I& getArea() const { return mArea;}

		/**	Gets the type of drop that this drop target is looking for. Only valid after a drop has been triggered. */
		DropTargetType getDropType() const { return mDropType; }

		/**
		 * Returns a list of files received by the drop target. Only valid after a drop of FileList type has been triggered.
		 */
		const Vector<Path>& getFileList() const { return mFileList; }

		/**
		 * Creates a new drop target. Any drop events that happen on the specified window's drop area will be reported
		 * through the target's events.
		 *
		 * @param[in]	window		Window to which the drop target will be attached to.
		 * @param[in]	area		Area, relative to the window, in which the drop events are allowed.
		 * @return					Newly created drop target.
		 */
		static SPtr<DropTarget> create(const RenderWindow* window, const Rect2I& area);

		/**
		 * Triggered when a pointer is being dragged over the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(INT32, INT32)> onDragOver;

		/**
		 * Triggered when the user completes a drop while pointer is over the drop area. Provides window coordinates of the
		 * pointer position.
		 */
		Event<void(INT32, INT32)> onDrop;

		/**
		 * Triggered when a pointer enters the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(INT32, INT32)> onEnter;

		/** Triggered when a pointer leaves the drop area. */
		Event<void()> onLeave;

		/** @name Internal
		 *  @{
		 */

		/** Clears all internal values. */
		void _clear();

		/** Sets the file list and marks the drop event as FileList. */
		void _setFileList(const Vector<Path>& fileList);

		/** Marks the drop area as inactive or active. */
		void _setActive(bool active) { mActive = active; }

		/**	Checks is the specified position within the current drop area. Position should be in window local coordinates. */
		bool _isInside(const Vector2I& pos) const;

		/** Returns true if the drop target is active. */
		bool _isActive() const { return mActive; }

		/**	Returns a render window this drop target is attached to. */
		const RenderWindow* _getOwnerWindow() const { return mOwnerWindow; }

		/** @} */
	private:
		friend class Platform;

		DropTarget(const RenderWindow* ownerWindow, const Rect2I& area);

		Rect2I mArea;
		bool mActive;
		const RenderWindow* mOwnerWindow;

		DropTargetType mDropType;
		Vector<Path> mFileList;
	};

	/** @} */
}

