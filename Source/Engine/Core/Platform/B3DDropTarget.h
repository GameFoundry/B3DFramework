//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DEvent.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup Platform
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
	class B3D_EXPORT DropTarget
	{
	public:
		~DropTarget();

		/**	Sets the drop target area, in local window coordinates. */
		void SetArea(const Area2I& area);

		/** Returns the drop target area, in local window coordinates. */
		const Area2I& GetArea() const { return mArea; }

		/**	Gets the type of drop that this drop target is looking for. Only valid after a drop has been triggered. */
		DropTargetType GetDropType() const { return mDropType; }

		/**
		 * Returns a list of files received by the drop target. Only valid after a drop of FileList type has been triggered.
		 */
		const Vector<Path>& GetFileList() const { return mFileList; }

		/**
		 * Creates a new drop target. Any drop events that happen on the specified window's drop area will be reported
		 * through the target's events.
		 *
		 * @param	window	Window to which the drop target will be attached to.
		 * @param	area	Area, relative to the window, in which the drop events are allowed.
		 * @return			Newly created drop target.
		 */
		static SPtr<DropTarget> Create(const RenderWindow* window, const Area2I& area);

		/**
		 * Triggered when a pointer is being dragged over the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(i32, i32)> OnDragOver;

		/**
		 * Triggered when the user completes a drop while pointer is over the drop area. Provides window coordinates of the
		 * pointer position.
		 */
		Event<void(i32, i32)> OnDrop;

		/**
		 * Triggered when a pointer enters the drop area. Provides window coordinates of the pointer position.
		 */
		Event<void(i32, i32)> OnEnter;

		/** Triggered when a pointer leaves the drop area. */
		Event<void()> OnLeave;

		/** @name Internal
		 *  @{
		 */

		/** Clears all internal values. */
		void ClearInternal();

		/** Sets the file list and marks the drop event as FileList. */
		void SetFileListInternal(const Vector<Path>& fileList);

		/** Marks the drop area as inactive or active. */
		void SetActiveInternal(bool active) { mActive = active; }

		/**	Checks is the specified position within the current drop area. Position should be in window local coordinates. */
		bool IsInsideInternal(const Vector2I& pos) const;

		/** Returns true if the drop target is active. */
		bool IsActiveInternal() const { return mActive; }

		/**	Returns a render window this drop target is attached to. */
		const RenderWindow* GetOwnerWindowInternal() const { return mOwnerWindow; }

		/** @} */
	protected:
		friend class Platform;

		DropTarget(const RenderWindow* ownerWindow, const Area2I& area);

		Area2I mArea;
		bool mActive;
		const RenderWindow* mOwnerWindow;

		DropTargetType mDropType;
		Vector<Path> mFileList;
	};

	/** @} */
} // namespace b3d
