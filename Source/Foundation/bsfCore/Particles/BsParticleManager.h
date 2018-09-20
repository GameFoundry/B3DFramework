//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsPixelData.h"
#include "Utility/BsModule.h"
#include "Math/BsAABox.h"
#include "CoreThread/BsCoreThread.h"
#include "BsParticleSystem.h"

namespace bs
{
	class ParticleSet;
	struct EvaluatedAnimationData;

	/** @addtogroup Particles-Internal
	 *  @{
	 */
	
	/** 
	 * Contains data resulting from a single frame of CPU particle simulation of a single particle system, used by all
	 * rendering modes. 
	 */
	struct BS_CORE_EXPORT ParticleRenderData
	{
		/** Contains mapping from unsorted to sorted particle indices. */
		Vector<UINT32> indices;

		/** Total number of particles in the particle system. */
		UINT32 numParticles;

		/** Bounds of the particle system, in the system's simulation space. */
		AABox bounds;
	};

	/** 
	 * Contains data used for rendering particles as billboards. Per-particle data is stored in a 2D square layout so it
	 * can be used for quickly initializing a texture.
	 */
	struct BS_CORE_EXPORT ParticleBillboardRenderData : ParticleRenderData
	{
		/** Contains particle positions in .xyz and 2D rotation in .w */
		PixelData positionAndRotation;

		/** Contains particle color in .xyz and transparency in .a. */
		PixelData color;

		/** Contains 2D particle size in .xy, frame index (used for animation) in .z. */
		PixelData sizeAndFrameIdx;
	};

	/** 
	 * Contains data used for rendering particles as meshes. Per-particle data is stored in a 2D square layout so it
	 * can be used for quickly initializing a texture.
	 */
	struct BS_CORE_EXPORT ParticleMeshRenderData : ParticleRenderData
	{
		/** Contains particle positions in .xyz with .w unused. */
		PixelData position;

		/** Contains particle color in .xyz and transparency in .a. */
		PixelData color;

		/** Contains particle size in .xyz with .w unused. */
		PixelData size;

		/** Contains particle rotation in radians in .xyz with .w unused. */
		PixelData rotation;
	};
	/** 
	 * Contains information about a single particle about to be inserted into the GPU simulation. Matches the structure
	 * of the vertex buffer element used for injecting shader data into the simulation.
	 */
	struct GpuParticleVertex
	{
		Vector3 position;
		float lifetime;
		Vector3 velocity;
		float invMaxLifetime;
		Vector2 size;
		float rotation;
		Vector2 dataUV;
	};

	/** Extension of GpuParticle that contains data not required by the injection vertex buffer. */
	struct GpuParticle : GpuParticleVertex
	{
		/** Gets a version of this object suitable for upload to the injection vertex buffer. */
		GpuParticleVertex getVertex() const
		{
			GpuParticleVertex output;
			output.position = position;
			output.lifetime = (initialLifetime - lifetime) / initialLifetime;
			output.velocity = velocity;
			output.invMaxLifetime = 1.0f / initialLifetime;
			output.size = size;
			output.rotation = rotation;
			output.dataUV = dataUV;

			return output;
		}

		float initialLifetime;
	};

	/** Contains inputs to the GPU particle simulation as provided by the particle system manager. */
	struct BS_CORE_EXPORT ParticleGPUSimulationData
	{
		/** A set of the particles to be inserted into the simulation. */
		Vector<GpuParticle> particles;
	};

	/** Contains simulation data resulting from all particle systems, for a single frame. */
	struct ParticlePerFrameData
	{
		UnorderedMap<UINT32, ParticleRenderData*> cpuData;
		UnorderedMap<UINT32, ParticleGPUSimulationData*> gpuData;
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
		 * that can be used for rendering & updating every active particle system.
		 */
		ParticlePerFrameData* update(const EvaluatedAnimationData& animData);

	private:
		friend class ParticleSystem;

		/** Must be called by a ParticleSystem upon construction. */
		UINT32 registerParticleSystem(ParticleSystem* system);

		/** Must be called by a ParticleSystem before destruction. */
		void unregisterParticleSystem(ParticleSystem* system);

		/** 
		 * Sorts the particles in the provided @p using the @p sortMode. Sorted particle indices are placed in the
		 * @p indices array which is expected to be pre-allocated with enough space to hold an index for each particle
		 * in a set. @p viewPoint is used as a reference point when using the Distance sort mode.
		 */
		void sortParticles(const ParticleSet& set, ParticleSortMode sortMode, const Vector3& viewPoint, UINT32* indices);

		Members* m;

		UINT32 mNextId = 1;
		UnorderedSet<ParticleSystem*> mSystems;

		bool mPaused = false;

		// Worker threads
		ParticlePerFrameData mSimulationData[CoreThread::NUM_SYNC_BUFFERS];

		UINT32 mReadBufferIdx = 1;
		UINT32 mWriteBufferIdx = 0;
		
		Signal mWorkerDoneSignal;
		Mutex mMutex;

		UINT32 mNumActiveWorkers = 0;
		bool mSwapBuffers = false;
	};

	/** @} */
}
