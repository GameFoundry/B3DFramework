//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraph.h"
#include "B3DFrameGraphCompiler.h"
#include "B3DFrameGraphExecutor.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraph::FrameGraph(GpuDevice& device)
	: mDevice(device)
{
	mCompiler = B3DMakeUnique<FrameGraphCompiler>(*this);
	mExecutor = B3DMakeUnique<FrameGraphExecutor>(*this);
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

void FrameGraph::DeclarePass(
	const StringView& name,
	FrameGraphPassSetupFunc setupFunc,
	FrameGraphPassExecuteFunc executeFunc,
	GpuQueueUsage queue)
{
	u32 passIndex = mNextPassIndex++;

	auto pass = B3DMakeUnique<FrameGraphPass>(passIndex, name, queue, this);
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

	B3D_ENSURE(mExecutor != nullptr);
	mExecutor->Execute(*mCompiledGraph);
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

	// Reset executor
	if (mExecutor)
		mExecutor->Reset();
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
