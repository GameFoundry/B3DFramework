//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuMemoryDefragmenter.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Image/B3DTexture.h"
#include "CoreObject/B3DRenderThread.h"

namespace b3d
{
	TConfigVariable<bool> gGpuDefragmentation("gpu.Defragmentation",
		"Moves textures and buffers in GPU memory at the end of each frame, so that free memory coalesces and heaps empty out.", false);
	TConfigVariable<u32> gGpuDefragmentationBytesPerFrame("gpu.DefragmentationBytesPerFrame",
		"Maximum number of bytes gpu.Defragmentation moves per frame. 0 for no limit.", 16 * 1024 * 1024);
	TConfigVariable<u32> gGpuDefragmentationMovesPerFrame("gpu.DefragmentationMovesPerFrame",
		"Maximum number of textures and buffers gpu.Defragmentation moves per frame. 0 for no limit.", 64);
} // namespace b3d

using namespace b3d;
using namespace b3d::render;

namespace
{
	/**
	 * Returns true if the texture or buffer that owns an allocation may move. Only reads state of the proxy, as it is called with
	 * the allocator locked.
	 */
	bool IsMovable(GpuAllocationOwner owner)
	{
		if(const render::Texture* texture = owner.GetTexture())
		{
			const TextureProperties& properties = texture->GetProperties();
			const TextureUsageFlags excludedUsage = TextureUsageFlag::StoreOnCPUWithGPUAccess | TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil;

			return texture->IsInitialized() && !texture->HasFixedLocation() && texture->GetFrameGraphResourceIndex() == ~0u &&
				!properties.Usage.IsSetAny(excludedUsage) && properties.SampleCount <= 1 && texture->GetGpuResource() != nullptr;
		}

		// Buffers in mapped memory stay in place: the GPU copies a moved buffer's contents later, which would overwrite CPU writes made
		// through the mapping before the copy.
		// TODO - Device buffers the device maps move anyway, with the same hazard. Revisit with virtual memory (Promptling task-TALZ5ylL).
		const render::GpuBuffer* buffer = owner.GetBuffer();
		const GpuBufferInformation& information = buffer->GetInformation();
		const bool isStaging = information.Type == GpuBufferType::StagingRead || information.Type == GpuBufferType::StagingWrite;

		return buffer->IsInitialized() && !buffer->HasFixedLocation() && buffer->GetFrameGraphResourceIndex() == ~0u &&
			information.Memory == GpuBufferMemory::Device && !isStaging && buffer->GetGpuResource() != nullptr;
	}

	/** Returns the native resource of the texture or buffer that owns an allocation. */
	IGpuResource* GetGpuResource(GpuAllocationOwner owner)
	{
		if(const render::Texture* texture = owner.GetTexture())
			return texture->GetGpuResource();

		return owner.GetBuffer()->GetGpuResource();
	}

	/** Move of a texture or a buffer into the memory of a target created there. */
	struct Relocation
	{
		TShared<render::Texture> SourceTexture;
		TShared<render::Texture> TargetTexture; /**< Texture created at the destination of the move, which receives the copy. */
		TShared<render::GpuBuffer> SourceBuffer;
		TShared<render::GpuBuffer> TargetBuffer; /**< Buffer created at the destination of the move, which receives the copy. */
		u64 Size = 0;
	};
}

GpuMemoryDefragmenter::GpuMemoryDefragmenter(GpuDevice& device)
	: mDevice(device)
{ }

