//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderAPI.h"
#include "CoreThread/BsCoreThread.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "BsVulkanGpuDevice.h"
#include "Managers/BsVulkanTextureManager.h"
#include "Managers/BsVulkanRenderWindowManager.h"
#include "Managers/BsVulkanRenderStateManager.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanGpuParameters.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuQueue.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderPass.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"
#include "Win32/BsWin32RenderWindow.h"

#include <vulkan/vulkan.h>


using namespace bs;
using namespace bs::ct;

const StringID& VulkanRenderAPI::GetName() const
{
	static StringID strName("VulkanRenderAPI");
	return strName;
}

void VulkanRenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	// TODO - Move this to CoreApplication once I get rid of VulkanRenderAPI
	GpuBackend::StartUp<VulkanGpuBackend>();

	// TODO - Currently the GpuBackend always only initializes a single device. Once we change it to support multiple, pick the device here and initialize it1
	mPrimaryGpuDevice = GpuBackend::Instance().GetDevice(0);
	mPrimaryGpuDevice->Initialize();

	RenderAPI::Initialize();
}

void VulkanRenderAPI::DestroyCore()
{
	THROW_IF_NOT_CORE_THREAD;

	GetVulkanSubmitThread().WaitUntilIdle(true);
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			SPtr<VulkanGpuCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(*it);
			vulkanCommandBuffer->OnDidComplete();

			it = mSubmittedCommandBuffers.erase(it);
		}
		else
			++it;
	}

	mPrimaryGpuDevice = nullptr;

	// TODO - Move this to CoreApplication once I get rid of VulkanRenderAPI
	GpuBackend::ShutDown();

	RenderAPI::DestroyCore();
}

void VulkanRenderAPI::SetGraphicsPipeline(const SPtr<GpuGraphicsPipelineState>& pipelineState, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetPipelineState(pipelineState);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetComputePipeline(const SPtr<GpuComputePipelineState>& pipelineState, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetPipelineState(pipelineState);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void VulkanRenderAPI::SetGpuParams(const SPtr<GpuParameters>& gpuParams, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);

	for(u32 i = 0; i < GPT_COUNT; i++)
	{
		SPtr<GpuProgramParameterDescription> paramDesc = gpuParams->GetParameterInformation((GpuProgramType)i);
		if(paramDesc == nullptr)
			continue;

		// Flush all param block buffers
		for(auto iter = paramDesc->DataParameterBlocks.begin(); iter != paramDesc->DataParameterBlocks.end(); ++iter)
		{
			SPtr<GpuBuffer> buffer = gpuParams->GetUniformBuffer(iter->second.Set, iter->second.Slot);

			if(buffer != nullptr)
				buffer->FlushCache();
		}
	}

	cb->CmdSetGpuParams(gpuParams);

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void VulkanRenderAPI::SetViewport(const Rect2& vp, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetNormalizedViewportArea(vp);
}

void VulkanRenderAPI::SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 numBuffers, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetVertexBuffers(index, buffers, numBuffers);

	B3D_INCREMENT_RENDER_STATISTIC(NumVertexBufferBinds);
}

void VulkanRenderAPI::SetIndexBuffer(const SPtr<GpuBuffer>& buffer, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetIndexBuffer(buffer);

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void VulkanRenderAPI::SetVertexDescription(const SPtr<VertexDescription>& vertexDescription, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetVertexDescription(vertexDescription);
}

void VulkanRenderAPI::SetDrawOperation(DrawOperationType op, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetDrawOp(op);
}

void VulkanRenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdDraw(vertexOffset, vertexCount, instanceCount, firstInstance);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void VulkanRenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	u32 primCount = 0;

	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdDrawIndexed(startIndex, indexCount, vertexOffset, instanceCount, firstInstance);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void VulkanRenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdDispatch(numGroupsX, numGroupsY, numGroupsZ);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void VulkanRenderAPI::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);

	Rect2I area(left, top, right - left, bottom - top);
	vulkanCommandBuffer->CmdEnableScissorTest(area);
}

void VulkanRenderAPI::DisableScissorTest(const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	vulkanCommandBuffer->CmdDisableScissorTest();
}

void VulkanRenderAPI::SetStencilRef(u32 value, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetStencilRef(value);
}

void VulkanRenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdClearViewport(buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanRenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdClearRenderTarget(buffers, color, depth, stencil, targetMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void VulkanRenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	VulkanGpuCommandBuffer* cb = EnsureCommandBuffer(commandBuffer);
	cb->CmdSetRenderTarget(target, readOnlyFlags, loadMask);

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void VulkanRenderAPI::BeginFrame()
{
	THROW_IF_NOT_CORE_THREAD

	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			SPtr<VulkanGpuCommandBuffer> vulkanCommandBuffer = std::static_pointer_cast<VulkanGpuCommandBuffer>(*it);
			vulkanCommandBuffer->OnDidComplete();

			it = mSubmittedCommandBuffers.erase(it);
		}
		else
			++it;
	}
}

