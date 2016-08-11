//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUISceneTreeView.h"
#include "BsSceneObject.h"
#include "BsSceneManager.h"
#include "BsGUISkin.h"
#include "BsCmdRecordSO.h"
#include "BsCmdReparentSO.h"
#include "BsCmdDeleteSO.h"
#include "BsCmdCloneSO.h"
#include "BsCmdCreateSO.h"
#include "BsDragAndDropManager.h"
#include "BsGUIResourceTreeView.h"
#include "BsGUIContextMenu.h"

namespace BansheeEngine
{
	const MessageId GUISceneTreeView::SELECTION_CHANGED_MSG = MessageId("SceneTreeView_SelectionChanged");
	const Color GUISceneTreeView::PREFAB_TINT = Color(1.0f, (168.0f / 255.0f), 0.0f, 1.0f);

	DraggedSceneObjects::DraggedSceneObjects(UINT32 numObjects)
		:numObjects(numObjects)
	{
		objects = bs_newN<HSceneObject>(numObjects);
	}

	DraggedSceneObjects::~DraggedSceneObjects()
	{
		bs_deleteN(objects, numObjects);
		objects = nullptr;
	}

	GUISceneTreeView::GUISceneTreeView(const String& backgroundStyle, const String& elementBtnStyle, 
		const String& foldoutBtnStyle, const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, 
		const String& editBoxStyle, const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions)
		:GUITreeView(backgroundStyle, elementBtnStyle, foldoutBtnStyle, highlightBackgroundStyle, selectionBackgroundStyle, editBoxStyle, dragHighlightStyle,
		dragSepHighlightStyle, dimensions), mCutFlag(false)
	{
		SceneTreeViewLocator::_provide(this);

		SPtr<GUIContextMenu> contextMenu = bs_shared_ptr_new<GUIContextMenu>();

		contextMenu->addMenuItem(L"New scene object", std::bind(&GUISceneTreeView::createNewSO, this), 50);
		contextMenu->addMenuItem(L"Rename", std::bind(&GUISceneTreeView::renameSelected, this), 49, ShortcutKey(ButtonModifier::None, BC_F2));
		contextMenu->addMenuItem(L"Delete", std::bind(&GUISceneTreeView::deleteSelection, this), 48, ShortcutKey(ButtonModifier::None, BC_DELETE));
		contextMenu->addSeparator(L"", 40);
		contextMenu->addMenuItem(L"Duplicate", std::bind(&GUISceneTreeView::duplicateSelection, this), 39, ShortcutKey(ButtonModifier::Ctrl, BC_D));
		contextMenu->addMenuItem(L"Copy", std::bind(&GUISceneTreeView::copySelection, this), 38, ShortcutKey(ButtonModifier::Ctrl, BC_C));
		contextMenu->addMenuItem(L"Cut", std::bind(&GUISceneTreeView::cutSelection, this), 37, ShortcutKey(ButtonModifier::Ctrl, BC_X));
		contextMenu->addMenuItem(L"Paste", std::bind(&GUISceneTreeView::paste, this), 36, ShortcutKey(ButtonModifier::Ctrl, BC_V));

		setContextMenu(contextMenu);
	}

	GUISceneTreeView::~GUISceneTreeView()
	{
		SceneTreeViewLocator::_remove(this);
	}

	GUISceneTreeView* GUISceneTreeView::create(const String& backgroundStyle, const String& elementBtnStyle, const String& foldoutBtnStyle, 
		const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, const String& editBoxStyle, const String& dragHighlightStyle,
		const String& dragSepHighlightStyle)
	{
		return new (bs_alloc<GUISceneTreeView>()) GUISceneTreeView(backgroundStyle, elementBtnStyle, foldoutBtnStyle, 
			highlightBackgroundStyle, selectionBackgroundStyle, editBoxStyle, dragHighlightStyle, dragSepHighlightStyle, GUIDimensions::create());
	}

	GUISceneTreeView* GUISceneTreeView::create(const GUIOptions& options, const String& backgroundStyle, const String& elementBtnStyle, 
		const String& foldoutBtnStyle, const String& highlightBackgroundStyle, const String& selectionBackgroundStyle,
		const String& editBoxStyle, const String& dragHighlightStyle, const String& dragSepHighlightStyle)
	{
		return new (bs_alloc<GUISceneTreeView>()) GUISceneTreeView(backgroundStyle, elementBtnStyle, 
			foldoutBtnStyle, highlightBackgroundStyle, selectionBackgroundStyle, editBoxStyle, 
			dragHighlightStyle, dragSepHighlightStyle, GUIDimensions::create(options));
	}

