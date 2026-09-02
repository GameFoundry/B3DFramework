//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanSwapChain.h"
#include "Managers/B3DVulkanVertexInputManager.h"
#include "B3DVulkanEventQuery.h"
#include "B3DVulkanRenderPass.h"
#include "B3DVulkanRenderTexture.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DIVulkanRenderWindowSurface.h"
#include "Managers/B3DVulkanQueries.h"
#include "Profiling/B3DRenderStats.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Utility/B3DVulkanBarrierHelper.h"

using namespace b3d;
using namespace b3d::render;

VulkanSemaphore::VulkanSemaphore(VulkanResourceManager* owner, const StringView& name)
	: VulkanResource(owner, true, name)
{
	VkSemaphoreCreateInfo semaphoreCI;
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;

	VkResult result = vkCreateSemaphore(owner->GetDevice().GetLogical(), &semaphoreCI, gVulkanAllocator, &mSemaphore);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanSemaphore::~VulkanSemaphore()
{
	vkDestroySemaphore(mOwner->GetDevice().GetLogical(), mSemaphore, gVulkanAllocator);
}

VulkanGpuCommandBufferPool::VulkanGpuCommandBufferPool(VulkanGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation)
	:  GpuCommandBufferPool(device, createInformation)
{
	const u32 queueFamily = device.GetQueueFamily(createInformation.Type);

	if (!B3D_ENSURE(queueFamily != ~0u))
		return;

	VkCommandPoolCreateInfo vulkanPoolCreateInformation;
	vulkanPoolCreateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vulkanPoolCreateInformation.pNext = nullptr;
	vulkanPoolCreateInformation.flags = createInformation.UsePoolReset ? 0 : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vulkanPoolCreateInformation.queueFamilyIndex = queueFamily;

	mQueueFamily = queueFamily;
	vkCreateCommandPool(device.GetLogical(), &vulkanPoolCreateInformation, gVulkanAllocator, &mVulkanPool);
}

VulkanGpuCommandBufferPool::~VulkanGpuCommandBufferPool()
{
	VulkanGpuCommandBufferPool::Destroy();
}

void VulkanGpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	EnsureValidThread();

	// Reset the pool before destroying it, so any command buffers in Done state transition to Ready state
	if(mInformation.UsePoolReset)
		Reset();

	bool areAnyCommandBuffersStillExecuting = false;
	for(const auto& commandBufferPair : mCommandBuffers)
	{
		if(commandBufferPair.second->GetState() != GpuCommandBufferState::Ready)
		{
			areAnyCommandBuffersStillExecuting = true;
			break;
		}
	}

	if(areAnyCommandBuffersStillExecuting)
		mGpuDevice.GetSubmitThread().WaitUntilIdle();

	mMessageQueue.PostRequestShutdownCommand(true);

	// Destroy all command buffers before destroying the pool
	for(const auto& commandBufferPair : mCommandBuffers)
		commandBufferPair.second->Destroy();

	mCommandBuffers.clear();
	vkDestroyCommandPool(static_cast<VulkanGpuDevice&>(mGpuDevice).GetLogical(), mVulkanPool, gVulkanAllocator);

	Base::Destroy();
}

TShared<GpuCommandBuffer> VulkanGpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	for(const auto& commandBufferPair : mCommandBuffers)
	{
		if (commandBufferPair.second->GetState() != GpuCommandBufferState::Ready)
			continue;

		commandBufferPair.second->SetName(createInformation.Name);
		commandBufferPair.second->Begin();

		return commandBufferPair.second;
	}

	return Create(createInformation);
}

TShared<GpuCommandBuffer> VulkanGpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	VkCommandBufferAllocateInfo cmdBufferAllocInfo;
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.pNext = nullptr;
	cmdBufferAllocInfo.commandPool = mVulkanPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBufferHandle = VK_NULL_HANDLE;
	VkResult result = vkAllocateCommandBuffers(static_cast<VulkanGpuDevice&>(mGpuDevice).GetLogical(), &cmdBufferAllocInfo, &commandBufferHandle);
	B3D_ASSERT(result == VK_SUCCESS);

	TShared<VulkanGpuCommandBuffer> commandBuffer = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuCommandBuffer>()) VulkanGpuCommandBuffer(static_cast<VulkanGpuDevice&>(mGpuDevice), *this, mNextCommandBufferId++, commandBufferHandle, mInformation.Thread, mInformation.Type, createInformation),
		[](VulkanGpuCommandBuffer* commandBuffer)
		{
			commandBuffer->Destroy();
			B3DDelete(commandBuffer);
		});

	mCommandBuffers[commandBuffer->GetId()] = commandBuffer;

	commandBuffer->SetShared(commandBuffer);
	commandBuffer->Begin();

	return commandBuffer;
}

void VulkanGpuCommandBufferPool::Reset()
{
	EnsureValidThread();

	VkDevice logicalDevice = static_cast<VulkanGpuDevice&>(mGpuDevice).GetLogical();

	for(const auto& entry : mCommandBuffers)
	{
		const GpuCommandBufferState state = entry.second->GetState();

		// Already reset and was not used since
		if(state == GpuCommandBufferState::Ready)
			continue;

		B3D_ASSERT(state == GpuCommandBufferState::Done || state == GpuCommandBufferState::RecordingDone);
		entry.second->NotifyParentPoolReset();
	}

	const VkResult result = vkResetCommandPool(logicalDevice, mVulkanPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	B3D_ASSERT(result == VK_SUCCESS);
}

const Color kDebugLabelColor = Color::kBansheeOrange;
constexpr u32 kMaximumBoundDescriptorSets = 64;

VulkanGpuCommandBuffer::VulkanGpuCommandBuffer(VulkanGpuDevice& device, VulkanGpuCommandBufferPool& pool, u32 id, VkCommandBuffer commandBufferHandle, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandBufferHandle(commandBufferHandle), mPool(pool), mOwnerThread(ownerThread), mGfxPipelineRequiresBind(true), mCmpPipelineRequiresBind(true), mGraphicsPushConstantsRequireBind(false), mComputePushConstantsRequireBind(false), mViewportRequiresBind(true), mStencilRefRequiresBind(true), mScissorRequiresBind(true), mBoundParamsDirty(false), mVertexInputsDirty(false), mBarrierHelper(&mResourceTracker)
{
	const u32 maximumBoundDescriptorSets = Math::Min(kMaximumBoundDescriptorSets, device.GetDeviceProperties().limits.maxBoundDescriptorSets);
	mDescriptorSetsTemp = (VkDescriptorSet*)B3DAllocate(sizeof(VkDescriptorSet) * maximumBoundDescriptorSets);

	VkFenceCreateInfo fenceCI;
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = 0;

	const VkResult result = vkCreateFence(GetVulkanGpuDevice().GetLogical(), &fenceCI, gVulkanAllocator, &mFence);
	B3D_ASSERT(result == VK_SUCCESS);

	SetName(createInformation.Name);
}

VulkanGpuCommandBuffer::~VulkanGpuCommandBuffer()
{
	if(IsRecording())
	{
		// If there are any non-submitted resources, this will release them
		End();
		Reset();
	}

	VkDevice device = GetVulkanGpuDevice().GetLogical();

	if(mState == GpuCommandBufferState::Executing)
	{
		// Wait 1s
		u64 waitTime = 1000 * 1000 * 1000;
		VkResult result = vkWaitForFences(device, 1, &mFence, true, waitTime);
		B3D_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT);

		if(result == VK_TIMEOUT)
			B3D_LOG(Warning, LogRenderBackend, "Freeing a command buffer before done executing because fence wait expired!");

		// Resources have been marked as used, make sure to notify them we're done with them
		Reset();
	}
	else if(mState != GpuCommandBufferState::Ready)
		mResourceTracker.NotifyUnbound();

	vkDestroyFence(device, mFence, gVulkanAllocator);
	B3DFree(mDescriptorSetsTemp);
}

void VulkanGpuCommandBuffer::Begin()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Ready);

	const VkResult resetResult = vkResetFences(GetVulkanGpuDevice().GetLogical(), 1, &mFence);
	B3D_ASSERT(resetResult == VK_SUCCESS);

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(mCommandBufferHandle, &beginInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	mState = GpuCommandBufferState::Recording;
	mPushConstants.Clear();
	mGraphicsPushConstantsRequireBind = false;
	mComputePushConstantsRequireBind = false;
}

void VulkanGpuCommandBuffer::End()
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	if(mIsDebugLabelOpen)
		EndLabel();

	VkResult result = vkEndCommandBuffer(mCommandBufferHandle);
	B3D_ASSERT(result == VK_SUCCESS);

	mRenderTarget = nullptr;
	mState = GpuCommandBufferState::RecordingDone;
}

