//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUIPointerEventTrackerTestSuite.h"

#include "GUI/B3DGUIElementContainer.h"
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUIManager.h"
#include "GUI/B3DGUIPointerEventTracker.h"

using namespace b3d;

namespace
{
	/** Container that records the mouse events it receives, handles the configured ones and can hold child elements. */
	class RecordingElement final : public GUIElementContainer
	{
	public:
		RecordingElement()
			: GUIElementContainer(GUISizeConstraints())
		{ }

		/** Makes @p element a child of the container, removing it from its previous parent. */
		void AddChild(GUIElement* element) { RegisterChildElement(element); }

		bool DoOnMouseEvent(const GUIMouseEvent& event) override
		{
			ReceivedEvents.Add(event);
			return HandledEvents.Contains(event.GetType());
		}

		bool HasCustomCursor(const GUIPhysicalPoint& position, CursorType& type) const override
		{
			if(!Cursor)
				return false;

			type = *Cursor;
			return true;
		}

		bool AcceptDragAndDrop(const GUIPhysicalPoint& position, u32 typeId) const override { return AcceptsDragAndDrop; }

		/** Returns the first received event of the provided type, or null if there is none. */
		const GUIMouseEvent* FindEvent(GUIMouseEventType type) const
		{
			for(const GUIMouseEvent& event : ReceivedEvents)
			{
				if(event.GetType() == type)
					return &event;
			}

			return nullptr;
		}

		/** Returns true if the element received exactly @p expectedEvents, in order, since the last call. */
		bool ConsumeEvents(std::initializer_list<GUIMouseEventType> expectedEvents)
		{
			const bool isMatching = ReceivedEvents.Size() == expectedEvents.size() && std::equal(expectedEvents.begin(), expectedEvents.end(), ReceivedEvents.begin(),
				[](GUIMouseEventType expectedType, const GUIMouseEvent& event) { return event.GetType() == expectedType; });

			ReceivedEvents.Clear();
			return isMatching;
		}

		TArray<GUIMouseEvent> ReceivedEvents;
		TArray<GUIMouseEventType> HandledEvents;
		TOptional<CursorType> Cursor;
		bool AcceptsDragAndDrop = false;
	};

	/** Returns a pointer event at the provided screen position, with no buttons or modifiers held. */
	PointerEvent CreatePointerEvent(i32 x = 0, i32 y = 0, PointerEventButton button = PointerEventButton::Left)
	{
		PointerEvent event;
		event.ScreenPos = Vector2I(x, y);
		event.Button = button;

		return event;
	}

	/** Returns a GUI position from pixel coordinates. */
	GUIPhysicalPoint CreatePosition(i32 x, i32 y)
	{
		return Vector2I(x, y).To<GUIPhysicalUnit>();
	}
} // namespace

GUIPointerEventTrackerTestSuite::GUIPointerEventTrackerTestSuite()
{
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestContainersAreEnteredThroughChildren)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestMovingBetweenSiblingsKeepsContainerEntered)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestPressReachesContainers)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestLeavingEndsPress)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestPressCapturesEnteredElements)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestMouseOverStopsAtHandler)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestMouseOutReachesEveryDepartedElement)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestMouseDownStopsAtHandler)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestMouseUpRequiresPress)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestDrag)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestCursor)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestDragAndDrop)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestPointerLeavingWindow)
	B3D_ADD_TEST(GUIPointerEventTrackerTestSuite::TestDestroyedElementsAreForgotten)
}

