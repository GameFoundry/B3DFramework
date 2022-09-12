//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIMouseEvent.h"

namespace bs
{
	GUIMouseEvent::GUIMouseEvent(bool buttonStates[(int)GUIMouseButton::Count], bool shift, bool ctrl, bool alt)
		:mType(GUIMouseEventType::MouseMove), mButton(GUIMouseButton::Left), mShift(shift), mCtrl(ctrl), mAlt(alt)
	{
		memcpy(mButtonStates, buttonStates, sizeof(mButtonStates));
	}

	void GUIMouseEvent::SetMouseOverData(const Vector2I& position)
	{
		mType = GUIMouseEventType::MouseOver;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseOutData(const Vector2I& position)
	{
		mType = GUIMouseEventType::MouseOut;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseMoveData(const Vector2I& position)
	{
		mType = GUIMouseEventType::MouseMove;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseWheelScrollData(float scrollAmount)
	{
		mType = GUIMouseEventType::MouseWheelScroll;
		mPosition = Vector2I();
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = scrollAmount;
	}

	void GUIMouseEvent::SetMouseUpData(const Vector2I& position, GUIMouseButton button)
	{
		mType = GUIMouseEventType::MouseUp;
		mPosition = position;
		mButton = button;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseDownData(const Vector2I& position, GUIMouseButton button)
	{
		mType = GUIMouseEventType::MouseDown;
		mPosition = position;
		mButton = button;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseDoubleClickData(const Vector2I& position, GUIMouseButton button)
	{
		mType = GUIMouseEventType::MouseDoubleClick;
		mPosition = position;
		mButton = button;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseDragData(const Vector2I& position, const Vector2I& dragAmount)
	{
		mType = GUIMouseEventType::MouseDrag;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = dragAmount;
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseDragStartData(const Vector2I& position, const Vector2I& dragStartPosition)
	{
		mType = GUIMouseEventType::MouseDragStart;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = dragStartPosition;
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetMouseDragEndData(const Vector2I& position)
	{
		mType = GUIMouseEventType::MouseDragEnd;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
	}

	void GUIMouseEvent::SetDragAndDropDroppedData(const Vector2I& position, UINT32 dragTypeId, void* dragData)
	{
		mType = GUIMouseEventType::MouseDragAndDropDropped;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
		mDragTypeId = dragTypeId;
		mDragData = dragData;
	}

	void GUIMouseEvent::SetDragAndDropDraggedData(const Vector2I& position, UINT32 dragTypeId, void* dragData)
	{
		mType = GUIMouseEventType::MouseDragAndDropDragged;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
		mDragTypeId = dragTypeId;
		mDragData = dragData;
	}

	void GUIMouseEvent::SetDragAndDropLeftData(const Vector2I& position, UINT32 dragTypeId, void* dragData)
	{
		mType = GUIMouseEventType::MouseDragAndDropLeft;
		mPosition = position;
		mButton = GUIMouseButton::Left;
		mDragAmount = Vector2I();
		mDragStartPosition = Vector2I();
		mWheelScrollAmount = 0.0f;
		mDragTypeId = dragTypeId;
		mDragData = dragData;
	}
}