void VulkanGpuCommandBuffer::BeginRenderPass(const RenderPassCreateInformation& createInformation)
{
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	const TShared<RenderTarget>& renderTarget = createInformation.Target;
	if(!B3D_ENSURE(renderTarget != nullptr))
		return;

	RenderSurfaceMask readOnlyMask = createInformation.ReadOnlyMask;
	RenderSurfaceMask loadMask = createInformation.LoadMask;

	VulkanFramebuffer* newFramebuffer;
	VulkanSwapChain* swapChain = nullptr;
	if(renderTarget->GetProperties().IsWindow)
	{
		RenderWindow* const renderWindow = static_cast<RenderWindow*>(renderTarget.get());

		IVulkanRenderWindowSurface* const renderWindowSurface = static_cast<IVulkanRenderWindowSurface*>(renderWindow->GetRenderWindowSurface().get());
		if(!B3D_ENSURE(renderWindowSurface != nullptr))
			return;

		if(!renderWindowSurface->IsSwapChainValid())
			renderWindow->RebuildSwapChain();

		newFramebuffer = renderWindowSurface->GetActiveFramebuffer();
		if(newFramebuffer != nullptr)
		{
			// Track surface (only add if not already tracked)
			auto found = std::find(mAcquiredSurfaces.begin(), mAcquiredSurfaces.end(), renderWindowSurface);
			if(found == mAcquiredSurfaces.end())
				mAcquiredSurfaces.push_back(renderWindowSurface);
		}
		else
		{
			B3D_LOG(Error, LogRenderBackend, "Binding render target failed. Unable to acquire swap chain image.");
		}

		swapChain = renderWindowSurface->GetSwapChain();
	}
	else
	{
		const VulkanRenderTexture* const renderTexture = static_cast<VulkanRenderTexture*>(renderTarget.get());
		newFramebuffer = renderTexture->GetFramebuffer();
	}

	if(!B3D_ENSURE(newFramebuffer != nullptr))
		return;

	mRenderTarget = renderTarget;
	mRenderTargetModified = false;

	// Warn if invalid load mask
	if(loadMask.IsSet(RT_DEPTH) && !loadMask.IsSet(RT_STENCIL))
	{
		B3D_LOG(Warning, LogRenderBackend, "SetRenderTarget() invalid load mask, depth enabled but stencil disabled. "
										   "This is not supported. Both will be loaded.");

		loadMask.Set(RT_STENCIL);
	}

	if(!loadMask.IsSet(RT_DEPTH) && loadMask.IsSet(RT_STENCIL))
	{
		B3D_LOG(Warning, LogRenderBackend, "SetRenderTarget() invalid load mask, stencil enabled but depth disabled. "
										   "This is not supported. Both will be loaded.");

		loadMask.Set(RT_DEPTH);
	}

	mFramebuffer = newFramebuffer;
	mRenderTargetReadOnlyMask = readOnlyMask;

	mResourceTracker.TrackResourceUsage(mFramebuffer, GpuAccessFlag::Write);

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	const GpuRenderPassAttachmentUsageArray renderPassAttachmentUsages = mFramebuffer->BuildRenderPassAttachmentUsages(readOnlyMask, loadMask, VulkanFramebuffer::GetLayoutPolicy());

	mResourceTracker.PrepareRenderPass(renderPassAttachmentUsages);

	if(swapChain)
		mResourceTracker.TrackSwapChainUsage(swapChain);

	// Pre-register all GPU parameters before the render pass, so we can automatically issue barriers
	for(const TShared<GpuParameterSet>& parameters : createInformation.Parameters)
	{
		if(parameters == nullptr)
			continue;

		VulkanGpuParameterSet* vkParams = static_cast<VulkanGpuParameterSet*>(parameters.get());
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		TInlineArray<u32, 4> tempDynamicOffsets;
		vkParams->PrepareForBind(mResourceTracker, mBarrierHelper, descriptorSet, tempDynamicOffsets);

		// Cache the preparation results for later use by SetGpuParameterSet
		CachedGpuParameterData& cacheData = mRenderPassGpuParameterSetCache[parameters.get()];
		cacheData.DescriptorSet = descriptorSet;
		cacheData.DynamicOffsets = std::move(tempDynamicOffsets);
	}

	const TArrayView<const GpuResolvedRenderPassAttachmentUsage> resolvedAttachments = mResourceTracker.BeginRenderPass(mBarrierHelper);
	B3D_ASSERT(resolvedAttachments.size() == renderPassAttachmentUsages.size());

	RenderSurfaceMask resolvedReadOnlyMask = RT_NONE;
	for(const GpuResolvedRenderPassAttachmentUsage& attachment : resolvedAttachments)
	{
		if(attachment.Access == GpuAccessFlag::Read)
			resolvedReadOnlyMask.Set(attachment.Surface);
	}

	mRenderTargetReadOnlyMask = resolvedReadOnlyMask;

	mBarrierHelper.Execute(*this);

	// Re-set the params as they will need to be re-bound
	for(const auto& entry : mBoundGpuParameterSets) // TODO - Can likely be removed
		SetGpuParameterSet(entry);

	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;

	const Area2I renderArea = GetRenderPassArea();

	const RenderSurfaceMask readMask = mRenderTargetReadOnlyMask;
	const RenderSurfaceMask originalClearMask = createInformation.ClearMask;
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues = BuildClearValues(originalClearMask);

	RenderSurfaceMask clearMask = createInformation.ClearMask;

#if B3D_DEBUG
	const VkClearColorValue kDebugClearColor = { { 1.0f, 0.0f, 1.0f, 1.0f } }; // Bright pink

	for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
	{
		const GpuFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachments()[sequentialColorAttachmentIndex];
		const RenderSurfaceMaskBits colorAttachmentBit = colorAttachment.Surface;
		if(loadMask.IsSet(colorAttachmentBit))
			continue;

		if(readMask.IsSet(colorAttachmentBit))
		{
			B3D_LOG(Error, LogRenderBackend, "Color attachment at index {0} cannot be read only if we're not loading it.", colorAttachment.GetIndex());
			continue;
		}

		// In debug mode clear not loaded values to the clear color
		if(!originalClearMask.IsSet(colorAttachmentBit))
		{
			clearMask |= colorAttachmentBit;
			clearValues[sequentialColorAttachmentIndex].color = kDebugClearColor;
		}
	}

	if(renderPass->HasDepthAttachment())
	{
		if(!loadMask.IsSet(RT_DEPTH))
		{
			if(readMask.IsSet(RT_DEPTH))
			{
				B3D_LOG(Error, LogRenderBackend, "Depth attachment cannot be read only if we're not loading it.");
			}
			else
			{
				if(!originalClearMask.IsSet(RT_DEPTH))
				{
					clearMask |= RT_DEPTH;
					clearValues[colorAttachmentCount].depthStencil.depth = 0.0f;
				}
			}
		}

		if(!loadMask.IsSet(RT_STENCIL))
		{
			if(readMask.IsSet(RT_STENCIL))
			{
				B3D_LOG(Error, LogRenderBackend, "Stencil attachment cannot be read only if we're not loading it.");
			}
			else
			{
				if(!originalClearMask.IsSet(RT_STENCIL))
				{
					clearMask |= RT_STENCIL;
					clearValues[colorAttachmentCount].depthStencil.stencil = 0;
				}
			}
		}
	}
#endif

	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.framebuffer = mFramebuffer->GetVulkanHandle();
	renderPassBeginInfo.renderPass = renderPass->GetVkRenderPass(loadMask, readMask, clearMask);
	renderPassBeginInfo.renderArea = VulkanUtility::ToVulkanRect(renderArea);
	renderPassBeginInfo.clearValueCount = renderPass->GetClearEntryCount(clearMask);
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(mCommandBufferHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	mState = GpuCommandBufferState::RecordingRenderPass;

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void VulkanGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask)
{
	EnsureValidThread();

	Area2I area(0, 0, mFramebuffer->GetWidth(), mFramebuffer->GetHeight());
	ClearAttachments(area, mask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask)
{
	EnsureValidThread();

	const Area2I viewportArea = GetViewportArea();
	ClearAttachments(viewportArea, mask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& state)
{
	EnsureValidThread();

	if(mGraphicsPipeline == state)
		return;

	mGraphicsPipeline = std::static_pointer_cast<VulkanGpuGraphicsPipelineState>(state);
	mGfxPipelineRequiresBind = true;
	mGraphicsPushConstantsRequireBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanGpuCommandBuffer::SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& state)
{
	EnsureValidThread();

	if(mComputePipeline == state)
		return;

	mComputePipeline = std::static_pointer_cast<VulkanGpuComputePipelineState>(state);
	mCmpPipelineRequiresBind = true;
	mComputePushConstantsRequireBind = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanGpuCommandBuffer::SetGpuParameterSet(const TShared<GpuParameterSet>& parameterSet)
{
	EnsureValidThread();

	if(!B3D_ENSURE(parameterSet != nullptr))
		return;

	if(!B3D_ENSURE(parameterSet->GetSet() < kMaximumBoundDescriptorSets))
		return;

	const TShared<VulkanGpuParameterSet>& vulkanParameterSet = std::static_pointer_cast<VulkanGpuParameterSet>(parameterSet);
	const u32 set = parameterSet->GetSet();

	if(set >= (u32)mBoundGpuParameterSets.Size())
		mBoundGpuParameterSets.Resize(set + 1);

	// Note: We keep an internal reference to GPU params even though we shouldn't keep a reference to a render thread
	// object. But it should be fine since we expect the resource to be externally synchronized so it should never
	// be allowed to go out of scope on a non-render thread anyway.
	mBoundGpuParameterSets[set] = std::static_pointer_cast<VulkanGpuParameterSet>(parameterSet);

	mBoundParamsDirty = true;
	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;

	if(set < mDynamicOffsetsOverridesPerSet.size())
		mDynamicOffsetsOverridesPerSet[set].clear();

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void VulkanGpuCommandBuffer::SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset)
{
	EnsureValidThread();

	// Ensure storage is sized
	while(mDynamicOffsetsOverridesPerSet.size() <= set)
		mDynamicOffsetsOverridesPerSet.Add(UnorderedMap<u32, u32>());

	mDynamicOffsetsOverridesPerSet[set][bufferIndex] = offset;
	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;

	// If GPU params were bound already, we retrieved the initial set of offsets, so just override it
	if(!mBoundParamsDirty && set < mDynamicOffsetsPerSet.size())
	{
		if(bufferIndex < mDynamicOffsetsPerSet[set].size())
		{
			mDynamicOffsetsPerSet[set][bufferIndex] = offset;
			RebuildFlatDynamicOffsets();
		}
	}
}

void VulkanGpuCommandBuffer::SetPushConstants(u32 offsetInBytes, u32 sizeInBytes, const void* data)
{
	EnsureValidThread();

	if(sizeInBytes == 0)
		return;

	if(!B3D_ENSURE_LOG(data != nullptr, "Push-constant data cannot be null for a non-empty update."))
		return;

	if(!B3D_ENSURE_LOG((offsetInBytes & 3u) == 0 && (sizeInBytes & 3u) == 0, "Push-constant offsets and sizes must be aligned to four bytes."))
		return;

	if(!B3D_ENSURE_LOG(offsetInBytes <= kMaxPushConstantSizeInBytes && sizeInBytes <= kMaxPushConstantSizeInBytes - offsetInBytes, "Push-constant update at offset {0} with size {1} exceeds the {2}-byte Vulkan block.", offsetInBytes, sizeInBytes, kMaxPushConstantSizeInBytes))
		return;

	mPushConstants.Write(offsetInBytes, sizeInBytes, data);
	mGraphicsPushConstantsRequireBind = true;
	mComputePushConstantsRequireBind = true;
}

void VulkanGpuCommandBuffer::SetViewport(const Area2& area)
{
	EnsureValidThread();

	if(mNormalizedViewportArea == area)
		return;

	mNormalizedViewportArea = area;
	mViewportRequiresBind = true;
}

void VulkanGpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
{
	EnsureValidThread();

	const Area2I area(left, top, right - left, bottom - top);

	if(mIsScissorTestEnabled && mScissor == area)
		return;

	mScissor = area;
	mIsScissorTestEnabled = true;
	mScissorRequiresBind = true;
}

void VulkanGpuCommandBuffer::DisableScissorTest()
{
	EnsureValidThread();

	if(!mIsScissorTestEnabled)
		return;

	mIsScissorTestEnabled = false;
	mScissorRequiresBind = true;
}

void VulkanGpuCommandBuffer::SetStencilReferenceValue(u32 value)
{
	EnsureValidThread();

	if(mStencilRef == value)
		return;

	mStencilRef = value;
	mStencilRefRequiresBind = true;
}

void VulkanGpuCommandBuffer::WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();

	VulkanGpuQueryPool* vulkanQueryPool = static_cast<VulkanGpuQueryPool*>(queryPool.get());
	vkCmdWriteTimestamp(mCommandBufferHandle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, vulkanQueryPool->GetVulkanHandle(), query.Id);

	mResourceTracker.TrackResourceUsage(vulkanQueryPool, GpuAccessFlag::Write);
}

void VulkanGpuCommandBuffer::BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags)
{
	EnsureValidThread();

	VulkanGpuQueryPool* vulkanQueryPool = static_cast<VulkanGpuQueryPool*>(queryPool.get());
	vkCmdBeginQuery(mCommandBufferHandle, vulkanQueryPool->GetVulkanHandle(), query.Id, flags.IsSet(GpuQueryFlag::PreciseOcclusion) ? VK_QUERY_CONTROL_PRECISE_BIT : 0);

#if B3D_BUILD_TYPE_DEVELOPMENT
	mOpenQueries.emplace_back(IsInRenderPass(), queryPool->GetQueryType(), (u64)queryPool.get());
#endif

	mResourceTracker.TrackResourceUsage(vulkanQueryPool, GpuAccessFlag::Write);
}

void VulkanGpuCommandBuffer::EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();

	VulkanGpuQueryPool* vulkanQueryPool = static_cast<VulkanGpuQueryPool*>(queryPool.get());
	vkCmdEndQuery(mCommandBufferHandle, vulkanQueryPool->GetVulkanHandle(), query.Id);

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(B3D_ENSURE(!mOpenQueries.empty()))
	{
		const QueryInformation& lastQueryInformation = mOpenQueries.back();
		B3D_ENSURE(lastQueryInformation.IsInRenderPass == IsInRenderPass());
		B3D_ENSURE(lastQueryInformation.Type == queryPool->GetQueryType());
		B3D_ENSURE(lastQueryInformation.PoolIdentifier == (u64)queryPool.get());
	}
#endif

	mResourceTracker.TrackResourceUsage(vulkanQueryPool, GpuAccessFlag::Write);
}

void VulkanGpuCommandBuffer::ResetQueries(const TShared<GpuQueryPool>& queryPool)
{
	EnsureValidThread();
	B3D_ENSURE(!IsInRenderPass());

	VulkanGpuQueryPool* vulkanQueryPool = static_cast<VulkanGpuQueryPool*>(queryPool.get());
	vkCmdResetQueryPool(mCommandBufferHandle, vulkanQueryPool->GetVulkanHandle(), 0, vulkanQueryPool->GetPoolSize());

	vulkanQueryPool->NotifyPoolReset();
	mResourceTracker.TrackResourceUsage(vulkanQueryPool, GpuAccessFlag::Write);
}

void VulkanGpuCommandBuffer::SetDrawOperation(DrawOperationType drawOperation)
{
	EnsureValidThread();

	if(mDrawOp == drawOperation)
		return;

	mDrawOp = drawOperation;
	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;
}

void VulkanGpuCommandBuffer::SetVertexBuffers(u32 startIndex, TShared<GpuBuffer>* buffers, u32 bufferCount)
{
	EnsureValidThread();

	const u32 endIndex = startIndex + bufferCount;
	if(endIndex <= mVertexBuffers.size())
	{
		bool isDifferenceFound = false;
		for(u32 vertexBufferIndex = startIndex; vertexBufferIndex < endIndex; vertexBufferIndex++)
		{
			if(mVertexBuffers[vertexBufferIndex] != buffers[vertexBufferIndex])
			{
				isDifferenceFound = true;
				break;
			}
		}

		if(!isDifferenceFound)
			return;
	}

	if(mVertexBuffers.size() < endIndex)
		mVertexBuffers.resize(endIndex);

	for(u32 vertexBufferIndex = startIndex; vertexBufferIndex < endIndex; vertexBufferIndex++)
		mVertexBuffers[vertexBufferIndex] = std::static_pointer_cast<VulkanGpuBuffer>(buffers[vertexBufferIndex]);

	mVertexInputsDirty = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumVertexBufferBinds);
}

