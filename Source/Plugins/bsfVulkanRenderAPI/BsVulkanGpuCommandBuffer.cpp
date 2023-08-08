//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanGpuDevice.h"
#include "BsVulkanGpuParameters.h"
#include "BsVulkanGpuQueue.h"
#include "BsVulkanTexture.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanFramebuffer.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanEventQuery.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanSwapChain.h"
#include "BsVulkanTimerQuery.h"
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanRenderPass.h"
#include "BsVulkanRenderTexture.h"
#include "BsVulkanGpuBackend.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
#	include "Win32/BsWin32RenderWindow.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
#	include "Linux/BsLinuxRenderWindow.h"
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	include "MacOS/BsMacOSRenderWindow.h"
#else
static_assert(false, "Other platforms go here");
#endif

using namespace bs;
using namespace bs::ct;

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
	const u32 queueFamily = device.GetQueueFamily(createInformation.Usage);

	if (!B3D_ENSURE(queueFamily != ~0u))
		return;

	VkCommandPoolCreateInfo vulkanPoolCreateInformation;
	vulkanPoolCreateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vulkanPoolCreateInformation.pNext = nullptr;
	vulkanPoolCreateInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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

	bool areAnyCommandBuffersStillExecuting = false;
	for(const auto& commandBufferPair : mCommandBuffers)
	{
		if(commandBufferPair.second->GetState() != CommandBufferState::Ready)
		{
			areAnyCommandBuffersStillExecuting = true;
			break;
		}
	}

	if(areAnyCommandBuffersStillExecuting)
		GetVulkanSubmitThread().WaitUntilIdle();

	mMessageQueue.PostRequestShutdownCommand(true);

	mCommandBuffers.clear();
	vkDestroyCommandPool(static_cast<VulkanGpuDevice&>(mGpuDevice).GetLogical(), mVulkanPool, gVulkanAllocator);

	Base::Destroy();
}

SPtr<GpuCommandBuffer> VulkanGpuCommandBufferPool::FindOrCreate(const GpuCommandBufferCreateInformation& createInformation)
{
	EnsureValidThread();

	for(const auto& commandBufferPair : mCommandBuffers)
	{
		if (commandBufferPair.second->GetState() != CommandBufferState::Ready)
			continue;

		commandBufferPair.second->SetName(createInformation.Name);
		commandBufferPair.second->Begin();

		return commandBufferPair.second;
	}

	return Create(createInformation);
}

SPtr<GpuCommandBuffer> VulkanGpuCommandBufferPool::Create(const GpuCommandBufferCreateInformation& createInformation)
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

	SPtr<VulkanGpuCommandBuffer> commandBuffer = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuCommandBuffer>()) VulkanGpuCommandBuffer(static_cast<VulkanGpuDevice&>(mGpuDevice), *this, mNextCommandBufferId++, commandBufferHandle, mInformation.Thread, mInformation.Usage, createInformation),
		[this](VulkanGpuCommandBuffer* commandBuffer)
		{
			VkCommandBuffer commandBufferHandle = commandBuffer->GetVulkanHandle();
			vkFreeCommandBuffers(static_cast<VulkanGpuDevice&>(mGpuDevice).GetLogical(), mVulkanPool, 1, &commandBufferHandle);

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

	// TODO - Not implemented
	B3D_ASSERT(false && "Not implemented");
}

