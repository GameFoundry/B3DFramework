//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphExecutor.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphExecutor::FrameGraphExecutor(FrameGraph& frameGraph)
	: mFrameGraph(frameGraph)
{
	// Create command buffer pools
	GpuCommandBufferPoolCreateInformation poolCreateInfo =
		GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);

	mGraphicsCommandPool = mFrameGraph.GetDevice().CreateGpuCommandBufferPool(poolCreateInfo);

	poolCreateInfo.Usage = GQT_COMPUTE;
	mComputeCommandPool = mFrameGraph.GetDevice().CreateGpuCommandBufferPool(poolCreateInfo);

	poolCreateInfo.Usage = GQT_TRANSFER;
	mTransferCommandPool = mFrameGraph.GetDevice().CreateGpuCommandBufferPool(poolCreateInfo);
}

FrameGraphExecutor::~FrameGraphExecutor() = default;

void FrameGraphExecutor::Execute(const CompiledFrameGraph& compiledGraph)
{
	for (FrameGraphPass* pass : compiledGraph.Passes)
	{
		ExecutePass(pass);
	}
}

void FrameGraphExecutor::ExecutePass(FrameGraphPass* pass)
{
	B3D_ENSURE(pass != nullptr);

	// Get command buffer pool for this queue
	SPtr<GpuCommandBufferPool> pool = GetPoolForQueue(pass->GetQueue());
	if (pool == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Pass '{0}': Unsupported queue type {1}",
			pass->GetName(), static_cast<u32>(pass->GetQueue()));
		return;
	}

	// Get/create command buffer (Begin() is called automatically by FindOrCreate)
	GpuCommandBufferCreateInformation cmdCreateInfo =
		GpuCommandBufferCreateInformation::Create(pass->GetName());

	SPtr<GpuCommandBuffer> cmd = pool->FindOrCreate(cmdCreateInfo);
	if (!B3D_ENSURE(cmd != nullptr))
	{
		B3D_LOG(Error, RenderBackend, "Pass '{0}': Failed to create command buffer", pass->GetName());
		return;
	}

	// Phase 1: No barriers, no resource allocation
	// Later phases will inject barriers here based on resource dependencies

	// Execute user lambda - this may call BeginRenderPass/EndRenderPass // TODO - Begin/End render pass calling should be done by the FrameGraph system itself
	pass->ExecuteCommands(*cmd);

	// End command buffer recording
	cmd->End();

	// Submit to appropriate queue
	SPtr<GpuQueue> queue = GetQueueForPass(pass);
	if (!B3D_ENSURE(queue != nullptr))
	{
		B3D_LOG(Error, RenderBackend, "Pass '{0}': Failed to get queue for type {1}",
			pass->GetName(), static_cast<u32>(pass->GetQueue()));
		return;
	}

	queue->SubmitCommandBuffer(cmd);
}

SPtr<GpuQueue> FrameGraphExecutor::GetQueueForPass(FrameGraphPass* pass)
{
	return mFrameGraph.GetDevice().GetQueue(pass->GetQueue(), 0);
}

SPtr<GpuCommandBufferPool> FrameGraphExecutor::GetPoolForQueue(GpuQueueUsage queueType)
{
	switch (queueType)
	{
		case GQT_GRAPHICS:
			return mGraphicsCommandPool;
		case GQT_COMPUTE:
			return mComputeCommandPool;
		case GQT_TRANSFER:
			return mTransferCommandPool;
		default:
			return nullptr;
	}
}

void FrameGraphExecutor::Reset()
{
	// Phase 1: Nothing to reset
	// Later: Reset resource pools
}