void VulkanGpuCommandBuffer::SetIndexBuffer(const TShared<GpuBuffer>& buffer)
{
	EnsureValidThread();

	if(mIndexBuffer == buffer)
		return;

	mIndexBuffer = std::static_pointer_cast<VulkanGpuBuffer>(buffer);
	mVertexInputsDirty = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void VulkanGpuCommandBuffer::SetVertexDescription(const TShared<VertexDescription>& vertexDescription)
{
	EnsureValidThread();

	if(mVertexDescription == vertexDescription)
		return;

	mVertexDescription = vertexDescription;
	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;
}

void VulkanGpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	EnsureValidThread();

	B3D_ENSURE(IsInRenderPass());

	if(!IsReadyForRender())
		return;

	BindGpuParameters(mGraphicsPipeline->GetParameterLayout(), mBarrierHelper);

	// All barriers should have been issued during begin render pass
	B3D_ENSURE(!mBarrierHelper.HasBarriers());

	if(mGfxPipelineRequiresBind)
	{
		if(!BindGraphicsPipeline())
			return;
	}
	else
		BindDynamicStates(false);

	// Important to call this after the pipeline is bound so we know how many vertex buffers it expects
	if(mVertexInputsDirty)
	{
		BindVertexInputs();
		mVertexInputsDirty = false;
	}

	if(mDescriptorSetsBindState.IsSet(DescriptorSetBindFlag::Graphics))
	{
		if(mBoundDescriptorSetCount > 0)
		{
			VkPipelineLayout pipelineLayout = mGraphicsPipeline->GetPipelineLayoutHandle();

			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mFlatDynamicOffsets.size(), mFlatDynamicOffsets.data());
		}

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Graphics);
	}

	BindPushConstants(true);

	if(instanceCount <= 0)
		instanceCount = 1;

	vkCmdDraw(mCommandBufferHandle, vertexCount, instanceCount, vertexOffset, firstInstance);
	NotifyRenderTargetModified();

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, 0); // TODO - Determine accurate primitive count
}

void VulkanGpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	EnsureValidThread();

	B3D_ENSURE(IsInRenderPass());

	if(indexCount == 0)
		return;

	if(!IsReadyForRender())
		return;

	BindGpuParameters(mGraphicsPipeline->GetParameterLayout(), mBarrierHelper);

	// All barriers should have been issued during begin render pass
	B3D_ENSURE(!mBarrierHelper.HasBarriers());

	if(mGfxPipelineRequiresBind)
	{
		if(!BindGraphicsPipeline())
			return;
	}
	else
		BindDynamicStates(false);

	// Important to call this after the pipeline is bound so we know how many vertex buffers it expects
	if(mVertexInputsDirty)
	{
		BindVertexInputs();
		mVertexInputsDirty = false;
	}

	if(mDescriptorSetsBindState.IsSet(DescriptorSetBindFlag::Graphics))
	{
		if(mBoundDescriptorSetCount > 0)
		{
			VkPipelineLayout pipelineLayout = mGraphicsPipeline->GetPipelineLayoutHandle();

			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mFlatDynamicOffsets.size(), mFlatDynamicOffsets.data());
		}

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Graphics);
	}

	BindPushConstants(true);

	if(instanceCount <= 0)
		instanceCount = 1;

	vkCmdDrawIndexed(mCommandBufferHandle, indexCount, instanceCount, startIndex, vertexOffset, firstInstance);
	NotifyRenderTargetModified();

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, 0); // TODO - Determine accurate primitive count
}

void VulkanGpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	EnsureValidThread();

	if(mComputePipeline == nullptr)
		return;

	if (groupCountX == 0 || groupCountY == 0 || groupCountZ == 0)
	{
		B3D_LOG(Warning, LogRenderBackend, "Ignoring call to DispatchCompute(). Thread count is zero.");
	}

	if(!B3D_ENSURE(!IsInRenderPass()))
		return;

	const TShared<GpuPipelineParameterLayout>& pipelineParameterLayout = mComputePipeline->GetParameterLayout();
	BindGpuParameters(pipelineParameterLayout, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	if(mCmpPipelineRequiresBind)
	{
		VulkanPipeline* pipeline = mComputePipeline->GetVulkanResource();
		if(pipeline == nullptr)
			return;

		mResourceTracker.TrackResourceUsage(pipeline, GpuAccessFlag::Read);
		mComputePipeline->RegisterShaderModuleResources(mResourceTracker);

		vkCmdBindPipeline(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetVulkanHandle());
		mCmpPipelineRequiresBind = false;
		mComputePushConstantsRequireBind = true;
	}

	if(mDescriptorSetsBindState.IsSet(DescriptorSetBindFlag::Compute))
	{
		if(mBoundDescriptorSetCount > 0)
		{
			VkPipelineLayout pipelineLayout = mComputePipeline->GetPipelineLayoutHandle();
			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mFlatDynamicOffsets.size(), mFlatDynamicOffsets.data());
		}

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Compute);
	}

	BindPushConstants(false);

	vkCmdDispatch(mCommandBufferHandle, groupCountX, groupCountY, groupCountZ);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void VulkanGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
{
	EnsureValidThread();

	auto* vulkanSource = static_cast<VulkanGpuBuffer*>(source.get());
	auto* vulkanDestination = static_cast<VulkanGpuBuffer*>(destination.get());

	VulkanBuffer* sourceBuffer = vulkanSource->GetVulkanResource();
	VulkanBuffer* destinationBuffer = vulkanDestination->GetVulkanResource();

	if(sourceBuffer == nullptr || destinationBuffer == nullptr)
		return;

	CopyBufferToBuffer(sourceBuffer, destinationBuffer, sourceOffset, destinationOffset, length);
}

void VulkanGpuCommandBuffer::CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer)
{
	B3D_ASSERT(bufferOffset == 0 && "Buffer offset not yet supported for texture copies");
	EnsureValidThread();

	auto* vulkanSource = static_cast<VulkanGpuBuffer*>(source.get());
	auto* vulkanDestination = static_cast<VulkanTexture*>(destination.get());

	VulkanBuffer* sourceBuffer = vulkanSource->GetVulkanResource();
	VulkanImage* destinationImage = vulkanDestination->GetVulkanResource();

	if(sourceBuffer == nullptr || destinationImage == nullptr)
		return;

	const TextureProperties& textureProperties = vulkanDestination->GetProperties();

	VkExtent3D extent;
	PixelUtility::GetSizeForMipLevel(textureProperties.Width, textureProperties.Height, textureProperties.Depth, mipLevel, extent.width, extent.height, extent.depth);

	const ImageSubresourcePitch pitch = vulkanDestination->GetStagingBufferPitchForSubresource(arrayLayer, mipLevel);

	GpuTextureSubresourceRange range;
	range.AspectMask = destinationImage->GetRange().AspectMask;
	range.BaseArrayLayer = arrayLayer;
	range.ArrayLayerCount = 1;
	range.BaseMipLevel = mipLevel;
	range.MipLevelCount = 1;

	GpuImageLayout transferLayout;
	if(vulkanDestination->IsDirectlyMappable())
		transferLayout = GpuImageLayout::General;
	else
		transferLayout = GpuImageLayout::TransferDestination;

	CopyBufferToImage(sourceBuffer, destinationImage, extent, range, transferLayout, pitch.RowPitch, pitch.SliceHeight);
}

void VulkanGpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
{
	B3D_ASSERT(bufferOffset == 0 && "Buffer offset not yet supported for texture copies");
	EnsureValidThread();

	auto* vulkanSource = static_cast<VulkanTexture*>(source.get());
	auto* vulkanDestination = static_cast<VulkanGpuBuffer*>(destination.get());

	VulkanImage* sourceImage = vulkanSource->GetVulkanResource();
	VulkanBuffer* destinationBuffer = vulkanDestination->GetVulkanResource();

	if(sourceImage == nullptr || destinationBuffer == nullptr)
		return;

	const TextureProperties& textureProperties = vulkanSource->GetProperties();

	VkExtent3D extent;
	PixelUtility::GetSizeForMipLevel(textureProperties.Width, textureProperties.Height, textureProperties.Depth, mipLevel, extent.width, extent.height, extent.depth);

	const ImageSubresourcePitch pitch = vulkanSource->GetStagingBufferPitchForSubresource(arrayLayer, mipLevel);

	GpuTextureSubresourceRange range;
	if(textureProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
		range.AspectMask = GpuTextureAspectFlag::Depth;
	else
		range.AspectMask = GpuTextureAspectFlag::Color;
	range.BaseArrayLayer = arrayLayer;
	range.ArrayLayerCount = 1;
	range.BaseMipLevel = mipLevel;
	range.MipLevelCount = 1;

	const GpuImageLayout transferLayout = GpuImageLayout::TransferSource;
	CopyImageToBuffer(sourceImage, destinationBuffer, extent, range, transferLayout, pitch.RowPitch, pitch.SliceHeight);
}

bool VulkanGpuCommandBuffer::CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation)
{
	if(!GpuCommandBuffer::CopyTexture(source, destination, copyInformation))
		return false;

	auto* vulkanSource = static_cast<VulkanTexture*>(source.get());
	auto* vulkanDestination = static_cast<VulkanTexture*>(destination.get());

	const TextureProperties& sourceProperties = vulkanSource->GetProperties();
	const TextureProperties& destinationProperties = vulkanDestination->GetProperties();

	VulkanImage* sourceImage = vulkanSource->GetVulkanResource();
	VulkanImage* destinationImage = vulkanDestination->GetVulkanResource();

	if(sourceImage == nullptr || destinationImage == nullptr)
		return false;

	const bool sourceHasMultipleSamples = sourceProperties.SampleCount > 1;
	const bool destinationHasMultipleSamples = destinationProperties.SampleCount > 1;
	const bool needsResolve = sourceHasMultipleSamples && !destinationHasMultipleSamples;

	bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	const GpuImageLayout sourceLayout = vulkanSource->IsDirectlyMappable() ? GpuImageLayout::General :
		(needsResolve ? GpuImageLayout::ResolveSource : GpuImageLayout::TransferSource);
	const GpuImageLayout destinationLayout = vulkanDestination->IsDirectlyMappable() ? GpuImageLayout::General :
		(needsResolve ? GpuImageLayout::ResolveDestination : GpuImageLayout::TransferDestination);

	u32 mipWidth, mipHeight, mipDepth;

	if(copyEntireSurface)
	{
		PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, copyInformation.SourceMip, mipWidth, mipHeight, mipDepth);
	}
	else
	{
		mipWidth = copyInformation.SourceVolume.GetWidth();
		mipHeight = copyInformation.SourceVolume.GetHeight();
		mipDepth = copyInformation.SourceVolume.GetDepth();
	}

	if(mipWidth == 0 || mipHeight == 0 || mipDepth == 0)
		return false;

	GpuTextureSubresourceRange sourceRange;
	sourceRange.AspectMask = GpuTextureAspectFlag::Color;
	sourceRange.BaseArrayLayer = copyInformation.SourceFace;
	sourceRange.ArrayLayerCount = copyInformation.FaceCount;
	sourceRange.BaseMipLevel = copyInformation.SourceMip;
	sourceRange.MipLevelCount = 1;

	GpuTextureSubresourceRange destinationRange;
	destinationRange.AspectMask = GpuTextureAspectFlag::Color;
	destinationRange.BaseArrayLayer = copyInformation.DestinationFace;
	destinationRange.ArrayLayerCount = copyInformation.FaceCount;
	destinationRange.BaseMipLevel = copyInformation.DestinationMip;
	destinationRange.MipLevelCount = 1;

	if(needsResolve)
	{
		VkImageResolve resolveRegion;
		resolveRegion.srcOffset = { (i32)copyInformation.SourceVolume.Left, (i32)copyInformation.SourceVolume.Top, (i32)copyInformation.SourceVolume.Front };
		resolveRegion.dstOffset = { copyInformation.DestinationPosition.X, copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z };
		resolveRegion.extent = { mipWidth, mipHeight, mipDepth };
		resolveRegion.srcSubresource.baseArrayLayer = copyInformation.SourceFace;
		resolveRegion.srcSubresource.layerCount = copyInformation.FaceCount;
		resolveRegion.srcSubresource.mipLevel = copyInformation.SourceMip;
		resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolveRegion.dstSubresource.baseArrayLayer = copyInformation.DestinationFace;
		resolveRegion.dstSubresource.layerCount = copyInformation.FaceCount;
		resolveRegion.dstSubresource.mipLevel = copyInformation.DestinationMip;
		resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		Resolve(sourceImage, destinationImage, sourceLayout, destinationLayout, sourceRange, destinationRange, 1, &resolveRegion);
	}
	else
	{
		VkImageCopy imageRegion;
		imageRegion.srcOffset = { (i32)copyInformation.SourceVolume.Left, (i32)copyInformation.SourceVolume.Top, (i32)copyInformation.SourceVolume.Front };
		imageRegion.dstOffset = { copyInformation.DestinationPosition.X, copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z };
		imageRegion.extent = { mipWidth, mipHeight, mipDepth };
		imageRegion.srcSubresource.baseArrayLayer = copyInformation.SourceFace;
		imageRegion.srcSubresource.layerCount = copyInformation.FaceCount;
		imageRegion.srcSubresource.mipLevel = copyInformation.SourceMip;
		imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.dstSubresource.baseArrayLayer = copyInformation.DestinationFace;
		imageRegion.dstSubresource.layerCount = copyInformation.FaceCount;
		imageRegion.dstSubresource.mipLevel = copyInformation.DestinationMip;
		imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		CopyImageToImage(sourceImage, destinationImage, sourceLayout, destinationLayout, sourceRange, destinationRange, 1, &imageRegion);
	}

	return true;
}

bool VulkanGpuCommandBuffer::BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation)
{
	if(!GpuCommandBuffer::BlitTexture(source, destination, blitInformation))
		return false;

	auto* vulkanSource = static_cast<VulkanTexture*>(source.get());
	auto* vulkanDestination = static_cast<VulkanTexture*>(destination.get());

	const TextureProperties& sourceProperties = vulkanSource->GetProperties();
	const TextureProperties& destinationProperties = vulkanDestination->GetProperties();

	VulkanImage* sourceImage = vulkanSource->GetVulkanResource();
	VulkanImage* destinationImage = vulkanDestination->GetVulkanResource();

	if(sourceImage == nullptr || destinationImage == nullptr)
		return false;

	GpuImageLayout transferSourceLayout = vulkanSource->IsDirectlyMappable() ? GpuImageLayout::General : GpuImageLayout::TransferSource;
	GpuImageLayout transferDestinationLayout = vulkanDestination->IsDirectlyMappable() ? GpuImageLayout::General : GpuImageLayout::TransferDestination;

	const bool copyFromEntireSurface = blitInformation.SourceVolume.GetWidth() == 0 ||
		blitInformation.SourceVolume.GetHeight() == 0 ||
		blitInformation.SourceVolume.GetDepth() == 0;

	PixelVolume sourceVolume = blitInformation.SourceVolume;
	if(copyFromEntireSurface)
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(sourceProperties.Width, sourceProperties.Height, sourceProperties.Depth, blitInformation.SourceMip, mipWidth, mipHeight, mipDepth);

		sourceVolume.Right = sourceVolume.Left + mipWidth;
		sourceVolume.Bottom = sourceVolume.Top + mipHeight;
		sourceVolume.Back = sourceVolume.Front + mipDepth;
	}

	const bool copyToEntireSurface = blitInformation.DestinationVolume.GetWidth() == 0 ||
		blitInformation.DestinationVolume.GetHeight() == 0 ||
		blitInformation.DestinationVolume.GetDepth() == 0;

	PixelVolume destinationVolume = blitInformation.DestinationVolume;
	if(copyToEntireSurface)
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(destinationProperties.Width, destinationProperties.Height, destinationProperties.Depth, blitInformation.DestinationMip, mipWidth, mipHeight, mipDepth);

		destinationVolume.Right = destinationVolume.Left + mipWidth;
		destinationVolume.Bottom = destinationVolume.Top + mipHeight;
		destinationVolume.Back = destinationVolume.Front + mipDepth;
	}

	GpuTextureSubresourceRange sourceRange;
	sourceRange.AspectMask = GpuTextureAspectFlag::Color;
	sourceRange.BaseArrayLayer = blitInformation.SourceFace;
	sourceRange.ArrayLayerCount = blitInformation.FaceCount;
	sourceRange.BaseMipLevel = blitInformation.SourceMip;
	sourceRange.MipLevelCount = 1;

	GpuTextureSubresourceRange destinationRange;
	destinationRange.AspectMask = GpuTextureAspectFlag::Color;
	destinationRange.BaseArrayLayer = blitInformation.DestinationFace;
	destinationRange.ArrayLayerCount = blitInformation.FaceCount;
	destinationRange.BaseMipLevel = blitInformation.DestinationMip;
	destinationRange.MipLevelCount = 1;

	VkImageBlit imageBlit;
	imageBlit.srcSubresource.baseArrayLayer = blitInformation.SourceFace;
	imageBlit.srcSubresource.layerCount = blitInformation.FaceCount;
	imageBlit.srcSubresource.mipLevel = blitInformation.SourceMip;
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcOffsets[0] = { (i32)sourceVolume.Left, (i32)sourceVolume.Top, (i32)sourceVolume.Front };
	imageBlit.srcOffsets[1] = { (i32)sourceVolume.Right, (i32)sourceVolume.Bottom, (i32)sourceVolume.Back };
	imageBlit.dstSubresource.baseArrayLayer = blitInformation.DestinationFace;
	imageBlit.dstSubresource.layerCount = blitInformation.FaceCount;
	imageBlit.dstSubresource.mipLevel = blitInformation.DestinationMip;
	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstOffsets[0] = { (i32)destinationVolume.Left, (i32)destinationVolume.Top, (i32)destinationVolume.Front };
	imageBlit.dstOffsets[1] = { (i32)destinationVolume.Right, (i32)destinationVolume.Bottom, (i32)destinationVolume.Back };

	Blit(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, sourceRange, destinationRange, 1, &imageBlit);

	return true;
}

