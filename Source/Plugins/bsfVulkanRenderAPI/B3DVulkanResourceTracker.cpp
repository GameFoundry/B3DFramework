//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResourceTracker.h"

#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSwapChain.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanUtility.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DVulkanBarrierHelper.h"

using namespace b3d;
using namespace b3d::render;

WriteHazardPipelineTracking::WriteHazardPipelineTracking()
{
	// Everything is safe to access by default
	std::fill(SafeAccess.begin(), SafeAccess.end(), VulkanAccessStageFlag::All);
}

void WriteHazardPipelineTracking::ClearStageSafeAccess(VulkanAccessStageFlags stages)
{
	u32 stagesAsInteger = (u32)stages;
	while(stagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(stagesAsInteger);
		SafeAccess[stageFlagIndex] = VulkanAccessStageFlag::None;

		stagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

void WriteHazardPipelineTracking::AddStageSafeAccess(VulkanAccessStageFlags sourceStages, VulkanAccessStageFlags destinationStages)
{
	u32 sourceStagesAsInteger = (u32)sourceStages;
	while(sourceStagesAsInteger != 0)
	{
		const u32 stageFlagIndex = Bitwise::LeastSignificantBit(sourceStagesAsInteger);
		SafeAccess[stageFlagIndex] |= destinationStages;

		sourceStagesAsInteger &= ~(1 << stageFlagIndex);
	}
}

bool WriteHazardPipelineTracking::IsAccessSafe(VulkanAccessStageFlags stages) const
{
	for(const auto& entry : SafeAccess)
	{
		if((entry & stages) != stages)
			return false;
	}

	return true;
}

VulkanAccessStageFlags WriteHazardPipelineTracking::GetUnsafeAccessStages(VulkanAccessStageFlags stages) const
{
	VulkanAccessStageFlags unsafeStages = VulkanAccessStageFlag::None;

	for(u32 stageFlagIndex = 0; stageFlagIndex < (u32)SafeAccess.size(); stageFlagIndex++)
	{
		if((SafeAccess[stageFlagIndex] & stages) != stages)
			unsafeStages |= (VulkanAccessStageFlag)(1 << stageFlagIndex);
	}

	return unsafeStages;
}

void WriteHazardPipelineTracking::LogUnsafeAccess(VulkanAccessStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const
{
	StringStream stream;
	for(u32 stageIndex = 0; stageIndex < (u32)SafeAccess.size(); stageIndex++)
	{
		const VulkanAccessStageFlags& safeStages = SafeAccess[stageIndex];

		if((safeStages & stages) != stages)
		{
			const VulkanAccessStageFlag accessStageFlags = (VulkanAccessStageFlag)(1 << stageIndex);

			stream << "A resource was previously " << (previousAccessType.IsSet(GpuAccessFlag::Write) ? "WRITTEN" : "READ") << " ";
			stream << "on stage [" << VulkanUtility::GetAccessStageName(accessStageFlags) << "], ";

			stream << "and it's now being accessed for ";
			stream << (currentAccessType.IsSet(GpuAccessFlag::Write) ? "WRITE" : "READ") << " on stage(s) [";

			VulkanUtility::GetAccessStageNames(stages, stream);

			stream << "] without a barrier being issued. Issue a barrier with correct usage between those two accesses.";
		}
	}

	B3D_LOG(Warning, RenderBackend, "{0}", stream.str());
}

void WriteHazardTracking::AddSafeAccess(VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	const bool needsMemoryBarrier = sourceAccess.IsSet(GpuAccessFlag::Write);
	const bool needsExecutionBarrier = sourceAccess.IsSet(GpuAccessFlag::Read) && destinationAccess.IsSet(GpuAccessFlag::Write) || needsMemoryBarrier;

	if(needsExecutionBarrier)
		ExecutionBarrierTracking.AddStageSafeAccess(sourceAccessStageFlags, destinationAccessStageFlags);

	if(needsMemoryBarrier)
		MemoryBarrierTracking.AddStageSafeAccess(sourceAccessStageFlags, destinationAccessStageFlags);
}

#if B3D_VERIFY_BARRIERS
void WriteHazardTracking::VerifySafeAccess(VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const
{
	// If this image has been previously used prevent read-after-write and write-after-read hazards
	if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		// Read-after-write (and write-after-write, as little sense does that make)
		if(Access.IsSet(GpuAccessFlag::Write))
		{
			// Triggers if user did not issue a RAW memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
			if(!MemoryBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
			{
				MemoryBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, destinationAccess, GpuAccessFlag::Write);
				B3D_ENSURE(false);
			}
		}
	}

	// No need to check for write-after-read barrier for framebuffer as it only needs an execution dependency and that is already handled by the render pass
	if(destinationAccessStageFlags.IsSetAny(VulkanAccessStageFlag::ColorAttachment | VulkanAccessStageFlag::EarlyFragmentTests | VulkanAccessStageFlag::LateFragmentTests))
	{
		if(destinationAccess.IsSet(GpuAccessFlag::Write))
		{
			// Write-after-read
			if(Access.IsSet(GpuAccessFlag::Read))
			{
				// Triggers if user did not issue a WAR memory barrier between a previous write and this usage (or did not specify all the relevant stages in the barrier)
				if(!ExecutionBarrierTracking.IsAccessSafe(destinationAccessStageFlags))
				{
					ExecutionBarrierTracking.LogUnsafeAccess(destinationAccessStageFlags, GpuAccessFlag::Write, GpuAccessFlag::Read);
					B3D_ENSURE(false);
				}
			}
		}
	}
}
#endif

VulkanResourceTracker::VulkanResourceTracker(VulkanGpuCommandBuffer* commandBuffer)
	: mCommandBuffer(commandBuffer)
{
}

VulkanResourceTracker::BufferTrackingState& VulkanResourceTracker::GetOrCreateBufferTrackingState(VulkanBuffer* buffer)
{
	auto insertResult = mBuffers.insert(std::make_pair(buffer, BufferTrackingState()));
	if(insertResult.second) // New element
	{
		BufferTrackingState& bufferTrackingState = insertResult.first->second;
		bufferTrackingState.UseFlags = GpuResourceUseFlag::Undefined;

		bufferTrackingState.UseHandle.Used = false;
		bufferTrackingState.UseHandle.Flags = GpuAccessFlag::None;
		bufferTrackingState.WriteHazardTracking = mWriteHazardPool.Construct<WriteHazardTracking>();

		buffer->NotifyBound();

		return bufferTrackingState;
	}
	else // Existing element
	{
		BufferTrackingState& bufferTrackingState = insertResult.first->second;
		return bufferTrackingState;
	}
}

void VulkanResourceTracker::TrackBufferUsage(VulkanBuffer* buffer, BufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	B3D_ASSERT(!bufferTrackingState.UseHandle.Used);

	const VulkanBarrierHelper::BarrierTrackingInfo* barrierTrackingInfo = barrierHelper.AddBufferBarrier(buffer, bufferTrackingState, useFlags, access);

	const VulkanAccessStageFlags accessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(useFlags);
	WriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the buffer.
	WriteHazardTracking writeHazardTrackingCopy = *writeHazardTracking;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.AddSafeAccess(barrierTrackingInfo->SourceAccessStages, barrierTrackingInfo->SourceAccess, barrierTrackingInfo->DestinationAccessStages, barrierTrackingInfo->DestinationAccess);

	writeHazardTrackingCopy.VerifySafeAccess(accessStageFlags, access);
#endif

	writeHazardTracking->Access |= access;

	if(access.IsSet(GpuAccessFlag::Read))
		writeHazardTracking->ExecutionBarrierTracking.ClearStageSafeAccess(accessStageFlags);

	if(access.IsSet(GpuAccessFlag::Write))
		writeHazardTracking->MemoryBarrierTracking.ClearStageSafeAccess(accessStageFlags);

	bufferTrackingState.UseHandle.Flags |= access;
	bufferTrackingState.UseFlags |= useFlags;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Calculate suballocation index from dynamic offset and track it
	const u32 suballocationIndex = buffer->GetSuballocationIndexForOffset(dynamicOffset);

	// Track this suballocation (avoid duplicates if same suballocation bound multiple times)
	bool alreadyTracked = false;
	for(u32 existingIndex : bufferTrackingState.BoundSuballocationIndices)
	{
		if(existingIndex == suballocationIndex)
		{
			alreadyTracked = true;
			break;
		}
	}

	if(!alreadyTracked)
	{
		bufferTrackingState.BoundSuballocationIndices.Add(suballocationIndex);
		buffer->NotifySuballocationBound(suballocationIndex);
	}
#endif
}

void VulkanResourceTracker::TrackBufferUsage(VulkanBuffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset)
{
	BufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	TrackBufferUsage(buffer, bufferTrackingState, useFlags, accessFlags, barrierHelper, dynamicOffset);
}

void VulkanResourceTracker::TrackImageUsage(VulkanImage* image, VkImageSubresourceRange subresourceRange, VkImageLayout layout, VkImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper)
{
	ImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	B3D_ASSERT(!imageTrackingState.UseHandle.Used);
	imageTrackingState.UseHandle.Flags |= accessFlags;

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		VulkanBarrierHelper* BarrierHelper;
		VulkanImage* Image;
		VkImageLayout Layout;
		VkImageLayout FinalLayout;
		GpuResourceUseFlags UseFlags;
		GpuAccessFlags AccessFlags;
	};

	CallbackParameters callbackParameters { this, &barrierHelper, image, layout, finalLayout, useFlags, accessFlags };
	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* const callbackParameters = (CallbackParameters*)userData;
		VulkanResourceTracker* self = callbackParameters->Self;

		self->TrackSubresourceUsage(callbackParameters->Image, globalSubresourceIndex, callbackParameters->Layout, callbackParameters->FinalLayout, callbackParameters->UseFlags, callbackParameters->AccessFlags, *callbackParameters->BarrierHelper);
	}, &callbackParameters);

	// Register any sub-resources
	B3D_ASSERT(subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS);
	B3D_ASSERT(subresourceRange.levelCount != VK_REMAINING_MIP_LEVELS);

	for(u32 layerIndex = 0; layerIndex < subresourceRange.layerCount; layerIndex++)
	{
		for(u32 levelIndex = 0; levelIndex < subresourceRange.levelCount; levelIndex++)
		{
			const u32 layer = subresourceRange.baseArrayLayer + layerIndex;
			const u32 mipLevel = subresourceRange.baseMipLevel + levelIndex;

			TrackResourceUsage(image->GetSubresource(layer, mipLevel), accessFlags);
		}
	}
}

