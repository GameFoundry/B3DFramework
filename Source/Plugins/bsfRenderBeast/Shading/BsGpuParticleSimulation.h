//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Utility/BsBitfield.h"
#include "Utility/BsModule.h"
#include "Particles/BsParticleManager.h"
#include "Allocators/BsPoolAlloc.h"
#include "Utility/BsTextureRowAllocator.h"
#include "Utility/BsGpuSort.h"

namespace bs { namespace ct 
{
	struct RendererParticles;
	class GpuParticleSimulateMat;
	struct GBufferTextures;
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

		/** 
		 * Sets information about the results of particle system sorting. 
		 * 
		 * @param[in]	sorted		True if the system has information in the sorted index buffer.
		 * @param[in]	offset		Offset into the sorted index buffer. Only relevant if @p sorted is true.
		 */
		void setSortInfo(bool sorted, UINT32 offset)
		{
			mSorted = sorted;

			if(sorted)
				mSortOffset = offset;
		}

		/** Returns true if the particle system has its indices stored in the sorted index buffer. */
		bool hasSortInfo() const { return mSorted; }

		/** 
		 * Returns offset into the sorted index buffer at which indices of the particle system start. Only available if
		 * hasSortInfo() returns true.
		 */
		UINT32 getSortOffset() const { return mSortOffset; }

	private:
		ParticleSystem* mParent = nullptr;
		Vector<GpuParticleTile> mTiles;
		Bitfield mActiveTiles;
		UINT32 mNumActiveTiles = 0;
		UINT32 mLastAllocatedTile = (UINT32)-1;
		float mTime = 0.0f;
		bool mSorted = false;
		UINT32 mSortOffset = 0;
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
		 * @param[in]	viewParams	Buffer containing properties of the view that's currently being rendered.
		 * @param[in]	gbuffer		Populated GBuffer with depths and normals.
		 * @param[in]	dt			Time step to advance the simulation by.
		 */
		void simulate(const SceneInfo& sceneInfo, const ParticleSimulationData* simData, 
			const SPtr<GpuParamBlockBuffer>& viewParams, const GBufferTextures& gbuffer, float dt);

		/** 
		 * Sorts the particle systems for the provided view. Only sorts systems using distance based sorting and only
		 * works on systems supporting compute. Sort results are written to a global buffer accessible through 
		 * getResources(), with offsets into the buffer written into particle system objects in @p sceneInfo.
		 */
		void sort(const RendererView& view);

		/** Returns textures used for storing particle data. */
		GpuParticleResources& getResources() const;
	private:
		/** Prepares buffer necessary for simulating the provided particle system. */
		void prepareBuffers(const GpuParticleSystem* system, const RendererParticles& rendererInfo);

		/** Clears out all the areas in particle textures as marked by the provided tiles to their default values. */
		void clearTiles(const Vector<UINT32>& tiles);

		/** Inserts the provided set of particles into the particle textures. */
		void injectParticles(const Vector<GpuParticle>& particles);

		Pimpl* m;
	};

	/** Contains textures that get updated with every run of the GPU particle simulation. */
	struct GpuParticleStateTextures
	{
		SPtr<Texture> positionAndTimeTex;
		SPtr<Texture> velocityTex;
	};

	/** Contains textures that contain data static throughout the particle's lifetime. */
	struct GpuParticleStaticTextures
	{
		SPtr<Texture> sizeAndRotationTex;
	};

	/** Contains a texture containing quantized versions of all curves used for the GPU particle system. */
	class GpuParticleCurves
	{
		static constexpr UINT32 TEX_SIZE = 1024;
		static constexpr UINT32 SCRATCH_NUM_VERTICES = 16384;
	public:
		GpuParticleCurves();
		~GpuParticleCurves();

		/** 
		 * Adds the provided set of pixels to the curve texture. Note you must call apply() to actually inject the
		 * pixels into the texture.
		 * 
		 * @param[in]	pixels		Pixels to inject into the curve.
		 * @param[in]	count		Number of pixels in the @p pixels array.
		 * @return					Allocation information about in which part of the texture the pixels were places.
		 */
		TextureRowAllocation alloc(Color* pixels, uint32_t count);

		/** Frees a previously allocated region. */
		void free(const TextureRowAllocation& alloc);

		/** 
		 * Injects all the newly added pixels into the curve texture (since the last call to this method). Should be
		 * called after alloc() has been called for all new entries, but before the texture is used for reading.
		 */
		void applyChanges();

		/** Returns the internal texture the curve data is written to. */
		const SPtr<Texture>& getTexture() const { return mCurveTexture; }

		/** Returns the UV coordinates at which the provided allocation starts. */
		static Vector2 getUVOffset(const TextureRowAllocation& alloc);

		/** 
		 * Returns a value which scales a value in range [0, 1] to a range of pixels of the provided allocation, where 0
		 * represents the left-most pixel, and 1 the right-most pixel.
		 */
		static float getUVScale(const TextureRowAllocation& alloc);

	private:
		/** Information about an allocation not yet injected into the curve texture. */
		struct PendingAllocation
		{
			Color* pixels;
			TextureRowAllocation allocation;
		};

		FrameAlloc mPendingAllocator;
		Vector<PendingAllocation> mPendingAllocations;

		SPtr<Texture> mCurveTexture;
		SPtr<RenderTexture> mRT;

		TextureRowAllocator<TEX_SIZE, TEX_SIZE> mRowAllocator;

		SPtr<VertexBuffer> mInjectUV;
		SPtr<IndexBuffer> mInjectIndices;
		SPtr<VertexDeclaration> mInjectVertexDecl;
		SPtr<VertexBuffer> mInjectScratch;
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

		/** Returns textures that contain the results from the previous simulation step. */
		GpuParticleStateTextures& getPreviousState() { return mStateTextures[mWriteBufferIdx ^ 0x1]; }

		/** Returns textures that contain the results from the last available simulation step. */
		GpuParticleStateTextures& getCurrentState() { return mStateTextures[mWriteBufferIdx]; }

		/** Returns a set of textures containing particle state that is static throughout the particle's lifetime. */
		const GpuParticleStaticTextures& getStaticTextures() const { return mStaticTextures; }

		/** Returns an object containing quantized curves for all particle systems. */
		GpuParticleCurves& getCurveTexture() { return mCurveTexture; }

		/** Returns the render target which can be used for injecting new particle data in the state textures. */
		const SPtr<RenderTexture>& getInjectTarget() const { return mInjectRT[mWriteBufferIdx ^ 0x1]; }

		/** Returns the render target which can be used for writing the results of the particle system simulation. */
		const SPtr<RenderTexture>& getSimulationTarget() const { return mSimulateRT[mWriteBufferIdx]; }

		/** Returns a global buffer containing particle indices for sorted particle systems. */
		const SPtr<GpuBuffer>& getSortedIndices() const;

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
		friend class GpuParticleSimulation;

		GpuParticleStateTextures mStateTextures[2];
		GpuParticleStaticTextures mStaticTextures;
		GpuParticleCurves mCurveTexture;
		GpuSortBuffers mSortBuffers;
		SPtr<GpuBuffer> mSortedIndices[2];
		UINT32 mSortedIndicesBufferIdx = 0;
		
		SPtr<RenderTexture> mSimulateRT[2];
		SPtr<RenderTexture> mInjectRT[2];

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