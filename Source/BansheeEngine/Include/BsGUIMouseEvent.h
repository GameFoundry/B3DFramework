//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsVector2I.h"

namespace bs
{
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
		Left, Right, Middle, Count
	};

	/**
	 * Contains data about a GUI mouse input event. This class may store data for many types of events, and some data might
	 * not be initialized for some event types. Caller must check event type before relying on the data inside.
	 */
	class BS_EXPORT GUIMouseEvent
	{
	public:
		GUIMouseEvent();
		GUIMouseEvent(bool buttonStates[(int)GUIMouseButton::Count], bool shift, bool ctrl, bool alt);

		/**
		 * The position of the mouse when the event happened. This is relative to the parent widget of the element this
		 * event is being sent to.
		 */
		const Vector2I& getPosition() const { return mPosition; }

		/**	Returns the internal type of the event. */
		GUIMouseEventType getType() const { return mType; }

		/**	Returns the mouse button involved in the event, if any. */
		GUIMouseButton getButton() const { return mButton; }

		/**	Returns drag amount in pixels, if event is drag related. */
		Vector2I getDragAmount() const { return mDragAmount; }

		/**	Returns the position where the drag was started from, if event is drag related. */
		Vector2I getDragStartPosition() const { return mDragStartPosition; }

		/**	Returns amount of mouse wheel scroll, if event is scroll wheel related. */
		float getWheelScrollAmount() const { return mWheelScrollAmount; }

		/**	Checks is the specified mouse button pressed. */
		bool isButtonDown(GUIMouseButton button) const { return mButtonStates[(int)button]; }

		/**	Gets the current drag and drop operation type id. Only valid if event is drag and drop related. */
		UINT32 getDragAndDropTypeId() const { return mDragTypeId; }

		/** Returns internal data being dragged by a drag and drop event. . Only valid if event is drag and drop related. */
		void* getDragAndDropData() const { return mDragData; }

		/**	Checks is the shift button being held. */
		bool isShiftDown() const { return mShift; }

		/**	Checks is the control button being held. */
		bool isCtrlDown() const { return mCtrl; }

		/**	Checks is the alt button being held. */
		bool isAltDown() const { return mAlt; }
	private:
		friend class GUIManager;

		/**	Initializes the event with MouseOver event data. */
		void setMouseOverData(const Vector2I& position);

		/**	Initializes the event with MouseOut event data. */
		void setMouseOutData(const Vector2I& position);

		/**	Initializes the event with MouseMove event data. */
		void setMouseMoveData(const Vector2I& position);

		/**	Initializes the event with MouseWheelScroll event data. */
		void setMouseWheelScrollData(float scrollAmount);

		/**	Initializes the event with MouseUp event data. */
		void setMouseUpData(const Vector2I& position, GUIMouseButton button);

		/**	Initializes the event with MouseDown event data. */
		void setMouseDownData(const Vector2I& position, GUIMouseButton button);

		/**	Initializes the event with MouseDoubleClick event data. */
		void setMouseDoubleClickData(const Vector2I& position, GUIMouseButton button);

		/**	Initializes the event with MouseDrag event data. */
		void setMouseDragData(const Vector2I& position, const Vector2I& dragStartPosition);

		/**	Initializes the event with MouseDragStart event data. */
		void setMouseDragStartData(const Vector2I& position, const Vector2I& dragStartPosition);

		/**	Initializes the event with MouseDragEnd event data. */
		void setMouseDragEndData(const Vector2I& position);

		/**	Initializes the event with DragAndDropDropped event data. */
		void setDragAndDropDroppedData(const Vector2I& position, UINT32 dragTypeId, void* dragData);

		/**	Initializes the event with DragAndDropDragged event data. */
		void setDragAndDropDraggedData(const Vector2I& position, UINT32 dragTypeId, void* dragData);

		/**	Initializes the event with DragAndDropLeft event data. */
		void setDragAndDropLeftData(const Vector2I& position, UINT32 dragTypeId, void* dragData);

		bool mButtonStates[(int)GUIMouseButton::Count];
		Vector2I mPosition;
		Vector2I mDragStartPosition;
		Vector2I mDragAmount;
		float mWheelScrollAmount;
		GUIMouseEventType mType;
		GUIMouseButton mButton;
		UINT32 mDragTypeId;
		void* mDragData;

		bool mShift;
		bool mCtrl;
		bool mAlt;
	};

	/** @} */
}