void VulkanResourceTracker::TrackSubresourceUsage(VulkanImage* image, u32 globalSubresourceIndex, VkImageLayout layout, VkImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper)
{
	const bool isShaderUse = useFlags.IsSet(GpuResourceUseFlag::ShaderAccess);
	const bool isFramebufferUse = useFlags.IsSetAny(GpuResourceUseFlag::ColorAttachment | GpuResourceUseFlag::DepthStencilAttachment);
	const bool isTransferUse = useFlags.IsSetAny(GpuResourceUseFlag::Transfer);

	ImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[globalSubresourceIndex];
	if(subresourceTrackingState.Access == GpuAccessFlag::None) // New subresource
	{
		subresourceTrackingState.InitialLayout = layout;
		subresourceTrackingState.InitialReadOnly = !accessFlags.IsSet(GpuAccessFlag::Write);
		subresourceTrackingState.RenderPassLayout = finalLayout; // TODO - Handle this below
		subresourceTrackingState.CurrentLayout = layout; // TODO - Handle this below
		subresourceTrackingState.RequiredLayout = layout; // TODO - Handle this below
	}
	// TODO - Unify existing and new subresource paths
	else
	{
		// Determine required layout
		if(isShaderUse)
		{
			// Register the necessary layout transition, but only if the image isn't bound for framebuffer bind. If it is
			// then we are forced to use the layout that's expected by the framebuffer.
			if(subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
			{
				// Currently the system doesn't support image being bound to framebuffer, yet being written to by the
				// shader. This seems like an unlikely scenario.
				B3D_ASSERT(!accessFlags.IsSet(GpuAccessFlag::Write));
			}
			else
			{
				// Check if the image had a layout previously assigned, and if so check if multiple different layouts
				// were requested. In that case we wish to transfer the image to GENERAL layout.

				const bool firstUseInRenderPass = !subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write)
					&& !subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write);
				if(firstUseInRenderPass || subresourceTrackingState.RequiredLayout == VK_IMAGE_LAYOUT_UNDEFINED)
					subresourceTrackingState.RequiredLayout = layout;
				else if(subresourceTrackingState.RequiredLayout != layout)
					subresourceTrackingState.RequiredLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
		}
		else if(isFramebufferUse)
		{
			// Framebuffer expects a certain layout and we must respect it. In the case when the FB attachment is also bound
			// for shader reads, this will override the layout required for shader read (GENERAL or DEPTH_READ_ONLY), but that
			// is fine because those transitions are handled automatically by render-pass layout transitions.
			subresourceTrackingState.RequiredLayout = layout;
			subresourceTrackingState.RenderPassLayout = finalLayout;
		}
		else if(isTransferUse)
		{
			subresourceTrackingState.RequiredLayout = layout;
		}
	}

	const VulkanBarrierHelper::BarrierTrackingInfo* const barrierTrackingInfo = barrierHelper.AddSubresourceBarrier(image, subresourceTrackingState, useFlags, accessFlags, subresourceTrackingState.RequiredLayout);

	const VulkanAccessStageFlags accessStageFlags = VulkanUtility::GetVulkanAccessStageFlags(useFlags);
	WriteHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

#if B3D_VERIFY_BARRIERS
	// Make a copy as we need to apply the safe access from the barrier that was registered. We assume the caller will issue the barrier before using the image.
	WriteHazardTracking writeHazardTrackingCopy = *writeHazardTracking;

	if(barrierTrackingInfo != nullptr)
		writeHazardTrackingCopy.AddSafeAccess(barrierTrackingInfo->SourceAccessStages, barrierTrackingInfo->SourceAccess, barrierTrackingInfo->DestinationAccessStages, barrierTrackingInfo->DestinationAccess);

	writeHazardTrackingCopy.VerifySafeAccess(accessStageFlags, accessFlags);
#endif

	writeHazardTracking->Access |= accessFlags;

	if(accessFlags.IsSet(GpuAccessFlag::Read))
		writeHazardTracking->ExecutionBarrierTracking.ClearStageSafeAccess(accessStageFlags);

	if(accessFlags.IsSet(GpuAccessFlag::Write))
		writeHazardTracking->MemoryBarrierTracking.ClearStageSafeAccess(accessStageFlags);

	subresourceTrackingState.Access |= accessFlags;

	if(isShaderUse)
	{
		subresourceTrackingState.ShaderUse |= accessFlags;
		mRenderPassSubresources.insert(globalSubresourceIndex);
	}
	else if(isFramebufferUse)
		subresourceTrackingState.FramebufferUse |= accessFlags;
}

