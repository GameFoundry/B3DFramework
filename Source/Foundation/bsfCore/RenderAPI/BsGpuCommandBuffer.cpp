//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "BsRenderAPI.h"

using namespace bs;

namespace bs { namespace ct
{
u32 CommandSyncMask::GetGlobalQueueMask(GpuQueueUsage type, u32 queueIdx)
{
	u32 bitShift = 0;
	switch(type)
	{
	case GQT_GRAPHICS:
		break;
	case GQT_COMPUTE:
		bitShift = 8;
		break;
	case GQT_TRANSFER:
		bitShift = 16;
		break;
	default:
		break;
	}

	return (1 << queueIdx) << bitShift;
}

u32 CommandSyncMask::GetGlobalQueueIdx(GpuQueueUsage type, u32 queueIdx)
{
	switch(type)
	{
	case GQT_COMPUTE:
		return 8 + queueIdx;
	case GQT_TRANSFER:
		return 16 + queueIdx;
	default:
		return queueIdx;
	}
}

u32 CommandSyncMask::GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueUsage& type)
{
	if(globalQueueIdx >= 16)
	{
		type = GQT_TRANSFER;
		return globalQueueIdx - 16;
	}

	if(globalQueueIdx >= 8)
	{
		type = GQT_COMPUTE;
		return globalQueueIdx - 8;
	}

	type = GQT_GRAPHICS;
	return globalQueueIdx;
}

GpuCommandBufferPool::GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mInformation(createInformation)
{}

GpuCommandBuffer::GpuCommandBuffer(ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mUsage(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}

void GpuCommandBuffer::SetGpuParameters(const SPtr<GpuParameters>& parameters)
{
	GetRenderAPI().SetGpuParams(parameters, GetShared());
}

void GpuCommandBuffer::SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState)
{
	GetRenderAPI().SetGraphicsPipeline(pipelineState, GetShared());
}

void GpuCommandBuffer::SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState)
{
	GetRenderAPI().SetComputePipeline(pipelineState, GetShared());
}

void GpuCommandBuffer::SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount)
{
	GetRenderAPI().SetVertexBuffers(index, buffers, bufferCount, GetShared());
}

void GpuCommandBuffer::SetIndexBuffer(const SPtr<GpuBuffer>& buffer)
{
	GetRenderAPI().SetIndexBuffer(buffer, GetShared());
}

void GpuCommandBuffer::SetVertexDescription(const SPtr<VertexDescription>& vertexDescription)
{
	GetRenderAPI().SetVertexDescription(vertexDescription, GetShared());
}

void GpuCommandBuffer::SetDrawOperation(DrawOperationType operation)
{
	GetRenderAPI().SetDrawOperation(operation, GetShared());
}

void GpuCommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GetRenderAPI().Draw(vertexOffset, vertexCount, instanceCount, firstInstance, GetShared());
}

void GpuCommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GetRenderAPI().DrawIndexed(startIndex, indexCount, vertexOffset, vertexCount, instanceCount, firstInstance, GetShared());
}

void GpuCommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	GetRenderAPI().DispatchCompute(groupCountX, groupCountY, groupCountZ, GetShared());
}

void GpuCommandBuffer::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask)
{
	GetRenderAPI().SetRenderTarget(target, readOnlyFlags, loadMask, GetShared());
}

void GpuCommandBuffer::SetViewport(const Rect2& area)
{
	GetRenderAPI().SetViewport(area, GetShared());
}

void GpuCommandBuffer::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetRenderAPI().ClearRenderTarget(buffers, color, depth, stencil, targetMask, GetShared());
}

void GpuCommandBuffer::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetRenderAPI().ClearViewport(buffers, color, depth, stencil, targetMask, GetShared());
}

void GpuCommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
{
	GetRenderAPI().EnableScissorTest(left, top, right, bottom, GetShared());
}

void GpuCommandBuffer::DisableScissorTest()
{
	GetRenderAPI().DisableScissorTest(GetShared());
}

void GpuCommandBuffer::SetStencilReferenceValue(u32 value)
{
	GetRenderAPI().SetStencilRef(value, GetShared());
}

void GpuCommandBuffer::BeginLabel(const StringView& name)
{
	GetRenderAPI().BeginLabel(name, GetShared());
}

void GpuCommandBuffer::EndLabel()
{
	GetRenderAPI().EndLabel(GetShared());
}

void GpuCommandBuffer::InsertLabel(const StringView& name)
{
	GetRenderAPI().InsertLabel(name, GetShared());
}
}}
