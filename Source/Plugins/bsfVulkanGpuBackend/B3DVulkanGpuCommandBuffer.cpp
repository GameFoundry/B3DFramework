//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
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
#include "Utility/B3DVulkanBarrierBatch.h"

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
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandBufferHandle(commandBufferHandle), mPool(pool), mOwnerThread(ownerThread), mGfxPipelineRequiresBind(true), mCmpPipelineRequiresBind(true), mGraphicsPushConstantsRequireBind(false), mComputePushConstantsRequireBind(false), mViewportRequiresBind(true), mStencilRefRequiresBind(true), mScissorRequiresBind(true), mBoundParamsDirty(false), mVertexInputsDirty(false)
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
	GpuCommandBuffer::BeginRenderPass(createInformation);
	EnsureValidThread();
	B3D_ASSERT(mState == GpuCommandBufferState::Recording);

	const TShared<RenderTarget>& renderTarget = createInformation.Target;
	if(!B3D_ENSURE(renderTarget != nullptr))
		return;

	RenderSurfaceMask readOnlyMask = createInformation.ReadOnlyMask;
	RenderSurfaceMask loadMask = createInformation.LoadMask;

	VulkanFramebuffer* newFramebuffer;
	if(renderTarget->GetProperties().IsWindow)
	{
		newFramebuffer = AcquireWindowFramebuffer(static_cast<RenderWindow&>(*renderTarget));
		if(newFramebuffer == nullptr)
			B3D_LOG(Error, LogRenderBackend, "Binding render target failed. Unable to acquire swap chain image.");
	}
	else
	{
		const VulkanRenderTexture* const renderTexture = static_cast<VulkanRenderTexture*>(renderTarget.get());
		newFramebuffer = renderTexture->GetFramebuffer();
	}

	if(!B3D_ENSURE(newFramebuffer != nullptr))
		return;

	mRenderTarget = renderTarget;

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
	const GpuRenderPassAttachmentUsageArray renderPassAttachmentUsages = mFramebuffer->BuildRenderPassAttachmentUsages(readOnlyMask, loadMask);

#if B3D_BUILD_TYPE_DEVELOPMENT
	mDrawAccessValidator.BeginRenderPass();
#endif
	mResourceTracker.BeginRenderPass(renderPassAttachmentUsages);

	// Surfaces the target does not have are not read-only
	RenderSurfaceMask attachmentReadOnlyMask = RT_NONE;
	for(const GpuRenderPassAttachmentUsage& attachment : renderPassAttachmentUsages)
	{
		if(attachment.Access == GpuAccessFlag::Read)
			attachmentReadOnlyMask.Set(attachment.Surface);
	}

	mRenderTargetReadOnlyMask = attachmentReadOnlyMask;

	// Re-set the params as they will need to be re-bound
	for(const auto& entry : mBoundGpuParameterSets) // TODO - Can likely be removed
		SetGpuParameterSet(entry);

	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;

	const Area2I renderArea = GetRenderPassArea();

	const RenderSurfaceMask readMask = mRenderTargetReadOnlyMask;
	RenderTargetClearValues targetClearValues;
	const RenderSurfaceMask clearMask = GpuBackendUtility::GetRenderPassClears(createInformation, targetClearValues);
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues = BuildClearValues(clearMask, targetClearValues);

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
	GpuCommandBuffer::SetGpuGraphicsPipelineState(state);
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
	GpuCommandBuffer::SetGpuComputePipelineState(state);
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
	GpuCommandBuffer::SetGpuParameterSet(parameterSet);
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

	if(set >= (u32)mBoundGpuParameterSets.Size() || mBoundGpuParameterSets[set] == nullptr)
		return;

	if(!B3D_ENSURE_LOG(bufferIndex < mBoundGpuParameterSets[set]->GetLayout()->GetDynamicOffsetCount(), "Dynamic offset index {0} is out of range for parameter set {1}.", bufferIndex, set))
		return;

	// Ensure storage is sized
	while(mDynamicOffsetsOverridesPerSet.size() <= set)
		mDynamicOffsetsOverridesPerSet.Add(UnorderedMap<u32, u32>());

	auto& overrides = mDynamicOffsetsOverridesPerSet[set];
	if(const auto found = overrides.find(bufferIndex); found != overrides.end() && found->second == offset)
		return;

	overrides[bufferIndex] = offset;
	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;

	// Already-prepared sets only need their bind-time offsets changed.
	if(!mBoundParamsDirty && set < mDynamicOffsetsPerSet.size() && bufferIndex < mDynamicOffsetsPerSet[set].size())
	{
		mDynamicOffsetsPerSet[set][bufferIndex] = offset;
#if B3D_BUILD_TYPE_DEVELOPMENT
		mBoundGpuParameterSets[set]->TrackDynamicUniformBufferOffsets(mResourceTracker, mDynamicOffsetsPerSet[set]);
#endif
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
	GpuCommandBuffer::SetVertexBuffers(startIndex, buffers, bufferCount);
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
	GpuCommandBuffer::SetIndexBuffer(buffer);
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
	GpuCommandBuffer::Draw(vertexOffset, vertexCount, instanceCount, firstInstance);
	EnsureValidThread();

	if(vertexCount == 0)
		return;

	B3D_ENSURE(IsInRenderPass());

	if(!IsReadyForRender())
		return;

	if(!BindGpuParameters(mGraphicsPipeline->GetParameterLayout()))
		return;

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
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->GetPipelineLayoutHandle());

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Graphics);
	}

	BindPushConstants(true);

	if(instanceCount <= 0)
		instanceCount = 1;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!ValidateDrawAccesses())
		return;