void VulkanResourceTracker::TrackResourceUsage(VulkanResource* resource, GpuAccessFlags access)
{
	auto insertResult = mResources.insert(std::make_pair(resource, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = access;

		resource->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= access;
	}
}

void VulkanResourceTracker::TrackFramebufferUsage(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, RenderSurfaceMask readOnlyMask, VulkanBarrierHelper& barrierHelper)
{
	auto insertResult = mResources.insert(std::make_pair(framebuffer, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = GpuAccessFlag::Write;

		framebuffer->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= GpuAccessFlag::Write;
	}

	// Register any sub-resources
	VulkanRenderPass* renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		VkImageLayout layout;
		if(loadMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)))
			layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			layout = VK_IMAGE_LAYOUT_UNDEFINED;

		GpuAccessFlag access = readOnlyMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		VkImageSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::ColorAttachment, access, barrierHelper);
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& attachment = framebuffer->GetDepthStencilAttachment();

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		VkImageLayout layout;
		if(loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL)) // Can't load one without the other
			layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else
			layout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Note: We purposefully don't check read-only stencil here as generally access tracking doesn't matter for it, as it's always an attachment and shader can't read/write it directly
		const GpuAccessFlag access = readOnlyMask.IsSet(RT_DEPTH) ? GpuAccessFlag::Read : GpuAccessFlag::Write;

		VkImageSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		TrackImageUsage(attachment.Image, range, layout, attachment.FinalLayout, GpuResourceUseFlag::DepthStencilAttachment, access, barrierHelper);
	}
}

