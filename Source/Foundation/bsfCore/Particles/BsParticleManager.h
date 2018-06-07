//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsPixelData.h"
#include "Utility/BsModule.h"
#include "Math/BsAABox.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */
	
	/** 
	 * Data used for rendering a ParticleSystem. Per-particle data is stored in a 2D square layout so it can be used for
	 * quickly initializing a texture.
	 */
	struct ParticleRenderData
	{
		/** Contains particle positions in .xyz and 2D rotation in .w */
		PixelData positionAndRotation;

		/** Contains particle color in .xyz and transparency in .a. */
		PixelData color;

		/** Contains 2D particle size in .xy. */
		PixelData size;

		/** Total number of particles in the particle system. */
		UINT32 numParticles;

		/** Local bounds of the particle system. */
		AABox bounds;
	};

	/** Contains all rendering data for all particle systems, for a single frame. */
	struct ParticleRenderDataGroup
	{
		UnorderedMap<UINT32, ParticleRenderData*> data;
	};

	/** Keeps track of all active ParticleSystem%s and performs per-frame updates. */
	class BS_CORE_EXPORT ParticleManager final : public Module<ParticleManager>
	{
		struct Members;
	public:
		ParticleManager();
		~ParticleManager();

		/** 
		 * Advances the simulation for all particle systems using the current frame time delta. Outputs a set of data
		 * that can be used for rendering every active particle system.
		 */
		ParticleRenderDataGroup* update();

	private:
		friend class ParticleSystem;

		/** Must be called by a ParticleSystem upon construction. */
		UINT32 registerParticleSystem(ParticleSystem* system);

		/** Must be called by a ParticleSystem before destruction. */
		void unregisterParticleSystem(ParticleSystem* system);

		Members* m;

		UINT32 mNextId = 1;
		UnorderedSet<ParticleSystem*> mSystems;

		bool mPaused = false;

		// Worker threads
		ParticleRenderDataGroup mRenderData[CoreThread::NUM_SYNC_BUFFERS];

		UINT32 mReadBufferIdx = 1;
		UINT32 mWriteBufferIdx = 0;
		
		Signal mWorkerDoneSignal;
		Mutex mMutex;

		UINT32 mNumActiveWorkers = 0;
		bool mSwapBuffers = false;
	};

	/** @} */
}
