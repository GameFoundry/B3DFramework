//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIPointerEventTracker.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIManager.h"
#include "Math/B3DMath.h"

using namespace b3d;

namespace
{
	/** Distance in pixels the pointer must move while the left button is held for a drag to start. */
	constexpr u32 kDragDistance = 3;

	/** Converts a pointer button to a GUI mouse button. */
	GUIMouseButton ToGUIMouseButton(PointerEventButton button)
	{
		switch(button)
		{
		case PointerEventButton::Left:
			return GUIMouseButton::Left;
		case PointerEventButton::Middle:
			return GUIMouseButton::Middle;
		case PointerEventButton::Right:
			return GUIMouseButton::Right;
		default:
			B3D_ENSURE_LOG(false, "Provided button is not a GUI supported mouse button.");
			return GUIMouseButton::Left;
		}
	}

	/** Creates an event with the button and modifier states of the pointer event. */
	GUIMouseEvent CreateMouseEvent(const PointerEvent& event)
	{
		bool buttonStates[(int)GUIMouseButton::Count];
		for(u32 buttonIndex = 0; buttonIndex < (u32)PointerEventButton::Count; buttonIndex++)
			buttonStates[(int)ToGUIMouseButton((PointerEventButton)buttonIndex)] = event.ButtonStates[buttonIndex];

		return GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);
	}

	/** Returns the entry of the element, or null if the element has no entry. */
	template<class Entry>
	Entry* FindEntry(TArray<Entry>& entries, const GUIInteractable* element)
	{
		for(Entry& entry : entries)
		{
			if(entry.Element == element)
				return &entry;
		}

		return nullptr;
	}
} // namespace

GUIPointerEventTracker::GUIPointerEventTracker(GUIManager& manager)
	: mManager(manager)
{ }

template<class FnPrepareEvent>
bool GUIPointerEventTracker::RouteEvent(FnPrepareEvent fnPrepareEvent)
{
	for(const ElementUnderPointer& entry : mElementsUnderPointer)
	{
		if(entry.Element->IsPendingDestroy() || !fnPrepareEvent(*entry.Element))
			continue;

		if(entry.Element->DoOnMouseEvent(mEvent))
			return true;
	}

	return false;
}

bool GUIPointerEventTracker::UpdateElementsUnderPointer(TArrayView<GUIInteractable* const> elementsUnderPointer, const PointerEvent& event, const TOptional<DragAndDropOperation>& dragAndDrop)
{
	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();

	mNewElementsUnderPointer.Clear();
	for(GUIInteractable* element : elementsUnderPointer)
	{
		ElementUnderPointer newEntry(element);

		const ElementUnderPointer* const entry = FindEntry(mElementsUnderPointer, element);
		if(entry != nullptr)
		{
			newEntry.UsesMouseOver = entry->UsesMouseOver;
			newEntry.ReceivedMouseOver = entry->ReceivedMouseOver;
		}

		mNewElementsUnderPointer.Add(newEntry);
	}

	bool isHandled = false;
	for(ElementUnderPointer& newEntry : mNewElementsUnderPointer)
	{
		if(newEntry.Element->IsPendingDestroy())
			continue;

		if(newEntry.ReceivedMouseOver)
		{
			newEntry.IsHovering = true;
			if(newEntry.UsesMouseOver)
				break;

			continue;
		}

		if(IsCapturing() && !mPress->PressedElements.Contains(newEntry.Element))
			continue;

		mEvent = CreateMouseEvent(event);
		mEvent.SetMouseOverData(ScreenToElementCoords(*newEntry.Element, screenPosition));
		newEntry.ReceivedMouseOver = true;
		newEntry.IsHovering = true;

		if(newEntry.Element->DoOnMouseEvent(mEvent))
		{
			newEntry.UsesMouseOver = true;
			isHandled = true;
			break;
		}
	}

	if(dragAndDrop)
	{
		for(const ElementUnderPointer& entry : mElementsUnderPointer)
		{
			if(entry.Element->IsPendingDestroy() || FindEntry(mNewElementsUnderPointer, entry.Element) != nullptr)
				continue;

			mEvent.SetDragAndDropLeftData(ScreenToElementCoords(*entry.Element, screenPosition), dragAndDrop->Data);
			if(entry.Element->DoOnMouseEvent(mEvent))
			{
				isHandled = true;
				break;
			}
		}
	}

	for(const ElementUnderPointer& entry : mElementsUnderPointer)
	{
		if(!entry.ReceivedMouseOver || entry.Element->IsPendingDestroy())
			continue;

		const ElementUnderPointer* const newEntry = FindEntry(mNewElementsUnderPointer, entry.Element);
		if(newEntry != nullptr && newEntry->IsHovering)
			continue;

		if(IsCapturing() && !mPress->PressedElements.Contains(entry.Element))
			continue;

		// Every element the pointer left must hear about it, so one handling the event cannot stop the others
		mEvent.SetMouseOutData(ScreenToElementCoords(*entry.Element, screenPosition));
		if(entry.Element->DoOnMouseEvent(mEvent))
			isHandled = true;
	}

	std::swap(mElementsUnderPointer, mNewElementsUnderPointer);
	UpdateEnteredElements(CreateMouseEvent(event));

	return isHandled;
}