void VulkanResourceTracker::TrackSwapChainUsage(VulkanSwapChain* swapChain)
{
	auto insertResult = mSwapChains.insert(std::make_pair(swapChain, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = GpuAccessFlag::Write;

		swapChain->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= GpuAccessFlag::Write;
	}
}

VulkanResourceTracker::ImageTrackingState& VulkanResourceTracker::GetOrCreateImageTrackingState(VulkanImage* image)
{
	const u32 nextImageTrackingIndex = (u32)mImageTrackingState.size();

	auto insertResult = mImages.insert(std::make_pair(image, nextImageTrackingIndex));
	if(insertResult.second) // New element
	{
		mImageTrackingState.push_back(ImageTrackingState());

		ImageTrackingState& imageTrackingState = mImageTrackingState[nextImageTrackingIndex];
		imageTrackingState.FirstSubresourceInfoIndex = ~0u;
		imageTrackingState.SubresourceInfoCount = 0;

		imageTrackingState.UseHandle.Used = false;
		imageTrackingState.UseHandle.Flags = GpuAccessFlag::None;

		image->NotifyBound();
		return imageTrackingState;
	}
	else // Existing element
	{
		const u32 imageTrackingIndex = insertResult.first->second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

		B3D_ASSERT(!imageTrackingState.UseHandle.Used);
		return imageTrackingState;
	}
}

u32 VulkanResourceTracker::FindImageTrackingStateIndex(VulkanImage* image) const
{
	auto found = mImages.find(image);
	if(found == mImages.end())
		return ~0u;

	return found->second;
}

const VulkanResourceTracker::ImageTrackingState* VulkanResourceTracker::FindImageTrackingState(VulkanImage* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	if(imageTrackingIndex == ~0u)
		return nullptr;

	return &mImageTrackingState[imageTrackingIndex];
}

const VulkanResourceTracker::ImageTrackingState& VulkanResourceTracker::GetImageTrackingState(VulkanImage* image) const
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

VulkanResourceTracker::ImageTrackingState& VulkanResourceTracker::GetImageTrackingState(VulkanImage* image)
{
	const u32 imageTrackingIndex = FindImageTrackingStateIndex(image);
	B3D_ASSERT(imageTrackingIndex != ~0u);

	return mImageTrackingState[imageTrackingIndex];
}

TArrayView<const VulkanResourceTracker::ImageSubresourceTrackingState> VulkanResourceTracker::GetSubresourceTrackingStatesForImage(VulkanImage* image) const
{
	const ImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

TArrayView<VulkanResourceTracker::ImageSubresourceTrackingState> VulkanResourceTracker::GetSubresourceTrackingStatesForImage(VulkanImage* image)
{
	ImageTrackingState& imageTrackingState = GetImageTrackingState(image);
	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
		return {};

	return TArrayView(&mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex], imageTrackingState.SubresourceInfoCount);
}

const VulkanResourceTracker::ImageSubresourceTrackingState& VulkanResourceTracker::GetSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip) const
{
	const ImageSubresourceTrackingState* const trackingState = FindSubresourceTrackingState(image, face, mip);
	if(!B3D_ENSURE(trackingState != nullptr))
	{
		// Fallback to first subresource
		const u32 imageTrackingIndex = mImages.find(image)->second;
		const ImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

		const ImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
		return subresourceTrackingStates[0];
	}

	return *trackingState;
}

