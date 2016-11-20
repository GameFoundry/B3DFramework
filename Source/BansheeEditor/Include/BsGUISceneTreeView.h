//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUITreeView.h"
#include "BsEvent.h"
#include "BsServiceLocator.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/** Contains SceneObject%s currently involved in a drag and drop operation. */
	struct BS_ED_EXPORT DraggedSceneObjects
	{
		DraggedSceneObjects(UINT32 numObjects);
		~DraggedSceneObjects();

		UINT32 numObjects;
		HSceneObject* objects;
	};

	/** GUI element that displays all SceneObject%s in the current scene in the active project in a tree view. */
	class BS_ED_EXPORT GUISceneTreeView : public GUITreeView
	{
		/**	Tree element with SceneObject%-specific data. */
		struct SceneTreeElement : public GUITreeView::TreeElement
		{
			SceneTreeElement()
				:mId(0), mIsPrefabInstance(false)
			{ }

			HSceneObject mSceneObject;
			UINT64 mId;
			bool mIsPrefabInstance;
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
		 * @param[in]	selectionBackgroundStyle	Name of the style for the background of selected elements.
		 * @param[in]	highlightBackgroundStyle	Name of the style for the background of highlighted elements.
		 * @param[in]	editBoxStyle				Name of the style for element that is being renamed.
		 * @param[in]	dragHighlightStyle			Name of the style for the element being dragged over.
		 * @param[in]	dragSepHighlightStyle		Name of the style for the separator displayed while dragging an element
		 *											between two other elements.
		 */	
		static GUISceneTreeView* create(
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
		static GUISceneTreeView* create(const GUIOptions& options, 
			const String& backgroundStyle = StringUtil::BLANK, const String& elementBtnStyle = StringUtil::BLANK, 
			const String& foldoutBtnStyle = StringUtil::BLANK, const String& highlightBackgroundStyle = StringUtil::BLANK,
			const String& selectionBackgroundStyle = StringUtil::BLANK, const String& editBoxStyle = StringUtil::BLANK, 
			const String& dragHighlightStyle = StringUtil::BLANK, const String& dragSepHighlightStyle = StringUtil::BLANK);

		/**	Returns a list of SceneObject&s currently selected (if any). */	
		Vector<HSceneObject> getSelection() const;

		/**	Changes the active selection to the provided SceneObject%s. */	
		void setSelection(const Vector<HSceneObject>& objects);

		/**	Scrolls to and highlights the selected object (does not select it). */
		void ping(const HSceneObject& object);

		/** @copydoc GUITreeView::duplicateSelection */
		void duplicateSelection() override;

		/** @copydoc GUITreeView::copySelection */
		void copySelection() override;
		
		/** @copydoc GUITreeView::cutSelection */
		void cutSelection() override;

		/** @copydoc GUITreeView::paste */
		void paste() override;

		/** Triggered whenever the selection changes. Call getSelection() to retrieve new selection. */
		Event<void()> onSelectionChanged; 

		/** 
		 * Triggered whenever the scene is modified in any way from within the scene tree view (for example object is 
		 * deleted, added, etc.).
		 */
		Event<void()> onModified; 

		/**
		 * Triggered when a resource drag and drop operation finishes over the scene tree view. Provided scene object
		 * is the tree view element that the operation finished over (or null if none), and the list of paths is the list 
		 * of relative paths of the resources that were dropped.
		 */
		Event<void(const HSceneObject&, const Vector<Path>&)> onResourceDropped;
		
		static const MessageId SELECTION_CHANGED_MSG;
	protected:
		virtual ~GUISceneTreeView();

	protected:
		SceneTreeElement mRootElement;

		GUISceneTreeView(const String& backgroundStyle, const String& elementBtnStyle, 
			const String& foldoutBtnStyle, const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, 
			const String& editBoxStyle, const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions);

		/**
		 * Checks it the SceneObject referenced by this tree element changed in any way and updates the tree element. This
		 * can involve recursing all children and updating them as well.
		 */
		void updateTreeElement(SceneTreeElement* element);

		/**
		 * Triggered when a drag and drop operation that was started by the tree view ends, regardless if it was processed
		 * or not.
		 */
		void dragAndDropFinalize();

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

		/** Deletes the internal TreeElement representation without actually deleting the referenced SceneObject. */
		void deleteTreeElementInternal(TreeElement* element);

		/**	Attempts to find a tree element referencing the specified scene object. */
		SceneTreeElement* findTreeElement(const HSceneObject& so);

		/**	Creates a new scene object as a child of the currently selected object (if any). */
		void createNewSO();

		/**	Removes all elements from the list used for copy/cut operations. */
		void clearCopyList();

		/**
		 * Cleans duplicate objects from the provided scene object list. This involves removing child elements if their
		 * parents are already part of the list.
		 */
		static void cleanDuplicates(Vector<HSceneObject>& objects);

		Vector<HSceneObject> mCopyList;
		bool mCutFlag;

		static const Color PREFAB_TINT;
	};

	typedef ServiceLocator<GUISceneTreeView> SceneTreeViewLocator;

	/** @} */
}