bool GUIPointerEventTracker::ProcessPointerMove(const PointerEvent& event, const TOptional<DragAndDropOperation>& dragAndDrop)
{
	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	const bool hasMoved = mLastPointerScreenPosition != event.ScreenPos;
	mLastPointerScreenPosition = event.ScreenPos;

	bool isHandled = false;
	if(mPress && mPress->Drag == DragState::HeldWithoutDrag && (u32)mPress->PressPosition.CalculateManhattanDistance(screenPosition) > kDragDistance)
	{
		for(GUIInteractable* element : mPress->PressedElements)
		{
			if(element->IsPendingDestroy())
				continue;

			mEvent.SetMouseDragStartData(ScreenToElementCoords(*element, screenPosition), ScreenToElementCoords(*element, mPress->PressPosition));
			if(element->DoOnMouseEvent(mEvent))
				isHandled = true;
		}

		mPress->Drag = DragState::Dragging;
		mPress->DragStartPosition = screenPosition;
	}

	if(mPress && mPress->Drag == DragState::Dragging)
	{
		if(hasMoved)
		{
			for(GUIInteractable* element : mPress->PressedElements)
			{
				if(element->IsPendingDestroy())
					continue;

				mEvent.SetMouseDragData(ScreenToElementCoords(*element, screenPosition), screenPosition - mPress->DragStartPosition);
				if(element->DoOnMouseEvent(mEvent))
					isHandled = true;
			}
		}

		if(dragAndDrop)
		{
			const u32 typeId = dragAndDrop->Data->GetTypeId();

			// The cursor shows whether the last element offered the dragged data accepts it
			bool isAccepted = true;
			const bool isDraggedHandled = RouteEvent([this, &isAccepted, &dragAndDrop, &screenPosition, typeId](GUIInteractable& element)
			{
				const GUIPhysicalPoint position = ScreenToElementCoords(element, screenPosition);

				isAccepted = !dragAndDrop->NeedsValidDropTarget || element.AcceptDragAndDrop(position, typeId);
				if(!isAccepted)
					return false;

				mEvent.SetDragAndDropDraggedData(position, dragAndDrop->Data);
				return true;
			});

			if(isDraggedHandled)
				isHandled = true;

			mCursor = isAccepted ? CursorType::ArrowDrag : CursorType::Deny;
		}

		return isHandled;
	}

	if(hasMoved)
	{
		// The cursor stays as it is while the left button is held, until a drag starts
		const bool isHeldWithoutDrag = mPress && mPress->Drag == DragState::HeldWithoutDrag;

		CursorType cursor = CursorType::Arrow;
		for(const ElementUnderPointer& entry : mElementsUnderPointer)
		{
			if(entry.Element->IsPendingDestroy())
				continue;

			const GUIPhysicalPoint position = ScreenToElementCoords(*entry.Element, screenPosition);

			mEvent.SetMouseMoveData(position);
			const bool isMoveHandled = entry.Element->DoOnMouseEvent(mEvent);

			CursorType elementCursor = CursorType::Arrow;
			if(!isHeldWithoutDrag && entry.Element->HasCustomCursor(position, elementCursor))
				cursor = elementCursor;

			if(isMoveHandled)
			{
				isHandled = true;
				break;
			}
		}

		if(!isHeldWithoutDrag)
			mCursor = cursor;
	}

	if(Math::Abs(event.MouseWheelScrollAmount) > 0.00001f)
	{
		const bool isScrollHandled = RouteEvent([this, &event](GUIInteractable&)
		{
			mEvent.SetMouseWheelScrollData(event.MouseWheelScrollAmount);
			return true;
		});

		if(isScrollHandled)
			isHandled = true;
	}

	return isHandled;
}