template <class T>
void GetPipelineStageFlags(const Vector<T>& barriers, VkPipelineStageFlags& src, VkPipelineStageFlags& dst)
{
	for(auto& entry : barriers)
	{
		src |= VulkanGpuCommandBuffer::GetPipelineStageFlags(entry.srcAccessMask);
		dst |= VulkanGpuCommandBuffer::GetPipelineStageFlags(entry.dstAccessMask);
	}

	if(src == 0)
		src = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if(dst == 0)
		dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

const Color kDebugLabelColor = Color::kBansheeOrange;
constexpr u32 kMaximumBoundDescriptorSets = 64;

VulkanGpuCommandBuffer::VulkanGpuCommandBuffer(VulkanGpuDevice& device, VulkanGpuCommandBufferPool& pool, u32 id, VkCommandBuffer commandBufferHandle, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation)
	: GpuCommandBuffer(device, ownerThread, queueType, createInformation), mId(id), mCommandBufferHandle(commandBufferHandle), mPool(pool), mOwnerThread(ownerThread), mNeedsWARMemoryBarrier(false), mNeedsRAWMemoryBarrier(false), mGfxPipelineRequiresBind(true), mCmpPipelineRequiresBind(true), mViewportRequiresBind(true), mStencilRefRequiresBind(true), mScissorRequiresBind(true), mBoundParamsDirty(false), mVertexInputsDirty(false)
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

	if(mState == State::Submitted || mState == State::Done)
	{
		// Wait 1s
		u64 waitTime = 1000 * 1000 * 1000;
		VkResult result = vkWaitForFences(device, 1, &mFence, true, waitTime);
		B3D_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT);

		if(result == VK_TIMEOUT)
			B3D_LOG(Warning, RenderBackend, "Freeing a command buffer before done executing because fence wait expired!");

		// Resources have been marked as used, make sure to notify them we're done with them
		Reset();
	}
	else if(mState != State::Ready)
	{
		// Notify any resources that they are no longer bound
		for(auto& entry : mResources)
		{
			ResourceUseHandle& useHandle = entry.second;
			B3D_ASSERT(!useHandle.Used);

			entry.first->NotifyUnbound();
		}

		for(auto& entry : mImages)
		{
			u32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ResourceUseHandle& useHandle = imageInfo.UseHandle;
			B3D_ASSERT(!useHandle.Used);

			entry.first->NotifyUnbound();
		}

		for(auto& entry : mBuffers)
		{
			ResourceUseHandle& useHandle = entry.second.UseHandle;
			B3D_ASSERT(!useHandle.Used);

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

	if(mIntraQueueSemaphore != nullptr)
		mIntraQueueSemaphore->Destroy();

	for(u32 i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; i++)
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
	B3D_ASSERT(mState == State::Ready);

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(mCommandBufferHandle, &beginInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	mState = State::Recording;
}

void VulkanGpuCommandBuffer::End()
{
	EnsureValidThread();
	B3D_ASSERT(mState == State::Recording);

	// If a clear is queued, execute the render pass with no additional instructions
	if(mClearMask)
		ExecuteClearPass();

	if(mIsDebugLabelOpen)
		EndLabel();

	VkResult result = vkEndCommandBuffer(mCommandBufferHandle);
	B3D_ASSERT(result == VK_SUCCESS);

	mRenderTarget = nullptr;
	mState = State::RecordingDone;
}

void VulkanGpuCommandBuffer::SetRenderTarget(const SPtr<RenderTarget>& renderTarget, u32 readOnlyFlags, RenderSurfaceMask loadMask)
{
	EnsureValidThread();
	B3D_ASSERT(mState != State::Submitted);

	VulkanFramebuffer* newFramebuffer;
	VulkanSwapChain* swapChain = nullptr;
	if(renderTarget != nullptr)
	{
		if(renderTarget->GetProperties().IsWindow)
		{
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
			Win32RenderWindow* window = static_cast<Win32RenderWindow*>(renderTarget.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
			LinuxRenderWindow* window = static_cast<LinuxRenderWindow*>(rt.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
			MacOSRenderWindow* window = static_cast<MacOSRenderWindow*>(rt.get());
#endif

			swapChain = window->GetSwapChain();

			if(!swapChain->IsValid())
			{
				window->RebuildSwapChain();
				swapChain = window->GetSwapChain();
			}

			u32 acquiredImageIndex;
			bool isImageAcquired = swapChain->TryGetFirstAcquiredImageIndex(acquiredImageIndex);

			// It's possible this is a fresh swap chain we haven't acquired any images for yet
			if(!isImageAcquired)
			{
				const u32 maximumColorImageCount = swapChain->GetColorImageCount();
				const u32 acquireableColorImageCount = maximumColorImageCount > 0 ? maximumColorImageCount - 1 : 0; // One is reserved for OS compositor
				for(u32 imageIndex = 0; imageIndex < acquireableColorImageCount; imageIndex++)
					GetVulkanSubmitThread().QueueImageAcquire(*swapChain);

				swapChain->WaitUntilFirstImageAcquired();
				isImageAcquired = swapChain->TryGetFirstAcquiredImageIndex(acquiredImageIndex);
			}

			if(isImageAcquired)
			{
				SwapChainImageInformation swapChainImageInformation;
				swapChainImageInformation.SwapChain = swapChain;
				swapChainImageInformation.ImageIndex = acquiredImageIndex;

				const auto found = std::find_if(mAcquiredSwapChainImages.begin(), mAcquiredSwapChainImages.end(), [swapChain](const SwapChainImageInformation& info) { return info.SwapChain == swapChain; });
				if(found == mAcquiredSwapChainImages.end())
					mAcquiredSwapChainImages.push_back(swapChainImageInformation);

				newFramebuffer = swapChain->GetFramebufferForImage(swapChainImageInformation.ImageIndex);
			}
			else
			{
				B3D_LOG(Error, RenderBackend, "Binding render target failed. Unable to acquire swap chain image.");

				swapChain = nullptr;
				newFramebuffer = nullptr;
			}
		}
		else
		{
			const VulkanRenderTexture* const renderTexture = static_cast<VulkanRenderTexture*>(renderTarget.get());
			newFramebuffer = renderTexture->GetFramebuffer();
		}
	}
	else
	{
		newFramebuffer = nullptr;
	}

	mRenderTarget = renderTarget;
	mRenderTargetModified = false;
	mIsRenderPassInterrupted = false;

	// Warn if invalid load mask
	if(loadMask.IsSet(RT_DEPTH) && !loadMask.IsSet(RT_STENCIL))
	{
		B3D_LOG(Warning, RenderBackend, "setRenderTarget() invalid load mask, depth enabled but stencil disabled. "
									   "This is not supported. Both will be loaded.");

		loadMask.Set(RT_STENCIL);
	}

	if(!loadMask.IsSet(RT_DEPTH) && loadMask.IsSet(RT_STENCIL))
	{
		B3D_LOG(Warning, RenderBackend, "setRenderTarget() invalid load mask, stencil enabled but depth disabled. "
									   "This is not supported. Both will be loaded.");

		loadMask.Set(RT_DEPTH);
	}

	if(mFramebuffer == newFramebuffer && mRenderTargetReadOnlyFlags == readOnlyFlags && mRenderTargetLoadMask == loadMask)
		return;

	if(IsInRenderPass())
		EndRenderPass();
	else
	{
		// If a clear is queued for previous FB, execute the render pass with no additional instructions
		if(mClearMask)
			ExecuteClearPass();
	}

	// Reset isFBAttachment flags for subresources from the old framebuffer
	if(mFramebuffer != nullptr)
	{
		VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
		u32 numColorAttachments = renderPass->GetColorAttachmentCount();
		for(u32 i = 0; i < numColorAttachments; i++)
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetColorAttachment(i);
			u32 imageInfoIdx = mImages[fbAttachment.Image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
			for(u32 j = 0; j < imageInfo.NumSubresourceInfos; j++)
			{
				ImageSubresourceInfo& entry = subresourceInfos[j];
				entry.UseFlags.Unset(ImageUseFlagBits::Framebuffer);
				entry.FbUse.Access = VulkanAccessFlag::None;
				entry.FbUse.Stages = 0;
			}
		}

		if(renderPass->HasDepthAttachment())
		{
			const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetDepthStencilAttachment();
			u32 imageInfoIdx = mImages[fbAttachment.Image];
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
			for(u32 j = 0; j < imageInfo.NumSubresourceInfos; j++)
			{
				ImageSubresourceInfo& entry = subresourceInfos[j];
				entry.UseFlags.Unset(ImageUseFlagBits::Framebuffer);
				entry.FbUse.Access = VulkanAccessFlag::None;
				entry.FbUse.Stages = 0;
			}
		}
	}

	if(newFramebuffer == nullptr)
	{
		mFramebuffer = nullptr;
		mRenderTargetReadOnlyFlags = 0;
		mRenderTargetLoadMask = RT_NONE;
	}
	else
	{
		mFramebuffer = newFramebuffer;
		mRenderTargetReadOnlyFlags = readOnlyFlags;
		mRenderTargetLoadMask = loadMask;
	}

	// Re-set the params as they will need to be re-bound
	SetGpuParameters(mBoundParams);

	if(mFramebuffer)
	{
		RegisterResource(mFramebuffer, loadMask, readOnlyFlags);

		if(swapChain)
			RegisterResource(swapChain);
	}

	mGfxPipelineRequiresBind = true;

	// Potentially need to rebind vertex buffers as we bind dummy vertex buffers for shaders attributes not provided by the user
	mVertexInputsDirty = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void VulkanGpuCommandBuffer::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	EnsureValidThread();

	Rect2I area(0, 0, mFramebuffer->GetWidth(), mFramebuffer->GetHeight());
	ClearViewport(area, buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	EnsureValidThread();

	const Rect2I viewportArea = GetViewportArea();
	ClearViewport(viewportArea, buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanGpuCommandBuffer::SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& state)
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

void VulkanGpuCommandBuffer::SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& state)
{
	EnsureValidThread();

	if(mComputePipeline == state)
		return;

	mComputePipeline = std::static_pointer_cast<VulkanGpuComputePipelineState>(state);
	mCmpPipelineRequiresBind = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanGpuCommandBuffer::SetGpuParameters(const SPtr<GpuParameters>& parameters)
{
	EnsureValidThread();

	// Note: We keep an internal reference to GPU params even though we shouldn't keep a reference to a core thread
	// object. But it should be fine since we expect the resource to be externally synchronized so it should never
	// be allowed to go out of scope on a non-core thread anyway.
	mBoundParams = std::static_pointer_cast<VulkanGpuParameters>(parameters);

	if (mBoundParams != nullptr)
	{
		const SPtr<GpuPipelineParameterLayout>& uniformLayout = mBoundParams->GetPipelineParameterInformation();

		// Flush all uniform buffers
		const u32 uniformBufferCount = uniformLayout->GetBindingCount(GpuParameterType::UniformBuffer);
		for (u32 uniformBufferIndex = 0; uniformBufferIndex < uniformBufferCount; uniformBufferIndex++)
		{
			const GpuParameterBinding& binding = uniformLayout->GetBinding(GpuParameterType::UniformBuffer, uniformBufferIndex);
			SPtr<GpuBuffer> buffer = parameters->GetUniformBuffer(binding.Set, binding.Slot);

			if (buffer != nullptr)
				buffer->FlushCache();
		}
	}

	if(mBoundParams != nullptr)
	{
		mBoundParamsDirty = true;
	}
	else
	{
		mBoundDescriptorSetCount = 0;
		mBoundParamsDirty = false;
	}

	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;
	mDynamicDescriptorOffsetsOverrides.clear();

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void VulkanGpuCommandBuffer::SetDynamicBufferOffset(u32 bufferIndex, u32 offset)
{
	EnsureValidThread();

	mDynamicDescriptorOffsetsOverrides[bufferIndex] = offset;
	mDescriptorSetsBindState = DescriptorSetBindFlag::Graphics | DescriptorSetBindFlag::Compute;

	// If GPU params were bound already, we retrieved the initial set of offsets, so just override it
	if(!mBoundParamsDirty)
	{
		if(!B3D_ENSURE(bufferIndex < (u32)mDynamicDescriptorOffsetsToBind.size()))
			return;

		mDynamicDescriptorOffsetsToBind[bufferIndex] = offset;
	}
}

void VulkanGpuCommandBuffer::SetViewport(const Rect2& area)
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

	const Rect2I area(left, top, right - left, bottom - top);

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

void VulkanGpuCommandBuffer::SetVertexBuffers(u32 startIndex, SPtr<GpuBuffer>* buffers, u32 bufferCount)
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

void VulkanGpuCommandBuffer::SetIndexBuffer(const SPtr<GpuBuffer>& buffer)
{
	EnsureValidThread();

	if(mIndexBuffer == buffer)
		return;

	mIndexBuffer = std::static_pointer_cast<VulkanGpuBuffer>(buffer);
	mVertexInputsDirty = true;

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void VulkanGpuCommandBuffer::SetVertexDescription(const SPtr<VertexDescription>& vertexDescription)
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

	if(!IsReadyForRender())
		return;

	// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
	BindGpuParams();

	if(!IsInRenderPass())
		BeginRenderPass();

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

			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mDynamicDescriptorOffsetsToBind.size(), mDynamicDescriptorOffsetsToBind.data());
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

	if(indexCount == 0)
		return;

	if(!IsReadyForRender())
		return;

	// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
	BindGpuParams();

	if(!IsInRenderPass())
		BeginRenderPass();

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

			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mDynamicDescriptorOffsetsToBind.size(), mDynamicDescriptorOffsetsToBind.data());
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
		B3D_LOG(Warning, RenderBackend, "Ignoring call to DispatchCompute(). Thread count is zero.");
	}

	if(IsInRenderPass())
		EndRenderPass();

	// Note: Should I restore the render target after? Note that this is only being done is framebuffer subresources
	// have their "isFBAttachment" flag reset, potentially I can just clear/restore those
	SetRenderTarget(nullptr, 0, RT_ALL);

	// Need to bind gpu params before starting render pass, in order to make sure any layout transitions execute
	BindGpuParams();
	ExecuteWriteHazardBarrier();
	ExecuteLayoutTransitions();

	if(mCmpPipelineRequiresBind)
	{
		VulkanPipeline* pipeline = mComputePipeline->GetVulkanResource();
		if(pipeline == nullptr)
			return;

		RegisterResource(pipeline, VulkanAccessFlag::Read);
		mComputePipeline->RegisterPipelineResources(*this);

		vkCmdBindPipeline(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetVulkanHandle());
		mCmpPipelineRequiresBind = false;
	}

	if(mDescriptorSetsBindState.IsSet(DescriptorSetBindFlag::Compute))
	{
		if(mBoundDescriptorSetCount > 0)
		{
			VkPipelineLayout pipelineLayout = mComputePipeline->GetPipelineLayoutHandle();
			vkCmdBindDescriptorSets(mCommandBufferHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, mBoundDescriptorSetCount, mDescriptorSetsTemp, (u32)mDynamicDescriptorOffsetsToBind.size(), mDynamicDescriptorOffsetsToBind.data());
		}

		mDescriptorSetsBindState.Unset(DescriptorSetBindFlag::Compute);
	}

	vkCmdDispatch(mCommandBufferHandle, groupCountX, groupCountY, groupCountZ);

	// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
	// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
	// images remain to be used in subsequent calls).
	for(auto& entry : mShaderBoundSubresourceInfos)
	{
		ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
		subresourceInfo.UseFlags.Unset(ImageUseFlagBits::Shader);
		subresourceInfo.ShaderUse.Access = VulkanAccessFlag::None;
		subresourceInfo.ShaderUse.Stages = 0;
	}

	mShaderBoundSubresourceInfos.clear();

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
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
	B3D_ASSERT(mState == State::Recording);

	if(mFramebuffer == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Attempting to begin a render pass but no render target is bound to the command buffer.");
		return;
	}

	const Rect2I renderArea = GetRenderPassArea();

	const RenderSurfaceMask readMask = GetFramebufferReadMask();
	RenderSurfaceMask loadMask = mRenderTargetLoadMask;
	const RenderSurfaceMask originalClearMask = mClearMask;

	// Check if an explicit clear is needed. This is done if the the clear area doesn't match the render area, or if the user requests both
	// clear and load flags for the same attachments (we can only do either during start of a render pass).
	bool isClearCommandRequired = false;
	if(mClearMask != RT_NONE)
	{
		if(mClearArea != renderArea || loadMask.IsSetAny(originalClearMask))
		{
			isClearCommandRequired = true;
			mClearMask = RT_NONE;
		}
	}

	ExecuteWriteHazardBarrier();
	ExecuteLayoutTransitions();

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	if(mIsRenderPassInterrupted)
	{
		loadMask = RT_ALL;
		mIsRenderPassInterrupted = false;
	}

	RenderSurfaceMask clearMask = mClearMask;
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
			B3D_LOG(Error, RenderBackend, "Color attachment at index {0} cannot be read only if we're not loading it.", colorAttachment.Index);
			continue;
		}

		// In debug mode clear not loaded values to the clear color
		if(!originalClearMask.IsSet(colorAttachmentBit))
		{
			clearMask |= colorAttachmentBit;
			mClearValues[colorAttachment.Index].color = kDebugClearColor;
		}
	}

	if(renderPass->HasDepthAttachment())
	{
		if(!loadMask.IsSet(RT_DEPTH))
		{
			if(readMask.IsSet(RT_DEPTH))
			{
				B3D_LOG(Error, RenderBackend, "Depth attachment cannot be read only if we're not loading it.");
			}
			else
			{
				if(!originalClearMask.IsSet(RT_DEPTH))
				{
					clearMask |= RT_DEPTH;
					mClearValues[colorAttachmentCount].depthStencil.depth = 0.0f;
				}
			}
		}

		if(!loadMask.IsSet(RT_STENCIL))
		{
			if(readMask.IsSet(RT_STENCIL))
			{
				B3D_LOG(Error, RenderBackend, "Stencil attachment cannot be read only if we're not loading it.");
			}
			else
			{
				if(!originalClearMask.IsSet(RT_STENCIL))
				{
					clearMask |= RT_STENCIL;
					mClearValues[colorAttachmentCount].depthStencil.stencil = 0;
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
	renderPassBeginInfo.pClearValues = mClearValues.data();

	vkCmdBeginRenderPass(mCommandBufferHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	mClearMask = RT_NONE;
	mState = State::RecordingRenderPass;

	if(isClearCommandRequired)
	{
		ExecuteClearCommand(mClearArea, originalClearMask, mClearValues);
	}
}

void VulkanGpuCommandBuffer::EndRenderPass(bool isInternalInterrupt)
{
	B3D_ASSERT(mState == State::RecordingRenderPass);

	vkCmdEndRenderPass(mCommandBufferHandle);

	// Execute any queued events
	for(auto& entry : mQueuedEvents)
		vkCmdSetEvent(mCommandBufferHandle, entry->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	mQueuedEvents.clear();

	// Remove any shader use flags on images. Note this relies on the fact that we re-bind all parameters on every
	// dispatch call and render pass, so they can reset this flags. Otherwise clearing the flags is wrong if the
	// images remain to be used in subsequent calls).
	for(auto& entry : mShaderBoundSubresourceInfos)
	{
		ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
		subresourceInfo.UseFlags.Unset(ImageUseFlagBits::Shader);
		subresourceInfo.ShaderUse.Access = VulkanAccessFlag::None;
		subresourceInfo.ShaderUse.Stages = 0;
	}

	mShaderBoundSubresourceInfos.clear();

	UpdateFinalLayouts();

	mState = State::Recording;
	mIsRenderPassInterrupted = isInternalInterrupt;

	// In case the same GPU params from last pass get used, this makes sure the states we reset above, get re-applied
	mBoundParamsDirty = true;
}

u32 VulkanGpuCommandBuffer::AllocateSignalSemaphores(SmallVector<VkSemaphore, 8>& outSemaphores)
{
	// TODO - Do I need multiple semaphores? Can't I just have one?

	u32 count = 0;

	if(mIntraQueueSemaphore != nullptr)
		mIntraQueueSemaphore->Destroy();

	mIntraQueueSemaphore = GetVulkanGpuDevice().GetResourceManager().Create<VulkanSemaphore>("IntraQueue");

	outSemaphores.Add(mIntraQueueSemaphore->GetHandle());
	count++;

	for(u32 i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; i++)
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
	if(mNumUsedInterQueueSemaphores >= BS_MAX_VULKAN_CB_DEPENDENCIES)
		return nullptr;

	return mInterQueueSemaphores[mNumUsedInterQueueSemaphores++];
}

GpuCommandBufferSubmitInformation VulkanGpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueUsage queueUsage, u32 queueIndex)
{
	AssertIfNotVulkanSubmitThread();
	B3D_ASSERT(IsSubmitted()); // Caller should already have set this flag

	GpuCommandBufferSubmitInformation submitInformation;
	VulkanGpuCommandBufferPool& commandBufferPool = GetVulkanSubmitThread().GetCommandBufferPool(queueUsage);

	// If there are any query resets needed, execute those first
	if(!mQueuedQueryResets.empty())
	{
		const SPtr<VulkanGpuCommandBuffer> queryResetCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Query reset")));

		const VkCommandBuffer vkCommandBuffer = queryResetCommandBuffer->GetVulkanHandle();

		for(auto& entry : mQueuedQueryResets)
			entry->Reset(vkCommandBuffer);

		queryResetCommandBuffer->End();
		submitInformation.QueryResetCommandBuffer = queryResetCommandBuffer;

		mQueuedQueryResets.clear();
	}

	// Issue pipeline barriers for queue transitions (need to happen on original queue first, then on new queue)
	for(auto& entry : mBuffers)
	{
		VulkanBuffer* resource = static_cast<VulkanBuffer*>(entry.first);

		if(!resource->IsExclusive())
			continue;

		const GpuQueueUsage oldQueueUsage = resource->GetQueueUsage();
		if(oldQueueUsage != GQT_UNKNOWN && oldQueueUsage != queueUsage)
		{
			Vector<VkBufferMemoryBarrier>& barriers = mTransitionInfoTemp[(i32)oldQueueUsage].BufferBarriers;

			barriers.push_back(VkBufferMemoryBarrier());
			VkBufferMemoryBarrier& barrier = barriers.back();
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;
			barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueUsage);
			barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueUsage);
			barrier.buffer = resource->GetVulkanHandle();
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
		}
	}

	// For images issue queue transitions, as above. Also issue layout transitions to their inital layouts.
	Vector<VkImageMemoryBarrier>& localBarriers = mTransitionInfoTemp[(i32)queueUsage].ImageBarriers;
	for(auto& entry : mImages)
	{
		VulkanImage* resource = static_cast<VulkanImage*>(entry.first);
		ImageInfo& imageInfo = mImageInfos[entry.second];

		const GpuQueueUsage oldQueueUsage = resource->GetQueueUsage();
		bool queueMismatch = resource->IsExclusive() && oldQueueUsage != GQT_UNKNOWN && oldQueueUsage != queueUsage;

		ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
		if(queueMismatch)
		{
			Vector<VkImageMemoryBarrier>& barriers = mTransitionInfoTemp[(i32)oldQueueUsage].ImageBarriers;

			for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
			{
				ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

				u32 startIdx = (u32)barriers.size();
				resource->GetBarriers(subresourceInfo.Range, barriers);

				for(u32 j = startIdx; j < (u32)barriers.size(); j++)
				{
					VkImageMemoryBarrier& barrier = barriers[j];

					barrier.dstAccessMask = 0;
					barrier.newLayout = barrier.oldLayout;
					barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueUsage);
					barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueUsage);
				}
			}
		}

		for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
		{
			ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

			const VkImageSubresourceRange& range = subresourceInfo.Range;
			u32 mipEnd = range.baseMipLevel + range.levelCount;
			u32 faceEnd = range.baseArrayLayer + range.layerCount;

			bool layoutMismatch = false;
			VkImageLayout initialLayout = subresourceInfo.InitialLayout;
			if(initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			{
				for(u32 mip = range.baseMipLevel; mip < mipEnd; mip++)
				{
					for(u32 face = range.baseArrayLayer; face < faceEnd; face++)
					{
						VulkanImageSubresource* subresource = resource->GetSubresource(face, mip);
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
				resource->GetBarriers(subresourceInfo.Range, localBarriers);

				for(u32 j = startIdx; j < (u32)localBarriers.size(); j++)
				{
					VkImageMemoryBarrier& barrier = localBarriers[j];

					barrier.dstAccessMask = resource->GetAccessFlags(initialLayout, subresourceInfo.InitialReadOnly);
					barrier.newLayout = layoutMismatch ? initialLayout : barrier.oldLayout;

					if(queueMismatch)
					{
						barrier.srcAccessMask = 0;
						barrier.srcQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(oldQueueUsage);
						barrier.dstQueueFamilyIndex = GetVulkanGpuDevice().GetQueueFamily(queueUsage);
					}
				}
			}

			for(u32 mip = range.baseMipLevel; mip < mipEnd; mip++)
			{
				for(u32 face = range.baseArrayLayer; face < faceEnd; face++)
				{
					VulkanImageSubresource* subresource = resource->GetSubresource(face, mip);
					subresource->SetLayout(subresourceInfo.CurrentLayout);
				}
			}
		}
	}

	B3D_ASSERT(B3DSize(mTransitionInfoTemp) == GQT_COUNT);
	for(u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; queueUsageIndex++)
	{
		const GpuQueueUsage transitionQueueUsage = (GpuQueueUsage)queueUsageIndex;
		TransitionInfo& transitionInformation = mTransitionInfoTemp[queueUsageIndex];

		bool empty = transitionInformation.ImageBarriers.empty() && transitionInformation.BufferBarriers.empty();
		if(empty)
			continue;

		// No queue transition needed for entries on this queue (this entry is most likely an image layout transition)
		if(transitionQueueUsage == GQT_UNKNOWN || transitionQueueUsage == queueUsage)
			continue;

		const SPtr<VulkanGpuCommandBuffer> sourceTransitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Source queue transition")));
		VkCommandBuffer vkCmdBuffer = sourceTransitionCommandBuffer->GetVulkanHandle();

		const u32 imageBarrierCount = (u32)transitionInformation.ImageBarriers.size();
		const u32 bufferBarrierCount = (u32)transitionInformation.BufferBarriers.size();

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		::GetPipelineStageFlags(transitionInformation.ImageBarriers, srcStage, dstStage);

		vkCmdPipelineBarrier(vkCmdBuffer, srcStage, dstStage, 0, 0, nullptr, bufferBarrierCount, transitionInformation.BufferBarriers.data(), imageBarrierCount, transitionInformation.ImageBarriers.data());

		sourceTransitionCommandBuffer->End();

		// Note: If I switch back to doing layout transitions here, I need to wait on present semaphore for this command buffer
		submitInformation.SourceQueueTransitionCommandBuffer[transitionQueueUsage] = sourceTransitionCommandBuffer;
	}

	// Wait on present (i.e. until the back buffer becomes available) for any swap chains
	for(auto& entry : mAcquiredSwapChainImages)
	{
		entry.SwapChain->AppendWaitSemaphoreIfRequired(entry.ImageIndex, submitInformation.Semaphores);
	}

	// Issue second part of transition pipeline barriers (on this queue)
	for(u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; queueUsageIndex++)
	{
		const GpuQueueUsage transitionQueueUsage = (GpuQueueUsage)queueUsageIndex;
		TransitionInfo& transitionInformation = mTransitionInfoTemp[queueUsageIndex];

		bool empty = transitionInformation.ImageBarriers.size() == 0 && transitionInformation.BufferBarriers.size() == 0;
		if(empty)
			continue;

		if(transitionQueueUsage != GQT_UNKNOWN && transitionQueueUsage != queueUsage)
			continue;

		SPtr<VulkanGpuCommandBuffer> transitionCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create("Queue and layout transitions")));

		VkCommandBuffer vkCmdBuffer = transitionCommandBuffer->GetVulkanHandle();

		u32 numImgBarriers = (u32)transitionInformation.ImageBarriers.size();
		u32 numBufferBarriers = (u32)transitionInformation.BufferBarriers.size();

		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		::GetPipelineStageFlags(transitionInformation.ImageBarriers, srcStage, dstStage);

		vkCmdPipelineBarrier(vkCmdBuffer, srcStage, dstStage, 0, 0, nullptr, numBufferBarriers, transitionInformation.BufferBarriers.data(), numImgBarriers, transitionInformation.ImageBarriers.data());

		transitionCommandBuffer->End();

		submitInformation.DestinationQueueTransitionCommandBuffer = transitionCommandBuffer;
	}

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(GetShared());

	mSubmittedQueueGlobalIndex = CommandSyncMask::GetGlobalQueueIdx(queueUsage, queueIndex);
	for(auto& entry : mResources)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(mSubmittedQueueGlobalIndex, useHandle.Flags);
	}

	for(auto& entry : mImages)
	{
		u32 imageInfoIdx = entry.second;
		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

		ResourceUseHandle& useHandle = imageInfo.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(mSubmittedQueueGlobalIndex, useHandle.Flags);
	}

	for(auto& entry : mBuffers)
	{
		ResourceUseHandle& useHandle = entry.second.UseHandle;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(mSubmittedQueueGlobalIndex, useHandle.Flags);
	}

	for(auto& entry : mSwapChains)
	{
		ResourceUseHandle& useHandle = entry.second;
		B3D_ASSERT(!useHandle.Used);

		useHandle.Used = true;
		entry.first->NotifyUsed(mSubmittedQueueGlobalIndex, useHandle.Flags);
	}

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
	mQueuedLayoutTransitions.clear();
	mBoundParams = nullptr;
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
	mVertexInputsDirty = true;
	mAcquiredSwapChainImages.clear();

	return submitInformation;
}

void VulkanGpuCommandBuffer::NotifyWillQueueForSubmit()
{
	// Clear everything not allowed on the submit thread
	mGraphicsPipeline = nullptr;
	mComputePipeline = nullptr;
	mBoundParams = nullptr;
	mIndexBuffer = nullptr;
	mVertexBuffers.clear();
}

bool VulkanGpuCommandBuffer::UpdateExecutionStatus(bool block)
{
	AssertIfNotVulkanSubmitThread();

	VkResult result = vkWaitForFences(GetVulkanGpuDevice().GetLogical(), 1, &mFence, true, block ? 1'000'000'000 : 0);
	B3D_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT);

	return result == VK_SUCCESS;
}

void VulkanGpuCommandBuffer::Reset()
{
	bool wasSubmitted = mState == State::Submitted || mState == State::Done;

	mState = State::Ready;
	vkResetCommandBuffer(mCommandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); // Note: Maybe better not to release resources?

	const VkResult result = vkResetFences(GetVulkanGpuDevice().GetLogical(), 1, &mFence);
	B3D_ASSERT(result == VK_SUCCESS);

	if(wasSubmitted)
	{
		for(auto& entry : mResources)
		{
			ResourceUseHandle& useHandle = entry.second;
			B3D_ASSERT(useHandle.Used);

			entry.first->NotifyDone(mSubmittedQueueGlobalIndex, useHandle.Flags);
		}

		for(auto& entry : mImages)
		{
			u32 imageInfoIdx = entry.second;
			ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

			ResourceUseHandle& useHandle = imageInfo.UseHandle;
			B3D_ASSERT(useHandle.Used);

			entry.first->NotifyDone(mSubmittedQueueGlobalIndex, useHandle.Flags);
		}

		for(auto& entry : mBuffers)
		{
			ResourceUseHandle& useHandle = entry.second.UseHandle;
			B3D_ASSERT(useHandle.Used);

			entry.first->NotifyDone(mSubmittedQueueGlobalIndex, useHandle.Flags);
		}

		// Must be done after images & framebuffer because swap chain does error checking if those were freed
		for(auto& entry : mSwapChains)
		{
			ResourceUseHandle& useHandle = entry.second;
			B3D_ASSERT(useHandle.Used);

			entry.first->NotifyDone(mSubmittedQueueGlobalIndex, useHandle.Flags);
		}
	}
	else
	{
		for(auto& entry : mResources)
			entry.first->NotifyUnbound();

		for(auto& entry : mImages)
			entry.first->NotifyUnbound();

		for(auto& entry : mBuffers)
			entry.first->NotifyUnbound();

		// Must be done after images & framebuffer because swap chain does error checking if those were freed
		for(auto& entry : mSwapChains)
			entry.first->NotifyUnbound();
	}

	mResources.clear();
	mImages.clear();
	mBuffers.clear();
	mSwapChains.clear();
	mOcclusionQueries.clear();
	mTimerQueries.clear();
	mImageInfos.clear();
	mSubresourceInfoStorage.clear();
	mShaderBoundSubresourceInfos.clear();
	mNeedsRAWMemoryBarrier = false;
	mNeedsWARMemoryBarrier = false;
	mMemoryBarrierDstAccess = 0;
	mMemoryBarrierSrcAccess = 0;
	mMemoryBarrierDstStages = 0;
	mMemoryBarrierSrcStages = 0;
	mIsRenderPassInterrupted = false;
	mSyncMask = 0;
}

void VulkanGpuCommandBuffer::ClearViewport(const Rect2I& area, u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	if(buffers == 0 || mFramebuffer == nullptr)
		return;

	// If a clear operation is queued we need to execute it first
	const bool isClearAlreadyQueued = !IsInRenderPass() && (mClearMask != RT_NONE) && (area != mClearArea);
	if(isClearAlreadyQueued)
	{
		// Render pass start will trigger a an implicit clear
		BeginRenderPass();
		B3D_ASSERT(mClearMask == RT_NONE);
	}

	mClearArea = area;

	// Determine which attachments require clearing, and their clear values
	const VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	const u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	if((buffers & FBT_COLOR) != 0)
	{
		for(u32 sequentialColorAttachmentIndex = 0; sequentialColorAttachmentIndex < colorAttachmentCount; sequentialColorAttachmentIndex++)
		{
			const VulkanFramebufferAttachment& colorAttachment = mFramebuffer->GetColorAttachment(sequentialColorAttachmentIndex);
			const RenderSurfaceMaskBits colorAttachmentBit = (RenderSurfaceMaskBits)(1 << colorAttachment.Index);

			if((colorAttachmentBit & targetMask) == 0)
				continue;

			mClearMask |= colorAttachmentBit;

			VkClearColorValue& colorAttachmentClearValue = mClearValues[sequentialColorAttachmentIndex].color;
			colorAttachmentClearValue.float32[0] = color.R;
			colorAttachmentClearValue.float32[1] = color.G;
			colorAttachmentClearValue.float32[2] = color.B;
			colorAttachmentClearValue.float32[3] = color.A;
		}
	}

	if((buffers & (FBT_DEPTH | FBT_STENCIL)) != 0)
	{
		if(renderPass->HasDepthAttachment())
		{
			u32 depthAttachmentSequentialIndex = colorAttachmentCount;

			if((buffers & FBT_DEPTH) != 0)
			{
				mClearValues[depthAttachmentSequentialIndex].depthStencil.depth = depth;
				mClearMask |= RT_DEPTH;
			}

			if((buffers & FBT_STENCIL) != 0)
			{
				mClearValues[depthAttachmentSequentialIndex].depthStencil.stencil = stencil;
				mClearMask |= RT_STENCIL;
			}
		}
	}

	// Nothing to do
	if(mClearMask == RT_NONE)
		return;

	// If currently in a render pass, execute a clear command. Otherwise we do the clear implicitly when beginning the render pass
	if(IsInRenderPass())
	{
		ExecuteClearCommand(mClearArea, mClearMask, mClearValues);
		mClearMask = RT_NONE;
	}

	NotifyRenderTargetModified();
}

void VulkanGpuCommandBuffer::ExecuteClearCommand(const Rect2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues)
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
				B3D_LOG(Error, RenderBackend, "All starting layers for frame buffer attachments must be matching when performing a clear command.");
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
					B3D_LOG(Error, RenderBackend, "All starting layers for frame buffer attachments must be matching when performing a clear command.");
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

	SPtr<VertexDescription> shaderInputVertexDescription = mGraphicsPipeline->GetInputDeclaration();
	if(shaderInputVertexDescription == nullptr)
		return false;

	return mFramebuffer != nullptr && mVertexDescription != nullptr;
}

bool VulkanGpuCommandBuffer::BindGraphicsPipeline()
{
	const SPtr<VertexDescription> vertexShaderInputDescription = mGraphicsPipeline->GetInputDeclaration();
	const SPtr<VulkanVertexInput> vertexShaderInput = VulkanVertexInputManager::Instance().GetVertexInfo(mVertexDescription, vertexShaderInputDescription);

	VulkanRenderPass *const renderPass = mFramebuffer->GetRenderPass();
	VulkanPipeline *const pipeline = mGraphicsPipeline->FindOrCreateVulkanResource(renderPass, mRenderTargetReadOnlyFlags, mDrawOp, vertexShaderInput);

	if(pipeline == nullptr)
		return false;

	// Check that pipeline matches the read-only state of any framebuffer attachments
	u32 colorAttachmentCount = renderPass->GetColorAttachmentCount();
	for(u32 i = 0; i < colorAttachmentCount; i++)
	{
		const VulkanFramebufferAttachment& framebufferAttachment = mFramebuffer->GetColorAttachment(i);
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(framebufferAttachment.Image, framebufferAttachment.Surface.Face, framebufferAttachment.Surface.MipLevel);

		if(subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader) && !pipeline->IsColorReadOnly(i))
		{
			B3D_LOG(Warning, RenderBackend, "Framebuffer attachment also used as a shader input, but color writes "
										   "aren't disabled. This will result in undefined behavior.");
		}
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& framebufferAttachment = mFramebuffer->GetDepthStencilAttachment();
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(framebufferAttachment.Image, framebufferAttachment.Surface.Face, framebufferAttachment.Surface.MipLevel);

		if(subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader) && !pipeline->IsDepthReadOnly())
		{
			B3D_LOG(Warning, RenderBackend, "Framebuffer attachment also used as a shader input, but depth/stencil "
										   "writes aren't disabled. This will result in undefined behavior.");
		}
	}

	mGraphicsPipeline->RegisterPipelineResources(*this);
	RegisterResource(pipeline, VulkanAccessFlag::Read);

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
			RegisterBuffer(resource, BufferUseFlagBits::Vertex, VulkanAccessFlag::Read);
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

			RegisterBuffer(resource, BufferUseFlagBits::Index, VulkanAccessFlag::Read);

			vkCmdBindIndexBuffer(mCommandBufferHandle, vkBuffer, 0, indexType);
		}
	}
}

