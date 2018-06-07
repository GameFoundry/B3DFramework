//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleSystem.h"
#include "Utility/BsTime.h"
#include "Threading/BsTaskScheduler.h"
#include "Allocators/BsPoolAlloc.h"
#include "Private/Particles/BsParticleSet.h"

namespace bs
{
	/** 
	 * Maintains a pool of buffers that are used for storing particle system rendering data. Buffers are always created
	 * in 2D square layout as they are ultimately used for initializing textures.
	 */
	class ParticleRenderDataPool
	{
		/** Contains a list of buffers for the specified size. */
		struct BuffersPerSize
		{
			Vector<ParticleRenderData*> buffers;
			UINT32 nextFreeIdx = 0;
		};

	public:
		~ParticleRenderDataPool()
		{
			for (auto& sizeEntry : mBufferList)
			{
				for (auto& entry : sizeEntry.second.buffers)
					mAlloc.destruct(entry);
			}
		}

		/** Returns a new set of render data buffers and populates them with data from the provided ParticleSet. */
		ParticleRenderData* alloc(const ParticleSet& particleSet)
		{
			const UINT32 size = particleSet.determineTextureSize();

			ParticleRenderData* output = nullptr;
			BuffersPerSize& buffers = mBufferList[size];
			if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
			{
				output = buffers.buffers[buffers.nextFreeIdx];
				buffers.nextFreeIdx++;
			}

			if (!output)
			{
				output = createNewBuffers(size);
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
				const PixelData& pixels = output->size;

				UINT8* dstData = pixels.getData();
				UINT8* ptr = dstData;
				UINT32 x = 0;

				for (UINT32 i = 0; i < count; i++)
				{
					UINT16& sizeX = *(UINT16*)ptr;
					ptr += sizeof(UINT16);

					UINT16& sizeY = *(UINT16*)ptr;
					ptr += sizeof(UINT16);

					sizeX = Bitwise::floatToHalf(particles.size[i].x);
					sizeY = Bitwise::floatToHalf(particles.size[i].y);

					x++;
					if (x >= size)
					{
						x = 0;
						ptr += pixels.getRowSkip();
					}
				}
			}

			return output;
		}

		void clear()
		{
			for(auto& buffers : mBufferList)
				buffers.second.nextFreeIdx = 0;
		}

	private:
		/** Allocates a new set of buffers of the provided @p size width and height. */
		ParticleRenderData* createNewBuffers(UINT32 size)
		{
			ParticleRenderData* output = mAlloc.construct<ParticleRenderData>();

			output->positionAndRotation = PixelData(size, size, 1, PF_RGBA32F);
			output->color = PixelData(size, size, 1, PF_RGBA8);
			output->size = PixelData(size, size, 1, PF_RG16F);

			// Note: Potentially allocate them all in one large block
			output->positionAndRotation.allocateInternalBuffer();
			output->color.allocateInternalBuffer();
			output->size.allocateInternalBuffer();

			mBufferList[size].buffers.push_back(output);

			return output;
		}

		UnorderedMap<UINT32, BuffersPerSize> mBufferList;
		PoolAlloc<sizeof(ParticleRenderData), 32> mAlloc;
	};

	struct ParticleManager::Members
	{
		// TODO - Perhaps sharing one pool is better
		ParticleRenderDataPool renderDataPool[CoreThread::NUM_SYNC_BUFFERS];
	};

	ParticleManager::ParticleManager()
		:m(bs_new<Members>())
	{ }

	ParticleManager::~ParticleManager()
	{
		bs_delete(m);
	}

	ParticleRenderDataGroup* ParticleManager::update()
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
			return &mRenderData[mReadBufferIdx];

		// TODO - Perform culling (but only on deterministic particle systems). In which case cache the bounds so we don't
		// need to recalculate them below

		// Prepare the write buffer
		ParticleRenderDataGroup& renderDataGroup = mRenderData[mWriteBufferIdx];
		renderDataGroup.data.clear();

		// Queue evaluation tasks
		{
			Lock lock(mMutex);
			mNumActiveWorkers = (UINT32)mSystems.size();
		}

		float timeDelta = gTime().getFrameDelta();

		ParticleRenderDataPool& renderDataPool = m->renderDataPool[mWriteBufferIdx];
		renderDataPool.clear();

		for (auto& system : mSystems)
		{
			const auto evaluateWorker = [this, timeDelta, system]()
			{
				system->_simulate(timeDelta);

				Lock lock(mMutex);
				{
					assert(mNumActiveWorkers > 0);
					mNumActiveWorkers--;
				}

				mWorkerDoneSignal.notify_one();
			};

			SPtr<Task> task = Task::create("ParticleWorker", evaluateWorker);
			TaskScheduler::instance().addTask(task);
		}

		// Wait for tasks to complete
		{
			Lock lock(mMutex);

			while (mNumActiveWorkers > 0)
				mWorkerDoneSignal.wait(lock);
		}

		// Generate output data from the current set
		for (auto& system : mSystems)
		{
			// TODO - Make the output data allocation, and bound calculation happen on the worker thread
			ParticleRenderData* renderData = renderDataPool.alloc(*system->mParticleSet);
			renderData->numParticles = system->mParticleSet->getParticleCount();
			renderData->bounds = system->_calculateBounds();

			renderDataGroup.data[system->mId] = renderData;
		}

		mSwapBuffers = true;

		return &mRenderData[mReadBufferIdx];
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
