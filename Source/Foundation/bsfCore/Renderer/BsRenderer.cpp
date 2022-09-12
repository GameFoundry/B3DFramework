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

namespace bs { namespace ct
{
	Renderer::Renderer()
		:mCallbacks(&compareCallback)
	{ }

	SPtr<RendererMeshData> Renderer::_createMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return bs_shared_ptr<RendererMeshData>(new (bs_alloc<RendererMeshData>())
			RendererMeshData(numVertices, numIndices, layout, indexType));
	}

	SPtr<RendererMeshData> Renderer::_createMeshData(const SPtr<MeshData>& meshData)
	{
		return bs_shared_ptr<RendererMeshData>(new (bs_alloc<RendererMeshData>())
			RendererMeshData(meshData));
	}

	void Renderer::SetGlobalShaderOverride(const SPtr<bs::Shader>& shader)
	{
		const Vector<bs::SubShader>& subShaders = shader->GetSubShaders();
		
		for(auto& entry : subShaders)
			setGlobalShaderOverride(entry.name, entry.shader);
	}

	bool Renderer::CompareCallback(const RendererExtension* a, const RendererExtension* b)
	{
		// Sort by alpha setting first, then by cull mode, then by index
		if (a->GetLocation() == b->getLocation())
		{
			if (a->GetPriority() == b->getPriority())
				return a > b; // Use address, at this point it doesn't matter, but std::set requires us to differentiate
			else
				return a->GetPriority() > b->getPriority();
		}
		else
			return (UINT32)a->GetLocation() < (UINT32)b->getLocation();
	}

	void Renderer::Update()
	{
		for(auto& entry : mUnresolvedTasks)
		{
			if (entry->IsComplete())
				entry->OnComplete();
			else if (!entry->IsCanceled())
				mRemainingUnresolvedTasks.push_back(entry);
		}

		mUnresolvedTasks.Clear();
		std::swap(mRemainingUnresolvedTasks, mUnresolvedTasks);
	}

	void Renderer::AddTask(const SPtr<RendererTask>& task)
	{
		Lock Lock(mTaskMutex);

		assert(task->mState != 1 && "Task is already executing, it cannot be executed again until it finishes.");
		task->mState.Store(0); // Reset state in case the task is getting re-queued

		mQueuedTasks.push_back(RendererTaskQueuedInfo(task, gTime().GetFrameIdx()));
		mUnresolvedTasks.push_back(task);
	}

	void Renderer::ProcessTasks(bool forceAll, UINT64 upToFrame)
	{
		// Move all tasks to the core thread queue
		{
			Lock Lock(mTaskMutex);

			for(UINT32 i = 0; i < (UINT32)mQueuedTasks.Size();)
			{
				if(mQueuedTasks[i].frameIdx <= upToFrame)
				{
					mRunningTasks.push_back(mQueuedTasks[i].task);
					bs_swap_and_erase(mQueuedTasks, mQueuedTasks.Begin() + i);
					
					continue;
				}

				i++;
			}
		}

		do
		{
			for (auto& entry : mRunningTasks)
			{
				if (entry->IsCanceled() || entry->isComplete())
					continue;

				entry->mState.Store(1);

				const bool complete = [&entry]()
				{
					ProfileGPUBlock SampleBlock("Renderer task: " + ProfilerString(entry->mName.Data(), entry->mName.size()));
					return entry->MTaskWorker();
				}();

				if (!complete)
					mRemainingTasks.push_back(entry);
				else
					entry->mState.Store(2);
			}

			mRunningTasks.Clear();
			std::swap(mRemainingTasks, mRunningTasks);
		} while (forceAll && !mRunningTasks.Empty());
	}

	void Renderer::ProcessTask(RendererTask& task, bool forceAll)
	{
		// Move task to the core thread queue
		{
			Lock Lock(mTaskMutex);

			for(UINT32 i = 0; i < (UINT32)mQueuedTasks.Size(); i++)
			{
				if(mQueuedTasks[i].task.Get() == &task)
				{
					mRunningTasks.push_back(mQueuedTasks[i].task);
					bs_swap_and_erase(mQueuedTasks, mQueuedTasks.Begin() + i);

					break;
				}
			}
		}

		bool complete = task.IsCanceled() || task.isComplete();
		while (!complete)
		{
			task.mState.Store(1);

			gProfilerGPU().BeginFrame();
			gProfilerCPU().BeginThread("RenderTask");
			{
				ProfileGPUBlock SampleBlock("Renderer task: " + ProfilerString(task.mName.Data(), task.mName.size()));
				complete = task.MTaskWorker();
			}
			gProfilerCPU().EndThread();
			gProfilerGPU().EndFrame(true);

			if (complete)
				task.mState.Store(2);

			if (!forceAll)
				break;
		}
	}

	SPtr<Renderer> GRenderer()
	{
		return std::static_pointer_cast<Renderer>(RendererManager::instance().GetActive());
	}

	RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool()> taskWorker)
		:mName(std::move(name)), mTaskWorker(std::move(taskWorker))
	{ }

	SPtr<RendererTask> RendererTask::Create(String name, std::function<bool()> taskWorker)
	{
		return bs_shared_ptr_new<RendererTask>(PrivatelyConstruct(), std::move(name), std::move(taskWorker));
	}

	bool RendererTask::IsComplete() const
	{
		return mState.Load() == 2;
	}

	bool RendererTask::IsCanceled() const
	{
		return mState.Load() == 3;
	}

	void RendererTask::Wait()
	{
		// Task is about to be executed outside of normal rendering workflow. Make sure to manually sync all changes to
		// the core thread first.
		// Note: wait() might only get called during serialization, in which case we might call these methods just once
		// before a level save, instead for every individual component
		gSceneManager()._updateCoreObjectTransforms();
		CoreObjectManager::instance().SyncToCore();

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
		mState.Store(3);
	}
}}
