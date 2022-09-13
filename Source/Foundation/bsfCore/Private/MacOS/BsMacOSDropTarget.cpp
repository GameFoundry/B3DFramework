//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		: MArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
	{
		CocoaDragAndDrop::registerDropTarget(this);
	}

	DropTarget::~DropTarget()
	{
		CocoaDragAndDrop::unregisterDropTarget(this);

		_clear();
	}

	void DropTarget::SetArea(const Rect2I& area)
	{
		mArea = area;

		CocoaDragAndDrop::updateDropTarget(this);
	}

	void CocoaDragAndDrop::RegisterDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Register, target->GetArea()));
	}

	void CocoaDragAndDrop::UnregisterDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Unregister));
	}

	void CocoaDragAndDrop::UpdateDropTarget(DropTarget* target)
	{
		Lock lock(sMutex);
		sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Update, target->GetArea()));
	}

	void CocoaDragAndDrop::Update()
	{
		THROW_IF_CORE_THREAD

		// First handle any queued registration/unregistration
		{
			Lock lock(sMutex);

			for(auto& entry : sQueuedAreaOperations)
			{
				CocoaWindow* areaWindow;
				entry.target->_getOwnerWindow()->GetCustomAttribute("COCOA_WINDOW", &areaWindow);

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

			sQueuedAreaOperations.Clear();
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
					op.target->OnEnter(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::DragOver:
					op.target->OnDragOver(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::Drop:
					op.target->_setFileList(op.fileList);
					op.target->OnDrop(op.position.x, op.position.y);
					break;
				case DragAndDropOpType::Leave:
					op.target->_clear();
					op.target->OnLeave();
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
			entry.target->_getOwnerWindow()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if(entry.area.Contains(position))
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
			entry.target->_getOwnerWindow()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
			if(areaWindowId != windowId)
				continue;

			if (entry.area.Contains(position))
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
			entry.target->_getOwnerWindow()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
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
			entry.target->_getOwnerWindow()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
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

