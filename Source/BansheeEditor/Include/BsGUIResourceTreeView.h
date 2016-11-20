//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsServiceLocator.h"
#include "BsGUITreeView.h"
#include "BsProjectLibrary.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/** Contains paths to resources currently involved in a drag and drop operation. */
	struct DraggedResources
	{
		Vector<Path> resourcePaths;
	};

	/**	GUI element that displays all resources in the active project in a tree view. */
	class GUIResourceTreeView : public GUITreeView
	{
		/**	Tree element with resource-specific data. */
		struct ResourceTreeElement : public GUITreeView::TreeElement
		{
			Path mFullPath;
			WString mElementName;
		};

		/** Contains paths to resources currently involved in a drag and drop operation initiated by this tree view. */
		struct InternalDraggedResources
		{
			InternalDraggedResources(UINT32 numObjects);
			~InternalDraggedResources();

			UINT32 numObjects;
			Path* resourcePaths;
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new resource tree view element.
		 *
		 * @param[in]	backgroundStyle				Name of the style for the tree view background.
		 * @param[in]	elementBtnStyle				Name of the style for a normal tree view element.
		 * @param[in]	foldoutBtnStyle				Name of the style for a foldout element (for example for a folder).
		 * @param[in]	highlightBackgroundStyle	Name of the style for the background of highlighted elements.
		 * @param[in]	selectionBackgroundStyle	Name of the style for the background of selected elements.
		 * @param[in]	editBoxStyle				Name of the style for element that is being renamed.
		 * @param[in]	dragHighlightStyle			Name of the style for the element being dragged over.
		 * @param[in]	dragSepHighlightStyle		Name of the style for the separator displayed while dragging
		 *											an element between two other elements.
		 */	
		static GUIResourceTreeView* create(
			const String& backgroundStyle = StringUtil::BLANK, const String& elementBtnStyle = StringUtil::BLANK, 
			const String& foldoutBtnStyle = StringUtil::BLANK, const String& highlightBackgroundStyle = StringUtil::BLANK,
			const String& selectionBackgroundStyle = StringUtil::BLANK, const String& editBoxStyle = StringUtil::BLANK, 
			const String& dragHighlightStyle = StringUtil::BLANK, const String& dragSepHighlightStyle = StringUtil::BLANK);

		/**
		 * Creates a new resource tree view element.
		 *
		 * @param[in]	options						Options that allow you to control how is the element positioned and 
		 *											sized. This will override any similar options set by style.
		 * @param[in]	backgroundStyle				Name of the style for the tree view background.
		 * @param[in]	elementBtnStyle				Name of the style for a normal tree view element.
		 * @param[in]	foldoutBtnStyle				Name of the style for a foldout element (for example for a folder).
		 * @param[in]	highlightBackgroundStyle	Name of the style for the background of highlighted elements.
		 * @param[in]	selectionBackgroundStyle	Name of the style for the background of selected elements.
		 * @param[in]	editBoxStyle				Name of the style for element that is being renamed.
		 * @param[in]	dragHighlightStyle			Name of the style for the element being dragged over.
		 * @param[in]	dragSepHighlightStyle		Name of the style for the separator displayed while dragging an element
		 *											between two other elements.
		 */	
		static GUIResourceTreeView* create(const GUIOptions& options, 
			const String& backgroundStyle = StringUtil::BLANK, const String& elementBtnStyle = StringUtil::BLANK, 
			const String& foldoutBtnStyle = StringUtil::BLANK, const String& highlightBackgroundStyle = StringUtil::BLANK, 
			const String& selectionBackgroundStyle = StringUtil::BLANK, const String& editBoxStyle = StringUtil::BLANK, 
			const String& dragHighlightStyle = StringUtil::BLANK, const String& dragSepHighlightStyle = StringUtil::BLANK);

		/**
		 * Returns a list of paths of currently selected resources (if any). Returned paths are relative to the project
		 * folder.
		 */	
		Vector<Path> getSelection() const;

		/** Changes the active selection to the provided resources. Paths can be absolute or relative. */	
		void setSelection(const Vector<Path>& paths);

		/** Triggered whenever the selection changes. Call getSelection() to retrieve the new selection. */
		Event<void()> onSelectionChanged; 

		static const MessageId SELECTION_CHANGED_MSG;

	protected:
		virtual ~GUIResourceTreeView();

	protected:
		InternalDraggedResources* mDraggedResources;
		ResourceTreeElement mRootElement;
		RenderWindow* mCurrentWindow;
		OSDropTarget* mDropTarget;
		bool mDropTargetDragActive;

		HEvent mDropTargetEnterConn;
		HEvent mDropTargetMoveConn;
		HEvent mDropTargetLeaveConn;
		HEvent mDropTargetDroppedConn;

		GUIResourceTreeView(const String& backgroundStyle, const String& elementBtnStyle, const String& foldoutBtnStyle, 
			const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, const String& editBoxStyle,
			const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions);

		/** @copydoc GUITreeView::_updateLayoutInternal */	
		virtual void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUITreeView::getRootElement */	
		virtual TreeElement& getRootElement() override { return mRootElement; }

		/** @copydoc GUITreeView::getRootElementConst */	
		virtual const TreeElement& getRootElementConst() const override { return mRootElement; }

		/** @copydoc GUITreeView::updateTreeElementHierarchy */	
		virtual void updateTreeElementHierarchy() override;

		/** @copydoc GUITreeView::renameTreeElement */	
		virtual void renameTreeElement(TreeElement* element, const WString& name) override;

		/** @copydoc GUITreeView::deleteTreeElement */	
		virtual void deleteTreeElement(TreeElement* element) override;

		/** @copydoc GUITreeView::acceptDragAndDrop */	
		virtual bool acceptDragAndDrop() const override;

		/** @copydoc GUITreeView::dragAndDropStart */	
		virtual void dragAndDropStart(const Vector<TreeElement*>& elements) override;

		/** @copydoc GUITreeView::dragAndDropEnded */	
		virtual void dragAndDropEnded(TreeElement* overTreeElement) override;

		/** @copydoc GUITreeView::_acceptDragAndDrop */	
		virtual bool _acceptDragAndDrop(const Vector2I position, UINT32 typeId) const override;

		/** @copydoc GUITreeView::selectionChanged */	
		virtual void selectionChanged() override;

		/** @copydoc GUITreeView::_changeParentWidget */	
		void _changeParentWidget(GUIWidget* widget) override;

		/**
		 * Triggered when a drag and drop operation that was started by the tree view ends, regardless if it was processed
		 * or not.
		 */
		void dragAndDropFinalize();

		/**	Updates the contents of the provided tree entry with a project library entry. */
		void updateFromProjectLibraryEntry(ResourceTreeElement* treeElement, const ProjectLibrary::LibraryEntry* libraryEntry);

		/**
		 * Creates a new tree view entry.
		 *
		 * @param[in]	parent		Parent tree view entry to create the new one for.
		 * @param[in]	fullPath	Absolute path to the new tree entry.
		 */
		ResourceTreeElement* addTreeElement(ResourceTreeElement* parent, const Path& fullPath);

		/**	Deletes the provided tree element. */
		void deleteTreeElement(ResourceTreeElement* element);

		/**	Sorts the children of the provided tree element by name. */
		void sortTreeElement(ResourceTreeElement* element);

		/** Attempts to find a tree element with the specified path. Returns null if one cannot be found. */
		ResourceTreeElement* findTreeElement(const Path& fullPath);

		/**	Called whenever a new entry is added to the project library. */
		void entryAdded(const Path& path); 

		/**	Called whenever an entry is removed from the project library. */
		void entryRemoved(const Path& path);

		/**
		 * Sets an OS drag and drop target that allows this element to receive OS-specific drag and drop events originating
		 * from other processes.
		 */
		void setDropTarget(RenderWindow* parentWindow, INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**	Removes the currently set OS drop target. */
		void clearDropTarget();

		/**
		 * Triggered whenever the user drags the pointer over the set drop target, while OS drag and drop operation is
		 * active.
		 *
		 * @param[in]	x	X coordinate of the pointer, relative to drop area.
		 * @param[in]	y	Y coordinate of the pointer, relative to drop area.
		 */
		void dropTargetDragMove(INT32 x, INT32 y);

		/** Triggered whenever pointer leaves the drop target, while OS drag and drop operation is active. */
		void dropTargetDragLeave();

		/**
		 * Triggered whenever the user releases the pointer over the set drop target, while OS drag and drop operation is
		 * active.
		 *
		 * @param[in]	x	X coordinate of the pointer, relative to drop area.
		 * @param[in]	y	Y coordinate of the pointer, relative to drop area.
		 */
		void dropTargetDragDropped(INT32 x, INT32 y);

		/**	Generates a unique path in the case that something already exists on the provided one. */
		Path findUniquePath(const Path& path);
	};

	typedef ServiceLocator<GUIResourceTreeView> ResourceTreeViewLocator;

	/** @} */
}