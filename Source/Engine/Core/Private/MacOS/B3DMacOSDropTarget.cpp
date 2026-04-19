//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "String/B3DUnicode.h"
#include "Platform/B3DPlatform.h"
#include "Platform/B3DDropTarget.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "Math/B3DRect2I.h"
#include "Private/MacOS/B3DMacOSDropTarget.h"
#include "Private/MacOS/B3DMacOSWindow.h"
#include "CoreThread/B3DCoreThread.h"

using namespace b3d;

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

	ClearInternal();
}

void DropTarget::setArea(const Rect2I& area)
{
	mArea = area;

	CocoaDragAndDrop::updateDropTarget(this);
}

void CocoaDragAndDrop::registerDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Register, target->GetArea()));
}

void CocoaDragAndDrop::unregisterDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Unregister));
}

void CocoaDragAndDrop::updateDropTarget(DropTarget* target)
{
	Lock lock(sMutex);
	sQueuedAreaOperations.push_back(DropAreaOp(target, DropAreaOpType::Update, target->GetArea()));
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
			entry.target->GetOwnerWindowInternal()->GetCustomAttribute("COCOA_WINDOW", &areaWindow);

			switch(entry.type)
			{
			case DropAreaOpType::Register:
				sDropAreas.push_back(DropArea(entry.target, entry.area));
				areaWindow->RegisterForDragAndDropInternal();
				break;
			case DropAreaOpType::Unregister:
				// Remove any operations queued for this target
				for(auto iter = sQueuedOperations.begin(); iter != sQueuedOperations.end();)
				{
					if(iter->target == entry.target)
						iter = sQueuedOperations.erase(iter);
					else
						++iter;
				}

				// Remove the area
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
												 { return area.target == entry.target; });

					sDropAreas.erase(iterFind);
				}

				areaWindow->UnregisterForDragAndDropInternal();

				break;
			case DropAreaOpType::Update:
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&](const DropArea& area)
												 { return area.target == entry.target; });

					if(iterFind != sDropAreas.end())
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
			op.target->SetFileListInternal(op.fileList);
			op.target->onDrop(op.position.x, op.position.y);
			break;
		case DragAndDropOpType::Leave:
			op.target->ClearInternal();
			op.target->onLeave();
			break;
		}
	}
}

bool CocoaDragAndDrop::NotifyDragEnteredInternal(u32 windowId, const Vector2I& position)
{
	THROW_IF_CORE_THREAD

	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		u32 areaWindowId = 0;
		entry.target->GetOwnerWindowInternal()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
		if(areaWindowId != windowId)
			continue;

		if(entry.area.contains(position))
		{
			if(!entry.target->IsActiveInternal())
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.target, position));

				entry.target->SetActiveInternal(true);
			}

			eventAccepted = true;
		}
	}

	return eventAccepted;
}

bool CocoaDragAndDrop::NotifyDragMovedInternal(u32 windowId, const Vector2I& position)
{
	THROW_IF_CORE_THREAD

	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		u32 areaWindowId = 0;
		entry.target->GetOwnerWindowInternal()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
		if(areaWindowId != windowId)
			continue;

		if(entry.area.contains(position))
		{
			if(entry.target->IsActiveInternal())
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::DragOver, entry.target, position));
			}
			else
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.target, position));
			}

			entry.target->SetActiveInternal(true);
			eventAccepted = true;
		}
		else
		{
			// Cursor left previously active target's area
			if(entry.target->IsActiveInternal())
			{
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.target));
				}

				entry.target->SetActiveInternal(false);
			}
		}
	}

	return eventAccepted;
}

void CocoaDragAndDrop::NotifyDragLeftInternal(u32 windowId)
{
	THROW_IF_CORE_THREAD

	for(auto& entry : sDropAreas)
	{
		u32 areaWindowId = 0;
		entry.target->GetOwnerWindowInternal()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
		if(areaWindowId != windowId)
			continue;

		if(entry.target->IsActiveInternal())
		{
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.target));
			}

			entry.target->SetActiveInternal(false);
		}
	}
}

bool CocoaDragAndDrop::NotifyDragDroppedInternal(u32 windowId, const Vector2I& position, const Vector<Path>& paths)
{
	THROW_IF_CORE_THREAD

	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		u32 areaWindowId = 0;
		entry.target->GetOwnerWindowInternal()->GetCustomAttribute("WINDOW_ID", &areaWindowId);
		if(areaWindowId != windowId)
			continue;

		if(!entry.target->IsActiveInternal())
			continue;

		Lock lock(sMutex);
		sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Drop, entry.target, position, paths));

		eventAccepted = true;
		entry.target->SetActiveInternal(false);
	}

	return eventAccepted;
}
