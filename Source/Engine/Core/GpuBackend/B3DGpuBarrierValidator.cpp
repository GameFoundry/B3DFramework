//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBarrierValidator.h"

#if B3D_BUILD_TYPE_DEVELOPMENT

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	void GpuBarrierValidator::BeginCommandBuffer(GpuQueueType queueType)
	{
		mQueueType = queueType;
		ClearBindings();
	}

	void GpuBarrierValidator::ValidateBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		for(const GpuBufferBarrier& barrier : barriers.BufferBarriers)
		{
			if(barrier.Object != nullptr)
				ValidateBufferBarrier(*barrier.Object, barrier, phase, split);
		}

		for(const GpuTextureBarrier& barrier : barriers.TextureBarriers)
		{
			if(barrier.Object != nullptr)
				ValidateImageBarrier(GetTextureSubresources(*barrier.Object, barrier.SubresourceRange), barrier, phase, split);
		}

		for(const GpuRenderTargetBarrier& barrier : barriers.RenderTargetBarriers)
		{
			if(barrier.Object == nullptr)
				continue;

			// The barrier covers the whole surface, restricted to the aspects it selects
			ImageSubresources image = GetSurfaceSubresources(*barrier.Object, barrier.SurfaceMask);
			image.Range.AspectMask &= barrier.SubresourceRange.AspectMask;
			if(image.Key.Object != nullptr)
				ValidateImageBarrier(image, barrier, phase, split);
		}

		// The release of a split alias acquire starts the new lifetime, so the acquire half has nothing left to start
		if(phase != GpuBarrierPhase::Acquire)
			StartAliasLifetimes(barriers);
	}

	void GpuBarrierValidator::DeclareAliasAcquire(const GpuBuffer& buffer, const GpuAliasAcquire& acquire)
	{
		DeclareAliasAcquire(buffer.GetGpuResource(), acquire, buffer.GetName());
	}

	void GpuBarrierValidator::DeclareAliasAcquire(const Texture& texture, const GpuAliasAcquire& acquire)
	{
		DeclareAliasAcquire(texture.GetGpuResource(), acquire, texture.GetName());
	}

	void GpuBarrierValidator::DeclareAliasAcquire(const IGpuResource* gpuResource, const GpuAliasAcquire& acquire, StringView name)
	{
		if(!B3D_ENSURE_LOG(gpuResource != nullptr, "An alias acquire can only be declared for a placed resource. Resource: '{0}'.", name))
			return;

		DeclaredAliasAcquire& declared = mAliasAcquires[gpuResource];
		declared.Source = acquire.Source;
		declared.Predecessors.Clear();

		for(const IGpuResource* predecessor : acquire.Predecessors)
			declared.Predecessors.Add(predecessor);
	}

	void GpuBarrierValidator::SetParameterSet(const TShared<GpuParameterSet>& parameters)
	{
		if(parameters == nullptr)
			return;

		const u32 set = parameters->GetSet();
		if(set >= mParameterSets.size())
			mParameterSets.resize(set + 1);

		mParameterSets[set] = parameters;
	}

	void GpuBarrierValidator::SetVertexBuffers(u32 index, const TShared<GpuBuffer>* buffers, u32 bufferCount)
	{
		if(index + bufferCount > mVertexBuffers.size())
			mVertexBuffers.resize(index + bufferCount);

		for(u32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
			mVertexBuffers[index + bufferIndex] = buffers[bufferIndex];
	}

	void GpuBarrierValidator::ValidateRenderPass(const RenderPassCreateInformation& createInformation)
	{
		mRenderPassAttachments.Clear();
		if(createInformation.Target == nullptr)
			return;

		const GpuStageFlags kDepthStencilStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		const auto fnValidateAttachment = [this, &createInformation](RenderSurfaceMaskBits surface, GpuTextureAspectFlags aspects, GpuStageFlags stages)
		{
			ImageSubresources image = GetSurfaceSubresources(*createInformation.Target, surface);
			image.Range.AspectMask &= aspects;
			if(image.Key.Object == nullptr || !image.Range.AspectMask)
				return;

			// Writable attachments also read, through loads, blending and depth/stencil tests
			const bool readOnly = createInformation.ReadOnlyMask.IsSet(surface);
			const GpuAccessFlags access = readOnly ? GpuAccessFlags(GpuAccessFlag::Read) : GpuAccessFlag::Read | GpuAccessFlag::Write;
			ValidateImageAccess(image, GpuBackendUtility::GetRenderPassAttachmentLayout(surface, createInformation.ReadOnlyMask), stages, access);

			mRenderPassAttachments.Add(image);
		};

		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			fnValidateAttachment((RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), GpuTextureAspectFlag::Color, GpuStageFlag::ColorAttachment);

		// The aspects of a depth/stencil image are read-only separately
		fnValidateAttachment(RT_DEPTH, GpuTextureAspectFlag::Depth, kDepthStencilStages);
		fnValidateAttachment(RT_STENCIL, GpuTextureAspectFlag::Stencil, kDepthStencilStages);
	}

	void GpuBarrierValidator::ValidateDraw(bool indexed)
	{
		mOperation = ++mOperationCount;

		for(const TShared<GpuBuffer>& buffer : mVertexBuffers)
		{
			if(buffer != nullptr)
				ValidateBufferAccess(*buffer, GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read);
		}

		if(indexed && mIndexBuffer != nullptr)
			ValidateBufferAccess(*mIndexBuffer, GpuStageFlag::VertexInputIndices, GpuAccessFlag::Read);

		if(mGraphicsPipeline != nullptr && mGraphicsPipeline->GetParameterLayout() != nullptr)
			ValidateBindings(*mGraphicsPipeline->GetParameterLayout(), true);

		mOperation = 0;
	}

	void GpuBarrierValidator::ValidateDispatch()
	{
		mOperation = ++mOperationCount;

		if(mComputePipeline != nullptr && mComputePipeline->GetParameterLayout() != nullptr)
			ValidateBindings(*mComputePipeline->GetParameterLayout(), false);

		mOperation = 0;
	}

	void GpuBarrierValidator::ValidateBufferAccess(const GpuBuffer& buffer, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* error = GetLifetimeError(buffer.IsUnplaced(), buffer.GetGpuResource(), nullptr, GpuBarrierPhase::Full);
		if(error == nullptr)
			error = ApplyAccess(mBuffers[&buffer], mQueueType, GpuImageLayout::Undefined, stages, access, GetUndeclaredState(GpuBackendUtility::GetRestingState(buffer, mQueueType)), mOperation);

		B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, buffer.GetName());
	}

	void GpuBarrierValidator::ValidateTextureAccess(const Texture& texture, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		ValidateImageAccess(GetTextureSubresources(texture, range), layout, stages, access);
	}

	void GpuBarrierValidator::ValidateRenderTargetAccess(const RenderTarget& target, RenderSurfaceMaskBits surface, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const ImageSubresources image = GetSurfaceSubresources(target, surface);
		if(image.Key.Object != nullptr)
			ValidateImageAccess(image, layout, stages, access);
	}

	void GpuBarrierValidator::Clear()
	{
		mBuffers.clear();
		mImages.clear();
		mAliasAcquires.clear();
		mSupersededResources.clear();
		ClearBindings();
	}

	void GpuBarrierValidator::ClearBindings()
	{
		mParameterSets.clear();
		mGraphicsPipeline = nullptr;
		mComputePipeline = nullptr;
		mVertexBuffers.clear();
		mIndexBuffer = nullptr;
		mRenderPassAttachments.Clear();
		mOperation = 0;
	}

	void GpuBarrierValidator::StartAliasLifetimes(const GpuBarriers& barriers)
	{
		TInlineArray<const IGpuResource*, 8> barrierResources;
		for(const GpuBufferBarrier& barrier : barriers.BufferBarriers)
		{
			if(barrier.Object != nullptr && barrier.Object->GetGpuResource() != nullptr && !barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease))
				barrierResources.Add(barrier.Object->GetGpuResource());
		}

		for(const GpuTextureBarrier& barrier : barriers.TextureBarriers)
		{
			if(barrier.Object != nullptr && barrier.Object->GetGpuResource() != nullptr && !barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease))
				barrierResources.Add(barrier.Object->GetGpuResource());
		}

		const auto fnStartLifetime = [this, &barrierResources](const IGpuResource* gpuResource, const GpuBarrier& barrier, StringView name)
		{
			if(gpuResource == nullptr || !barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire))
				return;

			mSupersededResources.erase(gpuResource);

			const auto found = mAliasAcquires.find(gpuResource);
			if(found == mAliasAcquires.end())
				return;

			for(const IGpuResource* predecessor : found->second.Predecessors)
			{
				// Barriers of one call are not ordered against each other, so a barrier of an earlier resource could overlap the acquire's barrier
				B3D_ENSURE_LOG(!barrierResources.Contains(predecessor), "A barrier of an earlier resource on the memory is issued together with the alias acquire that supersedes it. Resource: '{0}'.", name);
				mSupersededResources.insert(predecessor);
			}

			mAliasAcquires.erase(found);
		};

		for(const GpuBufferBarrier& barrier : barriers.BufferBarriers)
		{
			if(barrier.Object != nullptr)
				fnStartLifetime(barrier.Object->GetGpuResource(), barrier, barrier.Object->GetName());
		}

		for(const GpuTextureBarrier& barrier : barriers.TextureBarriers)
		{
			if(barrier.Object != nullptr)
				fnStartLifetime(barrier.Object->GetGpuResource(), barrier, barrier.Object->GetName());
		}
	}

	GpuBarrierValidator::ImageSubresources GpuBarrierValidator::GetTextureSubresources(const Texture& texture, const GpuTextureSubresourceRange& range) const
	{
		const TextureProperties& properties = texture.GetProperties();

		ImageSubresources image;
		image.Key.Object = &texture;
		image.FullRange = GpuTextureSubresourceRange(0, properties.MipMapCount + 1, 0, properties.GetFaceCount(), GpuBackendUtility::GetFormatAspects(properties.Format));
		image.Range = GpuBackendUtility::ClampRange(range, image.FullRange);
		image.Name = texture.GetName();
		image.RestingState = GpuBackendUtility::GetRestingState(texture, mQueueType);
		image.Texture = &texture;

		return image;
	}

	GpuBarrierValidator::ImageSubresources GpuBarrierValidator::GetSurfaceSubresources(const RenderTarget& target, RenderSurfaceMaskBits surface) const
	{
		const bool isColor = ((u32)surface & (u32)RT_COLOR_ALL) != 0;
		if(target.GetProperties().IsWindow)
		{
			// Windows have one color surface, and their depth and stencil share one image
			if(isColor && surface != RT_COLOR0)
				return ImageSubresources();

			ImageSubresources image;
			image.Key.Object = &target;
			image.Key.Surface = isColor ? 0 : 1;
			image.FullRange = GpuTextureSubresourceRange(0, 1, 0, 1, isColor ? GpuTextureAspectFlags(GpuTextureAspectFlag::Color) : GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
			image.Range = image.FullRange;
			image.Name = "Window";
			image.RestingState = GpuBackendUtility::GetRestingState(target, surface, mQueueType);

			return image;
		}

		const RenderTexture& renderTexture = static_cast<const RenderTexture&>(target);
		const RenderSurfaceInformation& surfaceInformation = isColor ? renderTexture.GetColorSurfaceInformation(Bitwise::LeastSignificantBit((u32)surface)) : renderTexture.GetDepthStencilSurfaceInformation();

		if(surfaceInformation.Texture == nullptr)
			return ImageSubresources();

		ImageSubresources image = GetTextureSubresources(*surfaceInformation.Texture, GpuTextureSubresourceRange::AllSubresources());
		image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, TextureSurface(surfaceInformation.MipLevel, 1, surfaceInformation.Face, surfaceInformation.FaceCount));

		return image;
	}

	void GpuBarrierValidator::ValidateBindings(const GpuPipelineParameterLayout& pipelineLayout, bool isDraw)
	{
		for(u32 set = 0; set < std::min(pipelineLayout.GetSetCount(), (u32)mParameterSets.size()); set++)
		{
			const GpuParameterSet* const parameters = mParameterSets[set].get();
			const TShared<GpuPipelineParameterSetLayout>& pipelineSetLayout = pipelineLayout.GetSet(set);
			if(parameters == nullptr || pipelineSetLayout == nullptr)
				continue;

			// Only the bindings the pipeline uses are accessed
			parameters->ForEachBoundResource(pipelineSetLayout.get(), [this, isDraw](const GpuParameterSetResource& resource)
			{
				const GpuResourceUseFlags stageUse = GpuBackendUtility::GetShaderResourceUseFlags(resource.Uniform->Usage);
				switch(resource.Uniform->Type)
				{
				case GpuParameterType::UniformBuffer:
					ValidateBufferAccess(*resource.Buffer, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::UniformBuffer), resource.Access);
					break;
				case GpuParameterType::StorageBuffer:
					ValidateBufferAccess(*resource.Buffer, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), resource.Access);
					break;
				case GpuParameterType::SampledTexture:
				{
					ImageSubresources image = GetTextureSubresources(*resource.Texture, GpuTextureSubresourceRange::AllSubresources());
					image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, resource.Surface);

					// Sampled depth/stencil views only read the depth aspect
					if(image.Range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
						image.Range.AspectMask = GpuTextureAspectFlag::Depth;

					// A sampled attachment is in the layout of the render pass, which checked it
					const GpuImageLayout layout = isDraw && IsRenderPassAttachment(image) ? GpuImageLayout::Undefined : GpuImageLayout::ShaderReadOnly;
					ValidateImageAccess(image, layout, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), resource.Access);
					break;
				}
				case GpuParameterType::StorageTexture:
				{
					ImageSubresources image = GetTextureSubresources(*resource.Texture, GpuTextureSubresourceRange::AllSubresources());
					image.Range = GpuBackendUtility::GetSurfaceRange(image.FullRange, resource.Surface);
					ValidateImageAccess(image, GpuImageLayout::General, GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), resource.Access);
					break;
				}
				default:
					break;
				}
			});
		}
	}

	void GpuBarrierValidator::ValidateImageBarrier(const ImageSubresources& image, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		// A texture outside a frame graph is at rest. So its first barrier starts from the resting state. An alias acquire starts the lifetime of its memory instead.
		const bool isAtRest = gGpuValidateRestingStates && image.Texture != nullptr && image.Texture->GetFrameGraphResourceIndex() == ~0u &&
			!barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) && phase != GpuBarrierPhase::Acquire;
		const bool declaresNoAccess = barrier.Source.Stages == GpuStageFlag::None && barrier.Source.Access == GpuAccessFlag::None;

		// A texture that rests on the graphics queue leaves its rest there. Barriers that discard the contents take it from no queue.
		const bool leavesRestOnOtherQueue = mQueueType != GQT_GRAPHICS && image.Texture != nullptr && !GpuBackendUtility::IsConcurrentlyReadable(image.Texture->GetProperties().Usage) &&
			!barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) && phase != GpuBarrierPhase::Acquire && barrier.Source.Layout != GpuImageLayout::Undefined;

		const char* error = image.Texture != nullptr ? GetLifetimeError(image.Texture->IsUnplaced(), image.Texture->GetGpuResource(), &barrier, phase) : nullptr;
		if(error == nullptr)
		{
			error = ForEachSubresourceState(image, [&](ResourceState& state)
			{
				const bool isFirstBarrier = !state.IsDeclared && !state.IsUsed();
				const char* const barrierError = ApplyBarrier(state, mQueueType, barrier, phase, split, true);
				if(barrierError == nullptr && isAtRest && isFirstBarrier && declaresNoAccess)
					return "The first barrier of a texture at rest declares no source. Start it from the resting state (see GpuBackendUtility::GetRestingState()), with GpuImageLayout::Undefined to discard the contents.";

				if(barrierError == nullptr && isFirstBarrier && leavesRestOnOtherQueue)
					return "The first barrier of a texture at rest is recorded on a queue other than graphics, which the texture rests on. Release it on the graphics queue, or import it into a frame graph.";

				return barrierError;
			});
		}

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image.Name);
	}

	void GpuBarrierValidator::ValidateBufferBarrier(const GpuBuffer& buffer, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split)
	{
		const char* error = GetLifetimeError(buffer.IsUnplaced(), buffer.GetGpuResource(), &barrier, phase);
		if(error == nullptr)
		{
			ResourceState& state = mBuffers[&buffer];
			error = ApplyBarrier(state, mQueueType, barrier, phase, split, false);
		}

		B3D_ENSURE_LOG(error == nullptr, "{0} Buffer: '{1}'.", error, buffer.GetName());
	}

	void GpuBarrierValidator::ValidateImageAccess(const ImageSubresources& image, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access)
	{
		const char* error = image.Texture != nullptr ? GetLifetimeError(image.Texture->IsUnplaced(), image.Texture->GetGpuResource(), nullptr, GpuBarrierPhase::Full) : nullptr;
		if(error == nullptr)
		{
			const GpuAccessState* const undeclaredState = GetUndeclaredState(image.RestingState);
			const bool restsOnGraphicsQueue = mQueueType != GQT_GRAPHICS && image.Texture != nullptr && !GpuBackendUtility::IsConcurrentlyReadable(image.Texture->GetProperties().Usage);
			error = ForEachSubresourceState(image, [&](ResourceState& state)
			{
				const char* const accessError = ApplyAccess(state, mQueueType, layout, stages, access, undeclaredState, mOperation);
				if(accessError == nullptr && restsOnGraphicsQueue && !state.IsDeclared)
					return "The texture rests on the graphics queue, and was accessed on another queue without a barrier. Import it into the frame graph.";

				return accessError;
			});
		}

		B3D_ENSURE_LOG(error == nullptr, "{0} Image: '{1}'.", error, image.Name);
	}

	bool GpuBarrierValidator::IsRenderPassAttachment(const ImageSubresources& image) const
	{
		return std::any_of(mRenderPassAttachments.begin(), mRenderPassAttachments.end(), [&image](const ImageSubresources& attachment)
		{
			return attachment.Key == image.Key && GpuBackendUtility::RangeOverlaps(attachment.Range, image.Range);
		});
	}

	const GpuAccessState* GpuBarrierValidator::GetUndeclaredState(const GpuAccessState& restingState)
	{
		return gGpuValidateRestingStates ? &restingState : nullptr;
	}

	const char* GpuBarrierValidator::GetLifetimeError(bool isUnplaced, const IGpuResource* gpuResource, const GpuBarrier* barrier, GpuBarrierPhase phase) const
	{
		if(isUnplaced)
			return "The resource was used while it is unplaced, without memory from its transient resource allocator. Place it first.";

		if(gpuResource == nullptr)
			return nullptr;

		// The acquire half of a split alias acquire follows the release, which started the new lifetime
		const bool isAliasAcquire = barrier != nullptr && barrier->Flags.IsSet(GpuBarrierFlag::AliasAcquire) && phase != GpuBarrierPhase::Acquire;
		const auto declaredAcquire = mAliasAcquires.find(gpuResource);
		if(!isAliasAcquire)
		{
			const bool isAliasRelease = barrier != nullptr && barrier->Flags.IsSet(GpuBarrierFlag::AliasRelease);
			if(!isAliasRelease && mSupersededResources.find(gpuResource) != mSupersededResources.end())
				return "The resource was used after an alias acquire of another resource took over its memory.";

			if(declaredAcquire != mAliasAcquires.end())
				return "The resource was used before its alias acquire.";

			return nullptr;
		}

		if(declaredAcquire == mAliasAcquires.end())
			return nullptr;

		const GpuAccessScope& acquireSource = declaredAcquire->second.Source;
		const GpuAccessState& source = barrier->Source;
		if(!source.Stages.IsSetAll(acquireSource.GetStages()) || (acquireSource.WriteStages != GpuStageFlag::None && !source.Access.IsSet(GpuAccessFlag::Write)))
			return "The source of an alias acquire is missing stages or writes of the source of its GpuAliasAcquire.";

		return nullptr;
	}

	const char* GpuBarrierValidator::ApplyBarrier(ResourceState& state, GpuQueueType queueType, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage)
	{
		// Another queue's accesses are ordered before the barrier by a queue wait, which makes writes visible to this queue but does not
		// order them against the barrier's layout transition. Its reads are fine.
		bool isWrittenOnOtherQueue = false;
		for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
			isWrittenOnOtherQueue |= queueTypeIndex != (u32)queueType && state.UsedAccess[queueTypeIndex].IsSet(GpuAccessFlag::Write);

		const char* error = nullptr;
		const GpuAccessState& source = barrier.Source;
		if(state.PendingAcquire != nullptr)
		{
			// The release already checked the source
			if(phase != GpuBarrierPhase::Acquire || state.PendingAcquire != split)
				error = "A barrier was recorded between the release and the acquire of a split barrier of the same resource.";
		}
		else if(barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) && (state.IsDeclared || state.IsUsed()))
			error = "An alias acquire must precede every other use of the resource.";
		else if(barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease) && ((isImage && barrier.Destination.Layout != GpuImageLayout::Undefined) || barrier.Destination.Access != GpuAccessFlag::None))
			error = "An alias release must leave the resource in GpuImageLayout::Undefined, without access.";
		else if(isImage && state.IsDeclared && source.Layout != GpuImageLayout::Undefined && source.Layout != state.Declared.Layout)
			error = "The source layout of a barrier differs from the destination layout of the previous barrier of the resource.";
		else if(!source.Stages.IsSetAll(state.UsedStages[queueType]))
			error = "The source of a barrier is missing stages that accessed the resource since the previous barrier.";
		else if(state.UsedAccess[queueType].IsSet(GpuAccessFlag::Write) && !source.Access.IsSet(GpuAccessFlag::Write))
			error = "The source of a barrier is missing the writes of the resource since the previous barrier.";
		else if(isWrittenOnOtherQueue)
			error = "The resource was written on another queue since the previous barrier. Bring it to this queue with a split barrier between the queues first.";

		if(phase == GpuBarrierPhase::Release)
			state.PendingAcquire = split;
		else
		{
			state.PendingAcquire = nullptr;
			state.Declared = barrier.Destination;
			state.DeclaredQueueType = queueType;
			state.IsDeclared = true;
		}

		for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			state.UsedStages[queueTypeIndex] = GpuStageFlag::None;
			state.UsedAccess[queueTypeIndex] = GpuAccessFlag::None;
		}

		state.WriterOperation = 0;
		return error;
	}

	const char* GpuBarrierValidator::ApplyAccess(ResourceState& state, GpuQueueType queueType, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access, const GpuAccessState* undeclaredState,
		u32 operation)
	{
		const GpuAccessState* const declared = state.IsDeclared ? &state.Declared : undeclaredState;

		// A queue wait orders accesses on another queue than the last barrier's after it, so the barrier's stages do not apply to them
		const bool isOnOtherQueue = state.IsDeclared && queueType != state.DeclaredQueueType;

		const char* error = nullptr;
		if(state.PendingAcquire != nullptr)
			error = "The resource was accessed between the release and the acquire of a split barrier.";
		else if(isOnOtherQueue && (state.Declared.Queues & GpuQueueMask(GpuQueueId(queueType, 0))).IsEmpty())
			error = "The resource was accessed on a queue that its last barrier does not declare in GpuAccessState::Queues.";
		else if(declared != nullptr && !isOnOtherQueue && !declared->Stages.IsSetAll(stages))
			error = state.IsDeclared ? "The resource was accessed in stages that its last barrier does not declare." : "The resource was accessed in stages its resting state does not allow, without a barrier.";
		else if(declared != nullptr && !declared->Access.IsSetAll(access))
			error = state.IsDeclared ? "The resource was accessed with reads or writes that its last barrier does not declare." : "The resource was accessed with reads or writes its resting state does not allow, without a barrier.";
		else if(declared != nullptr && layout != GpuImageLayout::Undefined && layout != declared->Layout)
			error = state.IsDeclared ? "The image was accessed in a different layout than its last barrier declares." : "The image was accessed in a different layout than its resting state, without a barrier.";
		else if(operation != 0 && state.WriterOperation != 0 && state.WriterOperation != operation)
			error = "The resource was accessed by a draw or dispatch after another draw or dispatch wrote it, without a barrier in between.";

		state.UsedStages[queueType] |= stages;
		state.UsedAccess[queueType] |= access;

		if(operation != 0 && access.IsSet(GpuAccessFlag::Write))
			state.WriterOperation = operation;

		return error;
	}

	template<class TFunction>
	const char* GpuBarrierValidator::ForEachSubresourceState(const ImageSubresources& image, TFunction&& function)
	{
		const GpuTextureSubresourceRange& fullRange = image.FullRange;
		const GpuTextureSubresourceRange& clampedRange = image.Range;

		Vector<ResourceState>& states = mImages[image.Key];
		if(states.empty())
			states.resize(fullRange.GetSubresourceCount());

		const char* firstError = nullptr;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!fullRange.AspectMask.IsSet(aspect) || !clampedRange.AspectMask.IsSet(aspect))
				continue;

			for(u32 face = clampedRange.BaseArrayLayer; face < clampedRange.BaseArrayLayer + clampedRange.ArrayLayerCount; face++)
			{
				for(u32 mip = clampedRange.BaseMipLevel; mip < clampedRange.BaseMipLevel + clampedRange.MipLevelCount; mip++)
				{
					const char* const error = function(states[fullRange.GetSubresourceIndex(mip, face, aspect)]);
					if(firstError == nullptr)
						firstError = error;
				}
			}
		}

		return firstError;
	}
} // namespace b3d::render

#endif
