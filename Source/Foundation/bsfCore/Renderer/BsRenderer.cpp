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

	void Renderer::setGlobalShaderOverride(const SPtr<bs::Shader>& shader)
	{
		const Vector<bs::SubShader>& subShaders = shader->getSubShaders();
		
		for(auto& entry : subShaders)
			setGlobalShaderOverride(entry.name, entry.shader);
	}

	bool Renderer::compareCallback(const RendererExtension* a, const RendererExtension* b)
	{
		// Sort by alpha setting first, then by cull mode, then by index
		if (a->getLocation() == b->getLocation())
		{
			if (a->getPriority() == b->getPriority())
				return a > b; // Use address, at this point it doesn't matter, but std::set requires us to differentiate
			else
				return a->getPriority() > b->getPriority();
		}
		else
			return (UINT32)a->getLocation() < (UINT32)b->getLocation();
	}

	void Renderer::update()
	{
		for(auto& entry : mUnresolvedTasks)
		{
			if (entry->isComplete())
				entry->onComplete();
			else if (!entry->isCanceled())
				mRemainingUnresolvedTasks.push_back(entry);
		}

		mUnresolvedTasks.clear();
		std::swap(mRemainingUnresolvedTasks, mUnresolvedTasks);
	}

	void Renderer::addTask(const SPtr<RendererTask>& task)
	{
		Lock lock(mTaskMutex);

		assert(task->mState != 1 && "Task is already executing, it cannot be executed again until it finishes.");
		task->mState.store(0); // Reset state in case the task is getting re-queued

		mQueuedTasks.push_back(task);
		mUnresolvedTasks.push_back(task);
	}

	void Renderer::processTasks(bool forceAll)
	{
		// Move all tasks to the core thread queue
		{
			Lock lock(mTaskMutex);

			mRunningTasks.insert(mRunningTasks.end(), mQueuedTasks.begin(), mQueuedTasks.end());
			mQueuedTasks.clear();
		}

		do
		{
			for (auto& entry : mRunningTasks)
			{
				if (entry->isCanceled() || entry->isComplete())
					continue;

				entry->mState.store(1);

				bool complete;
				
				{
					ProfileGPUBlock sampleBlock("Renderer task: "+ ProfilerString(entry->mName.data(), entry->mName.size()));
					complete = entry->mTaskWorker();
				}

				if (!complete)
					mRemainingTasks.push_back(entry);
				else
					entry->mState.store(2);
			}

			mRunningTasks.clear();
			std::swap(mRemainingTasks, mRunningTasks);
		} while (forceAll && !mRunningTasks.empty());
	}

	void Renderer::processTask(RendererTask& task, bool forceAll)
	{
		// Move all tasks to the core thread queue
		{
			Lock lock(mTaskMutex);

			mRunningTasks.insert(mRunningTasks.end(), mQueuedTasks.begin(), mQueuedTasks.end());
			mQueuedTasks.clear();
		}

		bool complete = task.isCanceled() || task.isComplete();
		while (!complete)
		{
			task.mState.store(1);

			{
				ProfileGPUBlock sampleBlock("Renderer task: " + ProfilerString(task.mName.data(), task.mName.size()));
				complete = task.mTaskWorker();
			}

			if (complete)
				task.mState.store(2);

			if (!forceAll)
				break;
		}
	}

	SPtr<Renderer> gRenderer()
	{
		return std::static_pointer_cast<Renderer>(RendererManager::instance().getActive());
	}

	RendererTask::RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool()> taskWorker) 
		:mName(std::move(name)), mTaskWorker(std::move(taskWorker))
	{ }

	SPtr<RendererTask> RendererTask::create(String name, std::function<bool()> taskWorker)
	{
		return bs_shared_ptr_new<RendererTask>(PrivatelyConstruct(), std::move(name), std::move(taskWorker));
	}

	bool RendererTask::isComplete() const
	{
		return mState.load() == 2;
	}

	bool RendererTask::isCanceled() const
	{
		return mState.load() == 3;
	}

	void RendererTask::wait()
	{
		// Task is about to be executed outside of normal rendering workflow. Make sure to manually sync all changes to
		// the core thread first.
		// Note: wait() might only get called during serialization, in which case we might call these methods just once
		// before a level save, instead for every individual component
		gSceneManager()._updateCoreObjectTransforms();
		CoreObjectManager::instance().syncToCore();

		auto worker = [this]()
		{
			gRenderer()->processTask(*this, true);
		};

		gCoreThread().queueCommand(worker);
		gCoreThread().submit(true);

		// Note: Tigger on complete callback and clear it from Renderer?
	}

	void RendererTask::cancel()
	{
		mState.store(3);
	}
}}
