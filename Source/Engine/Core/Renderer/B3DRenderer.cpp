//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRenderer.h"

#include "B3DGpuUniformBuffer.h"
#include "CoreObject/B3DRenderThread.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DMaterial.h"
#include "Renderer/B3DRendererExtension.h"
#include "Renderer/B3DRendererManager.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Material/B3DShader.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Profiling/B3DProfilerCPU.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "Scene/B3DSceneManager.h"

using namespace b3d;

namespace b3d { namespace render
{
void RendererViewContext::SetAttachments(FrameGraphRasterPass& pass) const
{
	if(Target != nullptr)
	{
		pass.SetRenderTarget(Target, FrameGraphAttachmentLoad::Load);
		return;
	}

	for(u32 colorIndex = 0; colorIndex < ColorAttachments.Size(); colorIndex++)
		pass.SetColorAttachment(colorIndex, ColorAttachments[colorIndex], FrameGraphAttachmentLoad::Load);

	if(DepthStencilAttachment != nullptr)
		pass.SetDepthStencilAttachment(DepthStencilAttachment, DepthStencilMode, FrameGraphAttachmentLoad::Load);
}

void Renderer::Initialize(const TShared<GpuDevice>& gpuDevice)
{
	mDevice = gpuDevice;

	GetRenderThread().PostCommand([this]() { InitializeOnRenderThread(); }, "Renderer::InitializeOnRenderThread");
}

GpuWorkContext& Renderer::GetGpuContext()
{
	B3D_ASSERT(mGpuContext != nullptr && "Renderer has no work context. The renderer must be made active before its work context is used.");
	EnsureRenderThread();

	return *mGpuContext;
}

void Renderer::BeginFrame()
{
	EnsureRenderThread();

	mDevice->BeginFrame();
}

void Renderer::EndFrame()
{
	GpuWorkContext& gpuContext = GetGpuContext(); // Also ensures we are on the render thread.

	// Record this frame's incremental defragmentation copies into the context's transfer command
	// buffer, then flush all pending transfers so they execute within the frame being ended.
	mDevice->RunDefragPass(gpuContext);
	gpuContext.SubmitTransferCommandBuffers();

	// Device frame-boundary work - signals end-of-frame to the submission thread and blocks until the
	// previous frame's resources are safe to reuse.
	mDevice->EndFrame();

	// Advance the context across the frame boundary: recycles its transfer pools and reclaims
	// scratch memory (retire each scratch allocator's active page, drain everything whose frame
	// is complete). Important this is done after the backend wait above, and before the frame tracker
	// increments so the retired pages are stamped with the frame that used them.
	gpuContext.AdvanceFrame();

	mFrameCompletionTracker.AdvanceFrame();
}

void Renderer::InitializeOnRenderThread()
{
	// Borrows the renderer's frame completion tracker; all thread-affine state (transfer pools, scratch
	// allocators, parameter set pool) binds lazily on first use, here on the render thread.
	mGpuContext = GpuWorkContext::Create(*mDevice, mFrameCompletionTracker);
}

void Renderer::ActivateOnRenderThread()
{
	GpuUniformBufferManager::StartUp();
}

void Renderer::DestroyOnRenderThread()
{
	GpuUniformBufferManager::ShutDown();
	GpuProfiler::Instance().Clear();

	// All GPU work this context drove must have drained before the context (and the scratch memory
	// and parameter sets it owns) is torn down. WaitAndReclaim also ensures the completion callbacks of
	// finished command buffers have run, releasing any scratch buffers they hold.
	if (mGpuContext != nullptr)
	{
		mGpuContext->WaitAndReclaim();
		mGpuContext = nullptr;
	}
}

TShared<RendererMeshData> Renderer::CreateMeshDataInternal(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	return B3DMakeSharedFromExisting<RendererMeshData>(new(B3DAllocate<RendererMeshData>())
											   RendererMeshData(numVertices, numIndices, layout, indexType));
}

TShared<RendererMeshData> Renderer::CreateMeshDataInternal(const TShared<MeshData>& meshData)
{
	return B3DMakeSharedFromExisting<RendererMeshData>(new(B3DAllocate<RendererMeshData>())
											   RendererMeshData(meshData));
}

void Renderer::Update()
{
	for(auto& entry : mUnresolvedTasks)
	{
		if(entry->IsComplete())
			entry->OnComplete();
		else if(!entry->IsCanceled())
			mRemainingUnresolvedTasks.push_back(entry);
	}

	mUnresolvedTasks.clear();
	std::swap(mRemainingUnresolvedTasks, mUnresolvedTasks);
}

void Renderer::AddTask(const TShared<RendererTask>& task)
{
	Lock lock(mTaskMutex);

	B3D_ASSERT(task->mState != 1 && "Task is already executing, it cannot be executed again until it finishes.");
	task->mState.store(0); // Reset state in case the task is getting re-queued

	mQueuedTasks.push_back(RendererTaskQueuedInfo(task, GetTime().GetCurrentFrameIndex()));
	mUnresolvedTasks.push_back(task);
}

void Renderer::ProcessTasks(bool forceAll, u64 upToFrame)
{
	// Move all tasks to the render thread queue
	{
		Lock lock(mTaskMutex);

		for(u32 i = 0; i < (u32)mQueuedTasks.size();)
		{
			if(mQueuedTasks[i].FrameIdx <= upToFrame)
			{
				mRunningTasks.push_back(mQueuedTasks[i].Task);
				B3DSwapAndErase(mQueuedTasks, mQueuedTasks.begin() + i);

				continue;
			}

			i++;
		}
	}

	while(!mRunningTasks.empty())
	{
		// Covers the graph's execution, because pass functions may read data the tasks allocated from the frame allocator
		FrameAllocatorScope frameScope;

		// The tasks of the round add their work to one graph
		FrameGraph graph(*mGpuContext, "RendererTasks");

		bool anyTaskRan = false;
		FrameVector<TShared<RendererTask>> completedTasks;
		for(auto& entry : mRunningTasks)
		{
			if(entry->IsCanceled() || entry->IsComplete())
				continue;

			entry->mState.store(1);
			anyTaskRan = true;

			if(entry->mTaskWorker(graph))
				completedTasks.push_back(entry);
			else
				mRemainingTasks.push_back(entry);
		}

		if(anyTaskRan)
		{
			graph.Compile();
			graph.Execute();
		}

		// Once the graph holding their work has executed
		for(auto& entry : completedTasks)
		{
			entry->mState.store(2);
			entry->mTaskWorker = nullptr;
		}

		mRunningTasks.clear();
		std::swap(mRemainingTasks, mRunningTasks);

		if(!forceAll)
			break;
	}
}

void Renderer::ProcessTask(RendererTask& task, bool forceAll)
{
	// Move task to the render thread queue
	{
		Lock lock(mTaskMutex);

		for(u32 i = 0; i < (u32)mQueuedTasks.size(); i++)
		{
			if(mQueuedTasks[i].Task.get() == &task)
			{
				mRunningTasks.push_back(mQueuedTasks[i].Task);
				B3DSwapAndErase(mQueuedTasks, mQueuedTasks.begin() + i);

				break;
			}
		}
	}

	bool complete = task.IsCanceled() || task.IsComplete();
	while(!complete)
	{
		task.mState.store(1);

		GetProfilerCPU().BeginThread("RenderTask");
		{
			FrameAllocatorScope frameScope;
			FrameGraph graph(*mGpuContext, "RendererTasks");
			complete = task.mTaskWorker(graph);

			graph.Compile();
			graph.Execute();
		}
		GetProfilerCPU().EndThread();

		if(complete)
		{
			task.mState.store(2);
			task.mTaskWorker = nullptr;
		}

		if(!forceAll)
			break;
	}
}

TShared<Renderer> GetRenderer()
{
	return std::static_pointer_cast<Renderer>(RendererManager::Instance().GetActive());
}

RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool(FrameGraph&)> taskWorker)
	: mName(std::move(name)), mTaskWorker(std::move(taskWorker))
{}

TShared<RendererTask> RendererTask::Create(String name, std::function<bool(FrameGraph&)> taskWorker)
{
	return B3DMakeShared<RendererTask>(PrivatelyConstruct(), std::move(name), std::move(taskWorker));
}

bool RendererTask::IsComplete() const
{
	return mState.load() == 2;
}

bool RendererTask::IsCanceled() const
{
	return mState.load() == 3;
}

void RendererTask::Wait()
{
	// Task is about to be executed outside of normal rendering workflow. Make sure to manually sync all changes to
	// the render thread first.
	// Note: wait() might only get called during serialization, in which case we might call these methods just once
	// before a level save, instead for every individual component
	CoreObjectManager::Instance().SyncToRenderThread(false);

	auto worker = [this]()
	{
		GetRenderer()->ProcessTask(*this, true);
	};

	GetRenderThread().PostCommand(worker, "RendererTask::Wait", true, mName);

	// Note: Tigger on complete callback and clear it from Renderer?
}

void RendererTask::Cancel()
{
	mState.store(3);
}
}}