void VulkanGpuCommandBuffer::BindGpuParams()
{
	if(!mBoundParamsDirty)
		return;

	if(mBoundParams != nullptr)
	{
		mBoundDescriptorSetCount = mBoundParams->GetSetCount();

		mDynamicDescriptorOffsetsToBind.clear();
		mBoundParams->PrepareForBind(*this, mDescriptorSetsTemp, mDynamicDescriptorOffsetsToBind);
	}
	else
		mBoundDescriptorSetCount = 0;

	for(const auto& pair : mDynamicDescriptorOffsetsOverrides)
	{
		if(!B3D_ENSURE(pair.first < (u32)mDynamicDescriptorOffsetsToBind.size()))
			continue;

		mDynamicDescriptorOffsetsToBind[pair.first] = pair.second;
	}

	mBoundParamsDirty = false;
}

void VulkanGpuCommandBuffer::ExecuteLayoutTransitions()
{
	auto createLayoutTransitionBarrier = [&](VulkanImage* image, ImageInfo& imageInfo)
	{
		ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
		for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
		{
			ImageSubresourceInfo& subresourceInfo = subresourceInfos[i];

			if(subresourceInfo.RequiredLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
			   subresourceInfo.CurrentLayout == subresourceInfo.RequiredLayout)
				continue;

			const bool isReadOnly =
				!subresourceInfo.FbUse.Access.IsSet(VulkanAccessFlag::Write) &&
				!subresourceInfo.ShaderUse.Access.IsSet(VulkanAccessFlag::Write);

			mLayoutTransitionBarriersTemp.push_back(VkImageMemoryBarrier());
			VkImageMemoryBarrier& barrier = mLayoutTransitionBarriersTemp.back();
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = 0; // Not relevant for layout transition
			barrier.dstAccessMask = image->GetAccessFlags(subresourceInfo.RequiredLayout, isReadOnly);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout = subresourceInfo.CurrentLayout;
			barrier.newLayout = subresourceInfo.RequiredLayout;
			barrier.image = image->GetVulkanHandle();
			barrier.subresourceRange = subresourceInfo.Range;

			subresourceInfo.CurrentLayout = subresourceInfo.RequiredLayout;
		}
	};

	// Note: These layout transitions will contain transitions for offscreen framebuffer attachments (while they
	// transition to shader read-only layout). This can be avoided, since they're immediately used by the render pass
	// as color attachments, making the layout change redundant.
	for(auto& entry : mQueuedLayoutTransitions)
	{
		u32 imageInfoIdx = entry.second;
		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

		createLayoutTransitionBarrier(entry.first, imageInfo);
	}

	VkPipelineStageFlags srcStage = 0;
	VkPipelineStageFlags dstStage = 0;
	::GetPipelineStageFlags(mLayoutTransitionBarriersTemp, srcStage, dstStage);

	if(!mLayoutTransitionBarriersTemp.empty())
	{
		vkCmdPipelineBarrier(
			mCommandBufferHandle,
			srcStage, dstStage,
			0, 0, nullptr,
			0, nullptr,
			(u32)mLayoutTransitionBarriersTemp.size(), mLayoutTransitionBarriersTemp.data());
	}

	mQueuedLayoutTransitions.clear();
	mLayoutTransitionBarriersTemp.clear();
}

