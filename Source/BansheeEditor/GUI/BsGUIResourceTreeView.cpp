//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "GUI/BsGUIResourceTreeView.h"
#include "Library/BsProjectLibrary.h"
#include "GUI/BsDragAndDropManager.h"
#include "FileSystem/BsFileSystem.h"
#include "GUI/BsGUIWidget.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Platform/BsDropTarget.h"

using namespace std::placeholders;

namespace bs
{
	const MessageId GUIResourceTreeView::SELECTION_CHANGED_MSG = MessageId("ResourceTreeView_SelectionChanged");

	GUIResourceTreeView::InternalDraggedResources::InternalDraggedResources(UINT32 numObjects)
		:numObjects(numObjects)
	{
		resourcePaths = bs_newN<Path>(numObjects);
	}

	GUIResourceTreeView::InternalDraggedResources::~InternalDraggedResources()
	{
		bs_deleteN(resourcePaths, numObjects);
		resourcePaths = nullptr;
	}

	GUIResourceTreeView::GUIResourceTreeView(const String& backgroundStyle, const String& elementBtnStyle, const String& foldoutBtnStyle, 
		const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, const String& editBoxStyle,
		const String& dragHighlightStyle, const String& dragSepHighlightStyle, const GUIDimensions& dimensions)
		:GUITreeView(backgroundStyle, elementBtnStyle, foldoutBtnStyle, highlightBackgroundStyle, 
		selectionBackgroundStyle, editBoxStyle, dragHighlightStyle, dragSepHighlightStyle, dimensions), 
		mDraggedResources(nullptr), mCurrentWindow(nullptr), mDropTarget(nullptr), mDropTargetDragActive(false)
	{
		ResourceTreeViewLocator::_provide(this);

		gProjectLibrary().onEntryAdded.connect(std::bind(&GUIResourceTreeView::entryAdded, this, _1));
		gProjectLibrary().onEntryRemoved.connect(std::bind(&GUIResourceTreeView::entryRemoved, this, _1));

		const ProjectLibrary::LibraryEntry* rootEntry = gProjectLibrary().getRootEntry();

		mRootElement.mFullPath = rootEntry->path;
		mRootElement.mElementName = mRootElement.mFullPath.getWTail();

		expandElement(&mRootElement);

		updateFromProjectLibraryEntry(&mRootElement, rootEntry);
	}

	GUIResourceTreeView::~GUIResourceTreeView()
	{
		clearDropTarget();

		ResourceTreeViewLocator::_provide(nullptr);
	}

	GUIResourceTreeView* GUIResourceTreeView::create(const String& backgroundStyle, const String& elementBtnStyle, 
		const String& foldoutBtnStyle, const String& highlightBackgroundStyle, const String& selectionBackgroundStyle, 
		const String& editBoxStyle, const String& dragHighlightStyle, const String& dragSepHighlightStyle)
	{
		return new (bs_alloc<GUIResourceTreeView>()) GUIResourceTreeView(backgroundStyle, elementBtnStyle, foldoutBtnStyle, 
			highlightBackgroundStyle, selectionBackgroundStyle, editBoxStyle, dragHighlightStyle, dragSepHighlightStyle, GUIDimensions::create());
	}

	GUIResourceTreeView* GUIResourceTreeView::create(const GUIOptions& options, const String& backgroundStyle,
		const String& elementBtnStyle, const String& foldoutBtnStyle, const String& highlightBackgroundStyle, 
		const String& selectionBackgroundStyle, const String& editBoxStyle, const String& dragHighlightStyle,
		const String& dragSepHighlightStyle)
	{
		return new (bs_alloc<GUIResourceTreeView>()) GUIResourceTreeView(backgroundStyle, elementBtnStyle, foldoutBtnStyle, 
			highlightBackgroundStyle, selectionBackgroundStyle, editBoxStyle, dragHighlightStyle, dragSepHighlightStyle, GUIDimensions::create(options));
	}

	void GUIResourceTreeView::_updateLayoutInternal(const GUILayoutData& data)
	{
		GUITreeView::_updateLayoutInternal(data);

		if(mDropTarget != nullptr)
			mDropTarget->setArea(data.area);
	}

	void GUIResourceTreeView::updateTreeElementHierarchy()
	{
		// Do nothing, updates are handled via callbacks
	}

	void GUIResourceTreeView::renameTreeElement(GUITreeView::TreeElement* element, const WString& name)
	{
		ResourceTreeElement* resourceTreeElement = static_cast<ResourceTreeElement*>(element);
		
		Path oldPath = resourceTreeElement->mFullPath;
		Path newPath = oldPath.getParent();
		newPath.append(name);

		gProjectLibrary().moveEntry(oldPath, findUniquePath(newPath));
	}