#endif

	vkCmdDraw(mCommandBufferHandle, vertexCount, instanceCount, vertexOffset, firstInstance);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, 0); // TODO - Determine accurate primitive count
}

void VulkanGpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GpuCommandBuffer::DrawIndexed(startIndex, indexCount, vertexOffset, vertexCount, instanceCount, firstInstance);
	EnsureValidThread();

	B3D_ENSURE(IsInRenderPass());

	if(indexCount == 0)
		return;

	if(!IsReadyForRender())
		return;

	if(!BindGpuParameters(mGraphicsPipeline->GetParameterLayout()))
		return;

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
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->GetPipelineLayoutHandle());

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Graphics);
	}

	BindPushConstants(true);

	if(instanceCount <= 0)
		instanceCount = 1;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!ValidateDrawAccesses())
		return;
#endif

	vkCmdDrawIndexed(mCommandBufferHandle, indexCount, instanceCount, startIndex, vertexOffset, firstInstance);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, 0); // TODO - Determine accurate primitive count
}

void VulkanGpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	GpuCommandBuffer::DispatchCompute(groupCountX, groupCountY, groupCountZ);
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
	if(!BindGpuParameters(pipelineParameterLayout))
		return;

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
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline->GetPipelineLayoutHandle());

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Compute);
	}

	BindPushConstants(false);

	vkCmdDispatch(mCommandBufferHandle, groupCountX, groupCountY, groupCountZ);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void VulkanGpuCommandBuffer::CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length)
{
	GpuCommandBuffer::CopyBufferToBuffer(source, destination, sourceOffset, destinationOffset, length);
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
	GpuCommandBuffer::CopyBufferToTexture(source, destination, bufferOffset, mipLevel, arrayLayer);
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
	if(vulkanDestination->UsesGeneralLayout())
		transferLayout = GpuImageLayout::General;
	else
		transferLayout = GpuImageLayout::TransferDestination;

	CopyBufferToImage(sourceBuffer, destinationImage, extent, range, transferLayout, pitch.RowPitch, pitch.SliceHeight);
}

void VulkanGpuCommandBuffer::CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset)
{
	GpuCommandBuffer::CopyTextureToBuffer(source, destination, mipLevel, arrayLayer, bufferOffset);
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

	GpuImageLayout transferLayout;
	if(vulkanSource->UsesGeneralLayout())
		transferLayout = GpuImageLayout::General;
	else
		transferLayout = GpuImageLayout::TransferSource;

	CopyImageToBuffer(sourceImage, destinationBuffer, extent, range, transferLayout, pitch.RowPitch, pitch.SliceHeight);
}

