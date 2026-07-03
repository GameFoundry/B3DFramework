//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuBuffer.h"
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

template <class T>
static void GetPipelineStageFlags(const TArray<T>& barriers, VkPipelineStageFlags& source, VkPipelineStageFlags& destination)
{
	for(auto& entry : barriers)
	{
		source |= VulkanUtility::GetPipelineStageFlags(entry.srcAccessMask);
		destination |= VulkanUtility::GetPipelineStageFlags(entry.dstAccessMask);
	}

	if(source == 0)
		source = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if(destination == 0)
		destination = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

const Color kDebugLabelColor = Color::kBansheeOrange;
constexpr u32 kMaximumBoundDescriptorSets = 64;

VulkanGpuCommandBuffer::VulkanGpuCommandBuffer(VulkanGpuDevice& device, VulkanGpuCommandBufferPool& pool, u32 id, VkCommandBuffer commandBufferHandle, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation)
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandBufferHandle(commandBufferHandle), mPool(pool), mOwnerThread(ownerThread), mGfxPipelineRequiresBind(true), mCmpPipelineRequiresBind(true), mViewportRequiresBind(true), mStencilRefRequiresBind(true), mScissorRequiresBind(true), mBoundParamsDirty(false), mVertexInputsDirty(false), mBarrierHelper(&mResourceTracker)
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

	if(mIntraQueueSemaphore != nullptr)
		mIntraQueueSemaphore->Destroy();

	for(u32 i = 0; i < B3D_MAX_COMMAND_BUFFER_DEPENDENCIES; i++)
	{
		if(mInterQueueSemaphores[i] != nullptr)
			mInterQueueSemaphores[i]->Destroy();
	}

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

	// Register framebuffer & swap chain. Note this needs to happen before binding parameters, because if texture is used as a read-only attachment GPU parameters need to be
	// aware to pick the correct layout
	mResourceTracker.TrackFramebufferUsage(mFramebuffer, loadMask, readOnlyMask, mBarrierHelper);

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
		vkParams->PrepareForBind(*this, mResourceTracker, mBarrierHelper, descriptorSet, tempDynamicOffsets);

		// Cache the preparation results for later use by SetGpuParameterSet
		CachedGpuParameterData& cacheData = mRenderPassGpuParameterSetCache[parameters.get()];
		cacheData.DescriptorSet = descriptorSet;
		cacheData.DynamicOffsets = std::move(tempDynamicOffsets);
	}

	mBarrierHelper.Execute(*this);

	// Re-set the params as they will need to be re-bound
	for(const auto& entry : mBoundGpuParameterSets) // TODO - Can likely be removed
		SetGpuParameterSet(entry);

	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;

	const Area2I renderArea = GetRenderPassArea();

	const RenderSurfaceMask readMask = mResourceTracker.GetFramebufferReadOnlyMask(mFramebuffer, mRenderTargetReadOnlyMask);
	const RenderSurfaceMask originalClearMask = createInformation.ClearMask;
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues = BuildClearValues(originalClearMask, createInformation.ClearColor, createInformation.ClearDepth, createInformation.ClearStencil);

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	RenderSurfaceMask clearMask = createInformation.ClearMask;