	void GUIResourceTreeView::deleteTreeElement(TreeElement* element) 
	{
		ResourceTreeElement* resourceTreeElement = static_cast<ResourceTreeElement*>(element);

		gProjectLibrary().deleteEntry(resourceTreeElement->mFullPath);
	}

	void GUIResourceTreeView::updateFromProjectLibraryEntry(ResourceTreeElement* treeElement, const ProjectLibrary::LibraryEntry* libraryEntry)
	{
		struct StackElem
		{
			StackElem(const ProjectLibrary::LibraryEntry* entry, ResourceTreeElement* treeElem)
				:entry(entry), treeElem(treeElem)
			{ }

			const ProjectLibrary::LibraryEntry* entry;
			ResourceTreeElement* treeElem;
		};

		if(libraryEntry->type == ProjectLibrary::LibraryEntryType::Directory)
		{
			Stack<StackElem> todo;
			todo.push(StackElem(libraryEntry, treeElement));

			while(!todo.empty())
			{
				StackElem curElem = todo.top();
				todo.pop();

				const ProjectLibrary::DirectoryEntry* dirEntry = static_cast<const ProjectLibrary::DirectoryEntry*>(curElem.entry);

				for(auto& child : dirEntry->mChildren)
				{
					ResourceTreeElement* newChild = addTreeElement(curElem.treeElem, child->path);

					if(child->type == ProjectLibrary::LibraryEntryType::Directory)
						todo.push(StackElem(child, newChild));
				}

				sortTreeElement(curElem.treeElem);
			}
		}
	}

	GUIResourceTreeView::ResourceTreeElement* GUIResourceTreeView::addTreeElement(ResourceTreeElement* parent, const Path& fullPath)
	{
		ResourceTreeElement* newChild = bs_new<ResourceTreeElement>();
		newChild->mParent = parent;
		newChild->mName = fullPath.getTail();
		newChild->mFullPath = fullPath;
		newChild->mSortedIdx = (UINT32)parent->mChildren.size();
		newChild->mIsVisible = parent->mIsVisible && parent->mIsExpanded;
		newChild->mElementName = fullPath.getWTail();

		parent->mChildren.push_back(newChild);

		updateElementGUI(parent);
		updateElementGUI(newChild);

		return newChild;
	}

	void GUIResourceTreeView::deleteTreeElement(ResourceTreeElement* element)
	{
		closeTemporarilyExpandedElements(); // In case this element is one of them

		if (element->mIsHighlighted)
			clearPing();

		if(element->mIsSelected)
			unselectElement(element);

		if(element->mParent != nullptr)
		{
			auto iterFind = std::find(element->mParent->mChildren.begin(), element->mParent->mChildren.end(), element);
			if(iterFind != element->mParent->mChildren.end())
				element->mParent->mChildren.erase(iterFind);

			sortTreeElement(static_cast<ResourceTreeElement*>(element->mParent));
			updateElementGUI(element->mParent);
		}

		if(&mRootElement != element)
			bs_delete(element);
	}

	void GUIResourceTreeView::sortTreeElement(ResourceTreeElement* element)
	{
		auto cmp = [&] (const TreeElement* a, const TreeElement* b)
		{
			return a->mName.compare(b->mName) < 0;
		};

		std::sort(element->mChildren.begin(), element->mChildren.end(), cmp);

		UINT32 idx = 0;
		for(auto& child : element->mChildren)
		{
			child->mSortedIdx = idx;
			idx++;
		}
	}

	GUIResourceTreeView::ResourceTreeElement* GUIResourceTreeView::findTreeElement(const Path& fullPath)
	{
		if (!mRootElement.mFullPath.includes(fullPath))
			return nullptr;

		Path relPath = fullPath.getRelative(mRootElement.mFullPath);
		UINT32 numElems = relPath.getNumDirectories() + (relPath.isFile() ? 1 : 0);
		UINT32 idx = 0;

		ResourceTreeElement* current = &mRootElement;
		while (current != nullptr)
		{
			if (idx == numElems)
				return current;

			WString curElem;
			if (relPath.isFile() && idx == (numElems - 1))
				curElem = relPath.getWFilename();
			else
				curElem = relPath[idx];

			bool foundChild = false;
			for (auto& child : current->mChildren)
			{
				ResourceTreeElement* resourceChild = static_cast<ResourceTreeElement*>(child);
				if (Path::comparePathElem(curElem, resourceChild->mElementName))
				{
					idx++;
					current = resourceChild;
					foundChild = true;
					break;
				}
			}

			if (!foundChild)
				current = nullptr;
		}

		return nullptr;
	}

