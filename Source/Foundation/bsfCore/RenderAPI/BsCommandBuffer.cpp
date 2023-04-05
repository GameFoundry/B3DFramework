//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsCommandBuffer.h"

#include "BsRenderAPI.h"
#include "Managers/BsCommandBufferManager.h"

using namespace bs;

namespace bs { namespace ct
{
void CommandSyncMask::AddDependency(const SPtr<CommandBuffer>& buffer)
{
	if(buffer == nullptr)
		return;

	mMask |= GetGlobalQueueMask(buffer->GetType(), buffer->GetQueueIdx());
}

u32 CommandSyncMask::GetGlobalQueueMask(GpuQueueType type, u32 queueIdx)
{
	u32 bitShift = 0;
	switch(type)
	{
	case GQT_GRAPHICS:
		break;
	case GQT_COMPUTE:
		bitShift = 8;
		break;
	case GQT_UPLOAD:
		bitShift = 16;
		break;
	default:
		break;
	}

	return (1 << queueIdx) << bitShift;
}

u32 CommandSyncMask::GetGlobalQueueIdx(GpuQueueType type, u32 queueIdx)
{
	switch(type)
	{
	case GQT_COMPUTE:
		return 8 + queueIdx;
	case GQT_UPLOAD:
		return 16 + queueIdx;
	default:
		return queueIdx;
	}
}

u32 CommandSyncMask::GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueType& type)
{
	if(globalQueueIdx >= 16)
	{
		type = GQT_UPLOAD;
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

CommandBuffer::CommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
	: mType(type), mDeviceIdx(deviceIdx), mQueueIdx(queueIdx), mIsSecondary(secondary)
{
}

CommandBuffer::~CommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}

SPtr<CommandBuffer> CommandBuffer::Create(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
{
	SPtr<CommandBuffer> commandBuffer = CommandBufferManager::Instance().Create(type, deviceIdx, queueIdx, secondary);
	commandBuffer->SetShared(commandBuffer);

	return commandBuffer;
}

void CommandBuffer::SetGpuParameters(const SPtr<GpuParameters>& parameters)
{
	GetRenderAPI().SetGpuParams(parameters, GetShared());
}

void CommandBuffer::SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState)
{
	GetRenderAPI().SetGraphicsPipeline(pipelineState, GetShared());
}

void CommandBuffer::SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState)
{
	GetRenderAPI().SetComputePipeline(pipelineState, GetShared());
}

void CommandBuffer::SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount)
{
	GetRenderAPI().SetVertexBuffers(index, buffers, bufferCount, GetShared());
}

void CommandBuffer::SetIndexBuffer(const SPtr<GpuBuffer>& buffer)
{
	GetRenderAPI().SetIndexBuffer(buffer, GetShared());
}

void CommandBuffer::SetVertexDescription(const SPtr<VertexDescription>& vertexDescription)
{
	GetRenderAPI().SetVertexDescription(vertexDescription, GetShared());
}

void CommandBuffer::SetDrawOperation(DrawOperationType operation)
{
	GetRenderAPI().SetDrawOperation(operation, GetShared());
}

void CommandBuffer::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GetRenderAPI().Draw(vertexOffset, vertexCount, instanceCount, firstInstance, GetShared());
}

void CommandBuffer::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance)
{
	GetRenderAPI().DrawIndexed(startIndex, indexCount, vertexOffset, vertexCount, instanceCount, firstInstance, GetShared());
}

void CommandBuffer::DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
{
	GetRenderAPI().DispatchCompute(groupCountX, groupCountY, groupCountZ, GetShared());
}

void CommandBuffer::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask)
{
	GetRenderAPI().SetRenderTarget(target, readOnlyFlags, loadMask, GetShared());
}

void CommandBuffer::SetViewport(const Rect2& area)
{
	GetRenderAPI().SetViewport(area, GetShared());
}

void CommandBuffer::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetRenderAPI().ClearRenderTarget(buffers, color, depth, stencil, targetMask, GetShared());
}

void CommandBuffer::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetRenderAPI().ClearViewport(buffers, color, depth, stencil, targetMask, GetShared());
}

void CommandBuffer::EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom)
{
	GetRenderAPI().EnableScissorTest(left, top, right, bottom, GetShared());
}

void CommandBuffer::DisableScissorTest()
{
	GetRenderAPI().DisableScissorTest(GetShared());
}

void CommandBuffer::SetStencilReferenceValue(u32 value)
{
	GetRenderAPI().SetStencilRef(value, GetShared());
}

void CommandBuffer::BeginLabel(const StringView& name)
{
	GetRenderAPI().BeginLabel(name);
}

void CommandBuffer::EndLabel()
{
	GetRenderAPI().EndLabel();
}

void CommandBuffer::InsertLabel(const StringView& name)
{
	GetRenderAPI().InsertLabel(name);
}
}}