const VulkanResourceTracker::ImageSubresourceTrackingState* VulkanResourceTracker::FindSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip) const
{
	const u32 imageTrackingIndex = mImages.find(image)->second;
	const ImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	const ImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		const ImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];

		if(face >= subresourceTrackingState.Range.baseArrayLayer && face < (subresourceTrackingState.Range.baseArrayLayer + subresourceTrackingState.Range.layerCount) &&
		   mip >= subresourceTrackingState.Range.baseMipLevel && mip < (subresourceTrackingState.Range.baseMipLevel + subresourceTrackingState.Range.levelCount))
		{
			return &subresourceTrackingState;
		}
	}

	return nullptr;
}

const VulkanResourceTracker::BufferTrackingState* VulkanResourceTracker::FindBufferTrackingState(VulkanBuffer* buffer) const
{
	auto found = mBuffers.find(buffer);
	if(found != mBuffers.end())
		return &found->second;

	return nullptr;
}

VulkanResourceTracker::ImageSubresourceTrackingState& VulkanResourceTracker::GetSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip)
{
	// Delegate to 'const' version and re-cast
	return const_cast<ImageSubresourceTrackingState&>(const_cast<const VulkanResourceTracker*>(this)->GetSubresourceTrackingState(image, face, mip));
}

void VulkanResourceTracker::IterateAndCreateOverlappingImageSubresourceTrackingState(VulkanImage* image, VkImageSubresourceRange subresourceRange, void (*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	ImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, subresourceRange, FnDoOnOverlappingSubresource, userData);
}

