//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIMouseEvent.h"

using namespace b3d;

GUIMouseEvent::GUIMouseEvent(bool buttonStates[(int)GUIMouseButton::Count], bool shift, bool ctrl, bool alt)
	: mType(GUIMouseEventType::MouseMove), mButton(GUIMouseButton::Left), mShift(shift), mCtrl(ctrl), mAlt(alt)
{
	memcpy(mButtonStates, buttonStates, sizeof(mButtonStates));
}

void GUIMouseEvent::SetMouseOverData(const GUIPhysicalPoint& position)
{
	mType = GUIMouseEventType::MouseOver;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseOutData(const GUIPhysicalPoint& position)
{
	mType = GUIMouseEventType::MouseOut;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseMoveData(const GUIPhysicalPoint& position)
{
	mType = GUIMouseEventType::MouseMove;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseWheelScrollData(float scrollAmount)
{
	mType = GUIMouseEventType::MouseWheelScroll;
	mPosition = GUIPhysicalPoint(kZeroTag);
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = scrollAmount;
}

void GUIMouseEvent::SetMouseUpData(const GUIPhysicalPoint& position, GUIMouseButton button)
{
	mType = GUIMouseEventType::MouseUp;
	mPosition = position;
	mButton = button;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseDownData(const GUIPhysicalPoint& position, GUIMouseButton button)
{
	mType = GUIMouseEventType::MouseDown;
	mPosition = position;
	mButton = button;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseDoubleClickData(const GUIPhysicalPoint& position, GUIMouseButton button)
{
	mType = GUIMouseEventType::MouseDoubleClick;
	mPosition = position;
	mButton = button;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseDragData(const GUIPhysicalPoint& position, const GUIPhysicalPoint& dragAmount)
{
	mType = GUIMouseEventType::MouseDrag;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = dragAmount;
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseDragStartData(const GUIPhysicalPoint& position, const GUIPhysicalPoint& dragStartPosition)
{
	mType = GUIMouseEventType::MouseDragStart;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = dragStartPosition;
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetMouseDragEndData(const GUIPhysicalPoint& position)
{
	mType = GUIMouseEventType::MouseDragEnd;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
}

void GUIMouseEvent::SetDragAndDropDroppedData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData)
{
	mType = GUIMouseEventType::MouseDragAndDropDropped;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
	mDragAndDropData = dragAndDropData;
}

void GUIMouseEvent::SetDragAndDropDraggedData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData)
{
	mType = GUIMouseEventType::MouseDragAndDropDragged;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
	mDragAndDropData = dragAndDropData;
}

void GUIMouseEvent::SetDragAndDropLeftData(const GUIPhysicalPoint& position, const SPtr<DragAndDropData>& dragAndDropData)
{
	mType = GUIMouseEventType::MouseDragAndDropLeft;
	mPosition = position;
	mButton = GUIMouseButton::Left;
	mDragAmount = GUIPhysicalPoint(kZeroTag);
	mDragStartPosition = GUIPhysicalPoint(kZeroTag);
	mWheelScrollAmount = 0.0f;
	mDragAndDropData = dragAndDropData;
}