#if B3D_DEBUG
	const VkClearColorValue kDebugClearColor = { { 1.0f, 0.0f, 1.0f, 1.0f } }; // Bright pink

	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachment(sequentialColorAttachmentIndex);
		const RenderSurfaceMaskBits colorAttachmentBit = (RenderSurfaceMaskBits)(1 << sequentialColorAttachmentIndex);
		if(loadMask.IsSet(colorAttachmentBit))
			continue;

		if(readMask.IsSet(colorAttachmentBit))
		{
			B3D_LOG(Error, LogRenderBackend, "Color attachment at index {0} cannot be read only if we're not loading it.", colorAttachment.Index);
			continue;
		}

		// In debug mode clear not loaded values to the clear color
		if(!originalClearMask.IsSet(colorAttachmentBit))
		{
			clearMask |= colorAttachmentBit;
			clearValues[colorAttachment.Index].color = kDebugClearColor;
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

void VulkanGpuCommandBuffer::ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
{
	EnsureValidThread();

	Area2I area(0, 0, mFramebuffer->GetWidth(), mFramebuffer->GetHeight());
	ClearAttachments(area, mask, color, depth, stencil);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil)
{
	EnsureValidThread();

	const Area2I viewportArea = GetViewportArea();
	ClearAttachments(viewportArea, mask, color, depth, stencil);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& state)
{
	EnsureValidThread();

	if(mGraphicsPipeline == state)
		return;

	mGraphicsPipeline = std::static_pointer_cast<VulkanGpuGraphicsPipelineState>(state);
	mGfxPipelineRequiresBind = true;

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

	vkCmdDispatch(mCommandBufferHandle, groupCountX, groupCountY, groupCountZ);

	// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
	// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
	// images remain to be used in subsequent calls).
	mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();

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

	bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	GpuImageLayout transferSourceLayout = vulkanSource->IsDirectlyMappable() ? GpuImageLayout::General : GpuImageLayout::TransferSource;
	GpuImageLayout transferDestinationLayout = vulkanDestination->IsDirectlyMappable() ? GpuImageLayout::General : GpuImageLayout::TransferDestination;

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

	if(sourceHasMultipleSamples && !destinationHasMultipleSamples)
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

		Resolve(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, sourceRange, destinationRange, 1, &resolveRegion);
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

		CopyImageToImage(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, sourceRange, destinationRange, 1, &imageRegion);
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

	// Execute any queued events
	for(auto& entry : mQueuedEvents)
		vkCmdSetEvent(mCommandBufferHandle, entry->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	mQueuedEvents.clear();

	// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
	// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
	// images remain to be used in subsequent calls).
	mResourceTracker.ClearShaderFlagsForAllRenderPassImageSubresources();

	if(mFramebuffer != nullptr)
	{
		mResourceTracker.MoveAllFramebufferAttachmentsToFinalLayouts(mFramebuffer);

		VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
		u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
		for(u32 i = 0; i < colorAttachmentCount; i++)
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetColorAttachment(i);
			mResourceTracker.ClearFramebufferFlagsForImage(fbAttachment.Image);
		}

		if(renderPass->HasDepthAttachment())
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetDepthStencilAttachment();
			mResourceTracker.ClearFramebufferFlagsForImage(fbAttachment.Image);
		}
	}

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

u32 VulkanGpuCommandBuffer::AllocateSignalSemaphores(TInlineArray<VkSemaphore, 8>& outSemaphores)
{
	// TODO - Do I need multiple semaphores? Can't I just have one?

	u32 count = 0;

	if(mIntraQueueSemaphore != nullptr)
		mIntraQueueSemaphore->Destroy();

	mIntraQueueSemaphore = GetVulkanGpuDevice().GetResourceManager().Create<VulkanSemaphore>("IntraQueue");

	outSemaphores.Add(mIntraQueueSemaphore->GetHandle());
	count++;

	for(u32 i = 0; i < B3D_MAX_COMMAND_BUFFER_DEPENDENCIES; i++)
	{
		if(mInterQueueSemaphores[i] != nullptr)
			mInterQueueSemaphores[i]->Destroy();

		mInterQueueSemaphores[i] = GetVulkanGpuDevice().GetResourceManager().Create<VulkanSemaphore>("InterQueue");
		outSemaphores.Add(mInterQueueSemaphores[i]->GetHandle());
		count++;
	}

	mNumUsedInterQueueSemaphores = 0;
	return count;
}

VulkanSemaphore* VulkanGpuCommandBuffer::RequestInterQueueSemaphore() const
{
	if(mNumUsedInterQueueSemaphores >= B3D_MAX_COMMAND_BUFFER_DEPENDENCIES)
		return nullptr;

	return mInterQueueSemaphores[mNumUsedInterQueueSemaphores++];
}

GpuCommandBufferSubmitInformation VulkanGpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted()); // Caller should already have set this flag

	GpuCommandBufferSubmitInformation submitInformation;
	GpuCommandBufferPool& commandBufferPool = GetVulkanGpuDevice().GetSubmitThread().GetCommandBufferPool(queueType);

	// Issue pipeline barriers for queue transitions (need to happen on original queue first, then on new queue)
	for(auto& entry : mResourceTracker.GetBuffers())
	{
		VulkanBuffer* resource = static_cast<VulkanBuffer*>(entry.first);

		if(!resource->IsExclusive())
			continue;

		const GpuQueueType oldQueueType = resource->GetOwnedQueueType();
		if(oldQueueType != GQT_UNKNOWN && oldQueueType != queueType)
		{
			TArray<VkBufferMemoryBarrier>& barriers = mTransitionInfoTemp[(i32)oldQueueType].BufferBarriers;

			barriers.Add(VkBufferMemoryBarrier());
			VkBufferMemoryBarrier& barrier = barriers.Back();
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;
			barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueType);
			barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueType);
			barrier.buffer = resource->GetVulkanHandle();
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
		}
	}

	// For images issue queue transitions, as above. Also issue layout transitions to their inital layouts.
	TArray<VkImageMemoryBarrier>& localBarriers = mTransitionInfoTemp[(i32)queueType].ImageBarriers;
	for(auto& entry : mResourceTracker.GetImages())
	{
		VulkanImage* const image = static_cast<VulkanImage*>(entry.first);
		TArrayView<GpuImageSubresourceTrackingState> subresourceTrackingStates = mResourceTracker.GetSubresourceTrackingStatesForImage(image);

		const GpuQueueType oldQueueType = image->GetOwnedQueueType();
		bool queueMismatch = image->IsExclusive() && oldQueueType != GQT_UNKNOWN && oldQueueType != queueType;

		if(queueMismatch)
		{
			TArray<VkImageMemoryBarrier>& barriers = mTransitionInfoTemp[(i32)oldQueueType].ImageBarriers;

			for(const auto& subresourceTrackingState : subresourceTrackingStates)
			{
				u32 startIdx = (u32)barriers.size();
				image->GetBarriers(VulkanUtility::ToVkImageSubresourceRange(subresourceTrackingState.Range), barriers);

				for(u32 j = startIdx; j < (u32)barriers.size(); j++)
				{
					VkImageMemoryBarrier& barrier = barriers[j];

					barrier.dstAccessMask = 0;
					barrier.newLayout = barrier.oldLayout;
					barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueType);
					barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueType);
				}
			}
		}

		for(const auto& subresourceTrackingState : subresourceTrackingStates)
		{
			const GpuTextureSubresourceRange& range = subresourceTrackingState.Range;
			u32 mipEnd = range.BaseMipLevel + range.MipLevelCount;
			u32 faceEnd = range.BaseArrayLayer + range.ArrayLayerCount;

			bool layoutMismatch = false;
			VkImageLayout initialLayout = VulkanUtility::ToVkImageLayout(subresourceTrackingState.InitialLayout);
			if(initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			{
				for(u32 mip = range.BaseMipLevel; mip < mipEnd; mip++)
				{
					for(u32 face = range.BaseArrayLayer; face < faceEnd; face++)
					{
						VulkanImageSubresource* subresource = image->GetSubresource(face, mip);
						if(subresource->GetLayout() != initialLayout)
						{
							layoutMismatch = true;
							break;
						}
					}

					if(layoutMismatch)
						break;
				}

			}

			if(layoutMismatch || queueMismatch)
			{
				u32 startIdx = (u32)localBarriers.size();
				image->GetBarriers(VulkanUtility::ToVkImageSubresourceRange(subresourceTrackingState.Range), localBarriers);

				for(u32 j = startIdx; j < (u32)localBarriers.size(); j++)
				{
					VkImageMemoryBarrier& barrier = localBarriers[j];

					barrier.dstAccessMask = image->GetAccessFlags(initialLayout, subresourceTrackingState.InitialReadOnly);
					barrier.newLayout = layoutMismatch ? initialLayout : barrier.oldLayout;

					if(queueMismatch)
					{
						barrier.srcAccessMask = 0;
						barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueType);
						barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueType);
					}
				}
			}

			for(u32 mip = range.BaseMipLevel; mip < mipEnd; mip++)
			{
				for(u32 face = range.BaseArrayLayer; face < faceEnd; face++)
				{
					VulkanImageSubresource* subresource = image->GetSubresource(face, mip);
					subresource->SetLayout(VulkanUtility::ToVkImageLayout(subresourceTrackingState.CurrentLayout));
				}
			}
		}
	}

	B3D_ASSERT(B3DSize(mTransitionInfoTemp) == GQT_COUNT);
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const GpuQueueType transitionQueueType = (GpuQueueType)queueTypeIndex;
		TransitionInfo& transitionInformation = mTransitionInfoTemp[queueTypeIndex];

		bool empty = transitionInformation.ImageBarriers.Empty() && transitionInformation.BufferBarriers.Empty();
		if(empty)
			continue;

		// No queue transition needed for entries on this queue (this entry is most likely an image layout transition)
		if(transitionQueueType == GQT_UNKNOWN || transitionQueueType == queueType)
			continue;

		const TShared<VulkanGpuCommandBuffer> sourceTransitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Source queue transition")));
		VkCommandBuffer vkCmdBuffer = sourceTransitionCommandBuffer->GetVulkanHandle();

		const u32 imageBarrierCount = (u32)transitionInformation.ImageBarriers.size();
		const u32 bufferBarrierCount = (u32)transitionInformation.BufferBarriers.size();

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		::GetPipelineStageFlags(transitionInformation.ImageBarriers, srcStage, dstStage);

		vkCmdPipelineBarrier(vkCmdBuffer, srcStage, dstStage, 0, 0, nullptr, bufferBarrierCount, transitionInformation.BufferBarriers.data(), imageBarrierCount, transitionInformation.ImageBarriers.data());

		sourceTransitionCommandBuffer->End();

		// Note: If I switch back to doing layout transitions here, I need to wait on present semaphore for this command buffer
		submitInformation.SourceQueueTransitionCommandBuffer[transitionQueueType] = sourceTransitionCommandBuffer;
	}

	// Wait on present (i.e. until the back buffer becomes available) for any surfaces
	for(IVulkanRenderWindowSurface* surface : mAcquiredSurfaces)
		surface->AppendWaitSemaphoresIfRequired(submitInformation.Semaphores);

	// Issue second part of transition pipeline barriers (on this queue)
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const GpuQueueType transitionQueueType = (GpuQueueType)queueTypeIndex;
		TransitionInfo& transitionInformation = mTransitionInfoTemp[queueTypeIndex];

		bool empty = transitionInformation.ImageBarriers.Empty() && transitionInformation.BufferBarriers.Empty();
		if(empty)
			continue;

		if(transitionQueueType != GQT_UNKNOWN && transitionQueueType != queueType)
			continue;

		TShared<VulkanGpuCommandBuffer> transitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Queue and layout transitions")));

		VkCommandBuffer vkCmdBuffer = transitionCommandBuffer->GetVulkanHandle();

		const u32 imageBarrierCount = (u32)transitionInformation.ImageBarriers.size();
		const u32 bufferBarrierCount = (u32)transitionInformation.BufferBarriers.size();

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		::GetPipelineStageFlags(transitionInformation.ImageBarriers, srcStage, dstStage);

		vkCmdPipelineBarrier(vkCmdBuffer, srcStage, dstStage, 0, 0, nullptr, bufferBarrierCount, transitionInformation.BufferBarriers.data(), imageBarrierCount, transitionInformation.ImageBarriers.data());

		transitionCommandBuffer->End();

		submitInformation.DestinationQueueTransitionCommandBuffer = transitionCommandBuffer;
	}

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(GetShared());

	mSubmittedQueueId = GpuQueueId(queueType, queueIndex);
	mResourceTracker.NotifyUsed(mSubmittedQueueId);

	// Note: Uncomment for debugging only, prevents any device concurrency issues.
	// vkQueueWaitIdle(queue->GetHandle());

	// Clear vectors but don't clear the actual map, as we want to re-use the memory since we expect queue family
	// indices to be the same
	for(auto& entry : mTransitionInfoTemp)
	{
		entry.ImageBarriers.clear();
		entry.BufferBarriers.clear();
	}

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