void VulkanGpuCommandBuffer::BeginLabel(const StringView& name)
{
	EnsureValidThread();

	if(!IsRecording() || vkCmdBeginDebugUtilsLabelEXT == nullptr)
		return;

	VkDebugUtilsLabelEXT labelInfo;
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pNext = nullptr;
	labelInfo.pLabelName = name.data();
	labelInfo.color[0] = kDebugLabelColor.R;
	labelInfo.color[1] = kDebugLabelColor.G;
	labelInfo.color[2] = kDebugLabelColor.B;
	labelInfo.color[3] = kDebugLabelColor.A;

	vkCmdBeginDebugUtilsLabelEXT(mCommandBufferHandle, &labelInfo);
	mIsDebugLabelOpen = true;
}

void VulkanGpuCommandBuffer::EndLabel()
{
	EnsureValidThread();

	if(!IsRecording() || vkCmdBeginDebugUtilsLabelEXT == nullptr)
		return;

	vkCmdEndDebugUtilsLabelEXT(mCommandBufferHandle);
	mIsDebugLabelOpen = false;
}

void VulkanGpuCommandBuffer::InsertLabel(const StringView& name)
{
	EnsureValidThread();

	if(!IsRecording() || vkCmdBeginDebugUtilsLabelEXT == nullptr)
		return;

	VkDebugUtilsLabelEXT labelInfo;
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pNext = nullptr;
	labelInfo.pLabelName = name.data();
	labelInfo.color[0] = kDebugLabelColor.R;
	labelInfo.color[1] = kDebugLabelColor.G;
	labelInfo.color[2] = kDebugLabelColor.B;
	labelInfo.color[3] = kDebugLabelColor.A;

	vkCmdInsertDebugUtilsLabelEXT(mCommandBufferHandle, &labelInfo);
}

void VulkanGpuCommandBuffer::BeginRenderPass()
{
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	if(mFramebuffer == nullptr)
	{
		B3D_LOG(Warning, LogRenderBackend, "Attempting to begin a render pass but no render target is bound to the command buffer.");
		return;
	}
}

void VulkanGpuCommandBuffer::EndRenderPass()
{
	B3D_ASSERT(mState == GpuCommandBufferState::RecordingRenderPass);

	vkCmdEndRenderPass(mCommandBufferHandle);

	// Resource uses recorded inside the render pass cannot be committed through the barrier helper while the pass is active.
	B3D_ASSERT(!mBarrierHelper.HasBarriers());
	mResourceTracker.CommitPendingHazardRegistrations();

	// Execute any queued events
	for(auto& entry : mQueuedEvents)
		vkCmdSetEvent(mCommandBufferHandle, entry->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	mQueuedEvents.clear();

	// Publish final attachment layouts and clear the render-pass tracking scope.
	mResourceTracker.EndRenderPass();

	mState = GpuCommandBufferState::Recording;
	mRenderTarget = nullptr;
	mRenderTargetModified = false;
	mRenderTargetReadOnlyMask = RT_NONE;
	mFramebuffer = nullptr;

	mRenderPassGpuParameterSetCache.clear();

	// In case the same GPU params from last pass get used, this makes sure the states we reset above, get re-applied
	mBoundParamsDirty = true;

	// TODO - Probably best to clear mBoundParams since I cleared the cache above
}

namespace
{
	class VulkanSubmissionTransitionVisitor : public GpuSubmissionTransitionVisitor
	{
	public:
		VulkanSubmissionTransitionVisitor(VulkanGpuDevice& device, GpuQueueId destinationQueueId, VulkanGpuCommandBufferSubmitInformation& submitInformation)
			: mDevice(device), mDestinationQueueId(destinationQueueId), mDestinationQueueFamily(device.GetQueueFamily(destinationQueueId.GetType())), mSubmitInformation(submitInformation)
		{ }

		void VisitBuffer(const GpuSubmissionBufferTransition& transition) override
		{
			VulkanBuffer* const buffer = static_cast<VulkanBuffer*>(transition.Buffer);

			GpuQueueId ownerQueueId;
			const bool hasOwnerQueue = buffer->GetOwnerQueueId(ownerQueueId);
			const u32 sourceQueueFamily = hasOwnerQueue ? mDevice.GetQueueFamily(ownerQueueId.GetType()) : mDestinationQueueFamily;
			const bool needsOwnershipTransfer = hasOwnerQueue && buffer->IsExclusive() && sourceQueueFamily != mDestinationQueueFamily;

			if(!needsOwnershipTransfer && transition.HasSameQueueDependency())
			{
				mDestinationQueueBarriers.AddBufferBarrier(buffer->GetVulkanHandle(), transition.MemoryBarrier);
				mDestinationQueueBarriers.AddBufferBarrier(buffer->GetVulkanHandle(), transition.ExecutionBarrier);
			}

			if(needsOwnershipTransfer)
			{
				const GpuAccessScope& sourceAccessScope = transition.SourceAccessScope;
				const GpuAccessScope& submissionBarrierAccessScope = transition.SubmissionBarrierAccessScope;
				VkPipelineStageFlags sourceStages, destinationStages;
				VkAccessFlags sourceAccess, destinationAccess;
				VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessScope.GetStages(), sourceAccessScope.GetAccess(), sourceStages, sourceAccess);
				VulkanUtility::GetPipelineStageAndAccessMask(submissionBarrierAccessScope.GetStages(), submissionBarrierAccessScope.GetAccess(), destinationStages, destinationAccess);

				if(sourceStages == 0)
					sourceStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

				if(destinationStages == 0)
					destinationStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

				SourceQueueTransitionInformation& sourceQueueTransitionInformation = GetSourceQueueTransitionInformation(ownerQueueId, transition.ExclusiveAccessWaitMask);
				sourceQueueTransitionInformation.Barriers.AddBufferBarrier(buffer->GetVulkanHandle(), sourceStages, sourceAccess, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, sourceQueueFamily, mDestinationQueueFamily);
				mDestinationQueueBarriers.AddBufferBarrier(buffer->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, destinationStages, destinationAccess, sourceQueueFamily, mDestinationQueueFamily);
			}
			else
			{
				// Explicit waits remain the preferred API; this covers resource-derived dependencies.
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;
			}

			buffer->SetOwnerQueueId(mDestinationQueueId);
		}

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			VulkanImage* const image = static_cast<VulkanImage*>(transition.Image);
			VulkanImageSubresource* const subresource = static_cast<VulkanImageSubresource*>(transition.StateResource);
			const VkImageLayout oldLayout = subresource->GetLayout();
			const VkImageLayout requestedInitialLayout = VulkanUtility::ToVkImageLayout(transition.InitialLayout);
			const VkImageLayout newLayout = requestedInitialLayout != VK_IMAGE_LAYOUT_UNDEFINED ? requestedInitialLayout : oldLayout;
			const bool layoutMismatch = requestedInitialLayout != VK_IMAGE_LAYOUT_UNDEFINED && oldLayout != newLayout;
			const VkImageSubresourceRange vkRange = VulkanUtility::ToVkImageSubresourceRange(transition.ImageRange);

			GpuQueueId ownerQueueId;
			const bool hasOwnerQueue = subresource->GetOwnerQueueId(ownerQueueId);
			const u32 sourceQueueFamily = hasOwnerQueue ? mDevice.GetQueueFamily(ownerQueueId.GetType()) : mDestinationQueueFamily;
			const bool needsOwnershipTransfer = hasOwnerQueue && subresource->IsExclusive() && sourceQueueFamily != mDestinationQueueFamily;
			const bool needsFullSync = needsOwnershipTransfer || layoutMismatch;

			if(!needsFullSync && transition.HasSameQueueDependency())
			{
				mDestinationQueueBarriers.AddImageBarrier(image->GetVulkanHandle(), vkRange, transition.MemoryBarrier, oldLayout, newLayout);
				mDestinationQueueBarriers.AddImageBarrier(image->GetVulkanHandle(), vkRange, transition.ExecutionBarrier, oldLayout, newLayout);
			}

			if(!needsFullSync)
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;

			const GpuAccessScope& sourceAccessScope = transition.SourceAccessScope;
			const GpuAccessScope& submissionBarrierAccessScope = transition.SubmissionBarrierAccessScope;
			VkPipelineStageFlags sourceStages, destinationStages;
			VkAccessFlags sourceAccess, destinationAccess;
			VulkanUtility::GetPipelineStageAndAccessMask(sourceAccessScope.GetStages(), sourceAccessScope.GetAccess(), sourceStages, sourceAccess);
			VulkanUtility::GetPipelineStageAndAccessMask(submissionBarrierAccessScope.GetStages(), submissionBarrierAccessScope.GetAccess(), destinationStages, destinationAccess);

			if(sourceStages == 0)
				sourceStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			if(destinationStages == 0)
				destinationStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

			if(needsOwnershipTransfer)
			{
				SourceQueueTransitionInformation& sourceQueueTransitionInformation = GetSourceQueueTransitionInformation(ownerQueueId, transition.ExclusiveAccessWaitMask);
				sourceQueueTransitionInformation.Barriers.AddImageBarrier(image->GetVulkanHandle(), vkRange, sourceStages, sourceAccess, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, oldLayout, newLayout, sourceQueueFamily, mDestinationQueueFamily);
				mDestinationQueueBarriers.AddImageBarrier(image->GetVulkanHandle(), vkRange, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, destinationStages, destinationAccess, oldLayout, newLayout, sourceQueueFamily, mDestinationQueueFamily);
			}
			else if(layoutMismatch)
			{
				mSubmitInformation.RequiredWaitMask |= transition.ExclusiveAccessWaitMask;
				mDestinationQueueBarriers.AddImageBarrier(image->GetVulkanHandle(), vkRange, sourceStages, sourceAccess, destinationStages, destinationAccess, oldLayout, newLayout);
			}

			subresource->SetLayout(VulkanUtility::ToVkImageLayout(transition.FinalLayout));
			subresource->SetOwnerQueueId(mDestinationQueueId);
		}

		void Finalize()
		{
			for(SourceQueueTransitionInformation& sourceQueueTransitionInformation : mSourceQueueTransitions)
			{
				GpuCommandBufferPool& sourceCommandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(sourceQueueTransitionInformation.QueueId.GetType());
				const TShared<VulkanGpuCommandBuffer> sourceCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(sourceCommandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Source queue transition")));
				sourceQueueTransitionInformation.Barriers.Execute(sourceCommandBuffer->GetVulkanHandle());
				sourceCommandBuffer->End();

				VulkanSourceQueueTransition sourceQueueTransition;
				sourceQueueTransition.QueueId = sourceQueueTransitionInformation.QueueId;
				sourceQueueTransition.WaitMask = sourceQueueTransitionInformation.WaitMask;
				sourceQueueTransition.CommandBuffer = sourceCommandBuffer;
				mSubmitInformation.SourceQueueTransitions.Add(std::move(sourceQueueTransition));
			}

			if(mDestinationQueueBarriers.HasBarriers())
			{
				GpuCommandBufferPool& destinationCommandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(mDestinationQueueId.GetType());
				TShared<VulkanGpuCommandBuffer> transitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(destinationCommandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Queue and layout transitions")));
				mDestinationQueueBarriers.Execute(transitionCommandBuffer->GetVulkanHandle());
				transitionCommandBuffer->End();
				mSubmitInformation.DestinationQueueTransitionCommandBuffer = transitionCommandBuffer;
			}
		}

	private:
		struct SourceQueueTransitionInformation
		{
			GpuQueueId QueueId;
			GpuQueueMask WaitMask;
			VulkanBarrierBatch Barriers;
		};

		SourceQueueTransitionInformation& GetSourceQueueTransitionInformation(GpuQueueId sourceQueueId, GpuQueueMask waitMask)
		{
			auto found = std::find_if(mSourceQueueTransitions.begin(), mSourceQueueTransitions.end(), [sourceQueueId](const SourceQueueTransitionInformation& sourceQueueTransitionInformation)
			{
				return sourceQueueTransitionInformation.QueueId.Id == sourceQueueId.Id;
			});

			if(found == mSourceQueueTransitions.end())
			{
				SourceQueueTransitionInformation sourceQueueTransitionInformation;
				sourceQueueTransitionInformation.QueueId = sourceQueueId;
				sourceQueueTransitionInformation.WaitMask = waitMask & ~GpuQueueMask(sourceQueueId);
				mSourceQueueTransitions.Add(std::move(sourceQueueTransitionInformation));
				return mSourceQueueTransitions.back();
			}

			found->WaitMask |= waitMask & ~GpuQueueMask(sourceQueueId);
			return *found;
		}

		VulkanGpuDevice& mDevice;
		GpuQueueId mDestinationQueueId;
		u32 mDestinationQueueFamily;
		VulkanGpuCommandBufferSubmitInformation& mSubmitInformation;
		VulkanBarrierBatch mDestinationQueueBarriers;
		TInlineArray<SourceQueueTransitionInformation, 4> mSourceQueueTransitions;
	};
}

VulkanGpuCommandBufferSubmitInformation VulkanGpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted()); // Caller should already have set this flag

	VulkanGpuCommandBufferSubmitInformation submitInformation;
	VulkanGpuDevice& device = GetVulkanGpuDevice();
	const GpuQueueId destinationQueueId(queueType, queueIndex);
	VulkanSubmissionTransitionVisitor transitionVisitor(device, destinationQueueId, submitInformation);
	mResourceTracker.ResolveSubmissionTransitions(destinationQueueId, transitionVisitor);
	transitionVisitor.Finalize();

	// Wait on present (i.e. until the back buffer becomes available) for any surfaces
	for(IVulkanRenderWindowSurface* surface : mAcquiredSurfaces)
		surface->AppendWaitSemaphoresIfRequired(submitInformation.WaitSemaphores);

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(GetShared());

	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mGfxPipelineRequiresBind = true;
	mCmpPipelineRequiresBind = true;
	mFramebuffer = nullptr;
	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;
	mBoundGpuParameterSets.Clear();
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
	mVertexInputsDirty = true;
	mAcquiredSurfaces.clear();

	return submitInformation;
}