void VulkanGpuCommandBuffer::CopyWindowToBuffer(const TShared<RenderWindow>& source, const TShared<GpuBuffer>& destination)
{
	GpuCommandBuffer::CopyWindowToBuffer(source, destination);
	EnsureValidThread();

	if(source == nullptr || destination == nullptr)
		return;

	auto* const surface = static_cast<IVulkanRenderWindowSurface*>(source->GetRenderWindowSurface().get());
	if(surface == nullptr)
		return;

	const TOptional<RenderWindowImageLayout> layout = surface->GetImageLayout();
	VulkanImage* const sourceImage = surface->GetCurrentColorImage();
	VulkanBuffer* const destinationBuffer = static_cast<VulkanGpuBuffer*>(destination.get())->GetVulkanResource();
	if(!layout || sourceImage == nullptr || destinationBuffer == nullptr)
		return;

	VkExtent3D extent;
	extent.width = layout->Width;
	extent.height = layout->Height;
	extent.depth = 1;

	const GpuTextureSubresourceRange range(0, 1, 0, 1, GpuTextureAspectFlag::Color);
	CopyImageToBuffer(sourceImage, destinationBuffer, extent, range, GpuImageLayout::TransferSource, layout->Pitch.RowPitch, layout->Pitch.SliceHeight);
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

	const GpuImageLayout sourceLayout = vulkanSource->UsesGeneralLayout() ? GpuImageLayout::General :
		(needsResolve ? GpuImageLayout::ResolveSource : GpuImageLayout::TransferSource);
	const GpuImageLayout destinationLayout = vulkanDestination->UsesGeneralLayout() ? GpuImageLayout::General :
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

	GpuImageLayout transferSourceLayout = vulkanSource->UsesGeneralLayout() ? GpuImageLayout::General : GpuImageLayout::TransferSource;
	GpuImageLayout transferDestinationLayout = vulkanDestination->UsesGeneralLayout() ? GpuImageLayout::General : GpuImageLayout::TransferDestination;

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

	// Clear the render-pass tracking scope; attachment layouts remain unchanged.
	mResourceTracker.EndRenderPass();

	mState = GpuCommandBufferState::Recording;
	mRenderTarget = nullptr;
	mRenderTargetReadOnlyMask = RT_NONE;
	mFramebuffer = nullptr;

	// Sets that stay bound are bound again for the commands that follow the render pass
	mBoundParamsDirty = true;
}

VulkanGpuCommandBufferSubmitInformation VulkanGpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted()); // Caller should already have set this flag

	VulkanGpuCommandBufferSubmitInformation submitInformation;

	// Transition the images that requested it to rest (see IGpuImageResource::RequestRestingTransition()). The command buffer's first barrier
	// from the resting state can be in any stage of the queue, so the transitions make the images available to all of them.
	VkPipelineStageFlags restingDestinationStages;
	VkAccessFlags restingDestinationAccess;
	VulkanUtility::GetPipelineStageAndAccessMask(GpuStageFlags(GpuStageFlag::All) & GpuBackendUtility::GetQueueStageFlags(queueType), GpuAccessFlag::Read | GpuAccessFlag::Write,
		restingDestinationStages, restingDestinationAccess);

	VulkanBarrierBatch restingTransitions;
	for(const auto& entry : mResourceTracker.GetImages())
	{
		VulkanImage& image = static_cast<VulkanImage&>(*entry.first);
		const GpuImageLayout restingLayout = image.ConsumeRestingTransition();
		if(restingLayout == GpuImageLayout::Undefined)
			continue;

		const VkImageLayout restingNativeLayout = VulkanUtility::ToVkImageLayout(restingLayout);
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!image.GetRange().AspectMask.IsSet(aspect))
				continue;

			GpuTextureSubresourceRange range = image.GetRange();
			range.AspectMask = aspect;

			// New images are undefined and window surfaces come from presentation, neither has contents another stage still accesses
			const VkImageLayout oldLayout = (VkImageLayout)image.GetNativeLayout(0, 0, aspect);
			if(oldLayout != restingNativeLayout)
			{
				restingTransitions.AddImageBarrier(image.GetVulkanHandle(), VulkanUtility::ToVkImageSubresourceRange(range), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
					restingDestinationStages, restingDestinationAccess, oldLayout, restingNativeLayout);
			}

			image.SetNativeLayout(range, (u32)restingNativeLayout);
		}
	}

	if(restingTransitions.HasBarriers())
	{
		GpuCommandBufferPool& commandBufferPool = GetVulkanGpuDevice().GetSubmitThread().GetCommandBufferPool(queueType);
		const TShared<VulkanGpuCommandBuffer> transitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.FindOrCreate(GpuCommandBufferCreateInformation::Create("Resting transitions")));
		restingTransitions.Execute(transitionCommandBuffer->GetVulkanHandle());
		transitionCommandBuffer->End();
		submitInformation.RestingTransitionCommandBuffer = transitionCommandBuffer;
	}

	PublishImageLayouts();

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

