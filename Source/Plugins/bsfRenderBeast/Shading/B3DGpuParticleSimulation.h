//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DBitfield.h"
#include "Utility/B3DModule.h"
#include "Particles/B3DParticleScene.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Utility/B3DTextureRowAllocator.h"
#include "Utility/B3DGpuSort.h"
#include "B3DGpuParticleSimulationMaterials.h"

namespace b3d
{
	namespace render
	{
		struct ParticleRenderState;
		class ParticleSystemProxy;
		class ParticleSystemObjectStorage;
		class RenderBeastScene;
		class GpuParticleSimulateMaterial;
		struct GBufferTextures;
		class RenderBeastScene;
		class GpuParticleResources;
		class FrameGraph;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Contains information about a single tile allocated in the particle texture used for GPU simulation. */
		struct GpuParticleTile
		{
			u32 Id = (u32)-1;
			u32 NumFreeParticles = 0;

			/**
			 * Seconds remaining until every particle written to the tile has expired. Tracked as a countdown rather
			 * than an absolute expiry time, since the particle system time wraps around for looping systems and
			 * therefore cannot represent expiry times past the loop point.
			 */
			float RemainingLifetime = 0.0f;
		};

		/** Contains functionality specific to a single particle system simulated on the GPU. */
		class GpuParticleSystem
		{
		public:
			GpuParticleSystem() = default;
			~GpuParticleSystem() = default;

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
			bool AllocateTiles(GpuParticleResources& resources, Vector<GpuParticle>& newParticles, Vector<u32>& newTiles);

			/**
			 * Ages all tiles by the provided time step, then detects which tiles had all of their particles expire and
			 * marks them inactive so they can be re-used on the next call to AllocateTiles().
			 *
			 * @param[in]	timeStep	Time in seconds by which the GPU simulation advances the particles this frame.
			 */
			void DetectInactiveTiles(float timeStep);

			/** Releases any tiles that were marked as inactive so they may be re-used by some other particle system. */
			bool FreeInactiveTiles(GpuParticleResources& resources);

			/** Returns a buffer containing UV coordinates to which each of the allocate tiles map to. */
			TShared<GpuBuffer> GetTileUVs() const { return mTileUVs; }

			/** Returns a buffer containing per-particle indices used for locating particle data in the particle textures. */
			TShared<GpuBuffer> GetParticleIndices() const { return mParticleIndices; }

			/**
			 * Returns the total number of tiles used by this particle system. This may include inactive tiles unless you have
			 * freed them using freeInactiveTiles earlier.
			 */
			u32 GetTileCount() const { return (u32)mTiles.size(); }

			/** Returns the number of particles in the tiles of this particle system, as counted by GetTileCount(). */
			u32 GetParticleCount() const { return GetTileCount() * GpuParticleConstants::kParticlesPerTile; }

			/** Rebuilds ths internal buffers that contain tile UVs and per-particle UVs. */
			void UpdateGpuBuffers();

			/** Time since the system was created. */
			void SetTime(float time) { mTime = time; }

			/** @copydoc SetTime */
			float GetTime() const { return mTime; }

			/** Returns the object that can be used for retrieving random numbers when evaluating this particle system. */
			const Random& GetRandom() const { return mRandom; }

			/**
			 * Sets information about the results of particle system sorting.
			 *
			 * @param[in]	sorted		True if the system has information in the sorted index buffer.
			 * @param[in]	offset		Offset into the sorted index buffer. Only relevant if @p sorted is true.
			 */
			void SetSortInfo(bool sorted, u32 offset)
			{
				mSorted = sorted;

				if(sorted)
					mSortOffset = offset;
			}

			/** Returns true if the particle system has its indices stored in the sorted index buffer. */
			bool HasSortInfo() const { return mSorted; }

			/**
			 * Returns offset into the sorted index buffer at which indices of the particle system start. Only available if
			 * hasSortInfo() returns true.
			 */
			u32 GetSortOffset() const { return mSortOffset; }

			/** Sets GPU parameters prepared for simulation. */
			void SetSimulateParameters(const TShared<GpuParameterSet>& params) { mSimulateParameters = params; }

			/** Returns GPU parameters used for simulation. Parameters must have been set via a previous call to SetSimulateParameters. */
			const TShared<GpuParameterSet>& GetSimulateParameters() const { return mSimulateParameters; }

		private:
			Vector<GpuParticleTile> mTiles;
			TBitfield<> mActiveTiles;
			u32 mNumActiveTiles = 0;
			u32 mLastAllocatedTile = (u32)-1;
			float mTime = 0.0f;
			bool mSorted = false;
			u32 mSortOffset = 0;
			Random mRandom;

			TShared<GpuBuffer> mTileUVs;
			TShared<GpuBuffer> mParticleIndices;

			TShared<GpuParameterSet> mSimulateParameters;
		};