void VulkanGpuCommandBuffer::Cleanup()
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

	OnDidComplete.Clear();
	OnDestroyed.Clear();
}

void VulkanGpuCommandBuffer::Reset()
{
	Cleanup();

	if(!mPool.GetUsePoolReset())
	{
		vkResetCommandBuffer(mCommandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		mState = GpuCommandBufferState::Ready;
	}
}

void VulkanGpuCommandBuffer::NotifyParentPoolReset()
{
	Cleanup();
	mState = GpuCommandBufferState::Ready;
}

Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> VulkanGpuCommandBuffer::BuildClearValues(RenderSurfaceMask clearMask, const Color& color, float depth, u16 stencil)
{
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues{};

	if(clearMask == RT_NONE || mFramebuffer == nullptr)
		return clearValues;

	// Determine which attachments require clearing, and their clear values
	const VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
	{
		const VulkanFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachment(sequentialColorAttachmentIndex);
		const RenderSurfaceMaskBits colorAttachmentBit = (RenderSurfaceMaskBits)(1 << colorAttachment.Index);

		if(!clearMask.IsSet(colorAttachmentBit))
			continue;

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
			clearValues[depthAttachmentSequentialIndex].depthStencil.depth = depth;

		if(clearMask.IsSet(RT_STENCIL))
			clearValues[depthAttachmentSequentialIndex].depthStencil.stencil = stencil;
	}

	return clearValues;
}

void VulkanGpuCommandBuffer::ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask, const Color& color, float depth, u16 stencil)
{
	ClearAttachments(area, clearMask, BuildClearValues(clearMask, color, depth, stencil));
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
		const VulkanFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachment(sequentialColorAttachmentIndex);
		const RenderSurfaceMaskBits colorAttachmentBit = (RenderSurfaceMaskBits)(1 << colorAttachment.Index);

		if(!clearMask.IsSet(colorAttachmentBit))
			continue;

		attachments[sequentialClearedAttachmentIndex].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachments[sequentialClearedAttachmentIndex].colorAttachment = colorAttachment.Index;
		attachments[sequentialClearedAttachmentIndex].clearValue.color = clearValues[sequentialClearedAttachmentIndex].color;

		const u32 colorAttachmentBaseLayer = colorAttachment.BaseLayer;
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
				attachments[sequentialClearedAttachmentIndex].clearValue.depthStencil.depth = clearValues[sequentialClearedAttachmentIndex].depthStencil.depth;
			}

			if(clearMask.IsSet(RT_STENCIL))
			{
				attachments[sequentialClearedAttachmentIndex].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				attachments[sequentialClearedAttachmentIndex].clearValue.depthStencil.stencil = clearValues[sequentialClearedAttachmentIndex].depthStencil.stencil;
			}

			attachments[sequentialClearedAttachmentIndex].colorAttachment = 0;

			const u32 depthStencilAttachmentBaseLayer = mFramebuffer->GetDepthStencilAttachment().BaseLayer;
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
	const TShared<VulkanVertexInput> vertexShaderInput = VulkanVertexInputManager::Instance().GetVertexInfo(mVertexDescription, vertexShaderInputDescription);

	VulkanRenderPass *const renderPass = mFramebuffer->GetRenderPass();
	VulkanPipeline *const pipeline = mGraphicsPipeline->FindOrCreateVulkanResource(renderPass, mRenderTargetReadOnlyMask, mDrawOp, vertexShaderInput);

	if(pipeline == nullptr)
		return false;

	// Check that pipeline matches the read-only state of any framebuffer attachments
	u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 i = 0; i < colorAttachmentCount; i++)
	{
		const VulkanFramebufferAttachment& framebufferAttachment = mFramebuffer->GetColorAttachment(i);
		const GpuImageSubresourceTrackingState& subresourceTrackingState = static_cast<const VulkanResourceTracker&>(mResourceTracker).GetSubresourceTrackingState(framebufferAttachment.Image, framebufferAttachment.Surface.Face, framebufferAttachment.Surface.MipLevel);

		if(subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write) && !pipeline->IsColorReadOnly(i))
		{
			B3D_LOG(Warning, LogRenderBackend, "Framebuffer attachment also used as a shader input, but color writes "
										   "aren't disabled. This will result in undefined behavior.");
		}
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& framebufferAttachment = mFramebuffer->GetDepthStencilAttachment();
		const GpuImageSubresourceTrackingState& subresourceTrackingState = static_cast<const VulkanResourceTracker&>(mResourceTracker).GetSubresourceTrackingState(framebufferAttachment.Image, framebufferAttachment.Surface.Face, framebufferAttachment.Surface.MipLevel);

		if(subresourceTrackingState.ShaderUse.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write) && !pipeline->IsDepthReadOnly())
		{
			B3D_LOG(Warning, LogRenderBackend, "Framebuffer attachment also used as a shader input, but depth/stencil "
										   "writes aren't disabled. This will result in undefined behavior.");
		}
	}

	mGraphicsPipeline->RegisterShaderModuleResources(mResourceTracker);
	mResourceTracker.TrackResourceUsage(pipeline, GpuAccessFlag::Read);

	vkCmdBindPipeline(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVulkanHandle());
	BindDynamicStates(true);

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
				boundGpuParameterSet->PrepareForBind(*this, mResourceTracker, barrierHelper, mDescriptorSetsTemp[set], setDynamicOffsets);
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
	mResourceTracker.TrackImageUsage(destination, subresourceRange, layout, layout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

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

	mResourceTracker.TrackImageUsage(source, subresourceRangeForBarrier, layout, layout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
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

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, sourceLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, destinationLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

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

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, sourceLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, destinationLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

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

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRangeForBarrier, sourceLayout, sourceLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, mBarrierHelper);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRangeForBarrier, destinationLayout, destinationLayout, GpuResourceUseFlag::Transfer, GpuAccessFlag::Write, mBarrierHelper);

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