void VulkanGpuCommandBuffer::NotifyWillQueueForSubmit(GpuQueueId queueId, GpuQueueMask syncMask)
{
	GpuCommandBuffer::NotifyWillQueueForSubmit(queueId, syncMask);

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

	ClearBarrierState(wasSubmitted);

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

Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> VulkanGpuCommandBuffer::BuildClearValues(RenderSurfaceMask clearMask, const RenderTargetClearValues& targetClearValues)
{
	Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> clearValues{};

	if(clearMask == RT_NONE || mFramebuffer == nullptr)
		return clearValues;

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
	if(mRenderTarget == nullptr)
		return;

	ClearAttachments(area, clearMask, BuildClearValues(clearMask, mRenderTarget->GetClearValues()));
}

void VulkanGpuCommandBuffer::ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues)
{
	if(clearMask == RT_NONE || mFramebuffer == nullptr)
		return;

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();

	Array<VkClearAttachment, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> attachments{};
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
			mResourceTracker.TrackBufferUsage(resource, GpuAccessFlag::Read);
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

			mResourceTracker.TrackBufferUsage(resource, GpuAccessFlag::Read);

			vkCmdBindIndexBuffer(mCommandBufferHandle, vkBuffer, 0, indexType);
		}
	}
}

bool VulkanGpuCommandBuffer::BindGpuParameters(const TShared<GpuPipelineParameterLayout>& pipelineParameterLayout)
{
	B3D_ASSERT(pipelineParameterLayout != nullptr);

	if(!mBoundParamsDirty && IsInRenderPass())
		return true;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(IsInRenderPass())
		mDrawAccessValidator.ClearBindings();
#endif

	mBoundDescriptorSetMask = 0;

	const u32 setCount = pipelineParameterLayout->GetSetCount();

	// Ensure per-set storage is sized
	while(mDynamicOffsetsPerSet.size() < setCount)
		mDynamicOffsetsPerSet.Add(TInlineArray<u32, 4>());

	while(mDynamicOffsetsOverridesPerSet.size() < setCount)
		mDynamicOffsetsOverridesPerSet.Add(UnorderedMap<u32, u32>());

	for(u32 set = 0; set < setCount; set++)
	{
		mDynamicOffsetsPerSet[set].clear();

		// A set the pipeline declares no resources in is not bound, even if one remains bound from an earlier draw
		const TShared<VulkanGpuParameterSet>& boundGpuParameterSet = mBoundGpuParameterSets[set];
		const TShared<GpuPipelineParameterSetLayout>& pipelineSetLayout = pipelineParameterLayout->GetSet(set);
		if(boundGpuParameterSet != nullptr && pipelineSetLayout != nullptr && pipelineSetLayout->GetResourceCount() > 0)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			if(IsInRenderPass())
				mDrawAccessValidator.AddParameterSet(*boundGpuParameterSet, *pipelineParameterLayout->GetSet(set));
#endif

			// Compute accesses are tracked on every dispatch, even if descriptor contents are unchanged.
			TInlineArray<u32, 4> setDynamicOffsets;
			if(!boundGpuParameterSet->PrepareForBind(mResourceTracker, setDynamicOffsets, mDescriptorSetsTemp[set]))
				return false;

			// Apply per-set dynamic offset overrides
			for(const auto& [index, offsetVal] : mDynamicOffsetsOverridesPerSet[set])
			{
				if(index < setDynamicOffsets.size())
					setDynamicOffsets[index] = offsetVal;
			}

#if B3D_BUILD_TYPE_DEVELOPMENT
			// Offset overrides only select other suballocations of the buffers the set tracked
			boundGpuParameterSet->TrackDynamicUniformBufferOffsets(mResourceTracker, setDynamicOffsets);
#endif

			mDynamicOffsetsPerSet[set] = setDynamicOffsets;
			mBoundDescriptorSetMask |= 1ull << set;
		}
	}

	mBoundParamsDirty = false;
	return true;
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

