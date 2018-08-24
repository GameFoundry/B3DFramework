//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuParticleSimulation.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsGpuResourcePool.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs { namespace ct 
{
	BS_PARAM_BLOCK_BEGIN(GpuParticleTileVertexParamsDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gUVToNDC)
	BS_PARAM_BLOCK_END

	GpuParticleTileVertexParamsDef gGpuParticleTileVertexParamsDef;
	
	/** Material used for clearing tiles in the texture used for particle GPU simulation. */
	class GpuParticleClearMat : public RendererMaterial<GpuParticleClearMat>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleClear.bsl");

	public:
		GpuParticleClearMat();

		/** Binds the material to the pipeline, along with the @p tileUVs buffer containing locations of tiles to clear. */
		void bind(const SPtr<GpuBuffer>& tileUVs);

	private:
		GpuParamBuffer mTileUVParam;
	};

	/** Material used for adding new particles into the particle state textures. */
	class GpuParticleInjectMat : public RendererMaterial<GpuParticleInjectMat>
	{
		RMAT_DEF("GpuParticleInject.bsl");

	public:
		GpuParticleInjectMat();
	};

	/** 
	 * Material used for performing GPU particle simulation. State is read from the provided input textures and output
	 * into the output textures bound as render targets.
	 */
	class GpuParticleSimulateMat : public RendererMaterial<GpuParticleSimulateMat>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleSimulate.bsl");

	public:
		GpuParticleSimulateMat();

		/** Binds the material to the pipeline along with any input textures. */
		void bind(GpuParticleResources& resources);

		/** Sets the UV offsets of individual tiles for a particular particle system that's being rendered. */
		void setTileUVs(const SPtr<GpuBuffer>& tileUVs);

	private:
		GpuParamBuffer mTileUVParam;
		GpuParamTexture mPosAndTimeTexParam;
		GpuParamTexture mVelocityParam;
	};

	BS_PARAM_BLOCK_BEGIN(GpuParticleBoundsParamsDef)
		BS_PARAM_BLOCK_ENTRY(UINT32, gIterationsPerGroup)
		BS_PARAM_BLOCK_ENTRY(UINT32, gNumExtraIterations)
		BS_PARAM_BLOCK_ENTRY(UINT32, gNumParticles)
	BS_PARAM_BLOCK_END

	GpuParticleBoundsParamsDef gGpuParticleBoundsParamsDef;

	/** Material used for calculating particle system bounds. */
	class GpuParticleBoundsMat : public RendererMaterial<GpuParticleBoundsMat>
	{
		static constexpr UINT32 NUM_THREADS = 64;

		RMAT_DEF_CUSTOMIZED("GpuParticleBounds.bsl");

	public:
		GpuParticleBoundsMat();

		/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
		void bind(const SPtr<Texture>& positionAndTime);

		/** 
		 * Executes the material, calculating the bounds. Note that this function reads back from the GPU and should not
		 * be called at runtime.
		 *
		 * @param[in]	indices			Buffer containing offsets into the position texture for each particle.
		 * @param[in]	numParticles	Number of particle in the provided indices buffer.
		 */
		AABox execute(const SPtr<GpuBuffer>& indices, UINT32 numParticles);

	private:
		GpuParamBuffer mParticleIndicesParam;
		GpuParamBuffer mOutputParam;
		GpuParamTexture mPosAndTimeTexParam;
		SPtr<GpuParamBlockBuffer> mInputBuffer;
	};

	static constexpr UINT32 TILES_PER_INSTANCE = 8;
	static constexpr UINT32 PARTICLES_PER_INSTANCE = TILES_PER_INSTANCE * GpuParticleResources::PARTICLES_PER_TILE;

	/** Contains a variety of helper buffers and declarations used for GPU particle simulation. */
	struct GpuParticleHelperBuffers
	{
		static constexpr UINT32 NUM_SCRATCH_TILES = 512;
		static constexpr UINT32 NUM_SCRATCH_PARTICLES = 4096;

		GpuParticleHelperBuffers();
		
		SPtr<VertexBuffer> spriteUVs;
		SPtr<VertexBuffer> particleUVs;
		SPtr<IndexBuffer> spriteIndices;
		SPtr<VertexDeclaration> tileVertexDecl;
		SPtr<VertexDeclaration> injectVertexDecl;
		SPtr<GpuBuffer> tileScratch;
		SPtr<VertexBuffer> injectScratch;
	};

	GpuParticleResources::GpuParticleResources()
	{
		// Allocate textures
		const POOLED_RENDER_TEXTURE_DESC positionAndTimeDesc =
			POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA32F, TEX_SIZE, TEX_SIZE, TU_RENDERTARGET);

		const POOLED_RENDER_TEXTURE_DESC velocityDesc =
			POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, TEX_SIZE, TEX_SIZE, TU_RENDERTARGET);

		for (UINT32 i = 0; i < 2; i++)
		{
			mStateTextures[i].positionAndTimeTex = GpuResourcePool::instance().get(positionAndTimeDesc);
			mStateTextures[i].velocityTex = GpuResourcePool::instance().get(velocityDesc);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = mStateTextures[i].positionAndTimeTex->texture;
			rtDesc.colorSurfaces[1].texture = mStateTextures[i].velocityTex->texture;

			mStateTextures[i].renderTarget = RenderTexture::create(rtDesc);
		}

		// TODO - Allocate textures for other properties (not necessarily double-buffered):
		//	      size, rotation, etc.

		// Clear the free tile linked list
		for (UINT32 i = 0; i < TILE_COUNT; i++)
			mFreeTiles[i] = TILE_COUNT - i - 1;
	}

	UINT32 GpuParticleResources::allocTile()
	{
		if (mNumFreeTiles > 0)
		{
			mNumFreeTiles--;
			return mFreeTiles[mNumFreeTiles];
		}

		return (UINT32)-1;
	}

	void GpuParticleResources::freeTile(UINT32 tile)
	{
		assert(tile < TILE_COUNT);
		assert(mNumFreeTiles < TILE_COUNT);

		mFreeTiles[mNumFreeTiles] = tile;
		mNumFreeTiles++;
	}

	Vector2I GpuParticleResources::getTileOffset(UINT32 tileId)
	{
		return Vector2I(
			(tileId % TILE_COUNT_1D) * TILE_SIZE,
			(tileId / TILE_COUNT_1D) * TILE_SIZE);
	}

	Vector2 GpuParticleResources::getTileCoords(UINT32 tileId)
	{
		return Vector2(
			Math::frac(tileId / (float)TILE_COUNT_1D),
			(UINT32)(tileId / TILE_COUNT_1D) / (float)TILE_COUNT_1D);
	}

	Vector2I GpuParticleResources::getParticleOffset(UINT32 subTileId)
	{
		return Vector2I(
			subTileId % TILE_SIZE,
			subTileId / TILE_SIZE);
	}

	Vector2 GpuParticleResources::getParticleCoords(UINT32 subTileId)
	{
		const Vector2I tileOffset = getParticleOffset(subTileId);
		return tileOffset / (float)TEX_SIZE;
	}

	GpuParticleHelperBuffers::GpuParticleHelperBuffers()
	{
		// Prepare vertex declaration for rendering tiles
		SPtr<VertexDataDesc> tileVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		tileVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		tileVertexDecl = VertexDeclaration::create(tileVertexDesc);

		// Prepare vertex declaration for injecting new particles
		SPtr<VertexDataDesc> injectVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		injectVertexDesc->addVertElem(VET_FLOAT4, VES_TEXCOORD, 0, 0, 1); // Position & time, per instance
		injectVertexDesc->addVertElem(VET_FLOAT4, VES_TEXCOORD, 1, 0, 1); // Velocity, per instance
		injectVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 2, 0, 1); // Data UV, per instance
		injectVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 3, 1); // Sprite texture coordinates

		injectVertexDecl = VertexDeclaration::create(injectVertexDesc);

		// Prepare UV coordinates for rendering tiles
		VERTEX_BUFFER_DESC spriteUVBufferDesc;
		spriteUVBufferDesc.numVerts = PARTICLES_PER_INSTANCE * 4;
		spriteUVBufferDesc.vertexSize = tileVertexDesc->getVertexStride();

		spriteUVs = VertexBuffer::create(spriteUVBufferDesc);

		auto* const spriteUVData = (Vector2*)spriteUVs->lock(GBL_WRITE_ONLY_DISCARD);
		const float spriteUVScale = GpuParticleResources::TILE_SIZE / (float)GpuParticleResources::TEX_SIZE;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			spriteUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * spriteUVScale;
			spriteUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * spriteUVScale;
			spriteUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * spriteUVScale;
			spriteUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * spriteUVScale;
		}

		spriteUVs->unlock();

		// Prepare UV coordinates for rendering particles
		VERTEX_BUFFER_DESC particleUVBufferDesc;
		particleUVBufferDesc.numVerts = PARTICLES_PER_INSTANCE * 4;
		particleUVBufferDesc.vertexSize = tileVertexDesc->getVertexStride();

		particleUVs = VertexBuffer::create(particleUVBufferDesc);

		auto* const particleUVData = (Vector2*)particleUVs->lock(GBL_WRITE_ONLY_DISCARD);
		const float particleUVScale = 1.0f / (float)GpuParticleResources::TEX_SIZE;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			particleUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * particleUVScale;
			particleUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * particleUVScale;
			particleUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * particleUVScale;
			particleUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * particleUVScale;
		}

		particleUVs->unlock();

		// Prepare indices for rendering tiles & particles
		INDEX_BUFFER_DESC spriteIndexBufferDesc;
		spriteIndexBufferDesc.indexType = IT_16BIT;
		spriteIndexBufferDesc.numIndices = PARTICLES_PER_INSTANCE * 6;

		spriteIndices = IndexBuffer::create(spriteIndexBufferDesc);

		auto* const indices = (UINT16*)spriteIndices->lock(GBL_WRITE_ONLY_DISCARD);
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			indices[i * 6 + 0] = i * 4 + 0; indices[i * 6 + 1] = i * 4 + 1; indices[i * 6 + 2] = i * 4 + 2;
			indices[i * 6 + 3] = i * 4 + 0; indices[i * 6 + 4] = i * 4 + 2; indices[i * 6 + 5] = i * 4 + 3;
		}

		spriteIndices->unlock();

		// Prepare a scratch buffer we'll use to clear tiles
		GPU_BUFFER_DESC tileScratchBufferDesc;
		tileScratchBufferDesc.type = GBT_STANDARD;
		tileScratchBufferDesc.format = BF_32X2F;
		tileScratchBufferDesc.elementCount = NUM_SCRATCH_TILES;
		tileScratchBufferDesc.usage = GBU_DYNAMIC;

		tileScratch = GpuBuffer::create(tileScratchBufferDesc);

		// Prepare a scratch buffer we'll use to inject new particles
		VERTEX_BUFFER_DESC injectScratchBufferDesc;
		injectScratchBufferDesc.numVerts = NUM_SCRATCH_PARTICLES;
		injectScratchBufferDesc.vertexSize = injectVertexDesc->getVertexStride(0);
		injectScratchBufferDesc.usage = GBU_DYNAMIC;

		injectScratch = VertexBuffer::create(injectScratchBufferDesc);
	}

	GpuParticleSystem::GpuParticleSystem(UINT32 id)
		:mId(id)
	{
		GpuParticleSimulation::instance().addSystem(this);
	}

	GpuParticleSystem::~GpuParticleSystem()
	{
		GpuParticleSimulation::instance().removeSystem(this);
	}

	bool GpuParticleSystem::allocateTiles(GpuParticleResources& resources, Vector<GpuParticle>& newParticles, 
		Vector<UINT32>& newTiles)
	{
		GpuParticleTile cachedTile = mLastAllocatedTile == (UINT32)-1 ? GpuParticleTile() : mTiles[mLastAllocatedTile];
		Vector2 tileUV = GpuParticleResources::getTileCoords(cachedTile.id);

		bool newTilesAdded = false;
		for (UINT32 i = 0; i < (UINT32)newParticles.size(); i++)
		{
			UINT32 tileIdx;

			// Use the last allocated tile if there's room
			if (cachedTile.numFreeParticles > 0)
				tileIdx = mLastAllocatedTile;
			else
			{
				// Otherwise try to find an inactive tile
				if (mNumActiveTiles < (UINT32)mTiles.size())
				{
					tileIdx = mActiveTiles.find(false);
					mActiveTiles[tileIdx] = true;
				}
				// And finally just allocate a new tile if no room elsewhere
				else
				{
					const UINT32 tileId = resources.allocTile();
					if (tileId == (UINT32)-1)
						return newTilesAdded; // Out of space in the texture

					GpuParticleTile newTile;
					newTile.id = tileId;
					newTile.lifetime = 0.0f;

					tileIdx = (UINT32)mTiles.size();
					newTiles.push_back(newTile.id);
					mTiles.push_back(newTile);
					mActiveTiles.add(true);

					newTilesAdded = true;
				}

				mLastAllocatedTile = tileIdx;
				tileUV = GpuParticleResources::getTileCoords(mTiles[tileIdx].id);
				mTiles[tileIdx].numFreeParticles = GpuParticleResources::PARTICLES_PER_TILE;

				cachedTile = mTiles[tileIdx];
				mNumActiveTiles++;
			}

			GpuParticleTile& tile = mTiles[tileIdx];
			GpuParticle& particle = newParticles[i];

			const UINT32 tileParticleIdx = GpuParticleResources::PARTICLES_PER_TILE - tile.numFreeParticles;
			particle.dataUV = tileUV + GpuParticleResources::getParticleCoords(tileParticleIdx);

			tile.numFreeParticles--;
			tile.lifetime = std::max(tile.lifetime, mTime + particle.lifetime);

			cachedTile.numFreeParticles--;


		}

		return newTilesAdded;
	}

	void GpuParticleSystem::detectInactiveTiles()
	{
		mNumActiveTiles = 0;
		for (UINT32 i = 0; i < (UINT32)mTiles.size(); i++)
		{
			if (mTiles[i].lifetime >= mTime)
			{
				mNumActiveTiles++;
				continue;
			}

			mActiveTiles[i] = false;

			if (mLastAllocatedTile == i)
				mLastAllocatedTile = (UINT32)-1;
		}
	}

	bool GpuParticleSystem::freeInactiveTiles(GpuParticleResources& resources)
	{
		const UINT32 numFreeTiles = (UINT32)mTiles.size() - mNumActiveTiles;
		for(UINT32 i = 0; i < numFreeTiles; i++)
		{
			const UINT32 freeIdx = mActiveTiles.find(false);
			assert(freeIdx != (UINT32)-1);

			const UINT32 lastIdx = (UINT32)mTiles.size() - 1;

			if (freeIdx != lastIdx)
			{
				std::swap(mTiles[freeIdx], mTiles[lastIdx]);
				std::swap(mActiveTiles[freeIdx], mActiveTiles[lastIdx]);
			}

			resources.freeTile(mTiles[lastIdx].id);

			mTiles.erase(mTiles.end() - 1);
			mActiveTiles.remove(lastIdx);
		}

		// Tile order changed so this might no longer be valid
		if (numFreeTiles > 0)
			mLastAllocatedTile = (UINT32)-1;

		return numFreeTiles > 0;
	}

	void GpuParticleSystem::updateGpuBuffers()
	{
		const auto numTiles = (UINT32)mTiles.size();
		const UINT32 numTilesToAllocates = Math::divideAndRoundUp(numTiles, TILES_PER_INSTANCE) * TILES_PER_INSTANCE;

		// Tile offsets buffer
		if(numTiles > 0)
		{
			GPU_BUFFER_DESC tilesBufferDesc;
			tilesBufferDesc.type = GBT_STANDARD;
			tilesBufferDesc.format = BF_32X2F;
			tilesBufferDesc.elementCount = numTilesToAllocates;
			tilesBufferDesc.usage = GBU_DYNAMIC;

			mTileUVs = GpuBuffer::create(tilesBufferDesc);

			auto* tileUVs = (Vector2*)mTileUVs->lock(GBL_WRITE_ONLY_NO_OVERWRITE);
			for (UINT32 i = 0; i < numTiles; i++)
				tileUVs[i] = GpuParticleResources::getTileCoords(mTiles[i].id);

			for (UINT32 i = numTiles; i < numTilesToAllocates; i++)
				tileUVs[i] = Vector2(0.0f, 0.0f); // Out of range

			mTileUVs->unlock();
		}

		// Particle data offsets
		const UINT32 numParticles = numTiles * GpuParticleResources::PARTICLES_PER_TILE;

		if(numParticles > 0)
		{
			GPU_BUFFER_DESC particleUVDesc;
			particleUVDesc.type = GBT_STANDARD;
			particleUVDesc.format = BF_16X2U;
			particleUVDesc.elementCount = numParticles;
			particleUVDesc.usage = GBU_DYNAMIC;

			mParticleIndices = GpuBuffer::create(particleUVDesc);
			auto* particleIndices = (UINT32*)mParticleIndices->lock(GBL_WRITE_ONLY_NO_OVERWRITE);

			UINT32 idx = 0;
			for (UINT32 i = 0; i < numTiles; i++)
			{
				const Vector2I tileOffset = GpuParticleResources::getTileOffset(mTiles[i].id);
				for (UINT32 y = 0; y < GpuParticleResources::TILE_SIZE; y++)
				{
					for (UINT32 x = 0; x < GpuParticleResources::TILE_SIZE; x++)
					{
						const Vector2I offset = tileOffset + Vector2I(x, y);
						particleIndices[idx++] = (offset.x & 0xFFFF) | (offset.y << 16);
					}
				}
			}

			mParticleIndices->unlock();
		}
	}

	struct GpuParticleSimulation::Pimpl
	{
		GpuParticleResources resources;
		GpuParticleHelperBuffers helperBuffers;
		UnorderedSet<GpuParticleSystem*> systems;
	};

	GpuParticleSimulation::GpuParticleSimulation()
		:m(bs_new<Pimpl>())
	{ }

	GpuParticleSimulation::~GpuParticleSimulation()
	{
		bs_delete(m);		
	}

	void GpuParticleSimulation::addSystem(GpuParticleSystem* system)
	{
		m->systems.insert(system);
	}

	void GpuParticleSimulation::removeSystem(GpuParticleSystem* system)
	{
		m->systems.erase(system);
	}

	void GpuParticleSimulation::simulate(const ParticleSimulationData* simData, float dt)
	{
		m->resources.swap();

		Vector<UINT32> newTiles;
		Vector<GpuParticle> allNewParticles;
		for (auto& entry : m->systems)
		{
			entry->detectInactiveTiles();

			bool tilesDirty = false;
			const auto iterFind = simData->gpuData.find(entry->getId());
			if(iterFind != simData->gpuData.end())
			{
				Vector<GpuParticle>& newParticles = iterFind->second->particles;
				tilesDirty = entry->allocateTiles(m->resources, newParticles, newTiles);

				allNewParticles.insert(allNewParticles.end(), newParticles.begin(), newParticles.end());
			}

			entry->advanceTime(dt);
			tilesDirty |= entry->freeInactiveTiles(m->resources);

			if (tilesDirty)
				entry->updateGpuBuffers();
		}

		GpuParticleStateTextures& readState = m->resources.getReadState();
		GpuParticleStateTextures& writeState = m->resources.getWriteState();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(readState.renderTarget);

		clearTiles(newTiles);
		injectParticles(allNewParticles);

		// Simulate
		rapi.setRenderTarget(writeState.renderTarget);

		GpuParticleSimulateMat* simulateMat = GpuParticleSimulateMat::get();
		simulateMat->bind(m->resources);

		rapi.setVertexDeclaration(m->helperBuffers.tileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.spriteUVs };
		rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.setIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		for (auto& entry : m->systems)
		{
			if(entry->getNumTiles() == 0)
				continue;

			simulateMat->setTileUVs(entry->getTileUVs());

			const UINT32 tileCount = entry->getNumTiles();
			const UINT32 numInstances = Math::divideAndRoundUp(tileCount, TILES_PER_INSTANCE);
			rapi.drawIndexed(0, PARTICLES_PER_INSTANCE * 6, 0, PARTICLES_PER_INSTANCE * 4, numInstances);
		}

		// TODO - (For later) Sort the particles (How to handle this per-simulation?)

		// TODO - (In another method) Actually send the particles for rendering using the particle buffer
		//   - Or the sorted buffer if available
	}

	void GpuParticleSimulation::clearTiles(const Vector<UINT32>& tiles)
	{
		const auto numTiles = (UINT32)tiles.size();
		if(numTiles == 0)
			return;

		const UINT32 numIterations = Math::divideAndRoundUp(numTiles, GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

		GpuParticleClearMat* clearMat = GpuParticleClearMat::get();
		clearMat->bind(m->helperBuffers.tileScratch);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setVertexDeclaration(m->helperBuffers.tileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.spriteUVs };
		rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.setIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		UINT32 tileStart = 0;
		for (UINT32 i = 0; i < numIterations; i++)
		{
			static_assert(GpuParticleHelperBuffers::NUM_SCRATCH_TILES % TILES_PER_INSTANCE == 0, 
				"Tile scratch buffer size must be divisble with number of tiles per instance.");

			const UINT32 tileEnd = std::min(numTiles, tileStart + GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

			auto* tileUVs = (Vector2*)m->helperBuffers.tileScratch->lock(GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = tileStart; j < tileEnd; j++)
				tileUVs[j] = GpuParticleResources::getTileCoords(tiles[j]);

			const UINT32 alignedTileEnd = Math::divideAndRoundUp(tileEnd, TILES_PER_INSTANCE) * TILES_PER_INSTANCE;
			for (UINT32 j = tileEnd; j < alignedTileEnd; j++)
				tileUVs[j] = Vector2(2.0f, 2.0f); // Out of bounds (we don't want to accidentaly clear used tiles)

			m->helperBuffers.tileScratch->unlock();

			const UINT32 numInstances = (alignedTileEnd - tileStart) / TILES_PER_INSTANCE;
			rapi.drawIndexed(0, PARTICLES_PER_INSTANCE * 6, 0, PARTICLES_PER_INSTANCE * 4, numInstances);

			tileStart = alignedTileEnd;
		}
	}

	void GpuParticleSimulation::injectParticles(const Vector<GpuParticle>& particles)
	{
		const auto numParticles = (UINT32)particles.size();
		const UINT32 numIterations = Math::divideAndRoundUp(numParticles, GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

		GpuParticleInjectMat* injectMat = GpuParticleInjectMat::get();
		injectMat->bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setVertexDeclaration(m->helperBuffers.injectVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.injectScratch, m->helperBuffers.particleUVs };
		rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.setIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		UINT32 particleStart = 0;
		for (UINT32 i = 0; i < numIterations; i++)
		{
			const UINT32 particleEnd = std::min(numParticles, particleStart + GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

			auto* particleData = (GpuParticleVertex*)m->helperBuffers.injectScratch->lock(GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = particleStart; j < particleEnd; j++)
				particleData[j] = particles[j].getVertex();

			m->helperBuffers.injectScratch->unlock();

			rapi.drawIndexed(0, 6, 0, 4, particleEnd - particleStart);
			particleStart = particleEnd;
		}
	}

	GpuParticleResources& GpuParticleSimulation::getResources() const
	{
		return m->resources;
	}

	SPtr<GpuParamBlockBuffer> createGpuParticleVertexInputBuffer()
	{
		SPtr<GpuParamBlockBuffer> inputBuffer = gGpuParticleTileVertexParamsDef.createBuffer();

		// [0, 1] -> [-1, 1] and flip Y
		Vector4 uvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		// Either of these flips the Y axis, but if they're both true they cancel out
		if (rapiInfo.isFlagSet(RenderAPIFeatureFlag::UVYAxisUp) ^ rapiInfo.isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown))
		{
			uvToNdc.y = -uvToNdc.y;
			uvToNdc.w = -uvToNdc.w;
		}

		gGpuParticleTileVertexParamsDef.gUVToNDC.set(inputBuffer, uvToNdc);

		return inputBuffer;
	}

	GpuParticleClearMat::GpuParticleClearMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();

		mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
		mParams->getBufferParam(GPT_VERTEX_PROGRAM, "gTileUVs", mTileUVParam);
	}

	void GpuParticleClearMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILES_PER_INSTANCE", TILES_PER_INSTANCE);
	}

	void GpuParticleClearMat::bind(const SPtr<GpuBuffer>& tileUVs)
	{
		mTileUVParam.set(tileUVs);

		RendererMaterial::bind();
	}

	GpuParticleInjectMat::GpuParticleInjectMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();
		mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
	}

	GpuParticleSimulateMat::GpuParticleSimulateMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();
		mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
		
		mParams->getBufferParam(GPT_VERTEX_PROGRAM, "gTileUVs", mTileUVParam);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gPosAndTimeTex", mPosAndTimeTexParam);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gVelocityTex", mVelocityParam);
	}

	void GpuParticleSimulateMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILES_PER_INSTANCE", TILES_PER_INSTANCE);
	}

	void GpuParticleSimulateMat::bind(GpuParticleResources& resources)
	{
		GpuParticleStateTextures& readState = resources.getReadState();
		GpuParticleStateTextures& writeState = resources.getWriteState();

		mPosAndTimeTexParam.set(readState.positionAndTimeTex->texture);
		mVelocityParam.set(readState.velocityTex->texture);

		RendererMaterial::bind();
	}

	void GpuParticleSimulateMat::setTileUVs(const SPtr<GpuBuffer>& tileUVs)
	{
		mTileUVParam.set(tileUVs);
	}

	GpuParticleBoundsMat::GpuParticleBoundsMat()
	{
		mInputBuffer = gGpuParticleBoundsParamsDef.createBuffer();
		mParams->setParamBlockBuffer(GPT_COMPUTE_PROGRAM, "Input", mInputBuffer);
		
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gParticleIndices", mParticleIndicesParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputParam);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gPosAndTimeTex", mPosAndTimeTexParam);
	}

	void GpuParticleBoundsMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("NUM_THREADS", NUM_THREADS);
	}

	void GpuParticleBoundsMat::bind(const SPtr<Texture>& positionAndTime)
	{
		mPosAndTimeTexParam.set(positionAndTime);

		RendererMaterial::bind();
	}

	AABox GpuParticleBoundsMat::execute(const SPtr<GpuBuffer>& indices, UINT32 numParticles)
	{
		static constexpr UINT32 MAX_NUM_GROUPS = 128;

		const UINT32 numIterations = Math::divideAndRoundUp(numParticles, NUM_THREADS);
		const UINT32 numGroups = std::min(numIterations, MAX_NUM_GROUPS);

		const UINT32 iterationsPerGroup = numIterations / numGroups;
		const UINT32 extraIterations = numIterations % numGroups;

		gGpuParticleBoundsParamsDef.gIterationsPerGroup.set(mInputBuffer, iterationsPerGroup);
		gGpuParticleBoundsParamsDef.gNumExtraIterations.set(mInputBuffer, extraIterations);
		gGpuParticleBoundsParamsDef.gNumParticles.set(mInputBuffer, numParticles);

		GPU_BUFFER_DESC outputDesc;
		outputDesc.type = GBT_STANDARD;
		outputDesc.format = BF_32X2U;
		outputDesc.elementCount = numGroups * 2;
		outputDesc.usage = GBU_DYNAMIC;

		SPtr<GpuBuffer> output = GpuBuffer::create(outputDesc);

		mParticleIndicesParam.set(indices);
		mOutputParam.set(output);

		RenderAPI::instance().dispatchCompute(numGroups);

		Vector3 min = Vector3::INF;
		Vector3 max = -Vector3::INF;

		const Vector3* data = (Vector3*)output->lock(GBL_READ_ONLY);
		for(UINT32 i = 0; i < numGroups; i++)
		{
			min = Vector3::min(min, data[i * 2 + 0]);
			max = Vector3::min(max, data[i * 2 + 1]);
		}

		output->unlock();

		return AABox(min, max);
	}

	/** Information about an allocation in a single row of a texture. */
	struct TextureRowAllocation
	{
		/** Starting pixels of the allocation. */
		uint16_t x = 0, y = 0;

		/** Number of pixels in the allocation. */
		uint32_t length = 0;
	};
		
	/** Allocates elements of variable size within rows of a texture. */
	template<uint32_t WIDTH, uint32_t HEIGHT>
	class TextureRowAllocator : public INonCopyable
	{
	public:
		TextureRowAllocator();
		~TextureRowAllocator();

		/** 
		 * Attempts to allocate a new region of size @p length pixels. Returned allocation will have the same size as the
		 * requested size if sucessful.
		 */
		TextureRowAllocation alloc(uint32_t length);

		/** Frees a previously allocated region. */
		void free(const TextureRowAllocation& alloc);

	private:
		/** Describes a single contigous region of a texture row. */
		struct RowRegion
		{
			// Note: 'next' must be the first member because of shenanigans we do in alloc() and free()
			RowRegion* next = nullptr;
			uint32_t x = 0;
			uint32_t length = WIDTH;
		};

		RowRegion* mFreeRegions[HEIGHT];
		PoolAlloc<sizeof(RowRegion), HEIGHT * 2> mAlloc;
	};

	template <uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocator<WIDTH, HEIGHT>::TextureRowAllocator()
	{
		for(uint32_t i = 0; i < HEIGHT; i++)
			mFreeRegions[i] = mAlloc.template construct<RowRegion>();
	}

	template <uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocator<WIDTH, HEIGHT>::~TextureRowAllocator()
	{
		for(uint32_t i = 0; i < HEIGHT; i++)
		{
			RowRegion* region = mFreeRegions[i];
			while(region)
			{
				RowRegion* curRegion = region;
				region = region->next;

				mAlloc.free(curRegion);
			}
		}
	}

	template<uint32_t WIDTH, uint32_t HEIGHT>
	TextureRowAllocation TextureRowAllocator<WIDTH, HEIGHT>::alloc(uint32_t length)
	{
		TextureRowAllocation output;

		for(uint32_t i = 0; i < HEIGHT; i++)
		{
			RowRegion* region = mFreeRegions[i];
			RowRegion* prevRegion = &mFreeRegions[i]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions
			while(region)
			{
				if(region->length == length)
				{
					output.x = region->x;
					output.y = i;
					output.length = length;

					prevRegion->next = region->next;
					mAlloc.free(region);

					return output;
				}

				if(region->length > length)
				{
					output.x = region->x;
					output.y = i;
					output.length = length;

					region->x += length;
					region->length -= length;

					return output;
				}

				prevRegion = region;
				region = region->next;
			}
		}

		return output;
	}

	template <uint32_t WIDTH, uint32_t HEIGHT>
	void TextureRowAllocator<WIDTH, HEIGHT>::free(const TextureRowAllocation& alloc)
	{
		if(alloc.length == 0)
			return;

		RowRegion* region = mFreeRegions[alloc.y];
		RowRegion* prevRegion = &mFreeRegions[alloc.y]; // This ensures an assignment to prevRegion->next changes the entry of mFreeRegions

		if(region)
		{
			// Find the location where to insert the free region
			while (region && alloc.x > (region->x + region->length))
			{
				prevRegion = region;
				region = region->next;

				// While we're iterating, also merge any contigous regions
				if (region && region->x == (prevRegion->x + prevRegion->length))
				{
					prevRegion->length += region->length;
					prevRegion->next = region->next;

					mAlloc.free(region);
					region = prevRegion;
				}
			}

			const uint32_t regionEnd = region->x + region->length;
			if(region)
			{
				// End of the allocation is the beginning of this region
				if((alloc.x + alloc.length) == region->x)
				{
					region->x -= alloc.length;
					region->length += alloc.length;
				}

				// Beginning of the allocation is at the end of this region
				if (alloc.x == regionEnd)
				{
					region->length += alloc.length;

					// Merge any directly following regions
					prevRegion = region;
					region = region->next;

					while(region && region->x == (prevRegion->x + prevRegion->length))
					{
						prevRegion->length += region->length;
						prevRegion->next = region->next;

						RowRegion* toDelete = region;
						region = region->next;

						mAlloc.free(toDelete);
					}

					return;
				}
			}
		}
			
		RowRegion* newRegion = mAlloc.template alloc<RowRegion>();
		newRegion->x = alloc.x;
		newRegion->length = alloc.length;
		newRegion->next = region;

		prevRegion->next = newRegion;
	}
}}