void VulkanRenderAPI::EndFrame()
{
	VulkanGpuDevice* const vulkanGpuDevice = static_cast<VulkanGpuDevice*>(mPrimaryGpuDevice.get());
	if(vulkanGpuDevice == nullptr)
		return;

	vulkanGpuDevice->SubmitTransferCommandBuffers();

	GetVulkanSubmitThread().QueueRefreshCommandBufferCompletionStates(vulkanGpuDevice);
}

void VulkanRenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD

	if(target == nullptr || !target->GetProperties().IsWindow)
		return;

	// Retrieve the swap chain before command buffer submit, as the submit might internally rebuild the swap chain.
	VulkanSwapChain* swapChain = nullptr;

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	Win32RenderWindow* window = static_cast<Win32RenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	LinuxRenderWindow* window = static_cast<LinuxRenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	MacOSRenderWindow* window = static_cast<MacOSRenderWindow*>(target.get());
#endif

	window->SwapBuffers();
	swapChain = window->GetSwapChain();

	VulkanGpuQueue* const presentQueue = static_cast<VulkanGpuQueue*>(GetVulkanGpuBackend().GetPresentDevice()->GetQueue(GQT_GRAPHICS, 0).get());
	GetVulkanSubmitThread().QueuePresent(*presentQueue, *swapChain, syncMask);

	// Ensure the acquire operation we queued the previous frame has finished. This also means the old image was presented.
	swapChain->WaitUntilFirstImageAcquired();

	GetVulkanSubmitThread().QueueImageAcquire(*swapChain);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

void VulkanRenderAPI::AddCommands(const SPtr<GpuCommandBuffer>& commandBuffer, const SPtr<GpuCommandBuffer>& secondary)
{
	B3D_EXCEPT(NotImplementedException, "Secondary command buffers not implemented");
}

void VulkanRenderAPI::BeginLabel(const StringView& name, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	vulkanCommandBuffer->CmdBeginLabel(name);
}

void VulkanRenderAPI::EndLabel(const SPtr<GpuCommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	vulkanCommandBuffer->CmdEndLabel();
}

void VulkanRenderAPI::InsertLabel(const StringView& name, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD

	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);
	vulkanCommandBuffer->CmdInsertLabel(name);
}

void VulkanRenderAPI::SubmitCommandBuffer(const SPtr<GpuCommandBuffer>& commandBuffer, u32 queueIndex, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	VulkanGpuCommandBuffer* vulkanCommandBuffer = EnsureCommandBuffer(commandBuffer);

	// Submit all transfer buffers first
	mPrimaryGpuDevice->SubmitTransferCommandBuffers();

	const SPtr<VulkanGpuQueue> queue = std::static_pointer_cast<VulkanGpuQueue>(mPrimaryGpuDevice->GetQueue(vulkanCommandBuffer->GetUsage(), queueIndex));
	if (!B3D_ENSURE(queue))
		return;

	B3D_ASSERT(queue != nullptr);
	vulkanCommandBuffer->Submit(*queue, syncMask);

	mSubmittedCommandBuffers.push_back(commandBuffer);
}

void VulkanRenderAPI::WaitUntilIdle() const
{
	GetVulkanSubmitThread().WaitUntilIdle();
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	for(auto it = mSubmittedCommandBuffers.begin(); it != mSubmittedCommandBuffers.end();)
	{
		if((*it)->GetState() == CommandBufferState::Done)
		{
			(*it)->OnDidComplete();
			it = mSubmittedCommandBuffers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void VulkanRenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	dest = matrix;

	// Flip Y axis
	dest[1][1] = -dest[1][1];

	// Convert depth range from [-1,1] to [0,1]
	dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
	dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
	dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
	dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
}

GpuDataParameterBlockInformation VulkanRenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params)
{
	GpuDataParameterBlockInformation block;
	block.BlockSize = 0;
	block.IsShareable = true;
	block.Name = name;
	block.Slot = 0;
	block.Set = 0;

	for(auto& param : params)
	{
		u32 size;
		if(param.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4
			size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;
		}
		else
			size = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(param.Type, param.ArraySize, block.BlockSize);

		param.ElementSize = size;
		param.ArrayElementStride = size;
		param.CpuOffset = block.BlockSize;
		param.GpuOffset = 0;
		block.BlockSize += size * param.ArraySize;
		param.ParamBlockSlot = 0;
		param.ParamBlockSet = 0;
	}

	// Constant buffer size must always be a multiple of 16
	if(block.BlockSize % 4 != 0)
		block.BlockSize += (4 - (block.BlockSize % 4));

	return block;
}

VulkanGpuCommandBuffer* VulkanRenderAPI::EnsureCommandBuffer(const SPtr<GpuCommandBuffer>& buffer)
{
	return static_cast<VulkanGpuCommandBuffer*>(buffer.get());
}

namespace bs { namespace ct {
VulkanRenderAPI& GetVulkanRenderAPI()
{
	return static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
}
}} // namespace bs::ct