void VulkanResourceTracker::IterateAndCreateOverlappingImageSubresourceTrackingState(ImageTrackingState& imageTrackingState, const VulkanImage& image, VkImageSubresourceRange subresourceRange, void(*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData)
{
	// Provide exact size as code below doesn't handle VK_REMAINING_* macros
	if(subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS)
		subresourceRange.layerCount = image.GetRange().layerCount;

	if(subresourceRange.levelCount == VK_REMAINING_MIP_LEVELS)
		subresourceRange.levelCount = image.GetRange().levelCount;

	if(imageTrackingState.FirstSubresourceInfoIndex == ~0u)
	{
		const u32 subresourceIndex = AddSubresourceTrackingState(subresourceRange);
		imageTrackingState.FirstSubresourceInfoIndex = subresourceIndex;
		imageTrackingState.SubresourceInfoCount = 1;

		FnDoOnOverlappingSubresource(subresourceIndex, userData);
		return;
	}

	ImageSubresourceTrackingState* const existingSubresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];

	// First test for the simplest and most common case (same range or no overlap) to avoid more complex computations.
	bool foundRange = false;
	for(u32 subresourceLocalIndex = 0; subresourceLocalIndex < imageTrackingState.SubresourceInfoCount; subresourceLocalIndex++)
	{
		ImageSubresourceTrackingState& existingSubresourceTrackingState = existingSubresourceTrackingStates[subresourceLocalIndex];
		if(VulkanUtility::RangeOverlaps(existingSubresourceTrackingState.Range, subresourceRange))
		{
			if(existingSubresourceTrackingState.Range.layerCount == subresourceRange.layerCount &&
			   existingSubresourceTrackingState.Range.levelCount == subresourceRange.levelCount &&
			   existingSubresourceTrackingState.Range.baseArrayLayer == subresourceRange.baseArrayLayer &&
			   existingSubresourceTrackingState.Range.baseMipLevel == subresourceRange.baseMipLevel)
			{
				const u32 subresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + subresourceLocalIndex;
				FnDoOnOverlappingSubresource(subresourceIndex, userData);
				return;
			}

			// This means there's a partial overlap which means there's no point searching further, we must subdivide
			break;
		}
	}

	// We'll need to update subresource ranges or add new ones. The hope is that this code is trigger VERY rarely
	// (for just a few specific textures per frame).
	if(!foundRange)
	{
		std::array<VkImageSubresourceRange, 5> cutRanges;

		B3DMarkAllocatorFrame();
		{
			// We orphan previously allocated memory (we reset after command buffer is done executing anyway)
			u32 newSubresourceTrackingStateIndex = (u32)mSubresourceTrackingState.size();

			FrameVector<u32> cutOverlappingRanges;
			for(u32 subresourceLocalIndex = 0; subresourceLocalIndex < imageTrackingState.SubresourceInfoCount; subresourceLocalIndex++)
			{
				const u32 globalSubresourceIndex = imageTrackingState.FirstSubresourceInfoIndex + subresourceLocalIndex;
				ImageSubresourceTrackingState& subresource = mSubresourceTrackingState[globalSubresourceIndex];

				if(!VulkanUtility::RangeOverlaps(subresource.Range, subresourceRange))
					CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, subresource.Range);
				else // Need to cut
				{
					u32 cutRangeCount;
					VulkanUtility::CutRange(subresource.Range, subresourceRange, cutRanges, cutRangeCount);

					for(u32 cutRangeIndex = 0; cutRangeIndex < cutRangeCount; cutRangeIndex++)
					{
						// Create a copy of the original subresource with the new range
						const u32 newGlobalSubresourceIndex = CopySubresourceTrackingStateWithNewRange(globalSubresourceIndex, cutRanges[cutRangeIndex]);

						if(VulkanUtility::RangeOverlaps(cutRanges[cutRangeIndex], subresourceRange))
						{
							FnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);

							// Keep track of the overlapping ranges for later
							cutOverlappingRanges.push_back((u32)mSubresourceTrackingState.size() - 1);
						}
					}
				}
			}

			// Our range doesn't overlap with any existing ranges, so just add it
			if(cutOverlappingRanges.empty())
			{
				const u32 newGlobalSubresourceIndex = AddSubresourceTrackingState(subresourceRange);
				FnDoOnOverlappingSubresource(newGlobalSubresourceIndex, userData);
			}
			else // Search if overlapping ranges fully cover the requested range, and insert non-covered regions
			{
				FrameQueue<VkImageSubresourceRange> sourceRanges;
				sourceRanges.push(subresourceRange);

				for(auto& entry : cutOverlappingRanges)
				{
					VkImageSubresourceRange& overlappingRange = mSubresourceTrackingState[entry].Range;

					const u32 sourceRangeCount = (u32)sourceRanges.size();
					for(u32 sourceRangeIndex = 0; sourceRangeIndex < sourceRangeCount; sourceRangeIndex++)
					{
						VkImageSubresourceRange sourceRange = sourceRanges.front();
						sourceRanges.pop();

						u32 cutRangeCount;
						VulkanUtility::CutRange(sourceRange, overlappingRange, cutRanges, cutRangeCount);

						for(u32 cutRangeIndex = 0; cutRangeIndex < cutRangeCount; cutRangeIndex++)
						{
							// We only care about ranges outside of the ones we already covered
							if(!VulkanUtility::RangeOverlaps(cutRanges[cutRangeIndex], overlappingRange))
								sourceRanges.push(cutRanges[cutRangeIndex]);
						}
					}
				}

				// Any remaining range hasn't been covered yet
				while(!sourceRanges.empty())
				{
					AddSubresourceTrackingState(sourceRanges.front());
					sourceRanges.pop();
				}
			}

			imageTrackingState.FirstSubresourceInfoIndex = newSubresourceTrackingStateIndex;
			imageTrackingState.SubresourceInfoCount = (u32)mSubresourceTrackingState.size() - newSubresourceTrackingStateIndex;
		}
		B3DClearAllocatorFrame();
	}
}

u32 VulkanResourceTracker::AddSubresourceTrackingState(const VkImageSubresourceRange& range)
{
	mSubresourceTrackingState.push_back(ImageSubresourceTrackingState());

	ImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState.back();
	subresourceTrackingState.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subresourceTrackingState.InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subresourceTrackingState.RequiredLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subresourceTrackingState.RenderPassLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	subresourceTrackingState.Range = range;
	subresourceTrackingState.WriteHazardTracking = mWriteHazardPool.Construct<WriteHazardTracking>();

	return (u32)mSubresourceTrackingState.size() - 1;
}

u32 VulkanResourceTracker::CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const VkImageSubresourceRange& newRange)
{
	ImageSubresourceTrackingState* const copyFromSubresource = &mSubresourceTrackingState[copyFromIndex];

	ImageSubresourceTrackingState subresourceCopy = *copyFromSubresource;
	subresourceCopy.Range = newRange;

	subresourceCopy.WriteHazardTracking = mWriteHazardPool.Construct<WriteHazardTracking>();

	if(B3D_ENSURE(copyFromSubresource->WriteHazardTracking != nullptr))
		*subresourceCopy.WriteHazardTracking = *copyFromSubresource->WriteHazardTracking;

	const u32 newSubresourceIndex = (u32)mSubresourceTrackingState.size();
	if(copyFromSubresource->ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		mRenderPassSubresources.insert(newSubresourceIndex);

	mSubresourceTrackingState.push_back(subresourceCopy);
	return (u32)mSubresourceTrackingState.size() - 1;
}

void VulkanResourceTracker::UpdateImageLayoutTrackingAfterBarrier(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	ImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		VkImageLayout OldLayout;
		VkImageLayout NewLayout;
	};

	CallbackParameters callbackParameters = { this, oldLayout, newLayout };

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		ImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];

		if(subresourceTrackingState.CurrentLayout != callbackParameters->OldLayout)
		{
			B3D_LOG(Warning, RenderBackend, "Image layout transition failed: current layout does not match expected old layout. "
				"Current layout: {0}, Expected old layout: {1}. The barrier's old layout must match the image's current layout.",
				VulkanUtility::GetImageLayoutName(subresourceTrackingState.CurrentLayout), VulkanUtility::GetImageLayoutName(callbackParameters->OldLayout));
		}

		B3D_ENSURE(subresourceTrackingState.CurrentLayout == callbackParameters->OldLayout);
		subresourceTrackingState.CurrentLayout = callbackParameters->NewLayout;
		subresourceTrackingState.RequiredLayout = callbackParameters->NewLayout; // TODO - RequiredLayout should no longer be necessary with explicit transitions
	}, &callbackParameters);
}

