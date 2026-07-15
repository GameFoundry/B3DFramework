//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/MacOS/B3DMacOSDropTarget.h"
#include "Private/MacOS/B3DMacOSPlatform.h"
#include "Private/MacOS/B3DMacOSWindow.h"
#include "Platform/B3DDropTarget.h"
#include "GpuBackend/B3DRenderWindow.h"

using namespace b3d;

Vector<CocoaDragAndDrop::DropArea> CocoaDragAndDrop::sDropAreas;
Mutex CocoaDragAndDrop::sMutex;
Vector<CocoaDragAndDrop::DragAndDropOp> CocoaDragAndDrop::sQueuedOperations;
Vector<CocoaDragAndDrop::DropAreaOp> CocoaDragAndDrop::sQueuedAreaOperations;

DropTarget::DropTarget(const RenderWindow* ownerWindow, const Area2I& area)
	: mArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
{
	CocoaDragAndDrop::RegisterDropTarget(this);
}

DropTarget::~DropTarget()
{
	CocoaDragAndDrop::UnregisterDropTarget(this);

	ClearInternal();
}

void DropTarget::SetArea(const Area2I& area)
{
	mArea = area;

	CocoaDragAndDrop::UpdateDropTarget(this);
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
	// First handle any queued registration/unregistration
	{
		Lock lock(sMutex);

		for(auto& entry : sQueuedAreaOperations)
		{
			const u32 areaWindowId = (u32)entry.Target->GetOwnerWindow()->GetPlatformWindowHandle();
			CocoaWindow* areaWindow = MacOSPlatform::GetWindow(areaWindowId);

			switch(entry.Type)
			{
			case DropAreaOpType::Register:
				sDropAreas.push_back(DropArea(entry.Target, entry.Area));

				if(areaWindow)
					areaWindow->RegisterForDragAndDropInternal();
				break;
			case DropAreaOpType::Unregister:
				// Remove any operations queued for this target
				for(auto iter = sQueuedOperations.begin(); iter != sQueuedOperations.end();)
				{
					if(iter->Target == entry.Target)
						iter = sQueuedOperations.erase(iter);
					else
						++iter;
				}

				// Remove the area
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&entry](const DropArea& area)
												 { return area.Target == entry.Target; });

					sDropAreas.erase(iterFind);
				}

				if(areaWindow)
					areaWindow->UnregisterForDragAndDropInternal();

				break;
			case DropAreaOpType::Update:
				{
					auto iterFind = std::find_if(sDropAreas.begin(), sDropAreas.end(), [&entry](const DropArea& area)
												 { return area.Target == entry.Target; });

					if(iterFind != sDropAreas.end())
						iterFind->Area = entry.Area;
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
		switch(op.Type)
		{
		case DragAndDropOpType::Enter:
			op.Target->OnEnter(op.Position.X, op.Position.Y);
			break;
		case DragAndDropOpType::DragOver:
			op.Target->OnDragOver(op.Position.X, op.Position.Y);
			break;
		case DragAndDropOpType::Drop:
			op.Target->SetFileList(op.FileList);
			op.Target->OnDrop(op.Position.X, op.Position.Y);
			break;
		case DragAndDropOpType::Leave:
			op.Target->ClearInternal();
			op.Target->OnLeave();
			break;
		}
	}
}

bool CocoaDragAndDrop::NotifyDragEnteredInternal(u32 windowId, const Vector2I& position)
{
	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		const u32 areaWindowId = (u32)entry.Target->GetOwnerWindow()->GetPlatformWindowHandle();
		if(areaWindowId != windowId)
			continue;

		if(entry.Area.Contains(position))
		{
			if(!entry.Target->IsActive())
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.Target, position));

				entry.Target->SetActive(true);
			}

			eventAccepted = true;
		}
	}

	return eventAccepted;
}

bool CocoaDragAndDrop::NotifyDragMovedInternal(u32 windowId, const Vector2I& position)
{
	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		const u32 areaWindowId = (u32)entry.Target->GetOwnerWindow()->GetPlatformWindowHandle();
		if(areaWindowId != windowId)
			continue;

		if(entry.Area.Contains(position))
		{
			if(entry.Target->IsActive())
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::DragOver, entry.Target, position));
			}
			else
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Enter, entry.Target, position));
			}

			entry.Target->SetActive(true);
			eventAccepted = true;
		}
		else
		{
			// Cursor left previously active target's area
			if(entry.Target->IsActive())
			{
				{
					Lock lock(sMutex);
					sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.Target));
				}

				entry.Target->SetActive(false);
			}
		}
	}

	return eventAccepted;
}

void CocoaDragAndDrop::NotifyDragLeftInternal(u32 windowId)
{
	for(auto& entry : sDropAreas)
	{
		const u32 areaWindowId = (u32)entry.Target->GetOwnerWindow()->GetPlatformWindowHandle();
		if(areaWindowId != windowId)
			continue;

		if(entry.Target->IsActive())
		{
			{
				Lock lock(sMutex);
				sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Leave, entry.Target));
			}

			entry.Target->SetActive(false);
		}
	}
}

bool CocoaDragAndDrop::NotifyDragDroppedInternal(u32 windowId, const Vector2I& position, const Vector<Path>& paths)
{
	bool eventAccepted = false;
	for(auto& entry : sDropAreas)
	{
		const u32 areaWindowId = (u32)entry.Target->GetOwnerWindow()->GetPlatformWindowHandle();
		if(areaWindowId != windowId)
			continue;

		if(!entry.Target->IsActive())
			continue;

		Lock lock(sMutex);
		sQueuedOperations.push_back(DragAndDropOp(DragAndDropOpType::Drop, entry.Target, position, paths));

		eventAccepted = true;
		entry.Target->SetActive(false);
	}

	return eventAccepted;
}
