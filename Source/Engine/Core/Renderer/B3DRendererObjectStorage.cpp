//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererObjectStorage.h"
#include "Allocators/B3DFrameAllocator.h"

namespace b3d
{
	RendererId RendererObjectStorage::AllocateRendererId()
	{
		RendererId objectId = mObjectIdAllocator.Allocate();
		mPendingAllocations.insert(objectId);
		return objectId;
	}

	void RendererObjectStorage::DeallocateRendererId(RendererId objectId)
	{
		auto found = mPendingAllocations.find(objectId);
		if(found != mPendingAllocations.end())
			mPendingAllocations.erase(found);
		else
			mDeallocations.Add(objectId);

		mObjectIdAllocator.Deallocate(objectId);
	}

	RendererObjectStorage::FlushedCommands RendererObjectStorage::FlushCommands(FrameAllocator& allocator)
	{
		const u32 deallocationCount = (u32)mDeallocations.size();
		const u32 allocationCount = (u32)mPendingAllocations.size();

		if(deallocationCount == 0 && allocationCount == 0)
			return {};

		FlushedCommands result;
		if(deallocationCount > 0)
		{
			RendererIdCommand* commands = reinterpret_cast<RendererIdCommand*>(allocator.AllocateAligned(sizeof(RendererIdCommand) * deallocationCount, alignof(RendererIdCommand)));
			for(u32 index = 0; index < deallocationCount; ++index)
			{
				commands[index].Type = RendererIdCommandType::Deallocate;
				commands[index].ObjectId = mDeallocations[index];
			}
			result.Deallocations = TArrayView<const RendererIdCommand>(commands, deallocationCount);
		}

		if(allocationCount > 0)
		{
			RendererIdCommand* commands = reinterpret_cast<RendererIdCommand*>(allocator.AllocateAligned(sizeof(RendererIdCommand) * allocationCount, alignof(RendererIdCommand)));
			u32 commandIndex = 0;
			for(const RendererId& pendingId : mPendingAllocations)
			{
				commands[commandIndex].Type = RendererIdCommandType::Allocate;
				commands[commandIndex].ObjectId = pendingId;
				++commandIndex;
			}
			result.Allocations = TArrayView<const RendererIdCommand>(commands, allocationCount);
		}

		mDeallocations.clear();
		mPendingAllocations.clear();

		return result;
	}
} // namespace b3d
