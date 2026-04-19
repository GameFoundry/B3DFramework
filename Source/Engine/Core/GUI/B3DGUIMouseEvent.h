//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DDragAndDrop.h"
#include "GUI/B3DGUIUnits.h"

namespace b3d
{
	class DragAndDropData;
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Types of GUI mouse events. */
	enum class GUIMouseEventType
	{
		MouseOver,
		MouseOut,
		MouseDown,
		MouseUp,
		MouseDoubleClick,
		MouseMove,
		MouseWheelScroll,
		MouseDrag,
		MouseDragStart,
		MouseDragEnd,
		MouseDragAndDropDragged,
		MouseDragAndDropDropped,
		MouseDragAndDropLeft
	};

	/**	Types of GUI mouse buttons. */
	enum class GUIMouseButton
	{
		Left,
		Right,
		Middle,
		Count
	};

	/**
	 * Contains data about a GUI mouse input event. This class may store data for many types of events, and some data might
	 * not be initialized for some event types. Caller must check event type before relying on the data inside.
	 */
	class B3D_EXPORT GUIMouseEvent
	{
	public:
		GUIMouseEvent() = default;
		GUIMouseEvent(bool buttonStates[(int)GUIMouseButton::Count], bool shift, bool ctrl, bool alt);

		/**
		 * The position of the mouse when the event happened. This is relative to the parent widget of the element this
		 * event is being sent to.
		 */
		const GUIPhysicalPoint& GetPosition() const { return mPosition; }

		/**	Returns the internal type of the event. */
		GUIMouseEventType GetType() const { return mType; }

		/**	Returns the mouse button involved in the event, if any. */
		GUIMouseButton GetButton() const { return mButton; }

		/**	Returns drag amount in pixels, if event is drag related. */
		GUIPhysicalPoint GetDragAmount() const { return mDragAmount; }

		/**	Returns the position where the drag was started from, if event is drag related. */
		GUIPhysicalPoint GetDragStartPosition() const { return mDragStartPosition; }

		/**	Returns amount of mouse wheel scroll, if event is scroll wheel related. */
		float GetWheelScrollAmount() const { return mWheelScrollAmount; }

		/**	Checks is the specified mouse button pressed. */
		bool IsButtonDown(GUIMouseButton button) const { return mButtonStates[(int)button]; }

		/** Returns data being dragged by a drag and drop event. Only valid if event is drag and drop related. */
		const SPtr<DragAndDropData>& GetDragAndDropData() const { return mDragAndDropData; }

		/**	Checks is the shift button being held. */
		bool IsShiftDown() const { return mShift; }

		/**	Checks is the control button being held. */
		bool IsCtrlDown() const { return mCtrl; }

		/**	Checks is the alt button being held. */
		bool IsAltDown() const { return mAlt; }

	private:
		friend class GUIManager;

		/**	Initializes the event with MouseOver event data. */
		void SetMouseOverData(const GUIPhysicalPoint& position);

		/**	Initializes the event with MouseOut event data. */
		void SetMouseOutData(const GUIPhysicalPoint& position);

		/**	Initializes the event with MouseMove event data. */
		void SetMouseMoveData(const GUIPhysicalPoint& position);

		/**	Initializes the event with MouseWheelScroll event data. */
		void SetMouseWheelScrollData(float scrollAmount);

		/**	Initializes the event with MouseUp event data. */
		void SetMouseUpData(const GUIPhysicalPoint& position, GUIMouseButton button);

		/**	Initializes the event with MouseDown event data. */
		void SetMouseDownData(const GUIPhysicalPoint& position, GUIMouseButton button);

		/**	Initializes the event with MouseDoubleClick event data. */
		void SetMouseDoubleClickData(const GUIPhysicalPoint& position, GUIMouseButton button);

		/**	Initializes the event with MouseDrag event data. */
		void SetMouseDragData(const GUIPhysicalPoint& position, const GUIPhysicalPoint& dragAmount);

		/**	Initializes the event with MouseDragStart event data. */
		void SetMouseDragStartData(const GUIPhysicalPoint& position, const GUIPhysicalPoint& dragStartPosition);

		/**	Initializes the event with MouseDragEnd event data. */
		void SetMouseDragEndData(const GUIPhysicalPoint& position);

		/**	Initializes the event with DragAndDropDropped event data. */
		void SetDragAndDropDroppedData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData);

		/**	Initializes the event with DragAndDropDragged event data. */
		void SetDragAndDropDraggedData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData);

		/**	Initializes the event with DragAndDropLeft event data. */
		void SetDragAndDropLeftData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData);

		bool mButtonStates[(int)GUIMouseButton::Count];
		GUIPhysicalPoint mPosition{kZeroTag};
		GUIPhysicalPoint mDragStartPosition{kZeroTag};
		GUIPhysicalPoint mDragAmount{kZeroTag};
		float mWheelScrollAmount = 0.0f;
		GUIMouseEventType mType = GUIMouseEventType::MouseMove;
		GUIMouseButton mButton = GUIMouseButton::Left;
		SPtr<DragAndDropData> mDragAndDropData;

		bool mShift = false;
		bool mCtrl = false;
		bool mAlt = false;
	};

	/** @} */
} // namespace b3d
