//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRenderer.h"
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
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Scene/B3DSceneManager.h"

using namespace b3d;

namespace b3d { namespace render
{
Renderer::Renderer()
{}

void Renderer::Initialize(const SPtr<GpuDevice>& gpuDevice)
{
	mDevice = gpuDevice;
}

void Renderer::InitializeOnRenderThread()
{
	GpuCommandBufferPoolCreateInformation createInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
	createInformation.UsePoolReset = true;

	mCommandBufferPoolRing = B3DMakeUnique<GpuCommandBufferPoolRing>(*mDevice, createInformation);

	// Create parameter set pool for render thread allocations
	GpuParameterSetPoolCreateInformation parameterSetPoolCreateInformation;
	parameterSetPoolCreateInformation.Mode = GpuParameterSetPoolMode::Persistent;
	mParameterSetPool = mDevice->CreateParameterSetPool(parameterSetPoolCreateInformation);
}

void Renderer::DestroyOnRenderThread()
{
	GpuProfiler::Instance().Clear();

	mParameterSetPool = nullptr;
	mCommandBufferPoolRing->Destroy();
	mCommandBufferPoolRing = nullptr;
}

SPtr<RendererMeshData> Renderer::CreateMeshDataInternal(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	return B3DMakeSharedFromExisting<RendererMeshData>(new(B3DAllocate<RendererMeshData>())
											   RendererMeshData(numVertices, numIndices, layout, indexType));
}

SPtr<RendererMeshData> Renderer::CreateMeshDataInternal(const SPtr<MeshData>& meshData)
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

void Renderer::AddTask(const SPtr<RendererTask>& task)
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
				entry->mState.store(2);
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
			task.mState.store(2);

		if(!forceAll)
			break;
	}
}

SPtr<Renderer> GetRenderer()
{
	return std::static_pointer_cast<Renderer>(RendererManager::Instance().GetActive());
}

RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool(GpuCommandBufferPool&)> taskWorker)
	: mName(std::move(name)), mTaskWorker(std::move(taskWorker))
{}

SPtr<RendererTask> RendererTask::Create(String name, std::function<bool(GpuCommandBufferPool&)> taskWorker)
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
