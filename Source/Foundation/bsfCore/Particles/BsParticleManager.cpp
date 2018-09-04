//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleSystem.h"
#include "Utility/BsTime.h"
#include "Threading/BsTaskScheduler.h"
#include "Allocators/BsPoolAlloc.h"
#include "Private/Particles/BsParticleSet.h"
#include "Animation/BsAnimationManager.h"

namespace bs
{
	void ParticleCPUSimulationData::updateSortIndices(const Vector3& referencePoint)
	{
		struct ParticleSortData
		{
			ParticleSortData(float key, UINT32 idx)
				:key(key), idx(idx)
			{ }

			float key;
			UINT32 idx;
		};

		const UINT32 size = positionAndRotation.getWidth();
		UINT8* positionPtr = positionAndRotation.getData();

		bs_frame_mark();
		{
			FrameVector<ParticleSortData> sortData;
			sortData.reserve(numParticles);

			UINT32 x = 0;
			for (UINT32 i = 0; i < numParticles; i++)
			{
				Vector4& posAndRot = *(Vector4*)positionPtr;
				Vector3 position(posAndRot);

				float distance = referencePoint.squaredDistance(position);
				sortData.emplace_back(distance, i);

				positionPtr += sizeof(Vector4);
				x++;

				if (x >= size)
				{
					x = 0;
					positionPtr += positionAndRotation.getRowSkip();
				}
			}

			std::sort(sortData.begin(), sortData.end(),
				[](const ParticleSortData& lhs, const ParticleSortData& rhs)
			{
				return rhs.key < lhs.key;
			});

			for (UINT32 i = 0; i < numParticles; i++)
				indices[i] = sortData[i].idx;
		}
		bs_frame_clear();
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
			Vector<ParticleCPUSimulationData*> buffers;
			UINT32 nextFreeIdx = 0;
		};

	public:
		~ParticleSimulationDataPool()
		{
			Lock lock(mMutex);

			for (auto& sizeEntry : mCPUBufferList)
			{
				for (auto& entry : sizeEntry.second.buffers)
					mCPUAlloc.destruct(entry);
			}

			for (auto& entry : mGPUBufferList)
				mGPUAlloc.destruct(entry);
		}

		/** 
		 * Returns a set of buffers containing particle data from the provided particle set. Usable for rendering the
		 * results of the CPU particle simulation.
		 */
		ParticleCPUSimulationData* allocCPU(const ParticleSet& particleSet)
		{
			const UINT32 size = particleSet.determineTextureSize();

			ParticleCPUSimulationData* output = nullptr;

			{
				Lock lock(mMutex);

				BuffersPerSize& buffers = mCPUBufferList[size];
				if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
				{
					output = buffers.buffers[buffers.nextFreeIdx];
					buffers.nextFreeIdx++;
				}
			}

			if (!output)
			{
				output = createNewBuffersCPU(size);

				Lock lock(mMutex);

				BuffersPerSize& buffers = mCPUBufferList[size];
				buffers.buffers.push_back(output);
				buffers.nextFreeIdx++;
			}

			// Populate buffer contents
			const UINT32 count = particleSet.getParticleCount();
			const ParticleSetData& particles = particleSet.getParticles();

			// TODO: Use non-temporal writes?
			{
				const PixelData& pixels = output->positionAndRotation;

				UINT8* dstData = pixels.getData();
				UINT8* ptr = dstData;
				UINT32 x = 0;

				for (UINT32 i = 0; i < count; i++)
				{
					Vector4& posAndRot = *(Vector4*)ptr;
					posAndRot.x = particles.position[i].x;
					posAndRot.y = particles.position[i].y;
					posAndRot.z = particles.position[i].z;
					posAndRot.w = particles.rotation[i].x;

					ptr += sizeof(Vector4);
					x++;

					if (x >= size)
					{
						x = 0;
						ptr += pixels.getRowSkip();
					}
				}
			}

			{
				const PixelData& pixels = output->color;

				UINT8* dstData = pixels.getData();
				UINT8* ptr = dstData;
				UINT32 x = 0;

				for (UINT32 i = 0; i < count; i++)
				{
					RGBA& color = *(RGBA*)ptr;
					color = particles.color[i];

					ptr += sizeof(RGBA);
					x++;

					if (x >= size)
					{
						x = 0;
						ptr += pixels.getRowSkip();
					}
				}
			}

			{
				const PixelData& pixels = output->sizeAndFrameIdx;

				UINT8* dstData = pixels.getData();
				UINT8* ptr = dstData;
				UINT32 x = 0;

				for (UINT32 i = 0; i < count; i++)
				{
					UINT16& sizeX = *(UINT16*)ptr;
					ptr += sizeof(UINT16);

					UINT16& sizeY = *(UINT16*)ptr;
					ptr += sizeof(UINT16);

					UINT16& frameIdx = *(UINT16*)ptr;
					ptr += sizeof(UINT16);

					// Unused
					ptr += sizeof(UINT16);

					sizeX = Bitwise::floatToHalf(particles.size[i].x);
					sizeY = Bitwise::floatToHalf(particles.size[i].y);
					frameIdx = Bitwise::floatToHalf(particles.frame[i]);

					x++;
					if (x >= size)
					{
						x = 0;
						ptr += pixels.getRowSkip();
					}
				}
			}

			output->indices.clear();
			output->indices.resize(count);

			return output;
		}

