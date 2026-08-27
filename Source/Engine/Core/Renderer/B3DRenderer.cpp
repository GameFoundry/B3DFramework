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
#include "GpuBackend/B3DRenderWindow.h"
#include "Scene/B3DSceneManager.h"

using namespace b3d;

namespace b3d { namespace render
{
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

	// Backend frame-boundary work - e.g. Vulkan signals end-of-frame to its submission thread and
	// blocks until the previous frame's resources are safe to reuse.
	mDevice->EndFrame();

	// Advance the context across the frame boundary: recycles its transfer pools and reclaims
	// transient memory (retire each transient allocator's active page, drain everything whose frame
	// is complete). Important this is done after the backend wait above, and before the frame tracker
	// increments so the retired pages are stamped with the frame that used them.
	gpuContext.AdvanceFrame();

	mFrameCompletionTracker.AdvanceFrame();
}

void Renderer::RequestScreenCapture(const TShared<RenderWindow>& window, TAsyncOp<TShared<PixelData>> asyncOp)
{
	if(window == nullptr)
	{
		asyncOp.CompleteOperation(nullptr);
		return;
	}

	for(ScreenCaptureRequest& request : mScreenCaptureRequests)
	{
		if(request.Window != window)
			continue;

		request.Operations.Add(std::move(asyncOp));
		return;
	}

	mScreenCaptureRequests.EmplaceBack(window, std::move(asyncOp));
}

bool Renderer::IsScreenCaptureRequested(const TShared<RenderWindow>& window) const
{
	if(window == nullptr)
		return false;

	for(const ScreenCaptureRequest& request : mScreenCaptureRequests)
	{
		if(request.Window == window)
			return true;
	}

	return false;
}

bool Renderer::ResolveScreenCaptures(GpuCommandBuffer& commandBuffer, const TShared<RenderWindow>& window)
{
	for(u32 requestIndex = 0; requestIndex < mScreenCaptureRequests.Size(); ++requestIndex)
	{
		ScreenCaptureRequest& request = mScreenCaptureRequests[requestIndex];
		if(request.Window != window)
			continue;

		TInlineArray<TAsyncOp<TShared<PixelData>>, 1> captureOps = std::move(request.Operations);
		mScreenCaptureRequests.Erase(mScreenCaptureRequests.Begin() + requestIndex);

		TAsyncOp<TShared<PixelData>> readOp = window->ReadAsync(GetGpuContext(), commandBuffer);
		if(readOp == nullptr)
		{
			for(TAsyncOp<TShared<PixelData>>& captureOp : captureOps)
				captureOp.CompleteOperation(nullptr);

			return false;
		}

		auto fnOnReadOpCompleted = [captureOps = std::move(captureOps), readOp]() mutable
		{
			for(TAsyncOp<TShared<PixelData>>& captureOp : captureOps)
				captureOp.CompleteOperation(readOp.GetReturnValue());
		};

		readOp.DoWhenComplete(std::move(fnOnReadOpCompleted));
		return true;
	}

	return false;
}

void Renderer::ResolveOutstandingScreenCaptures()
{
	while(!mScreenCaptureRequests.Empty())
	{
		const TShared<RenderWindow> window = mScreenCaptureRequests.Front().Window;
		TShared<GpuCommandBuffer> commandBuffer = mCommandBufferPoolRing->GetCurrentPool().Create(GpuCommandBufferCreateInformation::Create("Screen capture"));

		if(ResolveScreenCaptures(*commandBuffer, window))
			GetGpuContext().SubmitCommandBuffer(commandBuffer);
	}
}

void Renderer::InitializeOnRenderThread()
{
	// Borrows the renderer's frame completion tracker; all thread-affine state (transfer pools, transient
	// allocators, parameter set pool) binds lazily on first use, here on the render thread.
	mGpuContext = GpuWorkContext::Create(*mDevice, mFrameCompletionTracker);
}

void Renderer::ActivateOnRenderThread()
{
	GpuCommandBufferPoolCreateInformation createInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
	createInformation.UsePoolReset = true;

	mCommandBufferPoolRing = B3DMakeUnique<GpuCommandBufferPoolRing>(*mDevice, createInformation);

	GpuUniformBufferManager::StartUp();
}

void Renderer::DestroyOnRenderThread()
{
	// Cancel capture requests before the renderer's GPU state is torn down
	for(ScreenCaptureRequest& request : mScreenCaptureRequests)
	{
		for(TAsyncOp<TShared<PixelData>>& operation : request.Operations)
			operation.CompleteOperation(nullptr);
	}

	mScreenCaptureRequests.Clear();

	GpuUniformBufferManager::ShutDown();
	GpuProfiler::Instance().Clear();

	// All GPU work this context drove must have drained before the context (and the transient memory
	// and parameter sets it owns) is torn down. WaitAndReclaim also ensures the completion callbacks of
	// finished command buffers have run, releasing any transient buffers they hold.
	if (mGpuContext != nullptr)
	{
		mGpuContext->WaitAndReclaim();
		mGpuContext = nullptr;
	}

	mCommandBufferPoolRing->Destroy();
	mCommandBufferPoolRing = nullptr;
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

	do
	{
		for(auto& entry : mRunningTasks)
		{
			if(entry->IsCanceled() || entry->IsComplete())
				continue;

			entry->mState.store(1);

			const bool complete = [this, &entry]()
			{
				return entry->mTaskWorker(mCommandBufferPoolRing->GetCurrentPool());
			}();

			if(!complete)
				mRemainingTasks.push_back(entry);
			else
			{
				entry->mState.store(2);
				entry->mTaskWorker = nullptr;
			}
		}

		mRunningTasks.clear();
		std::swap(mRemainingTasks, mRunningTasks);
	}
	while(forceAll && !mRunningTasks.empty());
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
			complete = task.mTaskWorker(mCommandBufferPoolRing->GetCurrentPool());
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

RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool(GpuCommandBufferPool&)> taskWorker)
	: mName(std::move(name)), mTaskWorker(std::move(taskWorker))
{}

TShared<RendererTask> RendererTask::Create(String name, std::function<bool(GpuCommandBufferPool&)> taskWorker)
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