VkImageLayout VulkanGpuCommandBuffer::GetCurrentLayout(VulkanImage* image, const GpuTextureSubresourceRange& range, bool inRenderPass)
{
	if(inRenderPass)
		return VulkanUtility::ToVkImageLayout(mResourceTracker.GetCurrentSubresourceLayout(image, range, mFramebuffer, mRenderTargetReadOnlyMask));

	return VulkanUtility::ToVkImageLayout(mResourceTracker.GetCurrentSubresourceLayout(image, range));
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

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			mBarrierHelper.AddImageBarrier(vulkanImage, maskedRange, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout);
		else
			mBarrierHelper.AddImageBarrier(vulkanImage, maskedRange, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess, barrier.SourceLayout, barrier.DestinationLayout);
	};

	for(const auto& barrier : barriers.BufferBarriers)
	{
		VulkanGpuBuffer* const vulkanGpuBuffer = static_cast<VulkanGpuBuffer*>(barrier.Object.get());
		if(vulkanGpuBuffer == nullptr)
			continue;

		VulkanBuffer* const vulkanBuffer = vulkanGpuBuffer->GetVulkanResource();

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			mBarrierHelper.AddBufferBarrier(vulkanBuffer, barrier.DestinationUsage, barrier.DestinationAccess);
		else
			mBarrierHelper.AddBufferBarrier(vulkanBuffer, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess);
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

		// Process color attachments if specified in the surface mask
		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
		{
			const RenderSurfaceMaskBits colorMask = static_cast<RenderSurfaceMaskBits>(RT_COLOR0 << colorIndex);
			if(barrier.SurfaceMask == colorMask)
			{
				const VulkanFramebufferAttachment& attachment = framebuffer->GetColorAttachment(colorIndex);
				if(attachment.Image != nullptr)
					fnAddImageBarrier(attachment.Image, barrier.SubresourceRange, barrier);

				break;
			}
		}

		// Process depth/stencil attachment if specified in the surface mask
		if(barrier.SurfaceMask == RT_DEPTH || barrier.SurfaceMask == RT_STENCIL)
		{
			const VulkanFramebufferAttachment& attachment = framebuffer->GetDepthStencilAttachment();
			if(attachment.Image != nullptr)
				fnAddImageBarrier(attachment.Image, barrier.SubresourceRange, barrier);
		}
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