		/** 
		 * Returns a list of particles from the provided particle set that may be used for inserting the particles into the
		 * GPU simulation. 
		 */
		ParticleGPUSimulationData* allocGPU(const ParticleSet& particleSet)
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
				output = createNewBuffersGPU();

				Lock lock(mMutex);

				mGPUBufferList.push_back(output);
				mNextFreeGPUBuffer++;
			}

			// Populate buffer contents
			const UINT32 count = particleSet.getParticleCount();
			const ParticleSetData& particles = particleSet.getParticles();

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
		void clear()
		{
			Lock lock(mMutex);

			for(auto& buffers : mCPUBufferList)
				buffers.second.nextFreeIdx = 0;

			mNextFreeGPUBuffer = 0;
		}

	private:
		/** Allocates a new set of CPU buffers of the provided @p size width and height. */
		ParticleCPUSimulationData* createNewBuffersCPU(UINT32 size)
		{
			ParticleCPUSimulationData* output = mCPUAlloc.construct<ParticleCPUSimulationData>();

			output->positionAndRotation = PixelData(size, size, 1, PF_RGBA32F);
			output->color = PixelData(size, size, 1, PF_RGBA8);
			output->sizeAndFrameIdx = PixelData(size, size, 1, PF_RGBA16F);

			// Note: Potentially allocate them all in one large block
			output->positionAndRotation.allocateInternalBuffer();
			output->color.allocateInternalBuffer();
			output->sizeAndFrameIdx.allocateInternalBuffer();

			return output;
		}

		/** Allocates a new set of GPU buffers of the provided @p size width and height. */
		ParticleGPUSimulationData* createNewBuffersGPU()
		{
			return mGPUAlloc.construct<ParticleGPUSimulationData>();
		}

		UnorderedMap<UINT32, BuffersPerSize> mCPUBufferList;
		Vector<ParticleGPUSimulationData*> mGPUBufferList;
		UINT32 mNextFreeGPUBuffer = 0;

		PoolAlloc<sizeof(ParticleCPUSimulationData), 32, 4, true> mCPUAlloc;
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

	ParticleSimulationData* ParticleManager::update(const EvaluatedAnimationData& animData)
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
		ParticleSimulationData& simulationData = mSimulationData[mWriteBufferIdx];
		simulationData.cpuData.clear();
		simulationData.gpuData.clear();

		// Queue evaluation tasks
		{
			Lock lock(mMutex);
			mNumActiveWorkers = (UINT32)mSystems.size();
		}

		float timeDelta = gTime().getFrameDelta();

		ParticleSimulationDataPool& simDataPool = m->simDataPool[mWriteBufferIdx];
		simDataPool.clear();

		for (auto& system : mSystems)
		{
			const auto evaluateWorker = [this, timeDelta, system, &animData, &simDataPool, &simulationData]()
			{
				// Advance the simulation
				system->_simulate(timeDelta, &animData);

				ParticleCPUSimulationData* simulationDataCPU = nullptr;
				ParticleGPUSimulationData* simulationDataGPU = nullptr;
				if(system->mParticleSet)
				{
					// Generate simulation data to transfer to the core thread
					const UINT32 numParticles = system->mParticleSet->getParticleCount();

					if(system->getSettings().gpuSimulation)
					{
						simulationDataGPU = simDataPool.allocGPU(*system->mParticleSet);
					}
					else
					{
						simulationDataCPU = simDataPool.allocCPU(*system->mParticleSet);
						simulationDataCPU->numParticles = numParticles;
						simulationDataCPU->bounds = system->_calculateBounds();

						// If using a camera-independant sorting mode, sort the particles right away
						const ParticleSystemSettings& settings = system->getSettings();
						switch (settings.sortMode)
						{
						default:
						case ParticleSortMode::None: // No sort, just point the indices back to themselves
							for (UINT32 i = 0; i < numParticles; i++)
								simulationDataCPU->indices[i] = i;
							break;
						case ParticleSortMode::OldToYoung:
						case ParticleSortMode::YoungToOld:
							sortParticles(*system->mParticleSet, settings.sortMode, Vector3::ZERO, simulationDataCPU->indices.data());
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

			SPtr<Task> task = Task::create("ParticleWorker", evaluateWorker);
			TaskScheduler::instance().addTask(task);
		}

		// Wait for tasks to complete
		TaskScheduler::instance().addWorker(); // Make the current core available for work (since this thread waits)
		{
			Lock lock(mMutex);

			while (mNumActiveWorkers > 0)
				mWorkerDoneSignal.wait(lock);
		}
		TaskScheduler::instance().removeWorker();

		mSwapBuffers = true;

		return &mSimulationData[mReadBufferIdx];
	}

	void ParticleManager::sortParticles(const ParticleSet& set, ParticleSortMode sortMode, const Vector3& viewPoint, 
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

		const UINT32 count = set.getParticleCount();
		const ParticleSetData& particles = set.getParticles();

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
					float distance = viewPoint.squaredDistance(particles.position[i]);
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

	UINT32 ParticleManager::registerParticleSystem(ParticleSystem* system)
	{
		mSystems.insert(system);

		return mNextId++;
	}

	void ParticleManager::unregisterParticleSystem(ParticleSystem* system)
	{
		mSystems.erase(system);
	}
}