void GUIPointerEventTrackerTestSuite::TestContainersAreEnteredThroughChildren()
{
	// The layout in between is not interactable, so it is skipped without hiding the container above it
	RecordingElement* const outer = B3DNew<RecordingElement>();
	GUILayoutY* const layout = GUILayoutY::Create();
	RecordingElement* const inner = B3DNew<RecordingElement>();
	RecordingElement* const button = B3DNew<RecordingElement>();
	outer->AddChild(layout);
	layout->AddElement(inner);
	inner->AddChild(button);

	GUIPointerEventTracker tracker(GetGUIManager());

	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(inner->ConsumeEvents({ GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MouseEnter }))

	// Moving from the button onto the background of the container holding it only leaves the button
	tracker.UpdateElementsUnderPointer({ inner }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(inner->ConsumeEvents({ GUIMouseEventType::MouseOver }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({}))

	tracker.UpdateElementsUnderPointer({}, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(button->ConsumeEvents({}))
	B3D_TEST_ASSERT(inner->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MouseLeave }))

	outer->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestMovingBetweenSiblingsKeepsContainerEntered()
{
	RecordingElement* const outer = B3DNew<RecordingElement>();
	RecordingElement* const first = B3DNew<RecordingElement>();
	RecordingElement* const second = B3DNew<RecordingElement>();
	outer->AddChild(first);
	outer->AddChild(second);

	GUIPointerEventTracker tracker(GetGUIManager());

	tracker.UpdateElementsUnderPointer({ first }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(first->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MouseEnter }))

	tracker.UpdateElementsUnderPointer({ second }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(first->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(second->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({}))

	outer->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestPressReachesContainers()
{
	RecordingElement* const outer = B3DNew<RecordingElement>();
	RecordingElement* const button = B3DNew<RecordingElement>();
	outer->AddChild(button);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	button->ConsumeEvents({});
	outer->ConsumeEvents({});

	tracker.ProcessPointerPress(CreatePointerEvent());
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MousePressBegin, GUIMouseEventType::MouseDown }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressBegin }))

	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseUp, GUIMouseEventType::MousePressEnd }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressEnd }))

	// A disabled element cannot be pressed, but the container holding it still can
	button->SetDisabled(true);

	tracker.ProcessPointerPress(CreatePointerEvent());
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseDown }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressBegin }))

	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseUp }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressEnd }))

	outer->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestLeavingEndsPress()
{
	RecordingElement* const outer = B3DNew<RecordingElement>();
	RecordingElement* const button = B3DNew<RecordingElement>();
	outer->AddChild(button);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	button->ConsumeEvents({});
	outer->ConsumeEvents({});

	tracker.UpdateElementsUnderPointer({}, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MousePressEnd, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressEnd, GUIMouseEventType::MouseLeave }))

	// Coming back while the button is still held enters the elements again, without pressing them again
	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MouseEnter }))

	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseUp }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({}))

	outer->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestPressCapturesEnteredElements()
{
	RecordingElement* const outer = B3DNew<RecordingElement>();
	RecordingElement* const button = B3DNew<RecordingElement>();
	RecordingElement* const other = B3DNew<RecordingElement>();
	outer->AddChild(button);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	button->ConsumeEvents({});
	outer->ConsumeEvents({});

	// While held, the pointer can only enter the elements the press began in, and only the pressed elements receive
	// MouseOver
	tracker.UpdateElementsUnderPointer({ other }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(other->ConsumeEvents({}))
	B3D_TEST_ASSERT(button->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MousePressEnd, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressEnd, GUIMouseEventType::MouseLeave }))

	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(other->ConsumeEvents({}))

	tracker.UpdateElementsUnderPointer({ other }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(other->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))

	outer->Destroy();
	other->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestMouseOverStopsAtHandler()
{
	RecordingElement* const front = B3DNew<RecordingElement>();
	RecordingElement* const handler = B3DNew<RecordingElement>();
	RecordingElement* const back = B3DNew<RecordingElement>();
	handler->HandledEvents.Add(GUIMouseEventType::MouseOver);

	GUIPointerEventTracker tracker(GetGUIManager());

	tracker.UpdateElementsUnderPointer({ front, handler, back }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(front->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(handler->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(back->ConsumeEvents({}))

	// MouseOver is only sent once while an element stays under the pointer
	tracker.UpdateElementsUnderPointer({ front, handler, back }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(front->ConsumeEvents({}))
	B3D_TEST_ASSERT(handler->ConsumeEvents({}))
	B3D_TEST_ASSERT(back->ConsumeEvents({}))

	tracker.UpdateElementsUnderPointer({ back }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(front->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(handler->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(back->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))

	front->Destroy();
	handler->Destroy();
	back->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestMouseOutReachesEveryDepartedElement()
{
	RecordingElement* const first = B3DNew<RecordingElement>();
	RecordingElement* const second = B3DNew<RecordingElement>();
	first->HandledEvents.Add(GUIMouseEventType::MouseOut);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ first, second }, CreatePointerEvent(), {});
	first->ConsumeEvents({});
	second->ConsumeEvents({});

	tracker.UpdateElementsUnderPointer({}, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(first->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(second->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))

	// An element moving in front that handles MouseOver hides the element behind it
	RecordingElement* const front = B3DNew<RecordingElement>();
	front->HandledEvents.Add(GUIMouseEventType::MouseOver);

	tracker.UpdateElementsUnderPointer({ second }, CreatePointerEvent(), {});
	second->ConsumeEvents({});

	tracker.UpdateElementsUnderPointer({ front, second }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(front->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(second->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))

	first->Destroy();
	second->Destroy();
	front->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestMouseDownStopsAtHandler()
{
	RecordingElement* const front = B3DNew<RecordingElement>();
	RecordingElement* const handler = B3DNew<RecordingElement>();
	RecordingElement* const back = B3DNew<RecordingElement>();
	handler->HandledEvents.Add(GUIMouseEventType::MouseDown);
	handler->HandledEvents.Add(GUIMouseEventType::MouseUp);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ front, handler, back }, CreatePointerEvent(), {});
	front->ConsumeEvents({});
	handler->ConsumeEvents({});
	back->ConsumeEvents({});

	// Every element in front of the handler receives MouseDown and becomes pressed, while the elements behind it don't
	B3D_TEST_ASSERT(tracker.ProcessPointerPress(CreatePointerEvent()))
	B3D_TEST_ASSERT(front->ConsumeEvents({ GUIMouseEventType::MousePressBegin, GUIMouseEventType::MouseDown }))
	B3D_TEST_ASSERT(handler->ConsumeEvents({ GUIMouseEventType::MousePressBegin, GUIMouseEventType::MouseDown }))
	B3D_TEST_ASSERT(back->ConsumeEvents({ GUIMouseEventType::MousePressBegin }))

	// Another button is ignored while the press is in progress
	B3D_TEST_ASSERT(!tracker.ProcessPointerPress(CreatePointerEvent(0, 0, PointerEventButton::Right)))
	B3D_TEST_ASSERT(!tracker.ProcessPointerRelease(CreatePointerEvent(0, 0, PointerEventButton::Right)))
	B3D_TEST_ASSERT(front->ConsumeEvents({}))
	B3D_TEST_ASSERT(handler->ConsumeEvents({}))
	B3D_TEST_ASSERT(back->ConsumeEvents({}))

	B3D_TEST_ASSERT(tracker.ProcessPointerRelease(CreatePointerEvent()))
	B3D_TEST_ASSERT(front->ConsumeEvents({ GUIMouseEventType::MouseUp, GUIMouseEventType::MousePressEnd }))
	B3D_TEST_ASSERT(handler->ConsumeEvents({ GUIMouseEventType::MouseUp, GUIMouseEventType::MousePressEnd }))
	B3D_TEST_ASSERT(back->ConsumeEvents({ GUIMouseEventType::MousePressEnd }))

	front->Destroy();
	handler->Destroy();
	back->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestMouseUpRequiresPress()
{
	RecordingElement* const pressed = B3DNew<RecordingElement>();
	RecordingElement* const other = B3DNew<RecordingElement>();
	pressed->HandledEvents.Add(GUIMouseEventType::MouseDown);
	pressed->HandledEvents.Add(GUIMouseEventType::MouseUp);
	other->HandledEvents.Add(GUIMouseEventType::MouseUp);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ pressed }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	pressed->ConsumeEvents({});

	// The element in front was not pressed, so MouseUp skips it even though it would handle it
	tracker.UpdateElementsUnderPointer({ other, pressed }, CreatePointerEvent(), {});
	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(other->ConsumeEvents({}))
	B3D_TEST_ASSERT(pressed->ConsumeEvents({ GUIMouseEventType::MouseUp, GUIMouseEventType::MousePressEnd }))

	pressed->Destroy();
	other->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestDrag()
{
	RecordingElement* const element = B3DNew<RecordingElement>();
	element->HandledEvents.Add(GUIMouseEventType::MouseDown);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ element }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	element->ConsumeEvents({});

	// Moving no further than the drag distance does not start a drag
	tracker.ProcessPointerMove(CreatePointerEvent(2, 1), {});
	B3D_TEST_ASSERT(element->ConsumeEvents({ GUIMouseEventType::MouseMove }))

	tracker.ProcessPointerMove(CreatePointerEvent(3, 1), {});
	const GUIMouseEvent* const dragStartEvent = element->FindEvent(GUIMouseEventType::MouseDragStart);
	B3D_TEST_ASSERT(dragStartEvent != nullptr)
	B3D_TEST_ASSERT(dragStartEvent->GetPosition() == CreatePosition(3, 1))
	B3D_TEST_ASSERT(dragStartEvent->GetDragStartPosition() == CreatePosition(0, 0))
	B3D_TEST_ASSERT(element->ConsumeEvents({ GUIMouseEventType::MouseDragStart, GUIMouseEventType::MouseDrag }))

	// While dragging the element receives MouseDrag instead of MouseMove, and the wheel is ignored
	PointerEvent scrollingMove = CreatePointerEvent(5, 4);
	scrollingMove.MouseWheelScrollAmount = 1.0f;

	tracker.ProcessPointerMove(scrollingMove, {});
	const GUIMouseEvent* const dragEvent = element->FindEvent(GUIMouseEventType::MouseDrag);
	B3D_TEST_ASSERT(dragEvent != nullptr)
	B3D_TEST_ASSERT(dragEvent->GetDragAmount() == CreatePosition(2, 3))
	B3D_TEST_ASSERT(element->ConsumeEvents({ GUIMouseEventType::MouseDrag }))

	tracker.ProcessPointerRelease(CreatePointerEvent(5, 4));
	B3D_TEST_ASSERT(element->ConsumeEvents({ GUIMouseEventType::MouseUp, GUIMouseEventType::MouseDragEnd, GUIMouseEventType::MousePressEnd }))

	// Only the left button drags
	tracker.ProcessPointerPress(CreatePointerEvent(5, 4, PointerEventButton::Right));
	tracker.ProcessPointerMove(CreatePointerEvent(20, 20), {});
	tracker.ProcessPointerRelease(CreatePointerEvent(20, 20, PointerEventButton::Right));
	B3D_TEST_ASSERT(element->ConsumeEvents({ GUIMouseEventType::MousePressBegin, GUIMouseEventType::MouseDown, GUIMouseEventType::MouseMove,
		GUIMouseEventType::MouseUp, GUIMouseEventType::MousePressEnd }))

	element->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestCursor()
{
	RecordingElement* const front = B3DNew<RecordingElement>();
	RecordingElement* const back = B3DNew<RecordingElement>();
	front->HandledEvents.Add(GUIMouseEventType::MouseMove);
	back->Cursor = CursorType::IBeam;

	GUIPointerEventTracker tracker(GetGUIManager());

	tracker.UpdateElementsUnderPointer({ back }, CreatePointerEvent(1, 1), {});
	tracker.ProcessPointerMove(CreatePointerEvent(1, 1), {});
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::IBeam)

	// Elements behind the one handling MouseMove do not affect the cursor
	tracker.UpdateElementsUnderPointer({ front, back }, CreatePointerEvent(2, 2), {});
	tracker.ProcessPointerMove(CreatePointerEvent(2, 2), {});
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::Arrow)

	// While the left button is held, the cursor stays as it was
	tracker.UpdateElementsUnderPointer({ back }, CreatePointerEvent(3, 3), {});
	tracker.ProcessPointerMove(CreatePointerEvent(3, 3), {});
	tracker.ProcessPointerPress(CreatePointerEvent(3, 3));
	tracker.UpdateElementsUnderPointer({ front, back }, CreatePointerEvent(4, 3), {});
	tracker.ProcessPointerMove(CreatePointerEvent(4, 3), {});
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::IBeam)

	tracker.ProcessPointerRelease(CreatePointerEvent(4, 3));
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::Arrow)

	front->Destroy();
	back->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestDragAndDrop()
{
	RecordingElement* const source = B3DNew<RecordingElement>();
	RecordingElement* const target = B3DNew<RecordingElement>();
	RecordingElement* const rejecting = B3DNew<RecordingElement>();
	source->HandledEvents.Add(GUIMouseEventType::MouseDown);
	target->HandledEvents.Add(GUIMouseEventType::MouseDragAndDropDragged);
	target->HandledEvents.Add(GUIMouseEventType::MouseDragAndDropDropped);
	target->AcceptsDragAndDrop = true;

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ source }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	tracker.ProcessPointerMove(CreatePointerEvent(10, 0), {});

	const GUIPointerEventTracker::DragAndDropOperation dragAndDrop{ B3DMakeShared<DragAndDropData>(), true };

	tracker.UpdateElementsUnderPointer({ target }, CreatePointerEvent(20, 0), dragAndDrop);
	tracker.ProcessPointerMove(CreatePointerEvent(20, 0), dragAndDrop);
	B3D_TEST_ASSERT(target->ConsumeEvents({ GUIMouseEventType::MouseDragAndDropDragged }))
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::ArrowDrag)

	// Elements that do not accept the dragged data do not receive it, and the cursor shows the drop would be rejected
	tracker.UpdateElementsUnderPointer({ rejecting }, CreatePointerEvent(20, 5), dragAndDrop);
	tracker.ProcessPointerMove(CreatePointerEvent(20, 5), dragAndDrop);
	B3D_TEST_ASSERT(target->ConsumeEvents({ GUIMouseEventType::MouseDragAndDropLeft }))
	B3D_TEST_ASSERT(rejecting->ConsumeEvents({}))
	B3D_TEST_ASSERT(tracker.GetCursor() == CursorType::Deny)

	tracker.UpdateElementsUnderPointer({ target }, CreatePointerEvent(20, 0), dragAndDrop);
	B3D_TEST_ASSERT(tracker.ProcessDrop(CreatePointerEvent(20, 0), dragAndDrop))
	B3D_TEST_ASSERT(target->ConsumeEvents({ GUIMouseEventType::MouseDragAndDropDropped }))
	B3D_TEST_ASSERT(rejecting->ConsumeEvents({ GUIMouseEventType::MouseDragAndDropLeft }))

	source->Destroy();
	target->Destroy();
	rejecting->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestPointerLeavingWindow()
{
	RecordingElement* const first = B3DNew<RecordingElement>();
	RecordingElement* const second = B3DNew<RecordingElement>();

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ first, second }, CreatePointerEvent(), {});
	first->ConsumeEvents({});
	second->ConsumeEvents({});

	// Only the elements of the window the pointer left are no longer under it
	tracker.ProcessPointerLeftWindow({ first });
	B3D_TEST_ASSERT(first->ConsumeEvents({ GUIMouseEventType::MouseOut, GUIMouseEventType::MouseLeave }))
	B3D_TEST_ASSERT(second->ConsumeEvents({}))
	B3D_TEST_ASSERT(tracker.GetElementsUnderPointer().Size() == 1)
	B3D_TEST_ASSERT(tracker.GetElementsUnderPointer()[0].Element == second)

	first->Destroy();
	second->Destroy();
}

void GUIPointerEventTrackerTestSuite::TestDestroyedElementsAreForgotten()
{
	RecordingElement* const outer = B3DNew<RecordingElement>();
	RecordingElement* const button = B3DNew<RecordingElement>();
	RecordingElement* const other = B3DNew<RecordingElement>();
	outer->AddChild(button);
	button->HandledEvents.Add(GUIMouseEventType::MouseDown);

	GUIPointerEventTracker tracker(GetGUIManager());
	tracker.UpdateElementsUnderPointer({ button }, CreatePointerEvent(), {});
	tracker.ProcessPointerPress(CreatePointerEvent());
	outer->ConsumeEvents({});

	button->Destroy();
	tracker.RemovePendingDestroyElements();

	// With its only pressed element destroyed, the press no longer captures the pointer
	tracker.UpdateElementsUnderPointer({ other }, CreatePointerEvent(), {});
	B3D_TEST_ASSERT(other->ConsumeEvents({ GUIMouseEventType::MouseOver, GUIMouseEventType::MouseEnter }))
	B3D_TEST_ASSERT(outer->ConsumeEvents({ GUIMouseEventType::MousePressEnd, GUIMouseEventType::MouseLeave }))

	tracker.ProcessPointerRelease(CreatePointerEvent());
	B3D_TEST_ASSERT(other->ConsumeEvents({}))
	B3D_TEST_ASSERT(outer->ConsumeEvents({}))

	outer->Destroy();
	other->Destroy();
}
