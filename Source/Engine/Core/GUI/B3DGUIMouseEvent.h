//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

	/**
	 * Types of GUI mouse events. Each type reaches elements in one of these ways:
	 *  - Routed: offered to the elements under the pointer front to back, and stops at the first element that handles it
	 *    by returning true. Elements in front of that one, which ignored it, have received it too, but elements behind it
	 *    never do. This means the element the pointer is on handles the event, and containers behind it do not see it.
	 *  - Sent to the pressed elements: the elements MouseDown was routed to when the press began. Usually this is only the
	 *    element that handled the MouseDown, plus any elements in front of it that ignored it.
	 *  - Sent to every element the pointer is inside of, directly or through an element it contains. What the element
	 *    returns is ignored. Containers behind the element the pointer is on receive these as well.
	 *
	 * A press captures the pointer while any of its pressed elements remain, meaning they were not destroyed since.
	 */
	enum class GUIMouseEventType
	{
		/**
		 * The pointer moved over the element. Routed, once each time the pointer moves over the element. While a press
		 * captures the pointer, only the pressed elements receive it.
		 */
		MouseOver,

		/**
		 * The pointer is no longer over an element that received MouseOver, either because it moved away or because an
		 * element in front of it now handles MouseOver. Sent to every such element. While a press captures the pointer,
		 * only the pressed elements receive it.
		 */
		MouseOut,

		/**
		 * A pointer button was pressed, beginning a press. Routed, and every element it reached becomes a pressed element.
		 * Not sent while a press captures the pointer.
		 */
		MouseDown,

		/**
		 * The button that began the press was released. Routed, but only among the pressed elements still under the
		 * pointer, so an element only receives it if it also received the MouseDown.
		 */
		MouseUp,

		/** A pointer button was double clicked. Routed. */
		MouseDoubleClick,

		/** The pointer moved while no drag is in progress. Routed. */
		MouseMove,

		/** The mouse wheel was scrolled while no drag is in progress. Routed. */
		MouseWheelScroll,

		/** The pointer moved during a drag that began with MouseDragStart. Sent to the pressed elements. */
		MouseDrag,

		/** The pointer moved a few pixels away from where the left button was pressed. Sent to the pressed elements. */
		MouseDragStart,

		/** The left button was released, ending a drag that began with MouseDragStart. Sent to the pressed elements. */
		MouseDragEnd,

		/**
		 * The pointer moved during a drag and drop operation (see DragAndDrop). Routed, among the elements under the
		 * pointer that accept the data being dragged.
		 */
		MouseDragAndDropDragged,

		/**
		 * The data of a drag and drop operation was dropped. Routed, among the elements under the pointer that accept the
		 * data being dragged.
		 */
		MouseDragAndDropDropped,

		/**
		 * The pointer is no longer over the element during a drag and drop operation. Offered to the elements the pointer
		 * left, until one of them handles it.
		 */
		MouseDragAndDropLeft,

		/**
		 * The pointer moved inside the element, either directly or into one of the elements it contains. Sent to every
		 * element the pointer is inside of. While a press captures the pointer, only the elements the pointer was inside
		 * of when the press began receive it.
		 */
		MouseEnter,

		/** The pointer is no longer inside the element or any element it contains. Counterpart to MouseEnter. */
		MouseLeave,

		/**
		 * A press began while the pointer was inside the element. Sent to every element the pointer is inside of, except
		 * disabled ones, regardless of which element handles the MouseDown.
		 */
		MousePressBegin,

		/** The press that sent MousePressBegin was released, or the pointer is no longer inside the element. */
		MousePressEnd
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
		const TShared<DragAndDropData>& GetDragAndDropData() const { return mDragAndDropData; }

		/**	Checks is the shift button being held. */
		bool IsShiftDown() const { return mShift; }

		/**	Checks is the control button being held. */
		bool IsCtrlDown() const { return mCtrl; }

		/**	Checks is the alt button being held. */
		bool IsAltDown() const { return mAlt; }

	private:
		friend class GUIPointerEventTracker;
		friend class GUIInputBoxTestSuite;

		/** Initializes the event as a MouseEnter, MouseLeave, MousePressBegin or MousePressEnd event, which carry no data. */
		void SetNotificationData(GUIMouseEventType type);

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
		void SetDragAndDropDroppedData(const GUIPhysicalPoint& position, const TShared<DragAndDropData>& dragAndDropData);

		/**	Initializes the event with DragAndDropDragged event data. */
		void SetDragAndDropDraggedData(const GUIPhysicalPoint& position, const TShared<DragAndDropData>& dragAndDropData);

		/**	Initializes the event with DragAndDropLeft event data. */
		void SetDragAndDropLeftData(const GUIPhysicalPoint& position, const TShared<DragAndDropData>& dragAndDropData);

		bool mButtonStates[(int)GUIMouseButton::Count];
		GUIPhysicalPoint mPosition{kZeroTag};
		GUIPhysicalPoint mDragStartPosition{kZeroTag};
		GUIPhysicalPoint mDragAmount{kZeroTag};
		float mWheelScrollAmount = 0.0f;
		GUIMouseEventType mType = GUIMouseEventType::MouseMove;
		GUIMouseButton mButton = GUIMouseButton::Left;
		TShared<DragAndDropData> mDragAndDropData;

		bool mShift = false;
		bool mCtrl = false;
		bool mAlt = false;
	};

	/** @} */
} // namespace b3d