void GpuMemoryDefragmenter::Run(GpuWorkContext& gpuContext)
{
	ASSERT_IF_NOT_RENDER_THREAD

	mStatistics.LastPassMoveCount = 0;
	mStatistics.LastPassMovedByteCount = 0;

	if(!gGpuDefragmentation)
		return;

	TArray<IGpuAllocator*> allocators;
	mDevice.GetPersistentAllocators(allocators);
	if(allocators.Empty())
		return;

	// Plan with each allocator in turn until the budget runs out, starting with a different one every pass
	const u64 maximumByteCount = gGpuDefragmentationBytesPerFrame;
	const u32 maximumMoveCount = gGpuDefragmentationMovesPerFrame;

	TArray<GpuAllocationMove> moves;
	u64 plannedByteCount = 0;
	const u32 allocatorCount = (u32)allocators.Size();
	for(u32 allocatorOffset = 0; allocatorOffset < allocatorCount; allocatorOffset++)
	{
		IGpuAllocator& allocator = *allocators[(mFirstAllocatorIndex + allocatorOffset) % allocatorCount];
		if(!allocator.SupportsDefragmentation())
			continue;

		GpuDefragmentationBudget budget;
		budget.MaximumByteCount = maximumByteCount != 0 ? maximumByteCount - plannedByteCount : 0;
		budget.MaximumMoveCount = maximumMoveCount != 0 ? maximumMoveCount - (u32)moves.Size() : 0;

		const u32 firstMoveIndex = (u32)moves.Size();
		allocator.PlanDefragmentation(budget, IsMovable, moves);

		for(u32 moveIndex = firstMoveIndex; moveIndex < (u32)moves.Size(); moveIndex++)
			plannedByteCount += moves[moveIndex].Source.Size;

		const bool isByteBudgetUsed = maximumByteCount != 0 && plannedByteCount >= maximumByteCount;
		const bool isMoveBudgetUsed = maximumMoveCount != 0 && (u32)moves.Size() >= maximumMoveCount;
		if(isByteBudgetUsed || isMoveBudgetUsed)
			break;
	}

	mFirstAllocatorIndex = (mFirstAllocatorIndex + 1) % allocatorCount;

	// Create a target at the destination of each move. Moves whose native resource is referenced by a command buffer that was not
	// submitted yet are cancelled: the copy would be ordered before that command buffer, so writes it records would be lost.
	TArray<Relocation> relocations;
	for(GpuAllocationMove& move : moves)
	{
		Relocation relocation;
		relocation.Size = move.Source.Size;

		if(!GetGpuResource(move.Owner)->IsBound())
		{
			if(render::Texture* texture = move.Owner.GetTexture())
			{
				TextureCreateInformation createInformation(texture->GetProperties());
				createInformation.Name = texture->GetName();
				createInformation.Usage.Unset(TextureUsageFlag::Bindless);

				relocation.SourceTexture = std::static_pointer_cast<render::Texture>(texture->GetShared());
				relocation.TargetTexture = mDevice.CreateTexture(createInformation, move.Destination);
			}
			else
			{
				render::GpuBuffer* buffer = move.Owner.GetBuffer();
				GpuBufferCreateInformation createInformation(buffer->GetInformation());
				createInformation.Flags.Unset(GpuBufferFlag::Bindless);

				relocation.SourceBuffer = std::static_pointer_cast<render::GpuBuffer>(buffer->GetShared());
				relocation.TargetBuffer = mDevice.CreateGpuBuffer(createInformation, move.Destination);
			}
		}

		if(relocation.TargetTexture == nullptr && relocation.TargetBuffer == nullptr)
		{
			move.Destination.Allocator->Free(move.Destination);
			continue;
		}

		relocations.Add(relocation);
	}

	if(relocations.Empty())
		return;

	// The targets take the owners' resting state once the copies complete
	const TShared<GpuCommandBuffer>& commandBuffer = gpuContext.GetTransferCommandBuffer();
	const GpuQueueType queueType = commandBuffer->GetQueueType();

	TArray<GpuBufferBarrier> bufferBarriersBefore;
	TArray<GpuTextureBarrier> textureBarriersBefore;
	TArray<GpuBufferBarrier> bufferBarriersAfter;
	TArray<GpuTextureBarrier> textureBarriersAfter;
	for(const Relocation& relocation : relocations)
	{
		if(relocation.SourceTexture != nullptr)
		{
			const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*relocation.SourceTexture, queueType);
			const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
			const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);

			// The copy overwrites every subresource of the target
			GpuAccessState discardedState = GpuBackendUtility::GetRestingState(*relocation.TargetTexture, queueType);
			discardedState.Layout = GpuImageLayout::Undefined;

			textureBarriersBefore.Add(GpuTextureBarrier(relocation.SourceTexture, restingState, transferRead));
			textureBarriersBefore.Add(GpuTextureBarrier(relocation.TargetTexture, discardedState, transferWrite));
			textureBarriersAfter.Add(GpuTextureBarrier(relocation.TargetTexture, transferWrite, restingState));
		}
		else
		{
			const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*relocation.SourceBuffer, queueType);
			const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
			const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

			bufferBarriersBefore.Add(GpuBufferBarrier(relocation.SourceBuffer, restingState, transferRead));
			bufferBarriersBefore.Add(GpuBufferBarrier(relocation.TargetBuffer, GpuBackendUtility::GetRestingState(*relocation.TargetBuffer, queueType), transferWrite));
			bufferBarriersAfter.Add(GpuBufferBarrier(relocation.TargetBuffer, transferWrite, restingState));
		}
	}

	// Everything is recorded before the native resources are exchanged, so each command refers to the native resource it means
	commandBuffer->IssueBarriers(GpuBarriers(bufferBarriersBefore, textureBarriersBefore));

	for(const Relocation& relocation : relocations)
	{
		if(relocation.SourceTexture != nullptr)
		{
			const TextureProperties& properties = relocation.SourceTexture->GetProperties();

			TextureCopyInformation copyInformation;
			copyInformation.FaceCount = properties.GetFaceCount();
			for(u32 mipLevel = 0; mipLevel <= properties.MipMapCount; mipLevel++)
			{
				copyInformation.SourceMip = mipLevel;
				copyInformation.DestinationMip = mipLevel;
				commandBuffer->CopyTexture(relocation.SourceTexture, relocation.TargetTexture, copyInformation);
			}
		}
		else
			commandBuffer->CopyBufferToBuffer(relocation.SourceBuffer, relocation.TargetBuffer, 0, 0, relocation.SourceBuffer->GetTotalSize());
	}

	commandBuffer->IssueBarriers(GpuBarriers(bufferBarriersAfter, textureBarriersAfter));

	// The targets now hold the old native resources, released once the transfer command buffer and earlier work complete
	for(const Relocation& relocation : relocations)
	{
		if(relocation.SourceTexture != nullptr)
			relocation.SourceTexture->Relocate(*relocation.TargetTexture);
		else
			relocation.SourceBuffer->Relocate(*relocation.TargetBuffer);

		mStatistics.LastPassMoveCount++;
		mStatistics.LastPassMovedByteCount += relocation.Size;
	}

	mStatistics.MoveCount += mStatistics.LastPassMoveCount;
	mStatistics.MovedByteCount += mStatistics.LastPassMovedByteCount;
}