void VulkanGpuCommandBuffer::ExecuteWriteHazardBarrier()
{
	if(!mNeedsRAWMemoryBarrier && !mNeedsWARMemoryBarrier)
		return;

	// If read-after-write we need an actual memory barrier, otherwise we just need an execution dependency
	if(mNeedsRAWMemoryBarrier)
	{
		VkMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = nullptr;

		barrier.srcAccessMask = mMemoryBarrierSrcAccess;
		barrier.dstAccessMask = mMemoryBarrierDstAccess;

		vkCmdPipelineBarrier(GetVulkanHandle(), mMemoryBarrierSrcStages, mMemoryBarrierDstStages, 0, 1, &barrier, 0, nullptr, 0, nullptr);
	}
	else
	{
		vkCmdPipelineBarrier(GetVulkanHandle(), mMemoryBarrierSrcStages, mMemoryBarrierDstStages, 0, 0, nullptr, 0, nullptr, 0, nullptr);
	}

	mNeedsRAWMemoryBarrier = false;
	mNeedsWARMemoryBarrier = false;
	mMemoryBarrierSrcStages = 0;
	mMemoryBarrierDstStages = 0;
	mMemoryBarrierSrcAccess = 0;
	mMemoryBarrierDstAccess = 0;

	for(auto& entry : mShaderBoundSubresourceInfos)
	{
		ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage[entry];
		subresourceInfo.WriteHazardUse = subresourceInfo.NewWriteHazardUse;
		subresourceInfo.NewWriteHazardUse = ResourcePipelineUse();
	}

	// Note: I should keep track of any buffers modified and only clear those, to prevent excess iteration
	for(auto& entry : mBuffers)
	{
		BufferInfo& bufferInfo = entry.second;
		bufferInfo.WriteHazardUse = bufferInfo.NewWriteHazardUse;
		bufferInfo.NewWriteHazardUse = ResourcePipelineUse();
	}
}

