//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraph.h"
#include "B3DFrameGraphCompiler.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraph::FrameGraph(GpuDevice& device)
	: mDevice(device)
{
	mCompiler = B3DMakeUnique<FrameGraphCompiler>(*this);

	// Create command buffer pools
	GpuCommandBufferPoolCreateInformation poolCreateInfo =
		GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);

	mGraphicsCommandPool = mDevice.CreateGpuCommandBufferPool(poolCreateInfo);

	poolCreateInfo.Usage = GQT_COMPUTE;
	mComputeCommandPool = mDevice.CreateGpuCommandBufferPool(poolCreateInfo);

	poolCreateInfo.Usage = GQT_TRANSFER;
	mTransferCommandPool = mDevice.CreateGpuCommandBufferPool(poolCreateInfo);
}

FrameGraph::~FrameGraph() = default;

FrameGraphResourceId FrameGraph::ImportTexture(
	const StringView& name,
	const SPtr<Texture>& texture)
{
	if (!B3D_ENSURE(texture != nullptr))
		return kInvalidFrameGraphResourceId;

	FrameGraphResourceId id{mNextResourceId++};

	auto resource = B3DMakeUnique<FrameGraphTextureResource>(id, name, texture);
	mResources.push_back(std::move(resource));

	return id;
}

FrameGraphResourceId FrameGraph::ImportBuffer(
	const StringView& name,
	const SPtr<GpuBuffer>& buffer)
{
	if (!B3D_ENSURE(buffer != nullptr))
		return kInvalidFrameGraphResourceId;

	FrameGraphResourceId id{mNextResourceId++};

	auto resource = B3DMakeUnique<FrameGraphBufferResource>(id, name, buffer);
	mResources.push_back(std::move(resource));

	return id;
}

FrameGraphResourceId FrameGraph::ImportRenderTarget(
	const StringView& name,
	const SPtr<RenderTarget>& renderTarget,
	RenderSurfaceMaskBits surface)
{
	if (!B3D_ENSURE(renderTarget != nullptr))
		return kInvalidFrameGraphResourceId;

	FrameGraphResourceId id{mNextResourceId++};

	auto resource = B3DMakeUnique<FrameGraphRenderTargetResource>(id, name, renderTarget, surface);
	mResources.push_back(std::move(resource));

	B3D_LOG(Info, RenderBackend, "Imported render target '{0}' (surface mask: {1})", name, (u32)surface);

	return id;
}

void FrameGraph::DeclarePass(
	const StringView& name,
	FrameGraphPassSetupFunc setupFunc,
	FrameGraphPassExecuteFunc executeFunc,
	GpuQueueUsage queue)
{
	u32 passIndex = mNextPassIndex++;

	auto pass = B3DMakeUnique<FrameGraphPass>(passIndex, name, queue, this, FrameGraphPassType::Generic);
	pass->SetSetupFunction(std::move(setupFunc));
	pass->SetExecuteFunction(std::move(executeFunc));

	mPasses.push_back(std::move(pass));
}

void FrameGraph::DeclareRenderPass(
	const StringView& name,
	FrameGraphPassSetupFunc setupFunc,
	FrameGraphPassExecuteFunc executeFunc,
	GpuQueueUsage queue)
{
	B3D_ENSURE(queue == GQT_GRAPHICS); // Render passes must be on graphics queue

	u32 passIndex = mNextPassIndex++;

	auto pass = B3DMakeUnique<FrameGraphPass>(passIndex, name, queue, this, FrameGraphPassType::Render);
	pass->SetSetupFunction(std::move(setupFunc));
	pass->SetExecuteFunction(std::move(executeFunc));

	mPasses.push_back(std::move(pass));
}

void FrameGraph::DeclareComputePass(
	const StringView& name,
	FrameGraphPassSetupFunc setupFunc,
	FrameGraphPassExecuteFunc executeFunc)
{
	u32 passIndex = mNextPassIndex++;

	auto pass = B3DMakeUnique<FrameGraphPass>(passIndex, name, GQT_COMPUTE, this, FrameGraphPassType::Compute);
	pass->SetSetupFunction(std::move(setupFunc));
	pass->SetExecuteFunction(std::move(executeFunc));

	mPasses.push_back(std::move(pass));
}

void FrameGraph::Compile()
{
	// Validate that we have passes or resources to compile
	if (mPasses.empty() && mResources.empty())
	{
		B3D_LOG(Warning, RenderBackend, "Compiling empty frame graph (no passes or resources)");
	}

	// Check for duplicate compilation without reset
	if (mCompiledGraph != nullptr)
	{
		B3D_LOG(Warning, RenderBackend,
			"Frame graph compiled multiple times without Reset(). Previous compilation will be discarded.");
	}

	B3D_ENSURE(mCompiler != nullptr);
	mCompiledGraph = mCompiler->Compile();

	// Validation failed - log error
	if (mCompiledGraph == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Frame graph compilation failed due to validation errors");
	}
}