void VulkanGpuCommandBuffer::NotifyWasSubmitted(GpuQueueId queueId)
{
	AssertIfNotSubmitThread();

	mSubmittedQueueId = queueId;
	mResourceTracker.NotifyUsed(queueId);
}

void VulkanGpuCommandBuffer::NotifyWillQueueForSubmit()
{
	// Clear everything not allowed on the submit thread
	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mBoundGpuParameterSets.Clear();
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
}

bool VulkanGpuCommandBuffer::UpdateExecutionStatus(bool block)
{
	AssertIfNotSubmitThread();

	VkResult result = vkWaitForFences(GetVulkanGpuDevice().GetLogical(), 1, &mFence, true, block ? 1'000'000'000 : 0);

	// VK_ERROR_DEVICE_LOST here means the GPU faulted/hung (TDR) while this command buffer was executing. The fence
	// will never signal, so this would otherwise spin (timing out every wait) forever. Treat it as fatal.
	if(result == VK_ERROR_DEVICE_LOST)
		B3D_LOG(Fatal, LogRenderBackend, "vkWaitForFences reported VK_ERROR_DEVICE_LOST. The GPU device is in an unrecoverable state; aborting.");

	B3D_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT);

	return result == VK_SUCCESS;
}

void VulkanGpuCommandBuffer::Destroy()
{
	if(mIsDestroyed)
		return;

	VkCommandBuffer commandBufferHandle = GetVulkanHandle();
	vkFreeCommandBuffers(GetVulkanGpuDevice().GetLogical(), mPool.GetVulkanPool(), 1, &commandBufferHandle);

	GpuCommandBuffer::Destroy();
}

void VulkanGpuCommandBuffer::ClearRecordingState()
{
	const bool wasSubmitted = mState == GpuCommandBufferState::Executing || mState == GpuCommandBufferState::Done;

	if(wasSubmitted)
		mResourceTracker.NotifyDone(mSubmittedQueueId);
	else
		mResourceTracker.NotifyUnbound();

#if B3D_BUILD_TYPE_DEVELOPMENT
	mOpenQueries.clear();
#endif

	mResourceTracker.Clear();
	mQueueSyncMask = GpuQueueMask();
	mPushConstants.Clear();
	mGraphicsPushConstantsRequireBind = false;
	mComputePushConstantsRequireBind = false;

	OnDidComplete.Clear();
	OnDestroyed.Clear();
}

void VulkanGpuCommandBuffer::Reset()
{
	ClearRecordingState();

	if(!mPool.GetUsePoolReset())
	{
		vkResetCommandBuffer(mCommandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		mState = GpuCommandBufferState::Ready;
	}
}

void VulkanGpuCommandBuffer::NotifyParentPoolReset()
{
	ClearRecordingState();
	mState = GpuCommandBufferState::Ready;
}

Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> VulkanGpuCommandBuffer::BuildClearValues(RenderSurfaceMask clearMask)
{
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues{};

	if(clearMask == RT_NONE || mFramebuffer == nullptr || mRenderTarget == nullptr)
		return clearValues;

	const RenderTargetClearValues& targetClearValues = mRenderTarget->GetClearValues();

	// Determine which attachments require clearing, and their clear values
	const VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
	{
		const GpuFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachments()[sequentialColorAttachmentIndex];
		const RenderSurfaceMaskBits colorAttachmentBit = colorAttachment.Surface;

		if(!clearMask.IsSet(colorAttachmentBit))
			continue;

		const Color& color = targetClearValues.Colors[colorAttachment.GetIndex()];
		VkClearColorValue& colorAttachmentClearValue = clearValues[sequentialColorAttachmentIndex].color;
		colorAttachmentClearValue.float32[0] = color.R;
		colorAttachmentClearValue.float32[1] = color.G;
		colorAttachmentClearValue.float32[2] = color.B;
		colorAttachmentClearValue.float32[3] = color.A;
	}

	if(renderPass->HasDepthAttachment())
	{
		u32 depthAttachmentSequentialIndex = colorAttachmentCount;

		if(clearMask.IsSet(RT_DEPTH))
			clearValues[depthAttachmentSequentialIndex].depthStencil.depth = targetClearValues.Depth;

		if(clearMask.IsSet(RT_STENCIL))
			clearValues[depthAttachmentSequentialIndex].depthStencil.stencil = targetClearValues.Stencil;
	}

	return clearValues;
}

void VulkanGpuCommandBuffer::ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask)
{
	ClearAttachments(area, clearMask, BuildClearValues(clearMask));
}

void VulkanGpuCommandBuffer::ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues)
{
	if(clearMask == RT_NONE || mFramebuffer == nullptr)
		return;

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();

	Array<VkClearAttachment, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> attachments;
	u32 baseLayerIndex = 0;
	u32 sequentialClearedAttachmentIndex = 0; // Only counts attachments that we need to clear

	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
	{
		const GpuFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachments()[sequentialColorAttachmentIndex];
		const RenderSurfaceMaskBits colorAttachmentBit = colorAttachment.Surface;

		if(!clearMask.IsSet(colorAttachmentBit))
			continue;

		attachments[sequentialClearedAttachmentIndex].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachments[sequentialClearedAttachmentIndex].colorAttachment = colorAttachment.GetIndex();
		attachments[sequentialClearedAttachmentIndex].clearValue.color = clearValues[sequentialColorAttachmentIndex].color;

		const u32 colorAttachmentBaseLayer = colorAttachment.Range.BaseArrayLayer;
		if(sequentialClearedAttachmentIndex == 0)
		{
			baseLayerIndex = colorAttachmentBaseLayer;
		}
		else
		{
			if(baseLayerIndex != colorAttachmentBaseLayer)
			{
				B3D_LOG(Error, LogRenderBackend, "All starting layers for frame buffer attachments must be matching when performing a clear command.");
			}
		}

		sequentialClearedAttachmentIndex++;
	}

	if(clearMask.IsSet(RT_DEPTH) || clearMask.IsSet(RT_STENCIL))
	{
		if(renderPass->HasDepthAttachment())
		{
			attachments[sequentialClearedAttachmentIndex].aspectMask = 0;

			if(clearMask.IsSet(RT_DEPTH))
			{
				attachments[sequentialClearedAttachmentIndex].aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
				attachments[sequentialClearedAttachmentIndex].clearValue.depthStencil.depth = clearValues[colorAttachmentCount].depthStencil.depth;
			}

			if(clearMask.IsSet(RT_STENCIL))
			{
				attachments[sequentialClearedAttachmentIndex].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				attachments[sequentialClearedAttachmentIndex].clearValue.depthStencil.stencil = clearValues[colorAttachmentCount].depthStencil.stencil;
			}

			attachments[sequentialClearedAttachmentIndex].colorAttachment = 0;

			const GpuFramebufferAttachment* depthStencilAttachment = mFramebuffer->FindAttachment(RT_DEPTH);
			if(depthStencilAttachment == nullptr)
				depthStencilAttachment = mFramebuffer->FindAttachment(RT_STENCIL);

			B3D_ASSERT(depthStencilAttachment != nullptr);
			const u32 depthStencilAttachmentBaseLayer = depthStencilAttachment->Range.BaseArrayLayer;
			if(sequentialClearedAttachmentIndex == 0)
			{
				baseLayerIndex = depthStencilAttachmentBaseLayer;
			}
			else
			{
				if(baseLayerIndex != depthStencilAttachmentBaseLayer)
				{
					B3D_LOG(Error, LogRenderBackend, "All starting layers for frame buffer attachments must be matching when performing a clear command.");
				}
			}

			sequentialClearedAttachmentIndex++;
		}
	}

	const u32 attachmentsToClearCount = sequentialClearedAttachmentIndex;
	if(attachmentsToClearCount == 0)
		return;

	VkClearRect clearRect;
	clearRect.baseArrayLayer = baseLayerIndex;
	clearRect.layerCount = mFramebuffer->GetLayerCount();
	clearRect.rect.offset.x = area.X;
	clearRect.rect.offset.y = area.Y;
	clearRect.rect.extent.width = area.Width;
	clearRect.rect.extent.height = area.Height;

	vkCmdClearAttachments(mCommandBufferHandle, attachmentsToClearCount, attachments.data(), 1, &clearRect);
	NotifyRenderTargetModified();
}