void VulkanGpuCommandBuffer::UpdateFinalLayouts()
{
	if(mFramebuffer == nullptr)
		return;

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	u32 numColorAttachments = renderPass->GetColorAttachmentCount();
	for(u32 i = 0; i < numColorAttachments; i++)
	{
		const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetColorAttachment(i);
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceInfo.CurrentLayout = subresourceInfo.RenderPassLayout;
		subresourceInfo.RequiredLayout = subresourceInfo.RenderPassLayout;
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetDepthStencilAttachment();
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		subresourceInfo.CurrentLayout = subresourceInfo.RenderPassLayout;
		subresourceInfo.RequiredLayout = subresourceInfo.RenderPassLayout;
	}
}

void VulkanGpuCommandBuffer::ExecuteClearPass()
{
	B3D_ASSERT(mState == State::Recording);

	ExecuteWriteHazardBarrier();
	ExecuteLayoutTransitions();

	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();

	bool isClearCommandRequired = false;
	RenderSurfaceMask renderPassClearMask = mClearMask;

	// If both load and clear flags are specified, execute an explicit clear command instead. For some reason this is not working
	// during render pass begin on some cards.
	if(mRenderTargetLoadMask.IsSetAny(mClearMask))
	{
		isClearCommandRequired = true;
		renderPassClearMask = RT_NONE;
	}

	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.framebuffer = mFramebuffer->GetVulkanHandle();
	renderPassBeginInfo.renderPass = renderPass->GetVkRenderPass(mRenderTargetLoadMask, RT_NONE, renderPassClearMask);
	renderPassBeginInfo.renderArea.offset.x = mClearArea.X;
	renderPassBeginInfo.renderArea.offset.y = mClearArea.Y;
	renderPassBeginInfo.renderArea.extent.width = mClearArea.Width;
	renderPassBeginInfo.renderArea.extent.height = mClearArea.Height;
	renderPassBeginInfo.clearValueCount = renderPass->GetClearEntryCount(renderPassClearMask);
	renderPassBeginInfo.pClearValues = mClearValues.data();

	vkCmdBeginRenderPass(mCommandBufferHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	if(isClearCommandRequired)
	{
		ExecuteClearCommand(mClearArea, mClearMask, mClearValues);
	}

	vkCmdEndRenderPass(mCommandBufferHandle);

	UpdateFinalLayouts();

	mClearMask = RT_NONE;
}

void VulkanGpuCommandBuffer::SetEvent(VulkanEvent* event)
{
	if(IsInRenderPass())
		mQueuedEvents.push_back(event);
	else
		vkCmdSetEvent(mCommandBufferHandle, event->GetVulkanHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

void VulkanGpuCommandBuffer::ResetQuery(VulkanQuery* query)
{
	if(IsInRenderPass())
		mQueuedQueryResets.push_back(query);
	else
		query->Reset(mCommandBufferHandle);
}

void VulkanGpuCommandBuffer::UpdateBuffer(VulkanBuffer* destination, u8* data, VkDeviceSize offset, VkDeviceSize length)
{
	MemoryBarrier(destination->GetVulkanHandle(), destination->GetAccessFlags(), VK_ACCESS_TRANSFER_READ_BIT);
	vkCmdUpdateBuffer(GetVulkanHandle(), destination->GetVulkanHandle(), offset, length, (uint32_t*)data);
	MemoryBarrier(destination->GetVulkanHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, destination->GetAccessFlags());
}

void VulkanGpuCommandBuffer::CopyBufferToBuffer(VulkanBuffer* source, VulkanBuffer* destination, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset, VkDeviceSize length)
{
	VkBufferCopy region;
	region.size = length;
	region.srcOffset = sourceOffset;
	region.dstOffset = destinationOffset;

	MemoryBarrier(source->GetVulkanHandle(), source->GetAccessFlags(), VK_ACCESS_TRANSFER_READ_BIT);
	MemoryBarrier(destination->GetVulkanHandle(), destination->GetAccessFlags(), VK_ACCESS_TRANSFER_WRITE_BIT);

	vkCmdCopyBuffer(GetVulkanHandle(), source->GetVulkanHandle(), destination->GetVulkanHandle(), 1, &region);

	MemoryBarrier(source->GetVulkanHandle(), VK_ACCESS_TRANSFER_READ_BIT, source->GetAccessFlags());
	MemoryBarrier(destination->GetVulkanHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, destination->GetAccessFlags());

}

void VulkanGpuCommandBuffer::CopyBufferToImage(VulkanBuffer* source, VulkanImage* destination, const VkExtent3D& region, const VkImageSubresourceRange& subresourceRange, VkImageLayout layout, u32 rowPitch, u32 sliceHeight)
{
	RegisterBuffer(source, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
	RegisterImageTransfer(destination, subresourceRange, layout, VulkanAccessFlag::Write);

	VkImageSubresourceLayers rangeLayers;
	rangeLayers.aspectMask = subresourceRange.aspectMask;
	rangeLayers.baseArrayLayer = subresourceRange.baseArrayLayer;
	rangeLayers.layerCount = subresourceRange.layerCount;
	rangeLayers.mipLevel = subresourceRange.baseMipLevel;

	VkBufferImageCopy copyRegion;
	copyRegion.bufferRowLength = rowPitch;
	copyRegion.bufferImageHeight = sliceHeight;
	copyRegion.bufferOffset = 0;
	copyRegion.imageOffset.x = 0;
	copyRegion.imageOffset.y = 0;
	copyRegion.imageOffset.z = 0;
	copyRegion.imageExtent = region;
	copyRegion.imageSubresource = rangeLayers;

	MemoryBarrier(source->GetVulkanHandle(), source->GetAccessFlags(), VK_ACCESS_TRANSFER_READ_BIT);
	// TODO - Barriers for the image
	vkCmdCopyBufferToImage(GetVulkanHandle(), source->GetVulkanHandle(), destination->GetVulkanHandle(), layout, 1, &copyRegion);
	MemoryBarrier(source->GetVulkanHandle(), VK_ACCESS_TRANSFER_READ_BIT, source->GetAccessFlags());
}

void VulkanGpuCommandBuffer::CopyImageToBuffer(VulkanImage* source, VulkanBuffer* destination, const VkExtent3D& region, const VkImageSubresourceRange& subresourceRange, VkImageLayout layout, u32 rowPitch, u32 sliceHeight)
{
	RegisterImageTransfer(source, subresourceRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VulkanAccessFlag::Read);
	RegisterBuffer(destination, BufferUseFlagBits::Transfer, VulkanAccessFlag::Write);

	VkImageSubresourceLayers rangeLayers;
	rangeLayers.aspectMask = subresourceRange.aspectMask;
	rangeLayers.baseArrayLayer = subresourceRange.baseArrayLayer;
	rangeLayers.layerCount = subresourceRange.layerCount;
	rangeLayers.mipLevel = subresourceRange.baseMipLevel;

	VkBufferImageCopy copyRegion;
	copyRegion.bufferRowLength = rowPitch;
	copyRegion.bufferImageHeight = sliceHeight;
	copyRegion.bufferOffset = 0;
	copyRegion.imageOffset.x = 0;
	copyRegion.imageOffset.y = 0;
	copyRegion.imageOffset.z = 0;
	copyRegion.imageExtent = region;
	copyRegion.imageSubresource = rangeLayers;

	MemoryBarrier(destination->GetVulkanHandle(), destination->GetAccessFlags(), VK_ACCESS_TRANSFER_READ_BIT);
	// TODO - Barriers for the image
	vkCmdCopyImageToBuffer(GetVulkanHandle(), source->GetVulkanHandle(), layout, destination->GetVulkanHandle(), 1, &copyRegion);
	MemoryBarrier(destination->GetVulkanHandle(), VK_ACCESS_TRANSFER_READ_BIT, destination->GetAccessFlags());
}

void VulkanGpuCommandBuffer::CopyImageToImage(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageCopy* regions)
{
	RegisterImageTransfer(source, sourceSubresourceRange, sourceLayout, VulkanAccessFlag::Read);
	RegisterImageTransfer(destination, destinationSubresourceRange, destinationLayout, VulkanAccessFlag::Write);

	vkCmdCopyImage(GetVulkanHandle(), source->GetVulkanHandle(), sourceLayout, destination->GetVulkanHandle(), destinationLayout, regionCount, regions);
}

void VulkanGpuCommandBuffer::Blit(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageBlit* regions)
{
	RegisterImageTransfer(source, sourceSubresourceRange, sourceLayout, VulkanAccessFlag::Read);
	RegisterImageTransfer(destination, destinationSubresourceRange, destinationLayout, VulkanAccessFlag::Write);

	vkCmdBlitImage(GetVulkanHandle(), source->GetVulkanHandle(), sourceLayout, destination->GetVulkanHandle(), destinationLayout, regionCount, regions, VK_FILTER_LINEAR);
}

void VulkanGpuCommandBuffer::Resolve(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageResolve* regions)
{
	RegisterImageTransfer(source, sourceSubresourceRange, sourceLayout, VulkanAccessFlag::Read);
	RegisterImageTransfer(destination, destinationSubresourceRange, destinationLayout, VulkanAccessFlag::Write);

	vkCmdResolveImage(GetVulkanHandle(), source->GetVulkanHandle(), sourceLayout, destination->GetVulkanHandle(), destinationLayout, regionCount, regions);
}

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

void VulkanGpuCommandBuffer::MemoryBarrier(VkBuffer buffer, VkAccessFlags sourceAccessFlags, VkAccessFlags destinationAccessFlags)
{
	MemoryBarrier(buffer, sourceAccessFlags, destinationAccessFlags, GetPipelineStageFlags(sourceAccessFlags), GetPipelineStageFlags(destinationAccessFlags));
}

VkImageLayout VulkanGpuCommandBuffer::GetCurrentLayout(VulkanImage* image, const VkImageSubresourceRange& range, bool inRenderPass)
{
	u32 face = range.baseArrayLayer;
	u32 mip = range.baseMipLevel;

	// The assumption is that all the subresources in the range will have the same layout, as this should be handled
	// by registerResource(), or by external code (in the case of transfers). So we only check the first subresource.
	VulkanImageSubresource* subresource = image->GetSubresource(face, mip);

	auto iterFind = mImages.find(image);
	if(iterFind == mImages.end())
	{
		B3D_ASSERT(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	u32 imageInfoIdx = iterFind->second;
	ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

	VulkanRenderPass* renderPass = nullptr;
	if(mFramebuffer)
		renderPass = mFramebuffer->GetRenderPass();

	ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
	for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
	{
		ImageSubresourceInfo& entry = subresourceInfos[i];
		if(face >= entry.Range.baseArrayLayer && face < (entry.Range.baseArrayLayer + entry.Range.layerCount) &&
		   mip >= entry.Range.baseMipLevel && mip < (entry.Range.baseMipLevel + entry.Range.levelCount))
		{
			// If it's a FB attachment, retrieve its layout after the render pass begins
			if(entry.UseFlags.IsSet(ImageUseFlagBits::Framebuffer) && inRenderPass && mFramebuffer)
			{
				RenderSurfaceMask readMask = GetFramebufferReadMask();

				// Is it a depth-stencil attachment?
				if(renderPass->HasDepthAttachment() && mFramebuffer->GetDepthStencilAttachment().Image == image)
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
					u32 numColorAttachments = renderPass->GetColorAttachmentCount();
					for(u32 j = 0; j < numColorAttachments; j++)
					{
						const VulkanFramebufferAttachment& attachment = mFramebuffer->GetColorAttachment(j);

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

			return entry.RequiredLayout;
		}
	}

	B3D_ASSERT(false);
	return VK_IMAGE_LAYOUT_UNDEFINED;
}

/** Returns a set of pipeline stages that can are allowed to be used for the specified set of access flags. */
VkPipelineStageFlags VulkanGpuCommandBuffer::GetPipelineStageFlags(VkAccessFlags accessFlags)
{
	VkPipelineStageFlags flags = 0;

	if((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
	{
		flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// MoltenVK doesn't support geometry and tessellation shaders
		// Note: Once we upgrade to a newer version they should be supported and we can remove this
#if B3D_PLATFORM != B3D_PLATFORM_ID_MACOS
		flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
		flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
#endif
	}

	if((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;

	if(flags == 0)
		flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	return flags;
}

void VulkanGpuCommandBuffer::RegisterResource(VulkanResource* res, VulkanAccessFlags flags)
{
	auto insertResult = mResources.insert(std::make_pair(res, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = flags;

		res->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= flags;
	}
}

void VulkanGpuCommandBuffer::RegisterImageShader(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	B3D_ASSERT(layout != VK_IMAGE_LAYOUT_UNDEFINED);
	RegisterResource(image, range, ImageUseFlagBits::Shader, layout, layout, access, stages);
}

void VulkanGpuCommandBuffer::RegisterImageFramebuffer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	RegisterResource(image, range, ImageUseFlagBits::Framebuffer, layout, finalLayout, access, stages);
}

void VulkanGpuCommandBuffer::RegisterImageTransfer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VulkanAccessFlags access)
{
	B3D_ASSERT(layout != VK_IMAGE_LAYOUT_UNDEFINED);
	RegisterResource(image, range, ImageUseFlagBits::Transfer, layout, layout, access, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

void VulkanGpuCommandBuffer::RegisterResource(VulkanImage* image, const VkImageSubresourceRange& range, ImageUseFlagBits use, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	// This function either registers a brand new image resource that was never been used on this command buffer, or
	// if the resource has been used previously then it calculates the overlapping subresource sets and calls a relevant
	// function and further determines if any layout transitions and/or memory/execution barriers are necessary.

	u32 nextImageInfoIdx = (u32)mImageInfos.size();
	auto registerSubresourceInfo = [&](const VkImageSubresourceRange& subresourceRange)
	{
		mSubresourceInfoStorage.push_back(ImageSubresourceInfo());
		ImageSubresourceInfo& subresourceInfo = mSubresourceInfoStorage.back();
		subresourceInfo.CurrentLayout = layout;
		subresourceInfo.InitialLayout = layout;
		subresourceInfo.InitialReadOnly = !access.IsSet(VulkanAccessFlag::Write);
		subresourceInfo.RequiredLayout = layout;
		subresourceInfo.RenderPassLayout = finalLayout;
		subresourceInfo.Range = subresourceRange;

		switch(use)
		{
		default:
		case ImageUseFlagBits::Shader:
			subresourceInfo.ShaderUse.Access = access;
			subresourceInfo.ShaderUse.Stages = stages;
			subresourceInfo.WriteHazardUse.Access = access;
			subresourceInfo.WriteHazardUse.Stages = stages;
			subresourceInfo.NewWriteHazardUse = subresourceInfo.WriteHazardUse;
			break;
		case ImageUseFlagBits::Framebuffer:
			subresourceInfo.FbUse.Access = access;
			subresourceInfo.FbUse.Stages = stages;
			break;
		case ImageUseFlagBits::Transfer:
			subresourceInfo.TransferUse.Access = access;
			subresourceInfo.TransferUse.Stages = stages;
			break;
		}

		subresourceInfo.UseFlags = use;

		if(use == ImageUseFlagBits::Shader)
			mShaderBoundSubresourceInfos.insert((u32)mSubresourceInfoStorage.size() - 1);
	};

	auto insertResult = mImages.insert(std::make_pair(image, nextImageInfoIdx));
	if(insertResult.second) // New element
	{
		u32 imageInfoIdx = insertResult.first->second;
		mImageInfos.push_back(ImageInfo());

		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];
		imageInfo.SubresourceInfoIdx = (u32)mSubresourceInfoStorage.size();
		imageInfo.NumSubresourceInfos = 1;

		imageInfo.UseHandle.Used = false;
		imageInfo.UseHandle.Flags = access;

		registerSubresourceInfo(range);

		image->NotifyBound();
	}
	else // Existing element
	{
		u32 imageInfoIdx = insertResult.first->second;
		ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

		B3D_ASSERT(!imageInfo.UseHandle.Used);
		imageInfo.UseHandle.Flags |= access;

		// See if there is an overlap between existing ranges and the new range. And if so break them up accordingly.
		//// First test for the simplest and most common case (same range or no overlap) to avoid more complex
		//// computations.
		ImageSubresourceInfo* subresources = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];

		bool foundRange = false;
		for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
		{
			if(VulkanUtility::RangeOverlaps(subresources[i].Range, range))
			{
				if(subresources[i].Range.layerCount == range.layerCount &&
				   subresources[i].Range.levelCount == range.levelCount &&
				   subresources[i].Range.baseArrayLayer == range.baseArrayLayer &&
				   subresources[i].Range.baseMipLevel == range.baseMipLevel)
				{
					// Just update existing range
					switch(use)
					{
					default:
					case ImageUseFlagBits::Shader:
						UpdateShaderSubresource(image, imageInfoIdx, subresources[i], layout, access, stages);
						break;
					case ImageUseFlagBits::Framebuffer:
						UpdateFramebufferSubresource(image, imageInfoIdx, subresources[i], layout, finalLayout, access, stages);
						break;
					case ImageUseFlagBits::Transfer:
						UpdateTransferSubresource(image, subresources[i], layout, access, stages);
						break;
					}

					if(use == ImageUseFlagBits::Shader)
						mShaderBoundSubresourceInfos.insert(imageInfo.SubresourceInfoIdx + i);

					foundRange = true;
					break;
				}

				break;
			}
		}

		//// We'll need to update subresource ranges or add new ones. The hope is that this code is trigger VERY rarely
		//// (for just a few specific textures per frame).
		if(!foundRange)
		{
			std::array<VkImageSubresourceRange, 5> tempCutRanges;

			B3DMarkAllocatorFrame();
			{
				// We orphan previously allocated memory (we reset it after submit() anyway)
				u32 newSubresourceIdx = (u32)mSubresourceInfoStorage.size();

				FrameVector<u32> cutOverlappingRanges;
				for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
				{
					u32 subresourceIdx = imageInfo.SubresourceInfoIdx + i;
					ImageSubresourceInfo& subresource = mSubresourceInfoStorage[subresourceIdx];

					if(!VulkanUtility::RangeOverlaps(subresource.Range, range))
					{
						// Just copy as is
						mSubresourceInfoStorage.push_back(subresource);

						if(use == ImageUseFlagBits::Shader)
							mShaderBoundSubresourceInfos.insert((u32)mSubresourceInfoStorage.size() - 1);
					}
					else // Need to cut
					{
						u32 numCutRanges;
						VulkanUtility::CutRange(subresource.Range, range, tempCutRanges, numCutRanges);

						for(u32 j = 0; j < numCutRanges; j++)
						{
							// Create a copy of the original subresource with the new range
							ImageSubresourceInfo newInfo = mSubresourceInfoStorage[subresourceIdx];
							newInfo.Range = tempCutRanges[j];

							if(VulkanUtility::RangeOverlaps(tempCutRanges[j], range))
							{
								// Update overlapping sub-resource range with new data from this range
								switch(use)
								{
								default:
								case ImageUseFlagBits::Shader:
									UpdateShaderSubresource(image, imageInfoIdx, newInfo, layout, access, stages);
									break;
								case ImageUseFlagBits::Framebuffer:
									UpdateFramebufferSubresource(image, imageInfoIdx, newInfo, layout, finalLayout, access, stages);
									break;
								case ImageUseFlagBits::Transfer:
									UpdateTransferSubresource(image, newInfo, layout, access, stages);
									break;
								}

								// Keep track of the overlapping ranges for later
								cutOverlappingRanges.push_back((u32)mSubresourceInfoStorage.size());
							}

							mSubresourceInfoStorage.push_back(newInfo);

							if(use == ImageUseFlagBits::Shader)
								mShaderBoundSubresourceInfos.insert((u32)mSubresourceInfoStorage.size() - 1);
						}
					}
				}

				// Our range doesn't overlap with any existing ranges, so just add it
				if(cutOverlappingRanges.empty())
				{
					registerSubresourceInfo(range);
				}
				else // Search if overlapping ranges fully cover the requested range, and insert non-covered regions
				{
					FrameQueue<VkImageSubresourceRange> sourceRanges;
					sourceRanges.push(range);

					for(auto& entry : cutOverlappingRanges)
					{
						VkImageSubresourceRange& overlappingRange = mSubresourceInfoStorage[entry].Range;

						u32 numSourceRanges = (u32)sourceRanges.size();
						for(u32 i = 0; i < numSourceRanges; i++)
						{
							VkImageSubresourceRange sourceRange = sourceRanges.front();
							sourceRanges.pop();

							u32 numCutRanges;
							VulkanUtility::CutRange(sourceRange, overlappingRange, tempCutRanges, numCutRanges);

							for(u32 j = 0; j < numCutRanges; j++)
							{
								// We only care about ranges outside of the ones we already covered
								if(!VulkanUtility::RangeOverlaps(tempCutRanges[j], overlappingRange))
									sourceRanges.push(tempCutRanges[j]);
							}
						}
					}

					// Any remaining range hasn't been covered yet
					while(!sourceRanges.empty())
					{
						registerSubresourceInfo(sourceRanges.front());
						sourceRanges.pop();
					}
				}

				imageInfo.SubresourceInfoIdx = newSubresourceIdx;
				imageInfo.NumSubresourceInfos = (u32)mSubresourceInfoStorage.size() - newSubresourceIdx;
			}
			B3DClearAllocatorFrame();
		}
	}

	// Register any sub-resources
	for(u32 i = 0; i < range.layerCount; i++)
	{
		for(u32 j = 0; j < range.levelCount; j++)
		{
			u32 layer = range.baseArrayLayer + i;
			u32 mipLevel = range.baseMipLevel + j;

			RegisterResource(image->GetSubresource(layer, mipLevel), access);
		}
	}
}

void VulkanGpuCommandBuffer::RegisterBuffer(VulkanBuffer* res, BufferUseFlagBits useFlags, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	auto insertResult = mBuffers.insert(std::make_pair(res, BufferInfo()));
	if(insertResult.second) // New element
	{
		BufferInfo& bufferInfo = insertResult.first->second;
		bufferInfo.UseFlags = useFlags;

		bufferInfo.UseHandle.Used = false;
		bufferInfo.UseHandle.Flags = access;

		// Transfer write hazards are handled externally
		if(useFlags != BufferUseFlagBits::Transfer)
		{
			bufferInfo.WriteHazardUse.Access = access;
			bufferInfo.WriteHazardUse.Stages = stages;
			bufferInfo.NewWriteHazardUse = bufferInfo.WriteHazardUse;
		}

		res->NotifyBound();
	}
	else // Existing element
	{
		BufferInfo& bufferInfo = insertResult.first->second;

		B3D_ASSERT(!bufferInfo.UseHandle.Used);

		// Transfer write hazards are handled externally
		bool resetRenderPass = false;
		if(useFlags != BufferUseFlagBits::Transfer)
		{
			// If this buffer has been previously written to prevent read-after-write and write-after-read hazards
			if(access.IsSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
			{
				// Read-after-write (and write-after-write, as little sense does that make)
				if(bufferInfo.WriteHazardUse.Access.IsSet(VulkanAccessFlag::Write))
				{
					mNeedsRAWMemoryBarrier = true;
					mMemoryBarrierSrcStages |= bufferInfo.WriteHazardUse.Stages;
					mMemoryBarrierDstStages |= stages;
					mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

					switch(useFlags)
					{
					case BufferUseFlagBits::Generic:
						if(access.IsSet(VulkanAccessFlag::Read))
							mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_READ_BIT;

						if(access.IsSet(VulkanAccessFlag::Write))
							mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_WRITE_BIT;
						break;
					case BufferUseFlagBits::Index:
						mMemoryBarrierDstAccess |= VK_ACCESS_INDEX_READ_BIT;
						break;
					case BufferUseFlagBits::Vertex:
						mMemoryBarrierDstAccess |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
						break;
					case BufferUseFlagBits::Parameter:
						mMemoryBarrierDstAccess |= VK_ACCESS_UNIFORM_READ_BIT;
						break;
					case BufferUseFlagBits::Transfer:
						if(access.IsSet(VulkanAccessFlag::Read))
							mMemoryBarrierDstAccess |= VK_ACCESS_TRANSFER_READ_BIT;

						if(access.IsSet(VulkanAccessFlag::Write))
							mMemoryBarrierDstAccess |= VK_ACCESS_TRANSFER_WRITE_BIT;
						break;
					}

					// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
					// will force them to execute
					resetRenderPass = true;
				}
			}

			if(access.IsSet(VulkanAccessFlag::Write))
			{
				// Write-after-read
				if(bufferInfo.WriteHazardUse.Access.IsSet(VulkanAccessFlag::Read))
				{
					mNeedsWARMemoryBarrier = true;
					mMemoryBarrierSrcStages |= bufferInfo.WriteHazardUse.Stages;
					mMemoryBarrierDstStages |= stages;

					// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
					// will force them to execute
					resetRenderPass = true;
				}
			}

			bufferInfo.WriteHazardUse.Access |= access;
			bufferInfo.WriteHazardUse.Stages |= stages;

			bufferInfo.NewWriteHazardUse.Access |= access;
			bufferInfo.NewWriteHazardUse.Stages |= stages;
		}

		bufferInfo.UseHandle.Flags |= access;
		bufferInfo.UseFlags |= useFlags;

		// Need to end render pass in order to execute the barrier. Hopefully this won't trigger much since most
		// shader writes are done during compute
		if(resetRenderPass && IsInRenderPass())
			EndRenderPass(true);
	}
}

void VulkanGpuCommandBuffer::RegisterResource(VulkanFramebuffer* res, RenderSurfaceMask loadMask, u32 readMask)
{
	auto insertResult = mResources.insert(std::make_pair(res, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = VulkanAccessFlag::Write;

		res->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= VulkanAccessFlag::Write;
	}

	// Register any sub-resources
	VulkanRenderPass* renderPass = res->GetRenderPass();
	u32 numColorAttachments = renderPass->GetColorAttachmentCount();
	for(u32 i = 0; i < numColorAttachments; i++)
	{
		const VulkanFramebufferAttachment& attachment = res->GetColorAttachment(i);

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		VkImageLayout layout;
		if(loadMask.IsSet((RenderSurfaceMaskBits)(1 << i)))
			layout = attachment.Image->IsShaderReadAllowed() ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VulkanAccessFlag access = ((readMask & FBT_COLOR) != 0) ? VulkanAccessFlag::Read : VulkanAccessFlag::Write;

		VkImageSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		RegisterImageFramebuffer(attachment.Image, range, layout, attachment.FinalLayout, access, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& attachment = res->GetDepthStencilAttachment();

		// If image is being loaded, we need to transfer it to correct layout, otherwise it doesn't matter. We're using
		// these values because that's what VulkanFramebuffer expects as initialLayout.
		VkImageLayout layout;
		if(loadMask.IsSet(RT_DEPTH) || loadMask.IsSet(RT_STENCIL)) // Can't load one without the other
			layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else
			layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VulkanAccessFlag access = (((readMask & FBT_DEPTH) != 0) && ((readMask & FBT_STENCIL) != 0)) ? VulkanAccessFlag::Read : VulkanAccessFlag::Write;

		VkImageSubresourceRange range = attachment.Image->GetRange(attachment.Surface);
		RegisterImageFramebuffer(attachment.Image, range, layout, attachment.FinalLayout, access, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
	}
}

void VulkanGpuCommandBuffer::RegisterResource(VulkanSwapChain* res)
{
	auto insertResult = mSwapChains.insert(std::make_pair(res, ResourceUseHandle()));
	if(insertResult.second) // New element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;
		useHandle.Used = false;
		useHandle.Flags = VulkanAccessFlag::Write;

		res->NotifyBound();
	}
	else // Existing element
	{
		ResourceUseHandle& useHandle = insertResult.first->second;

		B3D_ASSERT(!useHandle.Used);
		useHandle.Flags |= VulkanAccessFlag::Write;
	}
}

void VulkanGpuCommandBuffer::UpdateShaderSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	// New layout is valid, check for transitions (UNDEFINED signifies the caller doesn't want a layout transition)
	if(layout != VK_IMAGE_LAYOUT_UNDEFINED)
	{
		// Register the necessary layout transition, but only if the image isn't bound for framebuffer bind. If it is
		// then we are forced to use the layout that's expected by the framebuffer.

		if(subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Framebuffer))
		{
			// Currently the system doesn't support image being bound to framebuffer, yet being written to by the
			// shader. This seems like an unlikely scenario.
			B3D_ASSERT(!access.IsSet(VulkanAccessFlag::Write));
		}
		else
		{
			// Check if the image had a layout previously assigned, and if so check if multiple different layouts
			// were requested. In that case we wish to transfer the image to GENERAL layout.

			bool firstUseInRenderPass = !subresourceInfo.UseFlags.IsSetAny(
				ImageUseFlagBits::Shader | ImageUseFlagBits::Framebuffer);
			if(firstUseInRenderPass || subresourceInfo.RequiredLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				subresourceInfo.RequiredLayout = layout;
			else if(subresourceInfo.RequiredLayout != layout)
				subresourceInfo.RequiredLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
	}

	bool isResetRenderPassRequired = false;
	if(subresourceInfo.CurrentLayout != subresourceInfo.RequiredLayout)
	{
		// Queue a layout transition
		mQueuedLayoutTransitions[image] = imageInfoIdx;

		// We also need to end the current pass, as we cannot do a layout transition within a pass
		isResetRenderPassRequired = true;
	}

	// If a FB attachment was just bound as a shader input, we might need to restart the render pass with a FB
	// attachment that supports read-only attachments using the GENERAL or DEPTH_READ_ONLY layout
	if(!subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader))
	{
		if(subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Framebuffer))
		{
			// Special case for depth: If user has set up proper read-only flags, then the render pass will have
			// taken care of setting the valid state anyway, so no need to end the render pass
			if(subresourceInfo.RequiredLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				isResetRenderPassRequired = ((mRenderTargetReadOnlyFlags & FBT_DEPTH) == 0 && (mRenderTargetReadOnlyFlags & FBT_STENCIL) == 0);
			}
			else
				isResetRenderPassRequired = true;
		}
	}

	// If this image has been previously used prevent read-after-write and write-after-read hazards
	if(access.IsSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
	{
		// Read-after-write (and write-after-write, as little sense does that make)
		if(subresourceInfo.WriteHazardUse.Access.IsSet(VulkanAccessFlag::Write))
		{
			mNeedsRAWMemoryBarrier = true;
			mMemoryBarrierSrcStages |= subresourceInfo.WriteHazardUse.Stages;
			mMemoryBarrierDstStages |= stages;
			mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

			if(access.IsSet(VulkanAccessFlag::Read))
				mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_READ_BIT;

			if(access.IsSet(VulkanAccessFlag::Write))
				mMemoryBarrierDstAccess |= VK_ACCESS_SHADER_WRITE_BIT;

			// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
			// will force them to execute
			isResetRenderPassRequired = true;
		}
	}

	if(access.IsSet(VulkanAccessFlag::Write))
	{
		// Write-after-read
		if(subresourceInfo.WriteHazardUse.Access.IsSet(VulkanAccessFlag::Read))
		{
			mNeedsWARMemoryBarrier = true;
			mMemoryBarrierSrcStages |= subresourceInfo.WriteHazardUse.Stages;
			mMemoryBarrierDstStages |= stages;

			// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
			// will force them to execute
			isResetRenderPassRequired = true;
		}
	}

	subresourceInfo.ShaderUse.Access |= access;
	subresourceInfo.ShaderUse.Stages |= stages;

	subresourceInfo.WriteHazardUse.Access |= access;
	subresourceInfo.WriteHazardUse.Stages |= stages;

	subresourceInfo.NewWriteHazardUse.Access |= access;
	subresourceInfo.NewWriteHazardUse.Stages |= stages;

	subresourceInfo.UseFlags |= ImageUseFlagBits::Shader;

	// If we need to switch frame-buffers or execute memory barriers, end current render pass
	if(isResetRenderPassRequired && IsInRenderPass())
		EndRenderPass(true);
}

void VulkanGpuCommandBuffer::UpdateFramebufferSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	// Framebuffer expects a certain layout and we must respect it. In the case when the FB attachment is also bound
	// for shader reads, this will override the layout required for shader read (GENERAL or DEPTH_READ_ONLY), but that
	// is fine because those transitions are handled automatically by render-pass layout transitions.
	subresourceInfo.RequiredLayout = layout;
	subresourceInfo.RenderPassLayout = finalLayout;

	if(subresourceInfo.CurrentLayout != subresourceInfo.RequiredLayout)
		mQueuedLayoutTransitions[image] = imageInfoIdx;

	// If the FB attachment was previously bound as a shader input, we might need to restart the render pass with a FB
	// attachment that supports read-only attachments using the GENERAL or DEPTH_READ_ONLY layout
	bool resetRenderPass = false;
	if(!subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Framebuffer))
		resetRenderPass = subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader);

	// If this image has been previously written to be a shader prevent read-after-write and write-after-write hazards
	// Note: This could be handled through sub-pass dependencies instead of explicit memory barriers, but those require
	// different render pass objects depending on access/stage flags, which is probably more overhead than just
	// executing the explicit barrier.
	if(access.IsSetAny(VulkanAccessFlag::Read | VulkanAccessFlag::Write))
	{
		// Read-after-write
		if(subresourceInfo.WriteHazardUse.Access.IsSet(VulkanAccessFlag::Write))
		{
			mNeedsRAWMemoryBarrier = true;
			mMemoryBarrierSrcStages |= subresourceInfo.WriteHazardUse.Stages;
			mMemoryBarrierDstStages |= stages;
			mMemoryBarrierSrcAccess |= VK_ACCESS_SHADER_WRITE_BIT;

			if(access.IsSet(VulkanAccessFlag::Read))
			{
				if((stages & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0)
					mMemoryBarrierDstAccess |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

				if((stages & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0 ||
				   (stages & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0)
					mMemoryBarrierDstAccess |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}

			if(access.IsSet(VulkanAccessFlag::Write))
			{
				if((stages & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0)
					mMemoryBarrierDstAccess |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				if((stages & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0 ||
				   (stages & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0)
					mMemoryBarrierDstAccess |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			// End render pass as we perform memory barriers at the beggining a render pass/dispatch call, so this
			// will force them to execute
			resetRenderPass = true;
		}
	}

	// No need to check for write-after-read barrier as it only needs an execution dependency and that is already
	// handled by the render pass

	subresourceInfo.FbUse.Access |= access;
	subresourceInfo.FbUse.Stages |= stages;

	subresourceInfo.UseFlags |= ImageUseFlagBits::Framebuffer;

	// If we need to switch frame-buffers or execute memory barriers, end current render pass
	if(resetRenderPass && IsInRenderPass())
		EndRenderPass(true);
}

void VulkanGpuCommandBuffer::UpdateTransferSubresource(VulkanImage* image, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages)
{
	// External code must end the render pass before attempting transfer operations
	B3D_ASSERT(!IsInRenderPass());

	// Ensure previously queued transitions execute
	ExecuteLayoutTransitions();
	B3D_ASSERT(subresourceInfo.CurrentLayout == subresourceInfo.RequiredLayout);

	// Transition to a valid transfer layout
	if(subresourceInfo.CurrentLayout != layout)
	{
		const VkAccessFlags sourceAccessFlags = image->GetAccessFlags(layout);
		const VkAccessFlags destinationAccessFlags = access.IsSet(VulkanAccessFlag::Write) ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_TRANSFER_READ_BIT;

		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = sourceAccessFlags;
		barrier.dstAccessMask = destinationAccessFlags;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout = subresourceInfo.CurrentLayout;
		barrier.newLayout = layout;
		barrier.image = image->GetVulkanHandle();
		barrier.subresourceRange = subresourceInfo.Range;

		const VkPipelineStageFlags sourceStage = GetPipelineStageFlags(sourceAccessFlags);
		const VkPipelineStageFlags destinationStage = GetPipelineStageFlags(destinationAccessFlags);

		vkCmdPipelineBarrier(GetVulkanHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	subresourceInfo.CurrentLayout = layout;
	subresourceInfo.RequiredLayout = layout;

	// These are currently not used nor cleared
	subresourceInfo.TransferUse.Access |= access;
	subresourceInfo.TransferUse.Stages |= stages;

	subresourceInfo.UseFlags |= ImageUseFlagBits::Transfer;
}

VulkanGpuCommandBuffer::ImageSubresourceInfo& VulkanGpuCommandBuffer::FindSubresourceInfo(VulkanImage* image, u32 face, u32 mip)
{
	u32 imageInfoIdx = mImages[image];
	ImageInfo& imageInfo = mImageInfos[imageInfoIdx];

	ImageSubresourceInfo* subresourceInfos = &mSubresourceInfoStorage[imageInfo.SubresourceInfoIdx];
	for(u32 i = 0; i < imageInfo.NumSubresourceInfos; i++)
	{
		ImageSubresourceInfo& entry = subresourceInfos[i];
		if(face >= entry.Range.baseArrayLayer && face < (entry.Range.baseArrayLayer + entry.Range.layerCount) &&
		   mip >= entry.Range.baseMipLevel && mip < (entry.Range.baseMipLevel + entry.Range.levelCount))
		{
			return entry;
		}
	}

	B3D_ASSERT(false); // Caller should ensure the subresource actually exists, so this shouldn't happen
	return subresourceInfos[0];
}

void VulkanGpuCommandBuffer::GetInProgressQueries(Vector<VulkanTimerQuery*>& timer, Vector<VulkanOcclusionQuery*>& occlusion) const
{
	for(auto& query : mTimerQueries)
	{
		if(query->IsInProgress())
			timer.push_back(query);
	}

	for(auto& query : mOcclusionQueries)
	{
		if(query->IsInProgress())
			occlusion.push_back(query);
	}
}

void VulkanGpuCommandBuffer::NotifyRenderTargetModified()
{
	if(mRenderTarget == nullptr || mRenderTargetModified)
		return;

	mRenderTarget->TickUpdateCountInternal();
	mRenderTargetModified = true;
}

RenderSurfaceMask VulkanGpuCommandBuffer::GetFramebufferReadMask()
{
	// Check if any frame-buffer attachments are also used as shader inputs, in which case we make them read-only
	VulkanRenderPass* renderPass = mFramebuffer->GetRenderPass();
	RenderSurfaceMask readMask = RT_NONE;

	u32 numColorAttachments = renderPass->GetColorAttachmentCount();
	for(u32 i = 0; i < numColorAttachments; i++)
	{
		const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetColorAttachment(i);
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		bool readOnly = subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader);

		if(readOnly)
			readMask.Set((RenderSurfaceMaskBits)(1 << i));
	}

	if(renderPass->HasDepthAttachment())
	{
		const VulkanFramebufferAttachment& fbAttachment = mFramebuffer->GetDepthStencilAttachment();
		ImageSubresourceInfo& subresourceInfo = FindSubresourceInfo(fbAttachment.Image, fbAttachment.Surface.Face, fbAttachment.Surface.MipLevel);

		bool readOnly = subresourceInfo.UseFlags.IsSet(ImageUseFlagBits::Shader);

		if(readOnly)
			readMask.Set(RT_DEPTH);

		if((mRenderTargetReadOnlyFlags & FBT_DEPTH) != 0)
			readMask.Set(RT_DEPTH);

		if((mRenderTargetReadOnlyFlags & FBT_STENCIL) != 0)
			readMask.Set(RT_STENCIL);
	}

	return readMask;
}

Rect2I VulkanGpuCommandBuffer::GetViewportArea() const
{
	Rect2I area;
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

Rect2I VulkanGpuCommandBuffer::GetRenderPassArea() const
{
	Rect2I area;
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

CommandBufferState VulkanGpuCommandBuffer::GetState() const
{
	switch(mState)
	{
	default:
	case State::Ready:
		return CommandBufferState::Ready;
	case State::Recording:
	case State::RecordingRenderPass:
	case State::RecordingDone:
		return CommandBufferState::Recording;
	case State::Submitted:
		return CommandBufferState::Executing;
	case State::Done:
		return CommandBufferState::Done;
	}
}


