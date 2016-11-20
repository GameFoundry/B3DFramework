//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"
#include "BsVirtualInput.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/**
	 * GUI element that displays some contents in a tree-view where elements are placed vertically above/beneath each other,
	 * and different elements may be nested within other elements.
	 *
	 * Elements may be selected, renamed, dragged and re-parented.
	 *
	 * This class is abstract and meant to be extended by an implementation specific to some content type (for example scene
	 * object hierarchy). 
	 */
	class BS_ED_EXPORT GUITreeView : public GUIElementContainer
	{
	protected:
		/**	Types of possible states when the tree view is auto-scrolling. */
		enum class ScrollState
		{
			None,
			Up,
			Down,
			TransitioningUp,
			TransitioningDown
		};

		/**
		 * Contains data about a single piece of content and all its children. This element may be visible and represented
		 * by a GUI element, but might not (for example its parent is collapsed).
		 */
		struct TreeElement
		{
			TreeElement();
			virtual ~TreeElement();

			TreeElement* mParent;
			Vector<TreeElement*> mChildren;

			GUIToggle* mFoldoutBtn;
			GUILabel* mElement;

			String mName;

			UINT32 mSortedIdx;
			bool mIsExpanded;
			bool mIsSelected;
			bool mIsHighlighted;
			bool mIsVisible;
			bool mIsCut;
			bool mIsDisabled;
			Color mTint;

			bool isParentRec(TreeElement* element) const;
		};

		/**
		 * Contains data about all visible elements in the tree view. This might be a TreeElement entry, or a separator
		 * (empty space) between two TreeElement%s.
		 */
		struct InteractableElement
		{
			InteractableElement(TreeElement* parent, UINT32 index, const Rect2I& bounds)
				:parent(parent), index(index), bounds(bounds)
			{ }

			bool isTreeElement() const { return index % 2 == 1; }
			TreeElement* getTreeElement() const;

			TreeElement* parent;
			UINT32 index;
			Rect2I bounds;
		};

		/**	Contains data about one of the currently selected tree elements. */
		struct SelectedElement
		{
			SelectedElement()
				:element(nullptr), background(nullptr)
			{ }

			SelectedElement(TreeElement* elem, GUITexture* back)
				:element(elem), background(back)
			{ }

			TreeElement* element;
			GUITexture* background;
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Deletes all currently selected elements. */
		void deleteSelection();

		/** Duplicates the currently selected entries. */
		virtual void duplicateSelection() { }

		/**	Marks the current selection for copying. */
		virtual void copySelection() { }
		
		/**	Marks the current selection for cutting. */
		virtual void cutSelection() { }

		/**	Pastes a set of entries previously marked for cut or copy. */
		virtual void paste() { }

		/**	Starts rename operation on the currently selected element. */
		void renameSelected();

		/** @name Internal
		 *  @{
		 */

		/** Updates tree view if dirty, among other operations. Must be called every frame. */
		void _update();

		/** @} */

	protected:
		GUITreeView(const String& backgroundStyle, const String& elementBtnStyle, 
			const String& foldoutBtnStyle, const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, const String& editBoxStyle,
			const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions);

		virtual ~GUITreeView();

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElement::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElement::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElement::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIElement::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

		/**
		 * Attempts to find an interactable element under the specified coordinates. Returns null if one cannot be found.
		 *
		 * @param[in]	coord	Coordinates relative to parent GUI widget.
		 */
		const InteractableElement* findElementUnderCoord(const Vector2I& coord) const;

		/**	Returns the top-most selected tree element if selection is active, null otherwise. */
		TreeElement* getTopMostSelectedElement() const;

		/** Returns the bottom-most selected tree element if selection is active, null otherwise. */
		TreeElement* getBottomMostSelectedElement() const;

		/**	Starts rename operation on the specified tree element, spawning an input box. */
		void enableEdit(TreeElement* element);

		/**
		 * Ends rename operation if one is currently active.
		 *
		 * @param[in]	acceptChanges	Should the new name be accepted or discarded.
		 */
		void disableEdit(bool acceptChanges);

		/**
		 * Triggered when the Foldout button for a tree element was been toggled (something was expanded or collapsed).
		 */
		void elementToggled(TreeElement* element, bool toggled);

		/**	Returns the top level TreeElement. */
		virtual TreeElement& getRootElement() = 0;

		/**	Returns the top level TreeElement that cannot be modified. */
		virtual const TreeElement& getRootElementConst() const = 0;

		/** Checks if the hierarchy needs any updates and performs those updates if needed. */
		virtual void updateTreeElementHierarchy() = 0;

		/**	Changes the name of the content associated with the provided tree element. */
		virtual void renameTreeElement(TreeElement* element, const WString& name) = 0;

		/**	Deletes the content associated with the provided tree element. */
		virtual void deleteTreeElement(TreeElement* element) = 0;

		/**	Checks whether the tree view can accept the currently active drag and drop operation. */
		virtual bool acceptDragAndDrop() const = 0;

		/**	Triggered when the user drags a tree element and starts a drag and drop operation. */
		virtual void dragAndDropStart(const Vector<TreeElement*>& elements) = 0;

		/**
		 * Triggered when the user ends a drag and drop operation over the tree view.
		 *
		 * @param[in]	overTreeElement	TreeElement the drag operation ended over, if any.
		 */
		virtual void dragAndDropEnded(TreeElement* overTreeElement) = 0;

		/**	Triggered whenever a TreeElement gets selected or deselected. */
		virtual void selectionChanged() { }

		/**	Are any tree elements currently selected. */
		bool isSelectionActive() const;

		/**	Expands the selection to the provided TreeElement. Doesn't clear previous selection. */
		void selectElement(TreeElement* element);

		/**	Unselects the provided TreeElement. */
		void unselectElement(TreeElement* element);

		/**
		 * Unselects all selected TreeElement%s.
		 *
		 * @param[in]	sendEvent	Determines should the external world be notified of this change.
		 */
		void unselectAll(bool sendEvent = true);

		/**	Expands all parents of the provided TreeElement making it interactable. */
		void expandToElement(TreeElement* element);

		/**	Expands the provided TreeElement making its children interactable. */
		void expandElement(TreeElement* element);

		/**	Collapses the provided TreeElement making its children hidden and not interactable. */
		void collapseElement(TreeElement* element);

		/**	Rebuilds the needed GUI elements for the provided TreeElement. */
		void updateElementGUI(TreeElement* element);

		/**	Close any elements that were temporarily expanded due to a drag operation hovering over them. */
		void closeTemporarilyExpandedElements();

		/**
		 * Temporarily expand the provided element. Temporarily expanded elements can be closed by calling
		 * closeTemporarilyExpandedElements().
		 */
		void temporarilyExpandElement(const GUITreeView::InteractableElement* mouseOverElement);

		/**
		 * Scrolls the parent GUIScrollArea (if any) so that the specified TreeElement is visible.
		 *
		 * @param[in]	element	Element to scroll to.
		 * @param[in]	center	If true the element will be centered in the scroll view, otherwise it will be at the top. 
		 */
		void scrollToElement(TreeElement* element, bool center);

		/**	Attempts to find the parent GUIScrollArea that the tree view is a child of. */
		GUIScrollArea* findParentScrollArea() const;

		/**	Scrolls the tree view to the specified element and highlights it. */
		void ping(TreeElement* element);

		/**	Clears the ping highlight on the currently highlighted element. */
		void clearPing();

		/**	Triggered when the user accepts the changes during a rename operation. */
		void onEditAccepted();

		/**	Triggered when the user rejects the changes during a rename operation. */
		void onEditCanceled();

		/**	Triggered when the user clicks outside of the edit box during a rename operation. */
		void onEditFocusLost();

		String mBackgroundStyle;
		String mElementBtnStyle;
		String mFoldoutBtnStyle;
		String mSelectionBackgroundStyle;
		String mHighlightBackgroundStyle;
		String mEditBoxStyle;
		String mDragHighlightStyle;
		String mDragSepHighlightStyle;

		GUITexture* mBackgroundImage;

		Vector<InteractableElement> mVisibleElements;

		bool mIsElementSelected;
		Vector<SelectedElement> mSelectedElements;

		bool mIsElementHighlighted;
		SelectedElement mHighlightedElement;

		TreeElement* mEditElement;
		GUITreeViewEditBox* mNameEditBox;

		Vector2I mDragStartPosition;
		Vector2I mDragPosition;
		bool mDragInProgress;
		GUITexture* mDragHighlight;
		GUITexture* mDragSepHighlight;

		Rect2I mTopScrollBounds;
		Rect2I mBottomScrollBounds;
		ScrollState mScrollState;
		float mLastScrollTime;

		Stack<TreeElement*> mAutoExpandedElements;
		TreeElement* mMouseOverDragElement;
		float mMouseOverDragElementTime;

		static const UINT32 ELEMENT_EXTRA_SPACING;
		static const UINT32 INDENT_SIZE;
		static const UINT32 INITIAL_INDENT_OFFSET;
		static const UINT32 DRAG_MIN_DISTANCE;
		static const float AUTO_EXPAND_DELAY_SEC;
		static const float SCROLL_AREA_HEIGHT_PCT;
		static const UINT32 SCROLL_SPEED_PX_PER_SEC;
		static const Color CUT_COLOR;
		static const Color DISABLED_COLOR;
	};

	/** @} */
}