bool VulkanGpuCommandBuffer::IsReadyForRender()
{
	if(mGraphicsPipeline == nullptr)
		return false;

	TShared<VertexDescription> shaderInputVertexDescription = mGraphicsPipeline->GetInputDeclaration();
	if(shaderInputVertexDescription == nullptr)
		return false;

	return mFramebuffer != nullptr && mVertexDescription != nullptr;
}

bool VulkanGpuCommandBuffer::BindGraphicsPipeline()
{
	const TShared<VertexDescription> vertexShaderInputDescription = mGraphicsPipeline->GetInputDeclaration();
	const TShared<VulkanVertexInput> vertexShaderInput = VulkanVertexInputManager::Instance().GetVertexInput(mVertexDescription, vertexShaderInputDescription);

	VulkanRenderPass *const renderPass = mFramebuffer->GetRenderPass();
	VulkanPipeline *const pipeline = mGraphicsPipeline->FindOrCreateVulkanResource(renderPass, mRenderTargetReadOnlyMask, mDrawOp, vertexShaderInput);

	if(pipeline == nullptr)
		return false;

	mGraphicsPipeline->RegisterShaderModuleResources(mResourceTracker);
	mResourceTracker.TrackResourceUsage(pipeline, GpuAccessFlag::Read);

	vkCmdBindPipeline(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVulkanHandle());
	BindDynamicStates(true);
	mGraphicsPushConstantsRequireBind = true;

	mRequiredVertexBufferBindingCount = pipeline->GetVertexBufferBindingCount();
	mGfxPipelineRequiresBind = false;
	return true;
}

void VulkanGpuCommandBuffer::BindDynamicStates(bool forceAll)
{
	if(mViewportRequiresBind || forceAll)
	{
		const VkViewport viewport = VulkanUtility::ToVulkanViewport(GetViewportArea(), 0.0f, 1.0f);

		vkCmdSetViewport(mCommandBufferHandle, 0, 1, &viewport);
		mViewportRequiresBind = false;
	}

	if(mStencilRefRequiresBind || forceAll)
	{
		vkCmdSetStencilReference(mCommandBufferHandle, VK_STENCIL_FRONT_AND_BACK, mStencilRef);
		mStencilRefRequiresBind = false;
	}

	if(mScissorRequiresBind || forceAll)
	{
		VkRect2D scissorRect;
		if(mIsScissorTestEnabled)
		{
			scissorRect.offset.x = mScissor.X;
			scissorRect.offset.y = mScissor.Y;
			scissorRect.extent.width = mScissor.Width;
			scissorRect.extent.height = mScissor.Height;
		}
		else
		{
			scissorRect.offset.x = 0;
			scissorRect.offset.y = 0;
			scissorRect.extent.width = mFramebuffer->GetWidth();
			scissorRect.extent.height = mFramebuffer->GetHeight();
		}

		vkCmdSetScissor(mCommandBufferHandle, 0, 1, &scissorRect);

		mScissorRequiresBind = false;
	}
}

void VulkanGpuCommandBuffer::BindVertexInputs()
{
	if(mRequiredVertexBufferBindingCount > 0)
	{
		const VulkanBuiltinResources& vulkanBuiltinResources = GetVulkanGpuDevice().GetBuiltinResources();
		VulkanBuffer *const dummyVertexBuffer = vulkanBuiltinResources.DummyVertexBuffer->GetVulkanResource();

		for(u32 bindingIndex = 0; bindingIndex < mRequiredVertexBufferBindingCount; bindingIndex++)
		{
			VulkanBuffer* resource = nullptr;
			if(bindingIndex < (u32)mVertexBuffers.size() && mVertexBuffers[bindingIndex] != nullptr)
				resource = mVertexBuffers[bindingIndex]->GetVulkanResource();

			if(resource == nullptr)
				resource = dummyVertexBuffer;

			mVertexBuffersTemp[bindingIndex] = resource->GetVulkanHandle();
			mResourceTracker.TrackBufferUsage(resource, GpuResourceUseFlag::VertexBuffer, GpuAccessFlag::Read, mBarrierHelper);
		}

		vkCmdBindVertexBuffers(mCommandBufferHandle, 0, mRequiredVertexBufferBindingCount, mVertexBuffersTemp, mVertexBufferOffsetsTemp);
	}

	if(mIndexBuffer != nullptr)
	{
		VulkanBuffer* resource = mIndexBuffer->GetVulkanResource();
		if(resource != nullptr)
		{
			VkBuffer vkBuffer = resource->GetVulkanHandle();
			VkIndexType indexType = VK_INDEX_TYPE_UINT32;

			if(B3D_ENSURE(mIndexBuffer->GetInformation().Type == GpuBufferType::Index))
				indexType = VulkanUtility::GetIndexType(mIndexBuffer->GetInformation().Index.Type);

			mResourceTracker.TrackBufferUsage(resource, GpuResourceUseFlag::IndexBuffer, GpuAccessFlag::Read, mBarrierHelper);

			vkCmdBindIndexBuffer(mCommandBufferHandle, vkBuffer, 0, indexType);
		}
	}

	// Not allowed to issue barriers at this point, everything has to be done before render pass. If this proves an issue then all vertex/index buffers will need to be pre-declared in RenderPassCreateInformation.
	B3D_ENSURE(!mBarrierHelper.HasBarriers());
}

void VulkanGpuCommandBuffer::BindGpuParameters(const TShared<GpuPipelineParameterLayout>& pipelineParameterLayout, VulkanBarrierHelper& barrierHelper)
{
	B3D_ASSERT(pipelineParameterLayout != nullptr);

	if(!mBoundParamsDirty)
		return;

	mBoundDescriptorSetCount = 0;

	const u32 setCount = pipelineParameterLayout->GetSetCount();

	// Ensure per-set storage is sized
	while(mDynamicOffsetsPerSet.size() < setCount)
		mDynamicOffsetsPerSet.Add(TInlineArray<u32, 4>());

	while(mDynamicOffsetsOverridesPerSet.size() < setCount)
		mDynamicOffsetsOverridesPerSet.Add(UnorderedMap<u32, u32>());

	for(u32 set = 0; set < setCount; set++)
	{
		mDynamicOffsetsPerSet[set].clear();

		const TShared<VulkanGpuParameterSet>& boundGpuParameterSet = mBoundGpuParameterSets[set];
		if(boundGpuParameterSet != nullptr)
		{
			TInlineArray<u32, 4> setDynamicOffsets;

			auto it = mRenderPassGpuParameterSetCache.find(boundGpuParameterSet.get());
			if(it != mRenderPassGpuParameterSetCache.end())
			{
				// Use cached preparation data (skip PrepareForBind)
				const CachedGpuParameterData& cacheData = it->second;

				mDescriptorSetsTemp[set] = cacheData.DescriptorSet;
				setDynamicOffsets = cacheData.DynamicOffsets;
			}
			else
			{
				// Render pass GPU resources must all be predeclared before render pass starts otherwise we cannot issue barriers & layout transitions
				if(IsInRenderPass())
				{
					B3D_ENSURE(false);
					B3D_LOG(Warning, LogRenderBackend, "SetGpuParameterSet() called with parameters not declared in RenderPassCreateInformation. Automatic resource barriers and layout transitions may not execute correctly.");
				}

				// Fallback: No cached data, call PrepareForBind now
				// This handles compute dispatch and non-render-pass scenarios
				boundGpuParameterSet->PrepareForBind(mResourceTracker, barrierHelper, mDescriptorSetsTemp[set], setDynamicOffsets);
			}

			// Apply per-set dynamic offset overrides
			for(const auto& [index, offsetVal] : mDynamicOffsetsOverridesPerSet[set])
			{
				if(index < setDynamicOffsets.size())
					setDynamicOffsets[index] = offsetVal;
			}

			mDynamicOffsetsPerSet[set] = setDynamicOffsets;
			mBoundDescriptorSetCount++;
		}
	}

	RebuildFlatDynamicOffsets();
	mBoundParamsDirty = false;
}

void VulkanGpuCommandBuffer::BindPushConstants(bool isGraphics)
{
	if((isGraphics && !mGraphicsPushConstantsRequireBind) || (!isGraphics && !mComputePushConstantsRequireBind))
		return;

	const VulkanGpuPipelineParameterLayout* parameterLayout = nullptr;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	if(isGraphics && mGraphicsPipeline != nullptr)
	{
		parameterLayout = static_cast<VulkanGpuPipelineParameterLayout*>(mGraphicsPipeline->GetParameterLayout().get());
		pipelineLayout = mGraphicsPipeline->GetPipelineLayoutHandle();
	}
	else if(!isGraphics && mComputePipeline != nullptr)
	{
		parameterLayout = static_cast<VulkanGpuPipelineParameterLayout*>(mComputePipeline->GetParameterLayout().get());
		pipelineLayout = mComputePipeline->GetPipelineLayoutHandle();
	}

	if(parameterLayout == nullptr)
		return;

	const TOptional<VkPushConstantRange>& pushConstantRange = parameterLayout->GetPushConstantRange();
	if(pushConstantRange)
	{
		B3D_ASSERT(pipelineLayout != VK_NULL_HANDLE);
		vkCmdPushConstants(mCommandBufferHandle, pipelineLayout, pushConstantRange->stageFlags, pushConstantRange->offset, pushConstantRange->size, mPushConstants.GetData());
	}

	if(isGraphics)
		mGraphicsPushConstantsRequireBind = false;
	else
		mComputePushConstantsRequireBind = false;
}

void VulkanGpuCommandBuffer::RebuildFlatDynamicOffsets()
{
	mFlatDynamicOffsets.clear();

	for(u32 set = 0; set < mBoundDescriptorSetCount; set++)
	{
		if(set >= (u32)mDynamicOffsetsPerSet.size())
			continue;

		for(u32 offset : mDynamicOffsetsPerSet[set])
			mFlatDynamicOffsets.Add(offset);
	}
}

