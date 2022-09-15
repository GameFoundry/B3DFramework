//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleSystem.h"
#include "Utility/BsTime.h"
#include "Threading/BsTaskScheduler.h"
#include "Allocators/BsPoolAlloc.h"
#include "Private/Particles/BsParticleSet.h"
#include "Animation/BsAnimationManager.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	/** Helper method used for writing particle data into the @p pixels buffer. */
	template<class T, class PR>
	void iterateOverPixels(PixelData& pixels, UINT32 count, UINT32 stride, PR predicate)
	{
		auto dest = (UINT8*)pixels.GetData();

		UINT32 x = 0;
		for (UINT32 i = 0; i < count; i++)
		{
			predicate((T*)dest, i);

			dest += stride;
			x++;

			if (x >= pixels.GetWidth())
			{
				x = 0;
				dest += pixels.GetRowSkip();
			}
		}
	}

	/** Helper method used for writing particle data into the @p pixels buffer. */
	template<class T, class PR>
	void iterateOverPixels(PixelData& pixels, UINT32 count, PR predicate)
	{
		iterateOverPixels<T>(pixels, count, sizeof(T), predicate);
	}

	/**
	 * Maintains a pool of buffers that are used for passing results of particle simulation from the simulation to the
	 * core thread.
	 */
	class ParticleSimulationDataPool
	{
		/** Contains a list of buffers for the specified size. */
		struct BuffersPerSize
		{
			Vector<ParticleRenderData*> buffers;
			UINT32 nextFreeIdx = 0;
		};

	public:
		~ParticleSimulationDataPool()
		{
			Lock lock(mMutex);

			for (auto& sizeEntry : mBillboardBufferList)
			{
				for (auto& entry : sizeEntry.second.buffers)
					mBillboardAlloc.Destruct(static_cast<ParticleBillboardRenderData*>(entry));
			}

			for (auto& sizeEntry : mMeshBufferList)
			{
				for (auto& entry : sizeEntry.second.buffers)
					mMeshAlloc.Destruct(static_cast<ParticleMeshRenderData*>(entry));
			}

			for (auto& entry : mGPUBufferList)
				mGPUAlloc.Destruct(entry);
		}

		/**
		 * Returns a set of buffers containing particle data from the provided particle set. Usable for rendering the
		 * results of the CPU particle simulation as billboards.
		 */
		ParticleBillboardRenderData* AllocCpuBillboard(const ParticleSet& particleSet)
		{
			const UINT32 size = particleSet.DetermineTextureSize();

			ParticleBillboardRenderData* output = nullptr;

			{
				Lock lock(mMutex);

				BuffersPerSize& buffers = mBillboardBufferList[size];
				if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
				{
					output = static_cast<ParticleBillboardRenderData*>(buffers.buffers[buffers.nextFreeIdx]);
					buffers.nextFreeIdx++;
				}
			}

			if (!output)
			{
				output = CreateNewBillboardBuffersCpu(size);

				Lock lock(mMutex);

				BuffersPerSize& buffers = mBillboardBufferList[size];
				buffers.buffers.push_back(output);
				buffers.nextFreeIdx++;
			}

			// Populate buffer contents
			const UINT32 count = particleSet.GetParticleCount();
			const ParticleSetData& particles = particleSet.GetParticles();

			// TODO: Use non-temporal writes?
			iterateOverPixels<Vector4>(output->positionAndRotation, count,
				[&particles](Vector4* dst, UINT32 idx)
			{
				dst->x = particles.position[idx].x;
				dst->y = particles.position[idx].y;
				dst->z = particles.position[idx].z;
				dst->w = particles.rotation[idx].x * Math::DEG2RAD;

			});

			iterateOverPixels<RGBA>(output->color, count,
				[&particles](RGBA* dst, UINT32 idx)
			{
				*dst = particles.color[idx];
			});

			iterateOverPixels<UINT16>(output->sizeAndFrameIdx, count, sizeof(UINT16) * 4,
				[&particles](UINT16* dst, UINT32 idx)
			{
				dst[0] = Bitwise::FloatToHalf(particles.size[idx].x);
				dst[1] = Bitwise::FloatToHalf(particles.size[idx].y);
				dst[2] = Bitwise::FloatToHalf(particles.frame[idx]);
			});

			output->indices.clear();
			output->indices.resize(count);

			return output;
		}

		/**
		 * Returns a set of buffers containing particle data from the provided particle set. Usable for rendering the
		 * results of the CPU particle simulation as 3D meshes.
		 */
		ParticleMeshRenderData* AllocCpuMesh(const ParticleSet& particleSet)
		{
			const UINT32 size = particleSet.DetermineTextureSize();

			ParticleMeshRenderData* output = nullptr;

			{
				Lock lock(mMutex);

				BuffersPerSize& buffers = mMeshBufferList[size];
				if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
				{
					output = static_cast<ParticleMeshRenderData*>(buffers.buffers[buffers.nextFreeIdx]);
					buffers.nextFreeIdx++;
				}
			}

			if (!output)
			{
				output = CreateNewMeshBuffersCpu(size);

				Lock lock(mMutex);

				BuffersPerSize& buffers = mMeshBufferList[size];
				buffers.buffers.push_back(output);
				buffers.nextFreeIdx++;
			}

			// Populate buffer contents
			const UINT32 count = particleSet.GetParticleCount();
			const ParticleSetData& particles = particleSet.GetParticles();

			// TODO: Use non-temporal writes?
			iterateOverPixels<Vector4>(output->position, count,
				[&particles](Vector4* dst, UINT32 idx)
			{
				dst->x = particles.position[idx].x;
				dst->y = particles.position[idx].y;
				dst->z = particles.position[idx].z;

			});

			iterateOverPixels<RGBA>(output->color, count,
				[&particles](RGBA* dst, UINT32 idx)
			{
				*dst = particles.color[idx];
			});

			iterateOverPixels<UINT16>(output->rotation, count, sizeof(UINT16) * 4,
				[&particles](UINT16* dst, UINT32 idx)
			{
				dst[0] = Bitwise::FloatToHalf(particles.rotation[idx].x * Math::DEG2RAD);
				dst[1] = Bitwise::FloatToHalf(particles.rotation[idx].y * Math::DEG2RAD);
				dst[2] = Bitwise::FloatToHalf(particles.rotation[idx].z * Math::DEG2RAD);
			});

			iterateOverPixels<UINT16>(output->size, count, sizeof(UINT16) * 4,
				[&particles](UINT16* dst, UINT32 idx)
			{
				dst[0] = Bitwise::FloatToHalf(particles.size[idx].x);
				dst[1] = Bitwise::FloatToHalf(particles.size[idx].y);
				dst[2] = Bitwise::FloatToHalf(particles.size[idx].z);
			});

			output->indices.clear();
			output->indices.resize(count);

			return output;
		}

		/**
		 * Returns a list of particles from the provided particle set that may be used for inserting the particles into the
		 * GPU simulation.
		 */
		ParticleGPUSimulationData* AllocGpu(const ParticleSet& particleSet)
		{
			ParticleGPUSimulationData* output = nullptr;

			{
				Lock lock(mMutex);

				if (mNextFreeGPUBuffer < (UINT32)mGPUBufferList.size())
				{
					output = mGPUBufferList[mNextFreeGPUBuffer];
					mNextFreeGPUBuffer++;
				}
			}

			if (!output)
			{
				output = CreateNewBuffersGpu();

				Lock lock(mMutex);

				mGPUBufferList.push_back(output);
				mNextFreeGPUBuffer++;
			}

			// Populate buffer contents
			const UINT32 count = particleSet.GetParticleCount();
			const ParticleSetData& particles = particleSet.GetParticles();

			output->particles.clear();
			output->particles.resize(count);

			// TODO: Use non-temporal writes?
			for (UINT32 i = 0; i < count; i++)
			{
				GpuParticle particle;
				particle.position = particles.position[i];
				particle.lifetime = particles.lifetime[i];
				particle.initialLifetime =  particles.initialLifetime[i];
				particle.velocity = particles.velocity[i];
				particle.size = Vector2(particles.size[i].x, particles.size[i].z);
				particle.rotation = particles.rotation[i].z;

				output->particles[i] = particle;
			}

			return output;
		}

		/** Makes all the buffers available for allocations. Does not free internal buffer memory. */
		void Clear()
		{
			Lock lock(mMutex);

			for(auto& buffers : mBillboardBufferList)
				buffers.second.nextFreeIdx = 0;

			for(auto& buffers : mMeshBufferList)
				buffers.second.nextFreeIdx = 0;

			mNextFreeGPUBuffer = 0;
		}

	private:
		/** Allocates a new set of CPU buffers used for billboard rendering of the provided @p size width and height. */
		ParticleBillboardRenderData* CreateNewBillboardBuffersCpu(UINT32 size)
		{
			auto output = mBillboardAlloc.Construct<ParticleBillboardRenderData>();

			output->positionAndRotation = PixelData(size, size, 1, PF_RGBA32F);
			output->color = PixelData(size, size, 1, PF_RGBA8);
			output->sizeAndFrameIdx = PixelData(size, size, 1, PF_RGBA16F);

			// Note: Potentially allocate them all in one large block
			output->positionAndRotation.AllocateInternalBuffer();
			output->color.AllocateInternalBuffer();
			output->sizeAndFrameIdx.AllocateInternalBuffer();

			return output;
		}

		/** Allocates a new set of CPU buffers used for mesh rendering of the provided @p size width and height. */
		ParticleMeshRenderData* CreateNewMeshBuffersCpu(UINT32 size)
		{
			auto output = mMeshAlloc.Construct<ParticleMeshRenderData>();

			output->position = PixelData(size, size, 1, PF_RGBA32F);
			output->color = PixelData(size, size, 1, PF_RGBA8);
			output->size = PixelData(size, size, 1, PF_RGBA16F);
			output->rotation = PixelData(size, size, 1, PF_RGBA16F);

			// Note: Potentially allocate them all in one large block
			output->position.AllocateInternalBuffer();
			output->color.AllocateInternalBuffer();
			output->size.AllocateInternalBuffer();
			output->rotation.AllocateInternalBuffer();

			return output;
		}

		/** Allocates a new set of GPU buffers of the provided @p size width and height. */
		ParticleGPUSimulationData* CreateNewBuffersGpu()
		{
			return mGPUAlloc.Construct<ParticleGPUSimulationData>();
		}

		UnorderedMap<UINT32, BuffersPerSize> mBillboardBufferList;
		UnorderedMap<UINT32, BuffersPerSize> mMeshBufferList;
		Vector<ParticleGPUSimulationData*> mGPUBufferList;
		UINT32 mNextFreeGPUBuffer = 0;

		PoolAlloc<sizeof(ParticleBillboardRenderData), 32, 4, true> mBillboardAlloc;
		PoolAlloc<sizeof(ParticleMeshRenderData), 32, 4, true> mMeshAlloc;
		PoolAlloc<sizeof(ParticleGPUSimulationData), 32, 4, true> mGPUAlloc;
		Mutex mMutex;
	};

	struct ParticleManager::Members
	{
		// TODO - Perhaps sharing one pool is better
		ParticleSimulationDataPool simDataPool[CoreThread::NUM_SYNC_BUFFERS];
	};

	ParticleManager::ParticleManager()
		:m(bs_new<Members>())
	{ }

	ParticleManager::~ParticleManager()
	{
		bs_delete(m);
	}

	ParticlePerFrameData* ParticleManager::Update(const EvaluatedAnimationData& animData)
	{
		// Note: Allow the worker threads to work alongside the main thread? Would require extra synchronization but
		// potentially no benefit?

		// Advance the buffers (last write buffer becomes read buffer)
		if (mSwapBuffers)
		{
			mReadBufferIdx = (mReadBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;
			mWriteBufferIdx = (mWriteBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;

			mSwapBuffers = false;
		}

		if(mPaused)
			return &mSimulationData[mReadBufferIdx];

		// TODO - Perform culling (but only on deterministic particle systems). In which case cache the bounds so we don't
		// need to recalculate them below

		// Prepare the write buffer
		ParticlePerFrameData& simulationData = mSimulationData[mWriteBufferIdx];
		simulationData.cpuData.clear();
		simulationData.gpuData.clear();

		// Queue evaluation tasks
		{
			Lock lock(mMutex);
			mNumActiveWorkers = (UINT32)mSystems.size();
		}

		float timeDelta = gTime().GetFrameDelta();

		ParticleSimulationDataPool& simDataPool = m->simDataPool[mWriteBufferIdx];
		simDataPool.Clear();

		for (auto& system : mSystems)
		{
			const auto evaluateWorker = [this, timeDelta, system, &animData, &simDataPool, &simulationData]()
			{
				// Advance the simulation
				system->SimulateInternal(timeDelta, &animData);

				ParticleRenderData* simulationDataCPU = nullptr;
				ParticleGPUSimulationData* simulationDataGPU = nullptr;
				if(system->mParticleSet)
				{
					// Generate simulation data to transfer to the core thread
					const UINT32 numParticles = system->mParticleSet->GetParticleCount();
					const ParticleSystemSettings& settings = system->GetSettings();

					if(settings.gpuSimulation)
						simulationDataGPU = simDataPool.AllocGpu(*system->mParticleSet);
					else
					{
						if(settings.renderMode == ParticleRenderMode::Billboard)
							simulationDataCPU = simDataPool.AllocCpuBillboard(*system->mParticleSet);
						else
							simulationDataCPU = simDataPool.AllocCpuMesh(*system->mParticleSet);

						simulationDataCPU->numParticles = numParticles;

						if(settings.useAutomaticBounds)
							simulationDataCPU->bounds = system->CalculateBoundsInternal();
						else
							simulationDataCPU->bounds = settings.customBounds;

						// If using a camera-independant sorting mode, sort the particles right away
						switch (settings.sortMode)
						{
						default:
						case ParticleSortMode::None: // No sort, just point the indices back to themselves
							for (UINT32 i = 0; i < numParticles; i++)
								simulationDataCPU->indices[i] = i;
							break;
						case ParticleSortMode::OldToYoung:
						case ParticleSortMode::YoungToOld:
							SortParticles(*system->mParticleSet, settings.sortMode, Vector3::ZERO, simulationDataCPU->indices.data());
							break;
						case ParticleSortMode::Distance: break;
						}
					}
				}

				{
					Lock lock(mMutex);

					assert(mNumActiveWorkers > 0);
					mNumActiveWorkers--;

					if(simulationDataCPU)
						simulationData.cpuData[system->mId] = simulationDataCPU;
					else if(simulationDataGPU)
						simulationData.gpuData[system->mId] = simulationDataGPU;
				}

				mWorkerDoneSignal.notify_one();
			};

			SPtr<Task> task = Task::Create("ParticleWorker", evaluateWorker);
			TaskScheduler::Instance().AddTask(task);
		}

		// Wait for tasks to complete
		TaskScheduler::Instance().AddWorker(); // Make the current core available for work (since this thread waits)
		{
			Lock lock(mMutex);

			while (mNumActiveWorkers > 0)
				mWorkerDoneSignal.wait(lock);
		}
		TaskScheduler::Instance().RemoveWorker();

		mSwapBuffers = true;

		return &mSimulationData[mWriteBufferIdx];
	}

	void ParticleManager::SortParticles(const ParticleSet& set, ParticleSortMode sortMode, const Vector3& viewPoint,
		UINT32* indices)
	{
		assert(sortMode != ParticleSortMode::None);

		struct ParticleSortData
		{
			ParticleSortData(float key, UINT32 idx)
				:key(key), idx(idx)
			{ }

			float key;
			UINT32 idx;
		};

		const UINT32 count = set.GetParticleCount();
		const ParticleSetData& particles = set.GetParticles();

		bs_frame_mark();
		{
			FrameVector<ParticleSortData> sortData;
			sortData.reserve(count);

			switch(sortMode)
			{
			default:
			case ParticleSortMode::Distance:
				for(UINT32 i = 0; i < count; i++)
				{
					float distance = viewPoint.SquaredDistance(particles.position[i]);
					sortData.emplace_back(distance, i);
				}
				break;
			case ParticleSortMode::OldToYoung:
				for(UINT32 i = 0; i < count; i++)
				{
					float lifetime = particles.lifetime[i];
					sortData.emplace_back(lifetime, i);
				}
				break;
			case ParticleSortMode::YoungToOld:
				for(UINT32 i = 0; i < count; i++)
				{
					float lifetime = particles.initialLifetime[i] - particles.lifetime[i];
					sortData.emplace_back(lifetime, i);
				}
				break;
			}

			std::sort(sortData.begin(), sortData.end(),
				[](const ParticleSortData& lhs, const ParticleSortData& rhs)
			{
				return rhs.key < lhs.key;
			});

			for (UINT32 i = 0; i < count; i++)
				indices[i] = sortData[i].idx;
		}
		bs_frame_clear();
	}

	UINT32 ParticleManager::RegisterParticleSystem(ParticleSystem* system)
	{
		mSystems.insert(system);

		return mNextId++;
	}

	void ParticleManager::UnregisterParticleSystem(ParticleSystem* system)
	{
		mSystems.erase(system);
	}
}