	void GUIResourceTreeView::entryAdded(const Path& path)
	{
		Path parentPath = path.getParent();

		ResourceTreeElement* parentElement = findTreeElement(parentPath);
		assert(parentElement != nullptr);

		ResourceTreeElement* newElement = addTreeElement(parentElement, path);
		sortTreeElement(parentElement);

		ProjectLibrary::LibraryEntry* libEntry = gProjectLibrary().findEntry(path);
		
		assert(libEntry != nullptr);
		updateFromProjectLibraryEntry(newElement, libEntry);

		_markLayoutAsDirty();
	}

	void GUIResourceTreeView::entryRemoved(const Path& path)
	{
		ResourceTreeElement* treeElement = findTreeElement(path);
		
		if(treeElement != nullptr)
			deleteTreeElement(treeElement);
	}

	void GUIResourceTreeView::setDropTarget(RenderWindow* parentWindow, INT32 x, INT32 y, UINT32 width, UINT32 height)
	{
		if(mDropTarget != nullptr)
		{
			mDropTarget = nullptr;

			mDropTargetEnterConn.disconnect();
			mDropTargetLeaveConn.disconnect();
			mDropTargetMoveConn.disconnect();
			mDropTargetDroppedConn.disconnect();
		}

		if(parentWindow != nullptr)
		{
			mCurrentWindow = parentWindow;
			mDropTarget = DropTarget::create(mCurrentWindow, mLayoutData.area);

			mDropTargetEnterConn = mDropTarget->onEnter.connect(std::bind(&GUIResourceTreeView::dropTargetDragMove, this, _1, _2));
			mDropTargetMoveConn = mDropTarget->onDragOver.connect(std::bind(&GUIResourceTreeView::dropTargetDragMove, this, _1, _2));
			mDropTargetLeaveConn = mDropTarget->onLeave.connect(std::bind(&GUIResourceTreeView::dropTargetDragLeave, this));
			mDropTargetDroppedConn = mDropTarget->onDrop.connect(std::bind(&GUIResourceTreeView::dropTargetDragDropped, this, _1, _2));
		}
		else
			mDropTarget = nullptr;
	}

	void GUIResourceTreeView::clearDropTarget()
	{
		setDropTarget(nullptr, 0, 0, 0, 0);
	}

	void GUIResourceTreeView::dropTargetDragMove(INT32 x, INT32 y)
	{
		mDragPosition = Vector2I(x, y);
		mDragInProgress = true;
		mDropTargetDragActive = true;
		_markLayoutAsDirty();

		if(mBottomScrollBounds.contains(mDragPosition))
		{
			if(mScrollState != ScrollState::Down)
				mScrollState = ScrollState::TransitioningDown;
		}
		else if(mTopScrollBounds.contains(mDragPosition))
		{
			if(mScrollState != ScrollState::Up)
				mScrollState = ScrollState::TransitioningUp;
		}
		else
			mScrollState = ScrollState::None;
	}

	void GUIResourceTreeView::dropTargetDragLeave()
	{
		mDragInProgress = false;
		mDropTargetDragActive = false;
		_markLayoutAsDirty();
	}

	void GUIResourceTreeView::dropTargetDragDropped(INT32 x, INT32 y)
	{
		const GUITreeView::InteractableElement* element = findElementUnderCoord(Vector2I(x, y));

		TreeElement* treeElement = nullptr;
		if(element != nullptr)
		{
			if(element->isTreeElement())
				treeElement = element->getTreeElement();
			else
				treeElement = element->parent;
		}

		if(mDropTarget->getDropType() == DropTargetType::FileList)
		{
			Vector<Path> fileList = mDropTarget->getFileList();

			mDraggedResources = bs_new<InternalDraggedResources>((UINT32)fileList.size());
			for(UINT32 i = 0; i < (UINT32)fileList.size(); i++)
				mDraggedResources->resourcePaths[i] = fileList[i];

			dragAndDropEnded(treeElement);

			bs_delete(mDraggedResources);
			mDraggedResources = nullptr;

			unselectAll();
		}

		mDragInProgress = false;
		mDropTargetDragActive = false;
		_markLayoutAsDirty();
	}

	Path GUIResourceTreeView::findUniquePath(const Path& path)
	{
		if(FileSystem::exists(path))
		{
			Path newPath = path;
			WString filename = path.getWFilename(false);
			UINT32 cnt = 1;
			do 
			{
				newPath.setBasename(filename + toWString(cnt));
				cnt++;
			} while (FileSystem::exists(newPath));

			return newPath;
		}
		else
			return path;
	}

