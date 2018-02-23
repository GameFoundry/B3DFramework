//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "String/BsUnicode.h"
#include "Platform/BsPlatform.h"
#include "Platform/BsDropTarget.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Math/BsRect2I.h"
#include "Private/MacOS/BsMacOSDropTarget.h"
#include "Private/MacOS/BsMacOSWindow.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	Vector<CocoaDragAndDrop::DropArea> CocoaDragAndDrop::sDropAreas;
	Mutex CocoaDragAndDrop::sMutex;
	Vector<CocoaDragAndDrop::DragAndDropOp> CocoaDragAndDrop::sQueuedOperations;
	Vector<CocoaDragAndDrop::DropAreaOp> CocoaDragAndDrop::sQueuedAreaOperations;

	DropTarget::DropTarget(const RenderWindow* ownerWindow, const Rect2I& area)
		: mArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
	{
		CocoaDragAndDrop::registerDropTarget(this);
	}

	DropTarget::~DropTarget()
	{
		CocoaDragAndDrop::unregisterDropTarget(this);

		_clear();
	}

	void DropTarget::setArea(const Rect2I& area)
	{
		mArea = area;

		CocoaDragAndDrop::updateDropTarget(this);
	}

	void CocoaDragAndDrop::registerDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Register, target->getArea()));
	}

	void CocoaDragAndDrop::unregisterDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Unregister));
	}

	void CocoaDragAndDrop::updateDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Update, target->getArea()));
	}

	void CocoaDragAndDrop::update()
	{
		THROW_IF_CORE_THREAD

		// First handle any queued registration/unregistration
		{
			Lock lock(sMutex);

			for(auto& entry : sQueuedAreaOperations)
			{
				CocoaWindow* areaWindow;
				entry.target->_getOwnerWindow()->getCustomAttribute("COCOA_WINDOW", &areaWindow);

				switch(entry.type)
				{
				case DropAreaOpType::Register:
					sDropAreas.push_back(DropArea(entry.target, entry.area));
					areaWindow->_registerForDragAndDrop();
					break;
				case DropAreaOpType::Unregister:
					// Remove any operations queued for this target
					for(auto iter = sQueuedOperations.begin(); iter !=sQueuedOperations.end();)
					{
						if(iter->target == entry.target)
							iter = sQueuedOperations.erase(iter);
						else
							++iter;
					}

					// Remove the area
					{
						auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
						{
							return area.target == entry.target;
						});

						sDropAreas.erase(iterFind);
					}

					areaWindow->_unregisterForDragAndDrop();

					break;
				case DropAreaOpType::Update:
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
					{
						return area.target == entry.target;
					});

					if (iterFind != sDropAreas.end())
						iterFind->area = entry.area;
				}
					break;
				}
			}

			sQueuedAreaOperations.clear();
		}

		// Actually trigger events
		Vector<DragAndDropOp> operations;

		{
			Lock lock(sMutex);
			std::swap(operations, sQueuedOperations);
		}

		for(auto& op : operations)
		{
			switch(op.type)
			{
				case DragAndDropOpType::Enter:
					op.target->onEnter(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::DragOver:
					op.target->onDragOver(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::Drop:
					op.target->_setFileList(op.fileList);
					op.target->onDrop(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::Leave:
					op.target->_clear();
					op.target->onLeave();
					break;
			}
		}
	}

	bool CocoaDragAndDrop::_notifyDragEntered(UINT32 windowId, const Vector2I& position)
	{
		THROW_IF_CORE_THREAD

		bool eventAccepted = false;
		for(auto& entry : sDropAreas)
		{
			UINT32 areaWindowId = 0;
			entry.target->_getOwnerWindow()->getCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if(entry.area.contains(position))
			{
				if(!entry.target->_isActive())
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.target,
						position));

					entry.target->_setActive(true);
				}

				eventAccepted = true;
			}
		}

		return eventAccepted;
	}

	bool CocoaDragAndDrop::_notifyDragMoved(UINT32 windowId, const Vector2I& position)
	{
		THROW_IF_CORE_THREAD

		bool eventAccepted = false;
		for(auto& entry : sDropAreas)
		{
			UINT32 areaWindowId = 0;
			entry.target->_getOwnerWindow()->getCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if (entry.area.contains(position))
			{
				if (entry.target->_isActive())
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::DragOver, entry.target, position));
				} else
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.target, position));
				}

				entry.target->_setActive(true);
				eventAccepted = true;
			}
			else
			{
				// Cursor left previously active target's area
				if (entry.target->_isActive())
				{
					{
						Lock lock(sMutex);
						sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.target));
					}

					entry.target->_setActive(false);
				}
			}
		}

		return eventAccepted;
	}

	void CocoaDragAndDrop::_notifyDragLeft(UINT32 windowId)
	{
		THROW_IF_CORE_THREAD

		for(auto& entry : sDropAreas)
		{
			UINT32 areaWindowId = 0;
			entry.target->_getOwnerWindow()->getCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if(entry.target->_isActive())
			{
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.target));
				}

				entry.target->_setActive(false);
			}
		}
	}

	bool CocoaDragAndDrop::_notifyDragDropped(UINT32 windowId, const Vector2I& position, const Vector<Path>& paths)
	{
		THROW_IF_CORE_THREAD

		bool eventAccepted = false;
		for(auto& entry : sDropAreas)
		{
			UINT32 areaWindowId = 0;
			entry.target->_getOwnerWindow()->getCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if(!entry.target->_isActive())
				continue;

			Lock lock(sMutex);
			sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Drop, entry.target, position, paths));

			eventAccepted = true;
			entry.target->_setActive(false);
		}

		return eventAccepted;
	}
}

