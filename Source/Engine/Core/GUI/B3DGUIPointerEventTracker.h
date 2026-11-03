//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "Input/B3DInputFwd.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**
	 * Tracks the GUI elements under the pointer and the press in progress, and sends GUI elements their mouse events as
	 * those change. See GUIMouseEventType for which elements receive which events.
	 */
	class B3D_EXPORT GUIPointerEventTracker
	{
	public:
		/** An element under the pointer. */
		struct ElementUnderPointer
		{
			explicit ElementUnderPointer(GUIInteractable* element)
				: Element(element)
			{ }

			GUIInteractable* Element;
			bool UsesMouseOver = false; /**< Handled the MouseOver it received, so elements behind it do not receive one. */
			bool ReceivedMouseOver = false; /**< Received MouseOver since it moved under the pointer. */
			bool IsHovering = false; /**< Received MouseOver, and no element in front of it handles MouseOver. */
		};

		/** A drag and drop operation in progress (see DragAndDrop). */
		struct DragAndDropOperation
		{
			TShared<DragAndDropData> Data;
			bool NeedsValidDropTarget = false; /**< If true, only elements accepting the type of Data receive its events. */
		};

		/** @param	manager		Manager owning the tracker, used to convert screen positions to widget positions. */
		explicit GUIPointerEventTracker(GUIManager& manager);

		/**
		 * Replaces the elements under the pointer, and sends the MouseOver, MouseOut, MouseDragAndDropLeft, MouseEnter,
		 * MouseLeave and MousePressEnd events that result. Must be called before the Process* methods for every pointer
		 * event.
		 *
		 * @param	elementsUnderPointer	Elements under the pointer, ordered front to back.
		 * @param	event					Pointer event that moved the pointer.
		 * @param	dragAndDrop				Drag and drop operation in progress, if any.
		 * @return							True if an element handled one of the routed events.
		 */
		bool UpdateElementsUnderPointer(TArrayView<GUIInteractable* const> elementsUnderPointer, const PointerEvent& event, const TOptional<DragAndDropOperation>& dragAndDrop);

		/**
		 * Sends the MouseDragStart, MouseDrag, MouseDragAndDropDragged, MouseMove and MouseWheelScroll events for a pointer
		 * move. Returns true if an element handled one of them.
		 */
		bool ProcessPointerMove(const PointerEvent& event, const TOptional<DragAndDropOperation>& dragAndDrop);

		/**
		 * Begins a press unless the press in progress still has pressed elements, sending MousePressBegin and then
		 * MouseDown. Returns true if an element handled the MouseDown.
		 */
		bool ProcessPointerPress(const PointerEvent& event);

		/**
		 * Ends the press in progress if the released button began it, sending MouseUp, MouseDragEnd and MousePressEnd.
		 * Returns true if an element handled the MouseUp or MouseDragEnd.
		 */
		bool ProcessPointerRelease(const PointerEvent& event);

		/** Sends MouseDoubleClick. Returns true if an element handled it. */
		bool ProcessPointerDoubleClick(const PointerEvent& event);

		/** Sends MouseDragAndDropDropped. Returns true if an element handled it. */
		bool ProcessDrop(const PointerEvent& event, const DragAndDropOperation& dragAndDrop);

		/**
		 * Removes elements that are no longer under the pointer because it left their window, sending MouseOut,
		 * MouseLeave and MousePressEnd.
		 *
		 * @param	windowElements	Elements under the pointer that belong to the window the pointer left.
		 */
		void ProcessPointerLeftWindow(TArrayView<GUIInteractable* const> windowElements);

		/** Forgets every element that is queued for destruction, without sending it any events. */
		void RemovePendingDestroyElements();

		/** Returns the elements under the pointer, ordered front to back. */
		TArrayView<const ElementUnderPointer> GetElementsUnderPointer() const { return mElementsUnderPointer; }

		/** Returns the cursor the elements under the pointer want shown. */
		CursorType GetCursor() const { return mCursor; }

	private:
		/** States of the pointer button that began a press. */
		enum class DragState
		{
			NoDrag,
			HeldWithoutDrag,
			Dragging
		};

		/** An element the pointer is inside of, either directly or through an element it contains. */
		struct EnteredElement
		{
			GUIInteractable* Element;
			bool IsPressed; /**< Received MousePressBegin, and not MousePressEnd since. */
		};

		/** A press of a pointer button, from when it began until the button is released or another press replaces it. */
		struct Press
		{
			GUIMouseButton Button = GUIMouseButton::Left;
			DragState Drag = DragState::NoDrag;
			TArray<GUIInteractable*> PressedElements; /**< Elements the MouseDown was routed to. */
			TArray<GUIInteractable*> CapturedElements; /**< Elements the pointer was inside of when the press began. */
			GUIPhysicalPoint PressPosition{kZeroTag}; /**< Screen position the press began at. */
			GUIPhysicalPoint DragStartPosition{kZeroTag}; /**< Screen position MouseDragStart was sent at. */
		};

		/**
		 * Returns true while the press in progress still has pressed elements. Only the pressed elements receive MouseOver
		 * and MouseOut, and only the captured elements can be entered, until then.
		 */
		bool IsCapturing() const { return mPress && !mPress->PressedElements.Empty(); }

		/** Converts a screen position to the coordinates of the widget the element belongs to. */
		GUIPhysicalPoint ScreenToElementCoords(const GUIInteractable& element, const GUIPhysicalPoint& screenPosition) const;

		/**
		 * Offers mEvent to the elements under the pointer front to back, skipping elements queued for destruction, until
		 * one of them handles it.
		 *
		 * @param	fnPrepareEvent	Called as fnPrepareEvent(element) before offering the event to an element.
		 *							Initializes mEvent for the element, or returns false to skip the element.
		 * @return					True if an element handled the event.
		 */
		template<class FnPrepareEvent>
		bool RouteEvent(FnPrepareEvent fnPrepareEvent);

		/** Sends the element a notification of the provided type, unless it is queued for destruction. */
		static void SendNotification(GUIInteractable* element, GUIMouseEvent& event, GUIMouseEventType type);

		/**
		 * Updates the entered elements to the elements under the pointer that are hovering, plus every interactable element
		 * that contains them, sending MousePressEnd, MouseLeave and MouseEnter.
		 */
		void UpdateEnteredElements(GUIMouseEvent event);

		/** Sends MousePressEnd to the entered elements that are pressed, and ends the press in progress. */
		void EndPress(GUIMouseEvent event);

		GUIManager& mManager;

		TArray<ElementUnderPointer> mElementsUnderPointer;
		TArray<ElementUnderPointer> mNewElementsUnderPointer;
		TArray<EnteredElement> mEnteredElements;
		TArray<EnteredElement> mNewEnteredElements;
		TOptional<Press> mPress;

		/**
		 * Event sent to elements by the routed and drag events. Buttons and modifiers are only updated by MouseOver and the
		 * press, release and double click events, so the other events report the state from the last of those.
		 */
		GUIMouseEvent mEvent;

		Vector2I mLastPointerScreenPosition{kZeroTag};
		CursorType mCursor = CursorType::Arrow;
	};

	/** @} */
} // namespace b3d