bool GUIPointerEventTracker::ProcessPointerPress(const PointerEvent& event)
{
	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	const GUIMouseButton button = ToGUIMouseButton(event.Button);
	mEvent = CreateMouseEvent(event);

	if(IsCapturing())
		return false;

	const bool hasElementsUnderPointer = std::any_of(mElementsUnderPointer.begin(), mElementsUnderPointer.end(), [](const ElementUnderPointer& entry)
													 { return !entry.Element->IsPendingDestroy(); });

	if(!hasElementsUnderPointer)
		return false;

	if(mPress)
		EndPress(mEvent);

	mPress.emplace();
	mPress->Button = button;
	mPress->PressPosition = screenPosition;

	if(button == GUIMouseButton::Left)
		mPress->Drag = DragState::HeldWithoutDrag;

	GUIMouseEvent notificationEvent = mEvent;
	for(EnteredElement& entry : mEnteredElements)
	{
		mPress->CapturedElements.Add(entry.Element);
		if(entry.Element->IsDisabled())
			continue;

		entry.IsPressed = true;
		SendNotification(entry.Element, notificationEvent, GUIMouseEventType::MousePressBegin);
	}

	for(const ElementUnderPointer& entry : mElementsUnderPointer)
	{
		if(entry.Element->IsPendingDestroy())
			continue;

		mEvent.SetMouseDownData(ScreenToElementCoords(*entry.Element, screenPosition), button);
		const bool isDownHandled = entry.Element->DoOnMouseEvent(mEvent);

		mPress->PressedElements.Add(entry.Element);
		if(isDownHandled)
			return true;
	}

	return false;
}

bool GUIPointerEventTracker::ProcessPointerRelease(const PointerEvent& event)
{
	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	const GUIMouseButton button = ToGUIMouseButton(event.Button);
	mEvent = CreateMouseEvent(event);
	mCursor = CursorType::Arrow;

	if(!mPress || mPress->Button != button)
		return false;

	bool isHandled = RouteEvent([this, &screenPosition, button](GUIInteractable& element)
	{
		if(!mPress->PressedElements.Contains(&element))
			return false;

		mEvent.SetMouseUpData(ScreenToElementCoords(element, screenPosition), button);
		return true;
	});

	if(mPress->Drag == DragState::Dragging)
	{
		for(GUIInteractable* element : mPress->PressedElements)
		{
			if(element->IsPendingDestroy())
				continue;

			mEvent.SetMouseDragEndData(ScreenToElementCoords(*element, screenPosition));
			if(element->DoOnMouseEvent(mEvent))
				isHandled = true;
		}
	}

	EndPress(mEvent);
	return isHandled;
}

bool GUIPointerEventTracker::ProcessPointerDoubleClick(const PointerEvent& event)
{
	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	const GUIMouseButton button = ToGUIMouseButton(event.Button);
	mEvent = CreateMouseEvent(event);

	return RouteEvent([this, &screenPosition, button](GUIInteractable& element)
	{
		mEvent.SetMouseDoubleClickData(ScreenToElementCoords(element, screenPosition), button);
		return true;
	});
}

bool GUIPointerEventTracker::ProcessDrop(const PointerEvent& event, const DragAndDropOperation& dragAndDrop)
{
	if(ToGUIMouseButton(event.Button) != GUIMouseButton::Left)
		return false;

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	const u32 typeId = dragAndDrop.Data->GetTypeId();

	return RouteEvent([this, &dragAndDrop, &screenPosition, typeId](GUIInteractable& element)
	{
		const GUIPhysicalPoint position = ScreenToElementCoords(element, screenPosition);
		if(dragAndDrop.NeedsValidDropTarget && !element.AcceptDragAndDrop(position, typeId))
			return false;

		mEvent.SetDragAndDropDroppedData(position, dragAndDrop.Data);
		return true;
	});
}