void VulkanResourceTracker::UpdateWriteHazardTrackingAfterBarrier(VulkanBuffer* buffer, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	BufferTrackingState& bufferTrackingState = GetOrCreateBufferTrackingState(buffer);
	WriteHazardTracking* const writeHazardTracking = bufferTrackingState.WriteHazardTracking;

	writeHazardTracking->AddSafeAccess(sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess);
}

void VulkanResourceTracker::UpdateWriteHazardTrackingAfterBarrier(VulkanImage* image, const VkImageSubresourceRange& range, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess)
{
	ImageTrackingState& imageTrackingState = GetOrCreateImageTrackingState(image);

	struct CallbackParameters
	{
		VulkanResourceTracker* Self;
		VulkanAccessStageFlags SourceAccessStageFlags;
		GpuAccessFlags SourceAccess;
		VulkanAccessStageFlags DestinationAccessStageFlags;
		GpuAccessFlags DestinationAccess;
	};

	CallbackParameters callbackParameters = { this, sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess };

	IterateAndCreateOverlappingImageSubresourceTrackingState(imageTrackingState, *image, range, [](u32 globalSubresourceIndex, void* userData)
	{
		CallbackParameters* callbackParameters = (CallbackParameters*)userData;

		ImageSubresourceTrackingState& subresourceTrackingState = callbackParameters->Self->mSubresourceTrackingState[globalSubresourceIndex];
		WriteHazardTracking* const writeHazardTracking = subresourceTrackingState.WriteHazardTracking;

		writeHazardTracking->AddSafeAccess(callbackParameters->SourceAccessStageFlags, callbackParameters->SourceAccess, callbackParameters->DestinationAccessStageFlags, callbackParameters->DestinationAccess);

	}, &callbackParameters);
}

void VulkanResourceTracker::NotifyUsed(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		BufferTrackingState& trackingState = entry.second;
		ResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);

#if B3D_BUILD_TYPE_DEVELOPMENT
		VulkanBuffer* buffer = static_cast<VulkanBuffer*>(entry.first);
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			buffer->NotifySuballocationUsed(suballocationIndex);
#endif
	}

	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(queueId, useHandle.Flags);
	}
}

void VulkanResourceTracker::NotifyDone(GpuQueueId queueId)
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		BufferTrackingState& trackingState = entry.second;
		ResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(useHandle.Used);

#if B3D_BUILD_TYPE_DEVELOPMENT
		VulkanBuffer* buffer = static_cast<VulkanBuffer*>(entry.first);
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			buffer->NotifySuballocationDone(suballocationIndex);
#endif

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(useHandle.Used);

		entry.first->NotifyDone(queueId, useHandle.Flags);
	}
}

void VulkanResourceTracker::NotifyUnbound()
{
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mImages)
	{
		const u32 trackingImageStateIndex = entry.second;
		ImageTrackingState& imageTrackingState = mImageTrackingState[trackingImageStateIndex];

		ResourceUseHandle& useHandle = imageTrackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}

	for(auto& entry : mBuffers)
	{
		BufferTrackingState& trackingState = entry.second;
		ResourceUseHandle& useHandle = trackingState.UseHandle;
		B3D_ASSERT(!useHandle.Used);

#if B3D_BUILD_TYPE_DEVELOPMENT
		VulkanBuffer* buffer = static_cast<VulkanBuffer*>(entry.first);
		for(u32 suballocationIndex : trackingState.BoundSuballocationIndices)
			buffer->NotifySuballocationUnbound(suballocationIndex);
#endif

		entry.first->NotifyUnbound();
	}

	// Must be done after images & framebuffer because swap chain does error checking if those were freed
	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		entry.first->NotifyUnbound();
	}
}

void VulkanResourceTracker::Clear()
{
	for(auto& entry : mBuffers)
	{
		if(entry.second.WriteHazardTracking != nullptr)
			mWriteHazardPool.Destruct(entry.second.WriteHazardTracking);
	}

	for(auto& entry : mSubresourceTrackingState)
	{
		if(entry.WriteHazardTracking != nullptr)
			mWriteHazardPool.Destruct(entry.WriteHazardTracking);
	}

	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mImageTrackingState.clear();
	mSubresourceTrackingState.clear();
	mRenderPassSubresources.clear();
}