	bool GUIResourceTreeView::acceptDragAndDrop() const
	{
		return mDropTargetDragActive || (DragAndDropManager::instance().isDragInProgress() &&
			DragAndDropManager::instance().getDragTypeId() == (UINT32)DragAndDropType::Resources);
	}

	void GUIResourceTreeView::dragAndDropStart(const Vector<TreeElement*>& elements)
	{
		assert(mDraggedResources == nullptr);

		DraggedResources* draggedResources = bs_new<DraggedResources>();
		InternalDraggedResources* internalDraggedResources = bs_new<InternalDraggedResources>((UINT32)mSelectedElements.size());

		UINT32 cnt = 0;
		for(auto& entry : elements)
		{
			ResourceTreeElement* resourceTreeElement = static_cast<ResourceTreeElement*>(entry);
			internalDraggedResources->resourcePaths[cnt] = resourceTreeElement->mFullPath; 
			draggedResources->resourcePaths.push_back(internalDraggedResources->resourcePaths[cnt]);

			cnt++;
		}

		mDraggedResources = internalDraggedResources;

		DragAndDropManager::instance().startDrag((UINT32)DragAndDropType::Resources, (void*)draggedResources, 
			std::bind(&GUIResourceTreeView::dragAndDropFinalize, this), true);
	}

	void GUIResourceTreeView::dragAndDropEnded(TreeElement* overTreeElement)
	{
		if(overTreeElement != nullptr && mDraggedResources != nullptr)
		{
			ResourceTreeElement* resourceTreeElement = static_cast<ResourceTreeElement*>(overTreeElement);

			Path destDir = resourceTreeElement->mFullPath;
			if(FileSystem::isFile(destDir))
				destDir = destDir.getParent();

			for(UINT32 i = 0; i < mDraggedResources->numObjects; i++)
			{
				WString filename = mDraggedResources->resourcePaths[i].getWFilename();
				Path currentParent = mDraggedResources->resourcePaths[i].getParent();

				if(currentParent != destDir)
				{
					Path newPath = destDir;
					newPath.append(filename);

					gProjectLibrary().moveEntry(mDraggedResources->resourcePaths[i], findUniquePath(newPath));
				}
			}
		}
	}

	void GUIResourceTreeView::dragAndDropFinalize()
	{
		mDragInProgress = false;
		_markLayoutAsDirty();

		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(DragAndDropManager::instance().getDragData());
		bs_delete(draggedResources);

		if(mDraggedResources != nullptr)
		{
			bs_delete(mDraggedResources);
			mDraggedResources = nullptr;
		}
	}

	void GUIResourceTreeView::_changeParentWidget(GUIWidget* widget)
	{
		GUITreeView::_changeParentWidget(widget);

		if (widget != nullptr && widget->getTarget()->getTarget()->getProperties().isWindow)
		{
			RenderWindow* parentWindow = static_cast<RenderWindow*>(widget->getTarget()->getTarget().get());
			setDropTarget(parentWindow, mLayoutData.area.x, mLayoutData.area.y, mLayoutData.area.width, mLayoutData.area.height);
		}
		else
			clearDropTarget();
	}

	bool GUIResourceTreeView::_acceptDragAndDrop(const Vector2I position, UINT32 typeId) const
	{
		return typeId == (UINT32)DragAndDropType::Resources && !_isDisabled();
	}

	void GUIResourceTreeView::selectionChanged()
	{
		onSelectionChanged();

		sendMessage(SELECTION_CHANGED_MSG);
	}

	Vector<Path> GUIResourceTreeView::getSelection() const
	{
		Vector<Path> selectedPaths;
		for (auto& selectedElem : mSelectedElements)
		{
			ResourceTreeElement* resTreeElement = static_cast<ResourceTreeElement*>(selectedElem.element);

			selectedPaths.push_back(resTreeElement->mFullPath);
		}

		return selectedPaths;
	}

	void GUIResourceTreeView::setSelection(const Vector<Path>& paths)
	{
		unselectAll();

		Stack<ResourceTreeElement*> todo;
		todo.push(&mRootElement);

		while (!todo.empty())
		{
			ResourceTreeElement* currentElem = todo.top();
			todo.pop();

			auto iterFind = std::find(paths.begin(), paths.end(), currentElem->mFullPath);
			if (iterFind != paths.end())
			{
				expandToElement(currentElem);
				selectElement(currentElem);
			}

			for (auto& child : currentElem->mChildren)
			{
				ResourceTreeElement* sceneChild = static_cast<ResourceTreeElement*>(child);
				todo.push(sceneChild);
			}
		}
	}

	const String& GUIResourceTreeView::getGUITypeName()
	{
		static String typeName = "ResourceTreeView";
		return typeName;
	}
}