	void GUISceneTreeView::updateTreeElement(SceneTreeElement* element)
	{
		HSceneObject currentSO = element->mSceneObject;

		// Check if SceneObject has changed in any way and update the tree element

		// Early exit case - Most commonly there will be no changes between active and cached data so 
		// we first do a quick check in order to avoid expensive comparison later
		bool completeMatch = true;
		UINT32 visibleChildCount = 0;
		for (UINT32 i = 0; i < currentSO->getNumChildren(); i++)
		{
			if (i >= element->mChildren.size())
			{
				completeMatch = false;
				break;
			}

			HSceneObject currentSOChild = currentSO->getChild(i);

#if BS_DEBUG_MODE == 0
			if (currentSOChild->hasFlag(SOF_Internal))
				continue;
#endif

			SceneTreeElement* currentChild = static_cast<SceneTreeElement*>(element->mChildren[visibleChildCount]);
			visibleChildCount++;

			UINT64 curId = currentSOChild->getInstanceId();
			if (curId != currentChild->mId)
			{
				completeMatch = false;
				break;
			}
		}

		completeMatch &= visibleChildCount == element->mChildren.size();

		// Not a complete match, compare everything and insert/delete elements as needed
		bool needsUpdate = false;
		if(!completeMatch)
		{
			Vector<TreeElement*> newChildren;

			bool* tempToDelete = (bool*)bs_stack_alloc(sizeof(bool) * (UINT32)element->mChildren.size());
			for(UINT32 i = 0; i < (UINT32)element->mChildren.size(); i++)
				tempToDelete[i] = true;

			for(UINT32 i = 0; i < currentSO->getNumChildren(); i++)
			{
				HSceneObject currentSOChild = currentSO->getChild(i);
				bool isInternal = currentSOChild->hasFlag(SOF_Internal);

				HSceneObject prefabParent = currentSOChild->getPrefabParent();

				// Only count it as a prefab instance if its not scene root (otherwise every object would be colored as a prefab)
				bool isPrefabInstance = prefabParent != nullptr && prefabParent->getParent() != nullptr;

#if BS_DEBUG_MODE == 0
				if (isInternal)
					continue;
#endif

				UINT64 curId = currentSOChild->getInstanceId();
				bool found = false;

				for(UINT32 j = 0; j < element->mChildren.size(); j++)
				{
					SceneTreeElement* currentChild = static_cast<SceneTreeElement*>(element->mChildren[j]);

					if(curId == currentChild->mId)
					{
						tempToDelete[j] = false;
						currentChild->mSortedIdx = (UINT32)newChildren.size();
						newChildren.push_back(currentChild);

						found = true;
						break;
					}
				}

				if(!found)
				{
					SceneTreeElement* newChild = bs_new<SceneTreeElement>();
					newChild->mParent = element;
					newChild->mSceneObject = currentSOChild;
					newChild->mId = currentSOChild->getInstanceId();
					newChild->mName = currentSOChild->getName();
					newChild->mSortedIdx = (UINT32)newChildren.size();
					newChild->mIsVisible = element->mIsVisible && element->mIsExpanded;
					newChild->mIsDisabled = !currentSOChild->getActive();
					newChild->mTint = isInternal ? Color::Red : (isPrefabInstance ? PREFAB_TINT : Color::White);
					newChild->mIsPrefabInstance = isPrefabInstance;

					newChildren.push_back(newChild);

					updateElementGUI(newChild);
				}
			}

			for(UINT32 i = 0; i < element->mChildren.size(); i++)
			{
				if(!tempToDelete[i])
					continue;

				deleteTreeElementInternal(element->mChildren[i]);
			}

			bs_stack_free(tempToDelete);

			element->mChildren = newChildren;
			needsUpdate = true;
		}

		// Check if name needs updating
		const String& name = element->mSceneObject->getName();
		if(element->mName != name)
		{
			element->mName = name;
			needsUpdate = true;	
		}

		// Check if active state needs updating
		bool isDisabled = !element->mSceneObject->getActive();
		if(element->mIsDisabled != isDisabled)
		{
			element->mIsDisabled = isDisabled;
			needsUpdate = true;
		}

		// Check if prefab instance state needs updating
		HSceneObject prefabParent = element->mSceneObject->getPrefabParent();

		// Only count it as a prefab instance if its not scene root (otherwise every object would be colored as a prefab)
		bool isPrefabInstance = prefabParent != nullptr && prefabParent->getParent() != nullptr;
		if (element->mIsPrefabInstance != isPrefabInstance)
		{
			element->mIsPrefabInstance = isPrefabInstance;

			bool isInternal = element->mSceneObject->hasFlag(SOF_Internal);
			element->mTint = isInternal ? Color::Red : (isPrefabInstance ? PREFAB_TINT : Color::White);

			needsUpdate = true;
		}

		if(needsUpdate)
			updateElementGUI(element);

		for(UINT32 i = 0; i < (UINT32)element->mChildren.size(); i++)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(element->mChildren[i]);
			updateTreeElement(sceneElement);
		}