void VulkanGpuCommandBuffer::SetEvent(VulkanEvent* event)
{
	if(IsInRenderPass())
		mQueuedEvents.push_back(event);
	else
		vkCmdSetEvent(mCommandBufferHandle, event->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	mResourceTracker.TrackResourceUsage(event, GpuAccessFlag::Read);
}

void VulkanGpuCommandBuffer::UpdateBuffer(VulkanBuffer* destination, u8* data, VkDeviceSize offset, VkDeviceSize length)
{
	// TODO - Down the line we should make these barriers explicit, so user can batch multiple updates and issue one set of barriers, rather than barriers for each update. Same applied to other transfer ops below.

	mResourceTracker.TrackBufferUsage(destination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);
	mBarrierHelper.Execute(*this);

	vkCmdUpdateBuffer(GetVulkanHandle(), destination->GetVulkanHandle(), offset, length, (uint32_t*)data);
}

void VulkanGpuCommandBuffer::CopyBufferToBuffer(VulkanBuffer* source, VulkanBuffer* destination, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset, VkDeviceSize length)
{
	B3D_ENSURE(!IsInRenderPass());

	VkBufferCopy region;
	region.size = length;
	region.srcOffset = sourceOffset;
	region.dstOffset = destinationOffset;

	mResourceTracker.TrackBufferUsage(source, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(destination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdCopyBuffer(GetVulkanHandle(), source->GetVulkanHandle(), destination->GetVulkanHandle(), 1, &region);
}

void VulkanGpuCommandBuffer::CopyBufferToImage(VulkanBuffer* source, VulkanImage* destination, const VkExtent3D& region, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, u32 rowPitch, u32 sliceHeight)
{
	B3D_ENSURE(!IsInRenderPass());

	const VkImageLayout vkLayout = VulkanUtility::ToVkImageLayout(layout);

	VkImageSubresourceLayers rangeLayers;
	rangeLayers.aspectMask = VulkanUtility::GetAspectMask(subresourceRange.AspectMask);
	rangeLayers.baseArrayLayer = subresourceRange.BaseArrayLayer;
	rangeLayers.layerCount = subresourceRange.ArrayLayerCount;
	rangeLayers.mipLevel = subresourceRange.BaseMipLevel;

	VkBufferImageCopy copyRegion;
	copyRegion.bufferRowLength = rowPitch;
	copyRegion.bufferImageHeight = sliceHeight;
	copyRegion.bufferOffset = 0;
	copyRegion.imageOffset.x = 0;
	copyRegion.imageOffset.y = 0;
	copyRegion.imageOffset.z = 0;
	copyRegion.imageExtent = region;
	copyRegion.imageSubresource = rangeLayers;

	mResourceTracker.TrackBufferUsage(source, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, subresourceRange, layout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdCopyBufferToImage(GetVulkanHandle(), source->GetVulkanHandle(), destination->GetVulkanHandle(), vkLayout, 1, &copyRegion);
}

void VulkanGpuCommandBuffer::CopyImageToBuffer(VulkanImage* source, VulkanBuffer* destination, const VkExtent3D& region, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, u32 rowPitch, u32 sliceHeight)
{
	B3D_ENSURE(!IsInRenderPass());

	const VkImageLayout vkLayout = VulkanUtility::ToVkImageLayout(layout);

	VkImageSubresourceLayers rangeLayers;
	rangeLayers.aspectMask = VulkanUtility::GetAspectMask(subresourceRange.AspectMask);
	rangeLayers.baseArrayLayer = subresourceRange.BaseArrayLayer;
	rangeLayers.layerCount = subresourceRange.ArrayLayerCount;
	rangeLayers.mipLevel = subresourceRange.BaseMipLevel;

	VkBufferImageCopy copyRegion;
	copyRegion.bufferRowLength = rowPitch;
	copyRegion.bufferImageHeight = sliceHeight;
	copyRegion.bufferOffset = 0;
	copyRegion.imageOffset.x = 0;
	copyRegion.imageOffset.y = 0;
	copyRegion.imageOffset.z = 0;
	copyRegion.imageExtent = region;
	copyRegion.imageSubresource = rangeLayers;

	// If the source image contains both depth & stencil, then both aspect flags need to provided for pipeline barrier. But for the copy operation there must only be one aspect.
	GpuTextureSubresourceRange subresourceRangeForBarrier = subresourceRange;
	subresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	mResourceTracker.TrackImageUsage(source, subresourceRangeForBarrier, layout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackBufferUsage(destination, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdCopyImageToBuffer(GetVulkanHandle(), source->GetVulkanHandle(), vkLayout, destination->GetVulkanHandle(), 1, &copyRegion);
}

void VulkanGpuCommandBuffer::CopyImageToImage(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageCopy* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	// If the source image contains both depth & stencil, then both aspect flags need to provided for pipeline barrier. But for the copy operation there must only be one aspect.
	GpuTextureSubresourceRange sourceSubresourceRangeForBarrier = sourceSubresourceRange;
	sourceSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	GpuTextureSubresourceRange destinationSubresourceRangeForBarrier = destinationSubresourceRange;
	destinationSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdCopyImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions);
}

void VulkanGpuCommandBuffer::Blit(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageBlit* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	// If the source image contains both depth & stencil, then both aspect flags need to provided for pipeline barrier. But for the copy operation there must only be one aspect.
	GpuTextureSubresourceRange sourceSubresourceRangeForBarrier = sourceSubresourceRange;
	sourceSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	GpuTextureSubresourceRange destinationSubresourceRangeForBarrier = destinationSubresourceRange;
	destinationSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdBlitImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions, VK_FILTER_LINEAR);
}

void VulkanGpuCommandBuffer::Resolve(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageResolve* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	GpuTextureSubresourceRange sourceSubresourceRangeForBarrier = sourceSubresourceRange;
	sourceSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	GpuTextureSubresourceRange destinationSubresourceRangeForBarrier = destinationSubresourceRange;
	destinationSubresourceRangeForBarrier.AspectMask = source->GetRange().AspectMask;

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, GpuResourceUseFlag::Resolve, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, GpuResourceUseFlag::Resolve, GpuAccessFlag::Write, mBarrierHelper);

	mBarrierHelper.Execute(*this);

	vkCmdResolveImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions);
}

// TODO - Deprecate
void VulkanGpuCommandBuffer::MemoryBarrier(VkBuffer buffer, VkAccessFlags sourceAccessFlags, VkAccessFlags destinationAccessFlags, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = sourceAccessFlags;
	barrier.dstAccessMask = destinationAccessFlags;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = 0;
	barrier.size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(GetVulkanHandle(), sourceStage, destinationStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void VulkanGpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
{
	if(!B3D_ENSURE(!IsInRenderPass()))
		return;

	// Helper lambda to process a single image with the barrier
	auto fnAddImageBarrier = [this](VulkanImage* vulkanImage, const GpuTextureSubresourceRange& subresourceRange, const GpuSurfaceBarrier& barrier)
	{
		if(vulkanImage == nullptr)
			return;

		// Filter out invalid aspect mask to avoid validation warnings
		GpuTextureSubresourceRange maskedRange = subresourceRange;
		maskedRange.AspectMask &= vulkanImage->GetRange().AspectMask;

		mResourceTracker.TrackExplicitImageBarrier(vulkanImage, maskedRange, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout, mBarrierHelper);
	};

	for(const auto& barrier : barriers.BufferBarriers)
	{
		VulkanGpuBuffer* const vulkanGpuBuffer = static_cast<VulkanGpuBuffer*>(barrier.Object.get());
		if(vulkanGpuBuffer == nullptr)
			continue;

		VulkanBuffer* const vulkanBuffer = vulkanGpuBuffer->GetVulkanResource();

		mResourceTracker.TrackExplicitBufferBarrier(vulkanBuffer, barrier.DestinationUsage, barrier.DestinationAccess, mBarrierHelper);
	}

	for(const auto& barrier : barriers.TextureBarriers)
	{
		VulkanTexture* const vulkanTexture = static_cast<VulkanTexture*>(barrier.Object.get());
		if(vulkanTexture == nullptr)
			continue;

		VulkanImage* const vulkanImage = vulkanTexture->GetVulkanResource();
		fnAddImageBarrier(vulkanImage, barrier.SubresourceRange, barrier);
	}

	for(const auto& barrier : barriers.RenderTargetBarriers)
	{
		if(barrier.Object == nullptr)
			continue;

		// Get framebuffer based on render target type
		VulkanFramebuffer* framebuffer = nullptr;
		if(barrier.Object->GetProperties().IsWindow)
		{
			// Handle RenderWindow - get the active framebuffer from the surface if it was acquired
			RenderWindow* const renderWindow = static_cast<RenderWindow*>(barrier.Object.get());
			IVulkanRenderWindowSurface* surface = static_cast<IVulkanRenderWindowSurface*>(renderWindow->GetRenderWindowSurface().get());

			if(surface != nullptr)
			{
				// Only get framebuffer if this surface was acquired (tracked in mAcquiredSurfaces)
				const auto found = std::find(mAcquiredSurfaces.begin(), mAcquiredSurfaces.end(), surface);
				if(found != mAcquiredSurfaces.end())
					framebuffer = surface->GetActiveFramebuffer(false);
			}
		}
		else
		{
			// Handle RenderTexture
			VulkanRenderTexture* const renderTexture = static_cast<VulkanRenderTexture*>(barrier.Object.get());
			framebuffer = renderTexture->GetFramebuffer();
		}

		if(framebuffer == nullptr)
			continue;

		const GpuFramebufferAttachment* const attachment = framebuffer->FindAttachment(barrier.SurfaceMask);
		if(attachment != nullptr)
			fnAddImageBarrier(static_cast<VulkanImage*>(attachment->Image), barrier.SubresourceRange, barrier);
	}

	mBarrierHelper.Execute(*this);
}

void VulkanGpuCommandBuffer::NotifyRenderTargetModified()
{
	if(mRenderTarget == nullptr || mRenderTargetModified)
		return;

	mRenderTarget->TickUpdateCount();
	mRenderTargetModified = true;
}

Area2I VulkanGpuCommandBuffer::GetViewportArea() const
{
	Area2I area;
	area.X = (i32)Math::Round(mNormalizedViewportArea.X * (float)mFramebuffer->GetWidth());
	area.Y = (i32)Math::Round(mNormalizedViewportArea.Y * (float)mFramebuffer->GetHeight());
	area.Width = (u32)Math::Round(mNormalizedViewportArea.Width * (float)mFramebuffer->GetWidth());
	area.Height = (u32)Math::Round(mNormalizedViewportArea.Height * (float)mFramebuffer->GetHeight());

	area.X = Math::Clamp(area.X, 0, std::max(0, (i32)mFramebuffer->GetWidth() - 1));
	area.Y = Math::Clamp(area.Y, 0, std::max(0, (i32)mFramebuffer->GetHeight() - 1));
	area.Width = (u32)(Math::Clamp(area.X + (i32)area.Width, 0, (i32)mFramebuffer->GetWidth()) - area.X);
	area.Height = (u32)(Math::Clamp(area.Y + (i32)area.Height, 0, (i32)mFramebuffer->GetHeight()) - area.Y);

	return area;
}

Area2I VulkanGpuCommandBuffer::GetRenderPassArea() const
{
	Area2I area;
	area.X = 0;
	area.Y = 0;
	area.Width = mFramebuffer != nullptr ? (i32)mFramebuffer->GetWidth() : 0;
	area.Height = mFramebuffer != nullptr ? (i32)mFramebuffer->GetHeight() : 0;

	return area;
}

void VulkanGpuCommandBuffer::SetName(const StringView& name)
{
	GpuCommandBuffer::SetName(name);

	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	VkDebugUtilsObjectNameInfoEXT objectNameInfo;
	objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	objectNameInfo.pNext = nullptr;
	objectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
	objectNameInfo.objectHandle = (uint64_t)mCommandBufferHandle;
	objectNameInfo.pObjectName = name.data();

	vkSetDebugUtilsObjectNameEXT(GetVulkanGpuDevice().GetLogical(), &objectNameInfo);
}
