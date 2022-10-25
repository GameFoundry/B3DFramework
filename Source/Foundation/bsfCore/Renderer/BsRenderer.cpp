//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderer.h"
#include "CoreThread/BsCoreThread.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Mesh/BsMesh.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsRendererExtension.h"
#include "Renderer/BsRendererManager.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "Scene/BsSceneManager.h"
#include "Material/BsShader.h"
#include "Profiling/BsProfilerGPU.h"
#include "Profiling/BsProfilerCPU.h"

using namespace bs;

namespace bs { namespace ct
{
Renderer::Renderer()
	: mCallbacks(&CompareCallback)
{}

SPtr<RendererMeshData> Renderer::CreateMeshDataInternal(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType)
{
	return bs_shared_ptr<RendererMeshData>(new(bs_alloc<RendererMeshData>())
											   RendererMeshData(numVertices, numIndices, layout, indexType));
}

SPtr<RendererMeshData> Renderer::CreateMeshDataInternal(const SPtr<MeshData>& meshData)
{
	return bs_shared_ptr<RendererMeshData>(new(bs_alloc<RendererMeshData>())
											   RendererMeshData(meshData));
}

void Renderer::SetGlobalShaderOverride(const SPtr<bs::Shader>& shader)
{
	const Vector<bs::SubShader>& subShaders = shader->GetSubShaders();

	for(auto& entry : subShaders)
		SetGlobalShaderOverride(entry.Name, entry.Shader);
}

bool Renderer::CompareCallback(const RendererExtension* a, const RendererExtension* b)
{
	// Sort by alpha setting first, then by cull mode, then by index
	if(a->GetLocation() == b->GetLocation())
	{
		if(a->GetPriority() == b->GetPriority())
			return a > b; // Use address, at this point it doesn't matter, but std::set requires us to differentiate
		else
			return a->GetPriority() > b->GetPriority();
	}
	else
		return (u32)a->GetLocation() < (u32)b->GetLocation();
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

	assert(task->mState != 1 && "Task is already executing, it cannot be executed again until it finishes.");
	task->mState.store(0); // Reset state in case the task is getting re-queued

	mQueuedTasks.push_back(RendererTaskQueuedInfo(task, gTime().GetFrameIdx()));
	mUnresolvedTasks.push_back(task);
}

void Renderer::ProcessTasks(bool forceAll, u64 upToFrame)
{
	// Move all tasks to the core thread queue
	{
		Lock lock(mTaskMutex);

		for(u32 i = 0; i < (u32)mQueuedTasks.size();)
		{
			if(mQueuedTasks[i].FrameIdx <= upToFrame)
			{
				mRunningTasks.push_back(mQueuedTasks[i].Task);
				bs_swap_and_erase(mQueuedTasks, mQueuedTasks.begin() + i);

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

			const bool complete = [&entry]()
			{
				ProfileGPUBlock sampleBlock("Renderer task: " + ProfilerString(entry->mName.data(), entry->mName.size()));
				return entry->mTaskWorker();
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
	// Move task to the core thread queue
	{
		Lock lock(mTaskMutex);

		for(u32 i = 0; i < (u32)mQueuedTasks.size(); i++)
		{
			if(mQueuedTasks[i].Task.get() == &task)
			{
				mRunningTasks.push_back(mQueuedTasks[i].Task);
				bs_swap_and_erase(mQueuedTasks, mQueuedTasks.begin() + i);

				break;
			}
		}
	}

	bool complete = task.IsCanceled() || task.IsComplete();
	while(!complete)
	{
		task.mState.store(1);

		gProfilerGPU().BeginFrame();
		gProfilerCPU().BeginThread("RenderTask");
		{
			ProfileGPUBlock sampleBlock("Renderer task: " + ProfilerString(task.mName.data(), task.mName.size()));
			complete = task.mTaskWorker();
		}
		gProfilerCPU().EndThread();
		gProfilerGPU().EndFrame(true);

		if(complete)
			task.mState.store(2);

		if(!forceAll)
			break;
	}
}

SPtr<Renderer> gRenderer()
{
	return std::static_pointer_cast<Renderer>(RendererManager::Instance().GetActive());
}

RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool()> taskWorker)
	: mName(std::move(name)), mTaskWorker(std::move(taskWorker))
{}

SPtr<RendererTask> RendererTask::Create(String name, std::function<bool()> taskWorker)
{
	return bs_shared_ptr_new<RendererTask>(PrivatelyConstruct(), std::move(name), std::move(taskWorker));
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
	// the core thread first.
	// Note: wait() might only get called during serialization, in which case we might call these methods just once
	// before a level save, instead for every individual component
	gSceneManager().UpdateCoreObjectTransformsInternal();
	CoreObjectManager::Instance().SyncToCore();

	auto worker = [this]()
	{
		gRenderer()->ProcessTask(*this, true);
	};

	gCoreThread().QueueCommand(worker);
	gCoreThread().Submit(true);

	// Note: Tigger on complete callback and clear it from Renderer?
}

void RendererTask::Cancel()
{
	mState.store(3);
}
}}