void VulkanGpuCommandBuffer::BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout)
{
	u32 set = 0;
	while(set < kMaximumBoundDescriptorSets)
	{
		if((mBoundDescriptorSetMask & (1ull << set)) == 0)
			set++;
		else
		{
			const u32 firstSet = set;

			TInlineArray<u32, 16> dynamicOffsets;
			while(set < kMaximumBoundDescriptorSets && (mBoundDescriptorSetMask & (1ull << set)) != 0)
			{
				for(u32 offset : mDynamicOffsetsPerSet[set])
					dynamicOffsets.Add(offset);

				set++;
			}

			vkCmdBindDescriptorSets(mCommandBufferHandle, bindPoint, pipelineLayout, firstSet, set - firstSet, &mDescriptorSetsTemp[firstSet], (u32)dynamicOffsets.size(), dynamicOffsets.data());
		}
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
	mResourceTracker.TrackBufferUsage(destination, GpuAccessFlag::Write);

	vkCmdUpdateBuffer(GetVulkanHandle(), destination->GetVulkanHandle(), offset, length, (uint32_t*)data);
}

void VulkanGpuCommandBuffer::CopyBufferToBuffer(VulkanBuffer* source, VulkanBuffer* destination, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset, VkDeviceSize length)
{
	B3D_ENSURE(!IsInRenderPass());

	VkBufferCopy region;
	region.size = length;
	region.srcOffset = sourceOffset;
	region.dstOffset = destinationOffset;

	mResourceTracker.TrackBufferUsage(source, GpuAccessFlag::Read);
	mResourceTracker.TrackBufferUsage(destination, GpuAccessFlag::Write);

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

	mResourceTracker.TrackBufferUsage(source, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destination, subresourceRange, GpuAccessFlag::Write);

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

	mResourceTracker.TrackImageUsage(source, subresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackBufferUsage(destination, GpuAccessFlag::Write);

	vkCmdCopyImageToBuffer(GetVulkanHandle(), source->GetVulkanHandle(), vkLayout, destination->GetVulkanHandle(), 1, &copyRegion);
}

void VulkanGpuCommandBuffer::CopyImageToImage(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageCopy* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRange, GpuAccessFlag::Write);

	vkCmdCopyImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions);
}

void VulkanGpuCommandBuffer::Blit(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageBlit* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRange, GpuAccessFlag::Write);

	vkCmdBlitImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions, VK_FILTER_LINEAR);
}

void VulkanGpuCommandBuffer::Resolve(VulkanImage* source, VulkanImage* destination, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& sourceSubresourceRange, const GpuTextureSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageResolve* regions)
{
	B3D_ENSURE(!IsInRenderPass());

	mResourceTracker.TrackImageUsage(source, sourceSubresourceRange, GpuAccessFlag::Read);
	mResourceTracker.TrackImageUsage(destination, destinationSubresourceRange, GpuAccessFlag::Write);

	vkCmdResolveImage(GetVulkanHandle(), source->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(sourceLayout), destination->GetVulkanHandle(), VulkanUtility::ToVkImageLayout(destinationLayout), regionCount, regions);
}