void VulkanResourceTracker::ClearFramebufferFlagsForImage(VulkanImage* image)
{
	const u32 imageTrackingIndex = mImages[image];
	ImageTrackingState& imageTrackingState = mImageTrackingState[imageTrackingIndex];

	ImageSubresourceTrackingState* const subresourceTrackingStates = &mSubresourceTrackingState[imageTrackingState.FirstSubresourceInfoIndex];
	for(u32 localSubresourceIndex = 0; localSubresourceIndex < imageTrackingState.SubresourceInfoCount; localSubresourceIndex++)
	{
		ImageSubresourceTrackingState& subresourceTrackingState = subresourceTrackingStates[localSubresourceIndex];
		subresourceTrackingState.FramebufferUse = GpuAccessFlag::None;
	}
}

void VulkanResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for(const auto& subresourceIndex : mRenderPassSubresources)
	{
		ImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[subresourceIndex];
		subresourceTrackingState.ShaderUse = GpuAccessFlag::None;
	}

	mRenderPassSubresources.clear();
}

VkImageLayout VulkanResourceTracker::GetCurrentSubresourceLayout(VulkanImage* image, const VkImageSubresourceRange& range, VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	const u32 face = range.baseArrayLayer;
	const u32 mip = range.baseMipLevel;

#if B3D_BUILD_TYPE_DEVELOPMENT
	const ImageTrackingState* const imageTrackingState = FindImageTrackingState(image);
	if(imageTrackingState == nullptr)
	{
		B3D_ASSERT(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}
#endif

	VulkanRenderPass* renderPass = nullptr;
	if(framebuffer != nullptr)
		renderPass = framebuffer->GetRenderPass();

	TArrayView<const ImageSubresourceTrackingState> subresourceTrackingStates = GetSubresourceTrackingStatesForImage(image);
	for(const auto& subresourceTrackingState : subresourceTrackingStates)
	{
		if(face >= subresourceTrackingState.Range.baseArrayLayer && face < (subresourceTrackingState.Range.baseArrayLayer + subresourceTrackingState.Range.layerCount) &&
		   mip >= subresourceTrackingState.Range.baseMipLevel && mip < (subresourceTrackingState.Range.baseMipLevel + subresourceTrackingState.Range.levelCount))
		{
			// If it's a FB attachment, retrieve its layout after the render pass begins
			if(subresourceTrackingState.FramebufferUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write) && framebuffer != nullptr)
			{
				RenderSurfaceMask readMask = GetFramebufferReadOnlyMask(framebuffer, explicitReadOnlyMask);

				// Is it a depth-stencil attachment?
				if(renderPass->HasDepthAttachment() && framebuffer->GetDepthStencilAttachment().Image == image)
				{
					if(readMask.IsSet(RT_DEPTH))
					{
						if(readMask.IsSet(RT_STENCIL))
							return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
						else // Depth readable but stencil isn't
							return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;
					}
					else
					{
						if(readMask.IsSet(RT_STENCIL)) // Stencil readable but depth isn't
							return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
						else
							return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					}
				}
				else // It is a color attachment
				{
					const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
					for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
					{
						const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorAttachmentIndex);

						if(attachment.Image == image)
						{
							if(readMask.IsSet((RenderSurfaceMaskBits)(1 << attachment.Index)))
								return VK_IMAGE_LAYOUT_GENERAL;
							else
								return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
					}
				}
			}

			return subresourceTrackingState.RequiredLayout;
		}
	}

	B3D_ASSERT(false);
	return VK_IMAGE_LAYOUT_UNDEFINED;
}

RenderSurfaceMask VulkanResourceTracker::GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const
{
	// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
	VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	RenderSurfaceMask readMask = RT_NONE;

	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		const ImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set((RenderSurfaceMaskBits)(1 << colorAttachmentIndex));
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		const ImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		const bool readOnly = subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write); // Note: Should report error if shader write is used

		if(readOnly)
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_DEPTH))
			readMask.Set(RT_DEPTH);

		if(explicitReadOnlyMask.IsSet(RT_STENCIL))
			readMask.Set(RT_STENCIL);
	}

	return readMask;
}

void VulkanResourceTracker::MoveAllFramebufferAttachmentsToFinalLayouts(VulkanFramebuffer* framebuffer)
{
	const VulkanRenderPass* const renderPass = framebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachmentCount; colorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetColorAttachment(colorAttachmentIndex);
		ImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = framebuffer->GetDepthStencilAttachment();
		ImageSubresourceTrackingState& subresourceTrackingState = GetSubresourceTrackingState(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceTrackingState.CurrentLayout = subresourceTrackingState.RenderPassLayout;
		subresourceTrackingState.RequiredLayout = subresourceTrackingState.RenderPassLayout;
	}
}