		/** Performs simulation for all particle systems that have GPU simulation enabled. */
		class GpuParticleSimulation : public Module<GpuParticleSimulation>
		{
			struct Pimpl;

		public:
			GpuParticleSimulation();
			~GpuParticleSimulation();

			/**
			 * Adds the passes that perform GPU particle simulation on all registered particle systems to @p graph. Simulates once per
			 * frame: later calls in the same frame do nothing.
			 *
			 * @param	graph		Graph to add the passes to.
			 * @param	scene		Scene currently being rendered.
			 * @param	simData		Particle simulation data output on the simulation thread.
			 * @param	viewParams	Buffer containing properties of the view that's currently being rendered.
			 * @param	gbuffer		Populated GBuffer with depths and normals, resources of @p graph.
			 * @param	timings		Timings of the frame, whose time step advances the simulation.
			 */
			void Simulate(FrameGraph& graph, const RenderBeastScene& scene, const EvaluatedParticleData* simData, const GpuBufferSuballocation& viewParams,
				const GBufferTextures& gbuffer, const FrameTimings& timings);

			/**
			 * Adds the passes that sort the particle systems for the provided view to @p graph. Only sorts systems using distance based
			 * sorting. Sort results are written to a global buffer accessible through GetResources(), with offsets into the buffer written
			 * into particle system objects in @p scene.
			 */
			void Sort(FrameGraph& graph, const RenderBeastScene& scene, const RendererView& view);

			/** Returns textures used for storing particle data. */
			GpuParticleResources& GetResources() const;

		private:
			/** Information for rendering a batch of tile clears. */
			struct TileClearBatch
			{
				TShared<GpuBuffer> TileUVs;
				TShared<GpuParameterSet> GpuParameters;
				u32 InstanceCount;
			};

			/** Information for rendering a batch of particle injections. */
			struct ParticleInjectBatch
			{
				TShared<GpuBuffer> Particles;
				u32 ParticleCount;
			};

			/** Prepares buffer necessary for simulating the provided particle system. */
			void PrepareBuffers(const GpuParticleSystem* system, const ParticleRenderState& particleRenderState);

			/** Prepares simulation parameter buffers for a single GPU particle system. */
			TShared<GpuParameterSet> PrepareSimulateParameters(const ParticleSystemProxy& proxy, const ParticleRenderState& renderState, const GpuParticleSystem& system, float dt);

			/**
			 * Writes the UVs of @p tiles into scratch buffers of the current frame, one per batch of tiles a single draw clears.
			 *
			 * @param tiles    List of tile IDs to clear.
			 * @return         Batches that clear the tiles, without their parameter sets assigned.
			 */
			TArray<TileClearBatch> PrepareClearTiles(const Vector<u32>& tiles);

			/** Clears out all the areas in particle textures as marked by the tiles of @p batches to their default values. */
			void DrawClearTiles(GpuCommandBuffer& commandBuffer, const TArray<TileClearBatch>& batches);

			/**
			 * Writes @p particles into scratch buffers of the current frame, one per batch of particles a single draw injects.
			 *
			 * @param particles    List of particles to inject.
			 * @return             Batches that inject the particles.
			 */
			TArray<ParticleInjectBatch> PrepareInjectParticles(const Vector<GpuParticle>& particles);

			/**
			 * Inserts the particles of @p batches into the particle textures.
			 *
			 * @param commandBuffer    Command buffer to record the draws to.
			 * @param parameters       Parameter set of the inject material.
			 * @param batches          Batches of particles to inject.
			 */
			void DrawInjectParticles(GpuCommandBuffer& commandBuffer, const TShared<GpuParameterSet>& parameters, const TArray<ParticleInjectBatch>& batches);

			Pimpl* m;
		};

		/** Contains textures that get updated with every run of the GPU particle simulation. */
		struct GpuParticleStateTextures
		{
			TShared<Texture> PositionAndTimeTex;
			TShared<Texture> VelocityTex;
		};

		/** Contains textures that contain data static throughout the particle's lifetime. */
		struct GpuParticleStaticTextures
		{
			TShared<Texture> SizeAndRotationTex;
		};

		/** Outputs of the GPU particle simulation that draws of GPU simulated particle systems read. */
		struct GpuParticleDrawInputs
		{
			TShared<Texture> PositionAndTime;
			TShared<Texture> SizeAndRotation;
			TShared<Texture> Curves;
			TShared<GpuBuffer> SortedIndices;

			/** Imports the resources into @p graph, which writes them in the passes of the simulation. */
			void Import(FrameGraph& graph) const;
		};

		/** Contains a texture containing quantized versions of all curves used for the GPU particle system. */
		class GpuParticleCurves
		{
			static constexpr u32 kTexSize = 1024;
			static constexpr u32 kScratchNumVertices = 16384;

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
			TextureRowAllocation Alloc(Color* pixels, uint32_t count);