VulkanFramebuffer* VulkanGpuCommandBuffer::AcquireWindowFramebuffer(RenderWindow& renderWindow)
{
	IVulkanRenderWindowSurface* const surface = static_cast<IVulkanRenderWindowSurface*>(renderWindow.GetRenderWindowSurface().get());
	if(!B3D_ENSURE(surface != nullptr))
		return nullptr;

	if(!surface->IsSwapChainValid())
		renderWindow.RebuildSwapChain();

	VulkanFramebuffer* const framebuffer = surface->GetActiveFramebuffer();
	if(framebuffer == nullptr)
		return nullptr;

	// Track surface (only add if not already tracked)
	auto found = std::find(mAcquiredSurfaces.begin(), mAcquiredSurfaces.end(), surface);
	if(found == mAcquiredSurfaces.end())
		mAcquiredSurfaces.push_back(surface);

	if(VulkanSwapChain* const swapChain = surface->GetSwapChain())
		mResourceTracker.TrackSwapChainUsage(swapChain);

	return framebuffer;
}

VulkanSplitBarrier::VulkanSplitBarrier(VulkanGpuDevice& device, TOptional<GpuQueueType> releaseQueue, TOptional<GpuQueueType> acquireQueue)
	: GpuSplitBarrier(releaseQueue, acquireQueue), mDevice(device.GetLogical())
{
	// Events only connect commands on one queue
	if(IsQueueTransfer())
		return;

	VkEventCreateInfo eventCreateInformation{};
	eventCreateInformation.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	eventCreateInformation.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;

	const VkResult result = vkCreateEvent(mDevice, &eventCreateInformation, gVulkanAllocator, &mEvent);
	B3D_ASSERT(result == VK_SUCCESS);
}

VulkanSplitBarrier::~VulkanSplitBarrier()
{
	// Command buffers that recorded either half keep the split barrier alive until they are done executing
	if(mEvent != VK_NULL_HANDLE)
		vkDestroyEvent(mDevice, mEvent, gVulkanAllocator);
}