		// Calculate the sorted index of the elements based on their name
		bs_frame_mark();
		FrameVector<SceneTreeElement*> sortVector;
		for (auto& child : element->mChildren)
			sortVector.push_back(static_cast<SceneTreeElement*>(child));

		std::sort(sortVector.begin(), sortVector.end(),
			[&](const SceneTreeElement* lhs, const SceneTreeElement* rhs)
		{
			return StringUtil::compare(lhs->mName, rhs->mName, false) < 0;
		});

		UINT32 idx = 0;
		for (auto& child : sortVector)
		{
			child->mSortedIdx = idx;
			idx++;
		}

		bs_frame_clear();
	}

	void GUISceneTreeView::updateTreeElementHierarchy()
	{
		HSceneObject root = gCoreSceneManager().getRootNode();
		mRootElement.mSceneObject = root;
		mRootElement.mId = root->getInstanceId();
		mRootElement.mSortedIdx = 0;
		mRootElement.mIsExpanded = true;

		updateTreeElement(&mRootElement);
	}

	void GUISceneTreeView::renameTreeElement(GUITreeView::TreeElement* element, const WString& name)
	{
		SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(element);

		HSceneObject so = sceneTreeElement->mSceneObject;
		CmdRecordSO::execute(so, false, L"Renamed \"" + toWString(so->getName()) + L"\"");
		so->setName(toString(name));

		onModified();
	}

	void GUISceneTreeView::deleteTreeElement(TreeElement* element)
	{
		SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(element);

		HSceneObject so = sceneTreeElement->mSceneObject;
		CmdDeleteSO::execute(so, L"Deleted \"" + toWString(so->getName()) + L"\"");

		onModified();
	}

	void GUISceneTreeView::deleteTreeElementInternal(GUITreeView::TreeElement* element)
	{
		closeTemporarilyExpandedElements(); // In case this element is one of them

		if (element->mIsHighlighted)
			clearPing();

		if(element->mIsSelected)
			unselectElement(element);

		bs_delete(element);
	}

	bool GUISceneTreeView::acceptDragAndDrop() const
	{
		return DragAndDropManager::instance().isDragInProgress() && 
			(DragAndDropManager::instance().getDragTypeId() == (UINT32)DragAndDropType::SceneObject ||
			DragAndDropManager::instance().getDragTypeId() == (UINT32)DragAndDropType::Resources);
	}

	void GUISceneTreeView::dragAndDropStart(const Vector<TreeElement*>& elements)
	{
		DraggedSceneObjects* draggedSceneObjects = bs_new<DraggedSceneObjects>((UINT32)elements.size());

		UINT32 cnt = 0;
		for(auto& entry : elements)
		{
			SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(entry);
			draggedSceneObjects->objects[cnt] = sceneTreeElement->mSceneObject;
			cnt++;
		}

		DragAndDropManager::instance().startDrag((UINT32)DragAndDropType::SceneObject, (void*)draggedSceneObjects, 
			std::bind(&GUISceneTreeView::dragAndDropFinalize, this), false);
	}

	void GUISceneTreeView::dragAndDropEnded(TreeElement* overTreeElement)
	{
		UINT32 dragTypeId = DragAndDropManager::instance().getDragTypeId();

		if (dragTypeId == (UINT32)DragAndDropType::SceneObject)
		{
			if (overTreeElement != nullptr)
			{
				DraggedSceneObjects* draggedSceneObjects = reinterpret_cast<DraggedSceneObjects*>(DragAndDropManager::instance().getDragData());

				Vector<HSceneObject> sceneObjects;
				SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(overTreeElement);
				HSceneObject newParent = sceneTreeElement->mSceneObject;

				for (UINT32 i = 0; i < draggedSceneObjects->numObjects; i++)
				{
					if (draggedSceneObjects->objects[i] != newParent)
						sceneObjects.push_back(draggedSceneObjects->objects[i]);
				}

				CmdReparentSO::execute(sceneObjects, newParent);
				onModified();
			}
		}
		else if (dragTypeId == (UINT32)DragAndDropType::Resources)
		{
			DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(DragAndDropManager::instance().getDragData());

			HSceneObject newParent;
			if (overTreeElement != nullptr)
			{
				SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(overTreeElement);
				newParent = sceneTreeElement->mSceneObject;
			}

			onResourceDropped(newParent, draggedResources->resourcePaths);
		}
	}

	void GUISceneTreeView::dragAndDropFinalize()
	{
		mDragInProgress = false;
		_markLayoutAsDirty();

		if (DragAndDropManager::instance().getDragTypeId() == (UINT32)DragAndDropType::SceneObject)
		{
			DraggedSceneObjects* draggedSceneObjects = reinterpret_cast<DraggedSceneObjects*>(DragAndDropManager::instance().getDragData());
			bs_delete(draggedSceneObjects);
		}
	}

	bool GUISceneTreeView::_acceptDragAndDrop(const Vector2I position, UINT32 typeId) const
	{
		return (typeId == (UINT32)DragAndDropType::SceneObject || typeId == (UINT32)DragAndDropType::Resources) && !_isDisabled();
	}

	void GUISceneTreeView::selectionChanged()
	{
		onSelectionChanged();
		sendMessage(SELECTION_CHANGED_MSG);
	}

	Vector<HSceneObject> GUISceneTreeView::getSelection() const
	{
		Vector<HSceneObject> selectedSOs;
		for (auto& selectedElem : mSelectedElements)
		{
			SceneTreeElement* sceneTreeElement = static_cast<SceneTreeElement*>(selectedElem.element);

			selectedSOs.push_back(sceneTreeElement->mSceneObject);
		}

		return selectedSOs;
	}

	void GUISceneTreeView::setSelection(const Vector<HSceneObject>& objects)
	{
		unselectAll(false);

		// Note: I could queue the selection update until after the next frame in order to avoid the hierarchy update here
		// for better performance.
		updateTreeElementHierarchy();

		Stack<SceneTreeElement*> todo;
		todo.push(&mRootElement);

		while (!todo.empty())
		{
			SceneTreeElement* currentElem = todo.top();
			todo.pop();

			auto iterFind = std::find(objects.begin(), objects.end(), currentElem->mSceneObject);
			if (iterFind != objects.end())
			{
				expandToElement(currentElem);
				selectElement(currentElem);
			}

			for (auto& child : currentElem->mChildren)
			{
				SceneTreeElement* sceneChild = static_cast<SceneTreeElement*>(child);
				todo.push(sceneChild);
			}
		}
	}

	void GUISceneTreeView::ping(const HSceneObject& object)
	{
		Stack<SceneTreeElement*> todo;
		todo.push(&mRootElement);

		while (!todo.empty())
		{
			SceneTreeElement* currentElem = todo.top();
			todo.pop();

			if (currentElem->mSceneObject == object)
			{
				GUITreeView::ping(currentElem);
				break;
			}

			for (auto& child : currentElem->mChildren)
			{
				SceneTreeElement* sceneChild = static_cast<SceneTreeElement*>(child);
				todo.push(sceneChild);
			}
		}
	}

	GUISceneTreeView::SceneTreeElement* GUISceneTreeView::findTreeElement(const HSceneObject& so)
	{
		Stack<SceneTreeElement*> todo;
		todo.push(&mRootElement);

		while (!todo.empty())
		{
			SceneTreeElement* currentElem = todo.top();
			todo.pop();

			if (so == currentElem->mSceneObject)
				return currentElem;

			for (auto& child : currentElem->mChildren)
			{
				SceneTreeElement* sceneChild = static_cast<SceneTreeElement*>(child);
				todo.push(sceneChild);
			}
		}

		return nullptr;
	}

	void GUISceneTreeView::duplicateSelection()
	{
		Vector<HSceneObject> duplicateList;
		for (auto& selectedElem : mSelectedElements)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(selectedElem.element);
			duplicateList.push_back(sceneElement->mSceneObject);
		}

		cleanDuplicates(duplicateList);

		if (duplicateList.size() == 0)
			return;

		WString message;
		if (duplicateList.size() == 1)
			message = L"Duplicated " + toWString(duplicateList[0]->getName());
		else
			message = L"Duplicated " + toWString(duplicateList.size()) + L" elements";

		CmdCloneSO::execute(duplicateList, message);
		onModified();
	}

	void GUISceneTreeView::copySelection()
	{
		clearCopyList();

		for (auto& selectedElem : mSelectedElements)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(selectedElem.element);
			mCopyList.push_back(sceneElement->mSceneObject);
		}

		mCutFlag = false;
	}

	void GUISceneTreeView::cutSelection()
	{
		clearCopyList();

		for (auto& selectedElem : mSelectedElements)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(selectedElem.element);
			mCopyList.push_back(sceneElement->mSceneObject);

			sceneElement->mIsCut = true;
			updateElementGUI(sceneElement);
		}

		mCutFlag = true;
		_markLayoutAsDirty();
	}

	void GUISceneTreeView::paste()
	{
		cleanDuplicates(mCopyList);

		if (mCopyList.size() == 0)
			return;

		HSceneObject parent = mRootElement.mSceneObject;
		if (mSelectedElements.size() > 0)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(mSelectedElements[0].element);
			parent = sceneElement->mSceneObject;
		}

		if (mCutFlag)
		{
			WString message;
			if (mCopyList.size() == 1)
				message = L"Moved " + toWString(mCopyList[0]->getName());
			else
				message = L"Moved " + toWString(mCopyList.size()) + L" elements";

			CmdReparentSO::execute(mCopyList, parent, message);
			clearCopyList();
		}
		else
		{
			WString message;
			if (mCopyList.size() == 1)
				message = L"Copied " + toWString(mCopyList[0]->getName());
			else
				message = L"Copied " + toWString(mCopyList.size()) + L" elements";

			Vector<HSceneObject> clones = CmdCloneSO::execute(mCopyList, message);
			for (auto& clone : clones)
				clone->setParent(parent);
		}

		onModified();
	}

	void GUISceneTreeView::clearCopyList()
	{
		for (auto& so : mCopyList)
		{
			if (so.isDestroyed())
				continue;

			TreeElement* treeElem = findTreeElement(so);

			if (treeElem != nullptr)
			{
				treeElem->mIsCut = false;
				updateElementGUI(treeElem);
			}
		}

		mCopyList.clear();
		_markLayoutAsDirty();
	}

	void GUISceneTreeView::createNewSO()
	{
		HSceneObject newSO = CmdCreateSO::execute("New", 0, L"Created a new SceneObject");

		if (mSelectedElements.size() > 0)
		{
			SceneTreeElement* sceneElement = static_cast<SceneTreeElement*>(mSelectedElements[0].element);
			newSO->setParent(sceneElement->mSceneObject);
		}

		updateTreeElementHierarchy();

		TreeElement* newTreeElement = findTreeElement(newSO);
		expandToElement(newTreeElement);
		setSelection({ newSO });
		renameSelected();

		onModified();
	}

	void GUISceneTreeView::cleanDuplicates(Vector<HSceneObject>& objects)
	{
		auto isChildOf = [&](const HSceneObject& parent, const HSceneObject& child)
		{
			HSceneObject elem = child;

			while (elem != nullptr && elem != parent)
				elem = elem->getParent();

			return elem == parent;
		};

		Vector<HSceneObject> cleanList;
		for (UINT32 i = 0; i < (UINT32)objects.size(); i++)
		{
			bool foundParent = false;
			for (UINT32 j = 0; j < (UINT32)objects.size(); j++)
			{
				if (i != j && isChildOf(objects[j], objects[i]))
				{
					foundParent = true;
					break;
				}
			}

			if (!foundParent)
				cleanList.push_back(objects[i]);
		}

		objects = cleanList;
	}

	const String& GUISceneTreeView::getGUITypeName()
	{
		static String typeName = "SceneTreeView";
		return typeName;
	}
}