			/** Frees a previously allocated region. */
			void Free(const TextureRowAllocation& alloc);

			/**
			 * Adds a pass that injects all the newly added pixels into the curve texture (since the last call to this method) to @p graph.
			 * Should be called after alloc() has been called for all new entries, but before the texture is used for reading.
			 */
			void ApplyChanges(FrameGraph& graph);

			/** Returns the internal texture the curve data is written to. */
			const TShared<Texture>& GetTexture() const { return mCurveTexture; }

			/** Returns the UV coordinates at which the provided allocation starts. */
			static Vector2 GetUvOffset(const TextureRowAllocation& alloc);

			/**
			 * Returns a value which scales a value in range [0, 1] to a range of pixels of the provided allocation, where 0
			 * represents the left-most pixel, and 1 the right-most pixel.
			 */
			static float GetUvScale(const TextureRowAllocation& alloc);

		private:
			/** Information about an allocation not yet injected into the curve texture. */
			struct PendingAllocation
			{
				Color* Pixels;
				TextureRowAllocation Allocation;
			};

			FrameAllocator mPendingAllocator;
			Vector<PendingAllocation> mPendingAllocations;

			TShared<Texture> mCurveTexture;

			TextureRowAllocator<kTexSize, kTexSize> mRowAllocator;

			TShared<GpuBuffer> mInjectUV;
			TShared<GpuBuffer> mInjectIndices;
			TShared<VertexDescription> mInjectVertexDescription;
		};

		/**
		 * Contains textures and buffers used for GPU particle simulation and handles allocation of tiles within the particle
		 * textures. State textures are double-buffered so one can be used for reading and other for writing during simulation.
		 */
		class GpuParticleResources
		{
		public:
			GpuParticleResources();

			/** Swap the read and write state textures. */
			void Swap() { mWriteBufferIdx ^= 0x1; }

			/** Returns textures that contain the results from the previous simulation step. */
			GpuParticleStateTextures& GetPreviousState() { return mStateTextures[mWriteBufferIdx ^ 0x1]; }

			/** Returns textures that contain the results from the last available simulation step. */
			GpuParticleStateTextures& GetCurrentState() { return mStateTextures[mWriteBufferIdx]; }

			/** @copydoc GetCurrentState() */
			const GpuParticleStateTextures& GetCurrentState() const { return mStateTextures[mWriteBufferIdx]; }

			/** Returns a set of textures containing particle state that is static throughout the particle's lifetime. */
			const GpuParticleStaticTextures& GetStaticTextures() const { return mStaticTextures; }

			/** Returns an object containing quantized curves for all particle systems. */
			GpuParticleCurves& GetCurveTexture() { return mCurveTexture; }

			/** @copydoc GetCurveTexture() */
			const GpuParticleCurves& GetCurveTexture() const { return mCurveTexture; }

			/** Returns a global buffer containing particle indices for sorted particle systems. */
			const TShared<GpuBuffer>& GetSortedIndices() const;

			/** Returns the outputs of the last available simulation step that draws read. */
			GpuParticleDrawInputs GetDrawInputs() const;

			/** Imports the state and static textures, the curve texture and the sorted index buffers into @p graph. */
			void Import(FrameGraph& graph) const;

			/**
			 * Attempts to allocate a new tile in particle textures. Returns index of the tile if successful or -1 if no more
			 * room.
			 */
			u32 AllocTile();

			/** Frees a tile previously allocated with allocTile(). */
			void FreeTile(u32 tile);

			/** Returns offset (in pixels) at which the tile with the specified index starts at. */
			static Vector2I GetTileOffset(u32 tileId);

			/** Returns the UV coordinates at which the tile with the specified index starts at. */
			static Vector2 GetTileCoords(u32 tileId);

			/**
			 * Returns the particle offset (in pixels) relative to the tile. @p subTileIdx represents he index of the particle
			 * in a tile.
			 */
			static Vector2I GetParticleOffset(u32 subTileId);

			/**
			 * Returns the particle coordinates relative to the tile. @p subTileIdx represents the index of the particle in
			 * a tile.
			 */
			static Vector2 GetParticleCoords(u32 subTileIdx);

		private:
			friend class GpuParticleSimulation;

			GpuParticleStateTextures mStateTextures[2];
			GpuParticleStaticTextures mStaticTextures;
			GpuParticleCurves mCurveTexture;
			TShared<GpuBuffer> mSortedIndices[2];
			u32 mSortedIndicesBufferIdx = 0;

			u32 mWriteBufferIdx = 0;

			u32 mFreeTiles[GpuParticleConstants::kTileCount];
			u32 mNumFreeTiles = GpuParticleConstants::kTileCount;
		};

		/** @} */
	} // namespace render
} // namespace b3d

namespace b3d
{
	B3D_IMPLEMENT_GLOBAL_POOL(render::GpuParticleSystem, 32)
}