void FrameGraph::Execute()
{
	// Validate that graph has been compiled
	if (!B3D_ENSURE(mCompiledGraph != nullptr))
	{
		B3D_LOG(Error, RenderBackend, "Cannot execute frame graph: not compiled. Call Compile() first.");
		return;
	}

	// Validate that we have passes to execute
	if (mCompiledGraph->SortedPasses.empty())
	{
		B3D_LOG(Warning, RenderBackend, "Executing frame graph with no passes");
		return;
	}

	B3D_LOG(Info, RenderBackend, "Executing frame graph with {0} passes",
		mCompiledGraph->SortedPasses.size());

	// Build a map from pass to barrier batches
	UnorderedMap<FrameGraphPass*, Vector<const FrameGraphBarrierBatch*>> passBarriers;
	for (const auto& batch : mCompiledGraph->BarrierBatches)
	{
		passBarriers[batch.DestinationPass].push_back(&batch);
	}

	// Execute passes in sorted order, batching by queue
	SPtr<GpuCommandBuffer> currentCmd;
	GpuQueueUsage currentQueue = GQT_GRAPHICS;
	bool hasActiveCmd = false;

	auto fnSubmitCurrentCmd = [&]()
	{
		if (hasActiveCmd && currentCmd)
		{
			currentCmd->End();

			SPtr<GpuQueue> queue = mDevice.GetQueue(currentQueue, 0);
			if (queue)
			{
				B3D_LOG(Info, RenderBackend, "Submitting command buffer for queue {0}",
					static_cast<u32>(currentQueue));
				queue->SubmitCommandBuffer(currentCmd);
			}
			else
			{
				B3D_LOG(Error, RenderBackend, "Failed to get queue for type {0}",
					static_cast<u32>(currentQueue));
			}

			hasActiveCmd = false;
			currentCmd = nullptr;
		}
	};

	for (FrameGraphPass* pass : mCompiledGraph->SortedPasses)
	{
		B3D_LOG(Info, RenderBackend, "Executing pass: {0} (type: {1})",
			pass->GetName(),
			pass->GetPassType() == FrameGraphPassType::Render ? "Render" :
			pass->GetPassType() == FrameGraphPassType::Compute ? "Compute" : "Generic");

		// If queue changed, submit current command buffer and start a new one
		if (hasActiveCmd && pass->GetQueue() != currentQueue)
		{
			fnSubmitCurrentCmd();
		}

		// Create new command buffer if needed
		if (!hasActiveCmd)
		{
			currentQueue = pass->GetQueue();
			SPtr<GpuCommandBufferPool> pool = GetPoolForQueue(currentQueue);

			GpuCommandBufferCreateInformation cmdCreateInfo =
				GpuCommandBufferCreateInformation::Create("FrameGraph");
			currentCmd = pool->FindOrCreate(cmdCreateInfo);

			hasActiveCmd = true;
		}

		// Issue barriers before this pass
		auto barrierIt = passBarriers.find(pass);
		if (barrierIt != passBarriers.end() && !barrierIt->second.empty())
		{
			for (const FrameGraphBarrierBatch* batch : barrierIt->second)
			{
				B3D_LOG(Info, RenderBackend,
					"Issuing {0} buffer barriers and {1} texture barriers before pass '{2}'",
					batch->Barriers.BufferBarriers.Size(),
					batch->Barriers.TextureBarriers.Size(),
					pass->GetName());

				currentCmd->IssueBarriers(batch->Barriers);
			}
		}

		// Handle render passes
		bool isRenderPass = pass->GetPassType() == FrameGraphPassType::Render;
		SPtr<RenderTarget> renderTarget;

		if (isRenderPass)
		{
			// Get pre-created render target
			auto rtIt = mCompiledGraph->RenderTargets.find(pass);
			if (rtIt == mCompiledGraph->RenderTargets.end())
			{
				B3D_LOG(Error, RenderBackend, "Render target not found for render pass '{0}'",
					pass->GetName());
				continue;
			}

			renderTarget = rtIt->second;

			// Begin render pass
			B3D_LOG(Info, RenderBackend, "Beginning render pass for '{0}'", pass->GetName());
			currentCmd->BeginRenderPass(renderTarget);
		}

		// Execute the pass
		pass->ExecuteCommands(*currentCmd);

		// End render pass if needed
		if (isRenderPass)
		{
			B3D_LOG(Info, RenderBackend, "Ending render pass for '{0}'", pass->GetName());
			currentCmd->EndRenderPass();
		}
	}

	// Submit final command buffer
	fnSubmitCurrentCmd();

	B3D_LOG(Info, RenderBackend, "Frame graph execution complete");
}

void FrameGraph::Reset()
{
	// Reset all passes before clearing (releases render targets, clears resource accesses)
	for (auto& pass : mPasses)
	{
		if (pass)
			pass->Reset();
	}

	// Clear all data structures
	mResources.clear();
	mPasses.clear();
	mCompiledGraph.reset();
	mOutputResources.clear();

	// Reset ID counters
	mNextResourceId = 0;
	mNextPassIndex = 0;
}

void FrameGraph::MarkAsOutput(FrameGraphResourceId resource)
{
	B3D_ENSURE(resource.IsValid());
	mOutputResources.insert(resource);
	B3D_LOG(Info, RenderBackend, "Marked resource {0} as output", resource.Index);
}

FrameGraphResource* FrameGraph::GetResource(FrameGraphResourceId id) const
{
	if (!id.IsValid() || id.Index >= mResources.size())
		return nullptr;

	return mResources[id.Index].get();
}

UnorderedMap<FrameGraphResourceId, SPtr<render::Texture>> FrameGraph::GetImportedTextures() const
{
	UnorderedMap<FrameGraphResourceId, SPtr<Texture>> textures;

	for (const auto& resource : mResources)
	{
		if (resource->GetType() == FrameGraphResourceType::Texture)
		{
			auto* textureResource = static_cast<FrameGraphTextureResource*>(resource.get());
			textures[resource->GetId()] = textureResource->GetTexture();
		}
	}

	return textures;
}

void FrameGraph::ExecutePass(FrameGraphPass* pass)
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

	// Execute user lambda - this may call BeginRenderPass/EndRenderPass
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

SPtr<GpuQueue> FrameGraph::GetQueueForPass(FrameGraphPass* pass)
{
	return mDevice.GetQueue(pass->GetQueue(), 0);
}

SPtr<GpuCommandBufferPool> FrameGraph::GetPoolForQueue(GpuQueueUsage queueType)
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
