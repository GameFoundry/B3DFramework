//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Utility/BsBitfield.h"
#include "Utility/BsModule.h"
#include "Particles/BsParticleManager.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs { namespace ct 
{
	struct SceneInfo;
	class GpuParticleResources;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Contains information about a single tile allocated in the particle texture used for GPU simulation. */
	struct GpuParticleTile
	{
		UINT32 id = (UINT32)-1;
		UINT32 numFreeParticles = 0;
		float lifetime = 0.0f;
	};

	/** Contains functionality specific to a single particle system simulated on the GPU. */
	class GpuParticleSystem
	{
	public:
		GpuParticleSystem(ParticleSystem* parent);
		~GpuParticleSystem();

		/** Returns the non-renderer particle system object that owns this object. */
		ParticleSystem* getParent() const { return mParent; }

		/**
		 * Attempts to allocate room for a set of particles. Particles will attempt to be inserted into an existing tile if
		 * there's room, or new tiles will be allocated otherwise. If the particle texture is full the allocation will
		 * silently fail.
		 * 
		 * @param[in]		resources		Object holding the global particle textures.
		 * @param[in,out]	newParticles	List of new particles for which space needs to be allocated. The particles will
		 *									get updated in-place with the UV coordinates at which their data is located.
		 * @param[in]		newTiles		Indices of the tiles that were newly allocated, if any.
		 * @return							True if any new tiles were allocated, false otherwise.
		 */
		bool allocateTiles(GpuParticleResources& resources, Vector<GpuParticle>& newParticles, 
			Vector<UINT32>& newTiles);

		/** 
		 * Detects which tiles had all of their particle's expire and marks the inactive so they can be re-used on the
		 * next call to allocateTiles().
		 */
		void detectInactiveTiles();

		/** Releases any tiles that were marked as inactive so they may be re-used by some other particle system. */
		bool freeInactiveTiles(GpuParticleResources& resources);

		/** Returns a buffer containing UV coordinates to which each of the allocate tiles map to. */
		SPtr<GpuBuffer> getTileUVs() const { return mTileUVs; }

		/** Returns a buffer containing per-particle indices used for locating particle data in the particle textures. */
		SPtr<GpuBuffer> getParticleIndices() const { return mParticleIndices; }

		/** 
		 * Returns the total number of tiles used by this particle system. This may include inactive tiles unless you have
		 * freed them using freeInactiveTiles earlier.
		 */
		UINT32 getNumTiles() const { return (UINT32)mTiles.size(); }

		/** Rebuilds ths internal buffers that contain tile UVs and per-particle UVs. */
		void updateGpuBuffers();

		/** Increments the internal time counter. */
		void advanceTime(float dt);

		/** Returns the time since the system was created. */
		float getTime() const { return mTime; }

		/** Returns the object that can be used for retrieving random numbers when evaluating this particle system. */
		const Random& getRandom() const { return mRandom; }

	private:
		ParticleSystem* mParent = nullptr;
		Vector<GpuParticleTile> mTiles;
		Bitfield mActiveTiles;
		UINT32 mNumActiveTiles = 0;
		UINT32 mLastAllocatedTile = (UINT32)-1;
		float mTime = 0.0f;
		Random mRandom;

		SPtr<GpuBuffer> mTileUVs;
		SPtr<GpuBuffer> mParticleIndices;
	};

	/** Performs simulation for all particle systems that have GPU simulation enabled. */
	class GpuParticleSimulation : public Module<GpuParticleSimulation>
	{
		struct Pimpl;
	public:
		GpuParticleSimulation();
		~GpuParticleSimulation();

		/** 
		 * Registers a new GPU particle system to simulate. The system will be simulated until removed by a call to
		 * removeSystem().
		 */
		void addSystem(GpuParticleSystem* system);

		/** Unregisters a previously registered particle system. */
		void removeSystem(GpuParticleSystem* system);

		/** 
		 * Performs GPU particle simulation on all registered particle systems. 
		 * 
		 * @param[in]	sceneInfo	Information about the scene currently being rendered.
		 * @param[in]	simData		Particle simulation data output on the simulation thread.
		 * @param[in]	dt			Time step to advance the simulation by.
		 */
		void simulate(const SceneInfo& sceneInfo, const ParticleSimulationData* simData, float dt);

		/** Returns textures used for storing particle data. */
		GpuParticleResources& getResources() const;
	private:
		/** Clears out all the areas in particle textures as marked by the provided tiles to their default values. */
		void clearTiles(const Vector<UINT32>& tiles);

		/** Inserts the provided set of particles into the particle textures. */
		void injectParticles(const Vector<GpuParticle>& particles);

		Pimpl* m;
	};

	/** Contains textures that get updated with every run of the GPU particle simulation. */
	struct GpuParticleStateTextures
	{
		SPtr<PooledRenderTexture> positionAndTimeTex;
		SPtr<PooledRenderTexture> velocityTex;
		SPtr<RenderTexture> renderTarget;
	};

	/** 
	 * Contains textures and buffers used for GPU particle simulation and handles allocation of tiles within the particle
	 * textures. State textures are double-buffered so one can be used for reading and other for writing during simulation.
	 */
	class GpuParticleResources
	{
	public:
		static constexpr UINT32 TEX_SIZE = 1024;
		static constexpr UINT32 TILE_SIZE = 4;
		static constexpr UINT32 PARTICLES_PER_TILE = TILE_SIZE * TILE_SIZE;
		static constexpr UINT32 TILE_COUNT_1D = TEX_SIZE / TILE_SIZE;
		static constexpr UINT32 TILE_COUNT = TILE_COUNT_1D * TILE_COUNT_1D;

		static_assert((TEX_SIZE & (TEX_SIZE - 1)) == 0, "Particle texture size not a power of two");
		static_assert((TILE_SIZE & (TILE_SIZE - 1)) == 0, "Particle tile size not a power of two");

		GpuParticleResources();

		/** Swap the read and write state textures. */
		void swap() { mWriteBufferIdx ^= 0x1; }

		/** Returns state textures that contain last state of the particle system. */
		GpuParticleStateTextures& getReadState() { return mStateTextures[mWriteBufferIdx ^ 0x1]; }

		/** Returns state textures that can be used for writing the new state of the particle system. */
		GpuParticleStateTextures& getWriteState() { return mStateTextures[mWriteBufferIdx]; }

		/** 
		 * Attempts to allocate a new tile in particle textures. Returns index of the tile if successful or -1 if no more
		 * room.
		 */
		UINT32 allocTile();

		/** Frees a tile previously allocated with allocTile(). */
		void freeTile(UINT32 tile);

		/** Returns offset (in pixels) at which the tile with the specified index starts at. */
		static Vector2I getTileOffset(UINT32 tileId);

		/** Returns the UV coordinates at which the tile with the specified index starts at. */
		static Vector2 getTileCoords(UINT32 tileId);

		/**
		 * Returns the particle offset (in pixels) relative to the tile. @p subTileIdx represents he index of the particle
		 * in a tile.
		 */
		static Vector2I getParticleOffset(UINT32 subTileId);

		/** 
		 * Returns the particle coordinates relative to the tile. @p subTileIdx represents the index of the particle in
		 * a tile.
		 */
		static Vector2 getParticleCoords(UINT32 subTileIdx);

	private:
		GpuParticleStateTextures mStateTextures[2];

		UINT32 mWriteBufferIdx = 0;

		UINT32 mFreeTiles[TILE_COUNT];
		UINT32 mNumFreeTiles = TILE_COUNT;
	};

	/** @} */
}}

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::GpuParticleSystem, 32)
}