void GUIPointerEventTracker::ProcessPointerLeftWindow(TArrayView<GUIInteractable* const> windowElements)
{
	mNewElementsUnderPointer.Clear();
	for(const ElementUnderPointer& entry : mElementsUnderPointer)
	{
		if(std::find(windowElements.begin(), windowElements.end(), entry.Element) == windowElements.end())
		{
			mNewElementsUnderPointer.Add(entry);
			continue;
		}

		if(entry.Element->IsPendingDestroy() || (IsCapturing() && !mPress->PressedElements.Contains(entry.Element)))
			continue;

		mEvent.SetMouseOutData(ScreenToElementCoords(*entry.Element, GUIPhysicalPoint(kZeroTag)));
		entry.Element->DoOnMouseEvent(mEvent);
	}

	std::swap(mElementsUnderPointer, mNewElementsUnderPointer);
	UpdateEnteredElements(mEvent);

	if(!mPress || mPress->Drag != DragState::Dragging)
		mCursor = CursorType::Arrow;
}

void GUIPointerEventTracker::RemovePendingDestroyElements()
{
	auto fnIsEntryPendingDestroy = [](const auto& entry) { return entry.Element->IsPendingDestroy(); };
	auto fnIsPendingDestroy = [](const GUIInteractable* element) { return element->IsPendingDestroy(); };

	mElementsUnderPointer.Erase(std::remove_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(), fnIsEntryPendingDestroy), mElementsUnderPointer.end());
	mEnteredElements.Erase(std::remove_if(mEnteredElements.begin(), mEnteredElements.end(), fnIsEntryPendingDestroy), mEnteredElements.end());

	if(mPress)
	{
		mPress->PressedElements.Erase(std::remove_if(mPress->PressedElements.begin(), mPress->PressedElements.end(), fnIsPendingDestroy), mPress->PressedElements.end());
		mPress->CapturedElements.Erase(std::remove_if(mPress->CapturedElements.begin(), mPress->CapturedElements.end(), fnIsPendingDestroy), mPress->CapturedElements.end());
	}
}

GUIPhysicalPoint GUIPointerEventTracker::ScreenToElementCoords(const GUIInteractable& element, const GUIPhysicalPoint& screenPosition) const
{
	return mManager.ScreenToWidgetCoords(element.GetParentWidget(), screenPosition);
}

void GUIPointerEventTracker::SendNotification(GUIInteractable* element, GUIMouseEvent& event, GUIMouseEventType type)
{
	if(element->IsPendingDestroy())
		return;

	event.SetNotificationData(type);
	element->DoOnMouseEvent(event);
}

void GUIPointerEventTracker::UpdateEnteredElements(GUIMouseEvent event)
{
	mNewEnteredElements.Clear();
	for(const ElementUnderPointer& entry : mElementsUnderPointer)
	{
		if(!entry.IsHovering)
			continue;

		for(GUIElement* element = entry.Element; element != nullptr; element = element->GetParent())
		{
			GUIInteractable* const interactableElement = B3DRTTICast<GUIInteractable>(element);
			if(interactableElement == nullptr || interactableElement->IsPendingDestroy())
				continue;

			// Every element containing this one was added along with it
			if(FindEntry(mNewEnteredElements, interactableElement) != nullptr)
				break;

			if(IsCapturing() && !mPress->CapturedElements.Contains(interactableElement))
				continue;

			const EnteredElement* const previousEntry = FindEntry(mEnteredElements, interactableElement);
			mNewEnteredElements.Add({ interactableElement, previousEntry != nullptr && previousEntry->IsPressed });
		}
	}

	for(const EnteredElement& previousEntry : mEnteredElements)
	{
		if(FindEntry(mNewEnteredElements, previousEntry.Element) != nullptr)
			continue;

		if(previousEntry.IsPressed)
			SendNotification(previousEntry.Element, event, GUIMouseEventType::MousePressEnd);

		SendNotification(previousEntry.Element, event, GUIMouseEventType::MouseLeave);
	}

	for(const EnteredElement& entry : mNewEnteredElements)
	{
		if(FindEntry(mEnteredElements, entry.Element) == nullptr)
			SendNotification(entry.Element, event, GUIMouseEventType::MouseEnter);
	}

	std::swap(mEnteredElements, mNewEnteredElements);
}

void GUIPointerEventTracker::EndPress(GUIMouseEvent event)
{
	mPress.reset();

	for(EnteredElement& entry : mEnteredElements)
	{
		if(!entry.IsPressed)
			continue;

		entry.IsPressed = false;
		SendNotification(entry.Element, event, GUIMouseEventType::MousePressEnd);
	}
}