bool VulkanGpuCommandBuffer::RecordBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, GpuSplitBarrier* split)
{
	VulkanGpuDevice& device = GetVulkanGpuDevice();

	// Halves on different queues are ordered by the queue wait of the acquire's submission, which also makes the source writes available
	// and visible. They only record queue family ownership transfers and layout transitions.
	const bool transfersQueue = split != nullptr && split->IsQueueTransfer();
	const u32 releaseQueueFamily = transfersQueue ? device.GetQueueFamily(*split->GetReleaseQueueType()) : VK_QUEUE_FAMILY_IGNORED;
	const u32 acquireQueueFamily = transfersQueue ? device.GetQueueFamily(*split->GetAcquireQueueType()) : VK_QUEUE_FAMILY_IGNORED;
	const bool isRelease = phase == GpuBarrierPhase::Release;

	// Halves of an ownership transfer each record their own side of the dependency. Only writes require memory availability, source
	// reads are ordered through the stage mask.
	const auto fnGetTransferScope = [isRelease](const GpuBarrier& barrier, VkPipelineStageFlags& outStages, VkAccessFlags& outAccess)
	{
		if(isRelease)
		{
			const GpuAccessFlags sourceAccess = barrier.Source.Access.IsSet(GpuAccessFlag::Write) ? GpuAccessFlags(GpuAccessFlag::Write) : GpuAccessFlags(GpuAccessFlag::None);
			VulkanUtility::GetPipelineStageAndAccessMask(barrier.Source.Stages, sourceAccess, outStages, outAccess);
		}
		else
			VulkanUtility::GetPipelineStageAndAccessMask(barrier.Destination.Stages, barrier.Destination.Access, outStages, outAccess);
	};

	// Read->read barrier needs to be issued due to stage chaining that Vulkan does. It makes prior writes visible to the new read stages, e.g.:
	// Some stage writes -> Stage A reads -> Stage B reads. The read->read barrier from stage A to stage B makes the write visible to stage B.
	const auto fnIsReadToRead = [](const GpuBarrier& barrier)
	{
		return barrier.Source.Access == GpuAccessFlag::Read && barrier.Destination.Access == GpuAccessFlag::Read;
	};

	VulkanBarrierBatch batch;
	for(const GpuBufferBarrier& barrier : barriers.BufferBarriers)
	{
		auto* const gpuBuffer = static_cast<VulkanGpuBuffer*>(barrier.Object.get());
		VulkanBuffer* const buffer = gpuBuffer != nullptr ? gpuBuffer->GetVulkanResource() : nullptr;
		if(buffer == nullptr)
			continue;

		mResourceTracker.TrackBufferUsage(buffer, barrier.Destination.Access | GpuAccessFlag::Read);

		const GpuBarrierScope scope(barrier.Source.Stages, barrier.Source.Access, barrier.Destination.Stages, barrier.Destination.Access);
		if(!transfersQueue && fnIsReadToRead(barrier))
			batch.AddMemoryBarrier(scope);
		else if(!transfersQueue)
			batch.AddBufferBarrier(buffer->GetVulkanHandle(), scope);
		else if(buffer->IsExclusive() && releaseQueueFamily != acquireQueueFamily)
		{
			VkPipelineStageFlags stages;
			VkAccessFlags access;
			fnGetTransferScope(barrier, stages, access);

			if(isRelease)
				batch.AddBufferBarrier(buffer->GetVulkanHandle(), stages, access, 0, 0, releaseQueueFamily, acquireQueueFamily);
			else
				batch.AddBufferBarrier(buffer->GetVulkanHandle(), 0, 0, stages, access, releaseQueueFamily, acquireQueueFamily);
		}
	}

	const auto fnAddImageBarrier = [&](VulkanImage& image, const GpuTextureSubresourceRange& range, const GpuBarrier& barrier, bool usesGeneralLayout)
	{
		// The alias acquire that supersedes the image flushes its writes through a global memory barrier, which covers images in any layout
		if(barrier.Flags.IsSet(GpuBarrierFlag::AliasRelease))
			return;

		const bool isAliasAcquire = barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire);
		if(isAliasAcquire && !B3D_ENSURE_LOG(barrier.Source.Layout == GpuImageLayout::Undefined && image.IsFullRange(range), "An alias acquire of an image must start from GpuImageLayout::Undefined and cover the whole image."))
			return;

		// Images in the general layout stay in it for shader and transfer accesses, see VulkanTexture::UsesGeneralLayout()
		const auto fnGetNativeLayout = [usesGeneralLayout](GpuImageLayout layout)
		{
			const bool isShaderOrTransferLayout = layout == GpuImageLayout::ShaderReadOnly || layout == GpuImageLayout::TransferSource || layout == GpuImageLayout::TransferDestination ||
				layout == GpuImageLayout::ResolveSource || layout == GpuImageLayout::ResolveDestination;

			return usesGeneralLayout && isShaderOrTransferLayout ? VK_IMAGE_LAYOUT_GENERAL : VulkanUtility::ToVkImageLayout(layout);
		};

		const VkImageLayout sourceLayout = fnGetNativeLayout(barrier.Source.Layout);
		const VkImageLayout destinationLayout = fnGetNativeLayout(barrier.Destination.Layout);

		// Layout transitions write the image
		GpuAccessFlags trackedAccess = barrier.Destination.Access | GpuAccessFlag::Read;
		if(sourceLayout != destinationLayout)
			trackedAccess |= GpuAccessFlag::Write;

		mResourceTracker.TrackImageUsage(&image, range, trackedAccess);

		const VkImageSubresourceRange vkRange = VulkanUtility::ToVkImageSubresourceRange(range);
		if(!transfersQueue)
		{
			// The image barrier's memory dependency covers only this image, so an alias acquire adds a global memory barrier for the
			// resources that used the memory before it
			const GpuBarrierScope scope(barrier.Source.Stages, barrier.Source.Access, barrier.Destination.Stages, barrier.Destination.Access);
			if(isAliasAcquire && barrier.Source.Access.IsSet(GpuAccessFlag::Write))
				batch.AddMemoryBarrier(scope);

			// A layout transition would carry the visibility itself
			if(sourceLayout == destinationLayout && fnIsReadToRead(barrier))
				batch.AddMemoryBarrier(scope);

			batch.AddImageBarrier(image.GetVulkanHandle(), vkRange, scope, sourceLayout, destinationLayout);
		}
		else if(image.IsExclusive() && releaseQueueFamily != acquireQueueFamily)
		{
			// Both halves of an ownership transfer specify the layout transition, and it executes once
			VkPipelineStageFlags stages;
			VkAccessFlags access;
			fnGetTransferScope(barrier, stages, access);

			if(isRelease)
				batch.AddImageBarrier(image.GetVulkanHandle(), vkRange, stages, access, 0, 0, sourceLayout, destinationLayout, releaseQueueFamily, acquireQueueFamily);
			else
				batch.AddImageBarrier(image.GetVulkanHandle(), vkRange, 0, 0, stages, access, sourceLayout, destinationLayout, releaseQueueFamily, acquireQueueFamily);
		}
		else if(isRelease && sourceLayout != destinationLayout)
		{
			VkPipelineStageFlags stages;
			VkAccessFlags access;
			fnGetTransferScope(barrier, stages, access);

			batch.AddImageBarrier(image.GetVulkanHandle(), vkRange, stages, access, 0, 0, sourceLayout, destinationLayout);
		}

		// The release leaves the transition in flight until the acquire
		if(!isRelease)
			AddImageLayout(image, range, (u32)destinationLayout);
	};

	for(const GpuTextureBarrier& barrier : barriers.TextureBarriers)
	{
		auto* const texture = static_cast<VulkanTexture*>(barrier.Object.get());
		VulkanImage* const image = texture != nullptr ? texture->GetVulkanResource() : nullptr;
		if(image == nullptr)
			continue;

		fnAddImageBarrier(*image, GpuBackendUtility::ClampRange(barrier.SubresourceRange, image->GetRange()), barrier, texture->UsesGeneralLayout());
	}

	for(const GpuRenderTargetBarrier& barrier : barriers.RenderTargetBarriers)
	{
		RenderTarget* const renderTarget = barrier.Object.get();
		if(renderTarget == nullptr)
			continue;

		// The barrier covers the surface's faces and mip levels, and the aspects it selects
		if(renderTarget->GetProperties().IsWindow)
		{
			// Barriers can precede the render pass that would otherwise acquire the swap chain image
			VulkanFramebuffer* const framebuffer = AcquireWindowFramebuffer(static_cast<RenderWindow&>(*renderTarget));
			const GpuFramebufferAttachment* const attachment = framebuffer != nullptr ? framebuffer->FindAttachment(barrier.SurfaceMask) : nullptr;
			if(attachment == nullptr)
				continue;

			VulkanImage& image = static_cast<VulkanImage&>(*attachment->Image);
			GpuTextureSubresourceRange range = image.GetRange();
			range.AspectMask &= barrier.SubresourceRange.AspectMask;

			fnAddImageBarrier(image, range, barrier, false);
			continue;
		}

		const RenderTexture& renderTexture = static_cast<const RenderTexture&>(*renderTarget);
		const bool isColor = ((u32)barrier.SurfaceMask & (u32)RT_COLOR_ALL) != 0;
		const RenderSurfaceInformation& surfaceInformation = isColor ? renderTexture.GetColorSurfaceInformation(Bitwise::LeastSignificantBit((u32)barrier.SurfaceMask)) : renderTexture.GetDepthStencilSurfaceInformation();

		auto* const texture = static_cast<VulkanTexture*>(surfaceInformation.Texture.get());
		VulkanImage* const image = texture != nullptr ? texture->GetVulkanResource() : nullptr;
		if(image == nullptr)
			continue;

		GpuTextureSubresourceRange range = GpuBackendUtility::GetSurfaceRange(image->GetRange(), TextureSurface(surfaceInformation.MipLevel, 1, surfaceInformation.Face, surfaceInformation.FaceCount));
		range.AspectMask &= barrier.SubresourceRange.AspectMask;

		fnAddImageBarrier(*image, range, barrier, texture->UsesGeneralLayout());
	}

	// Halves on one queue are connected by the event, which performs the transitions in the release. Halves on different queues record
	// their own part of the ownership transfer.
	const VkEvent event = split != nullptr ? static_cast<VulkanSplitBarrier*>(split)->GetEvent() : VK_NULL_HANDLE;
	if(event != VK_NULL_HANDLE)
	{
		if(isRelease)
			batch.SetEvent(mCommandBufferHandle, event);
		else
			batch.WaitEvent(mCommandBufferHandle, event);
	}
	else if(transfersQueue || !isRelease)
		batch.Execute(mCommandBufferHandle);

	return true;
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
