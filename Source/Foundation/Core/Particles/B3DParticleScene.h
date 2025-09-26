//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"
#include "Utility/B3DModule.h"
#include "Math/B3DAABox.h"
#include "CoreObject/B3DRenderThread.h"
#include "Components/B3DParticleSystem.h"

namespace b3d
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
	struct B3D_EXPORT ParticleRenderData
	{
		/** Contains mapping from unsorted to sorted particle indices. */
		Vector<u32> Indices;

		/** Total number of particles in the particle system. */
		u32 NumParticles;

		/** Bounds of the particle system, in the system's simulation space. */
		AABox Bounds;
	};

	/**
	 * Contains data used for rendering particles as billboards. Per-particle data is stored in a 2D square layout so it
	 * can be used for quickly initializing a texture.
	 */
	struct B3D_EXPORT ParticleBillboardRenderData : ParticleRenderData
	{
		/** Contains particle positions in .xyz and 2D rotation in .w */
		PixelData PositionAndRotation;

		/** Contains particle color in .xyz and transparency in .a. */
		PixelData Color;

		/** Contains 2D particle size in .xy, frame index (used for animation) in .z. */
		PixelData SizeAndFrameIdx;
	};

	/**
	 * Contains data used for rendering particles as meshes. Per-particle data is stored in a 2D square layout so it
	 * can be used for quickly initializing a texture.
	 */
	struct B3D_EXPORT ParticleMeshRenderData : ParticleRenderData
	{
		/** Contains particle positions in .xyz with .w unused. */
		PixelData Position;

		/** Contains particle color in .xyz and transparency in .a. */
		PixelData Color;

		/** Contains particle size in .xyz with .w unused. */
		PixelData Size;

		/** Contains particle rotation in radians in .xyz with .w unused. */
		PixelData Rotation;
	};

	/**
	 * Contains information about a single particle about to be inserted into the GPU simulation. Matches the structure
	 * of the vertex buffer element used for injecting shader data into the simulation.
	 */
	struct GpuParticleVertex
	{
		Vector3 Position;
		float Lifetime;
		Vector3 Velocity;
		float InvMaxLifetime;
		Vector2 Size;
		float Rotation;
		Vector2 DataUv;
	};

	/** Extension of GpuParticle that contains data not required by the injection vertex buffer. */
	struct GpuParticle : GpuParticleVertex
	{
		/** Gets a version of this object suitable for upload to the injection vertex buffer. */
		GpuParticleVertex GetVertex() const
		{
			GpuParticleVertex output;
			output.Position = Position;
			output.Lifetime = (InitialLifetime - Lifetime) / InitialLifetime;
			output.Velocity = Velocity;
			output.InvMaxLifetime = 1.0f / InitialLifetime;
			output.Size = Size;
			output.Rotation = Rotation;
			output.DataUv = DataUv;

			return output;
		}

		float InitialLifetime;
	};

	/** Contains inputs to the GPU particle simulation as provided by the particle system manager. */
	struct B3D_EXPORT ParticleGPUSimulationData
	{
		/** A set of the particles to be inserted into the simulation. */
		Vector<GpuParticle> Particles;
	};

	/** Contains simulation data resulting from all particle systems, for a single frame. */
	struct EvaluatedParticleData
	{
		UnorderedMap<u32, ParticleRenderData*> CpuData;
		UnorderedMap<u32, ParticleGPUSimulationData*> GpuData;
	};

	/** Keeps track of all active ParticleSystem%s in a particular scene and performs per-frame updates. */
	class B3D_EXPORT ParticleScene final
	{
		struct Members;

	public:
		ParticleScene();
		~ParticleScene();

		/**
		 * Advances the simulation for all particle systems using the current frame time delta. Outputs a set of data
		 * that can be used for rendering & updating every active particle system.
		 */
		EvaluatedParticleData* Update(const EvaluatedAnimationData& animData);

		/** Creates a new empty particle scene. */
		static SPtr<ParticleScene> Create() { return B3DMakeShared<ParticleScene>(); }

	private:
		friend class ParticleSystem;

		/** Must be called by a ParticleSystem upon construction. */
		u32 RegisterParticleSystem(ParticleSystem* system);

		/** Must be called by a ParticleSystem before destruction. */
		void UnregisterParticleSystem(ParticleSystem* system);

		/**
		 * Sorts the particles in the provided @p using the @p sortMode. Sorted particle indices are placed in the
		 * @p indices array which is expected to be pre-allocated with enough space to hold an index for each particle
		 * in a set. @p viewPoint is used as a reference point when using the Distance sort mode.
		 */
		void SortParticles(const ParticleSet& set, ParticleSortMode sortMode, const Vector3& viewPoint, u32* indices);

		Members* m;

		u32 mNextId = 1;
		UnorderedSet<ParticleSystem*> mSystems;

		bool mPaused = false;

		// Worker threads
		EvaluatedParticleData mSimulationData[RenderThread::kSyncBufferCount];

		u32 mReadBufferIdx = 1;
		u32 mWriteBufferIdx = 0;

		Mutex mMutex;

		bool mSwapBuffers = false;
	};

	/** @} */
} // namespace b3d
