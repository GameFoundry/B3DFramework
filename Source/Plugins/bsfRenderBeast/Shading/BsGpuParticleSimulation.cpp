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

	static constexpr UINT32 TILES_PER_INSTANCE = 8;
	static constexpr UINT32 PARTICLES_PER_INSTANCE = TILES_PER_INSTANCE * GpuParticleResources::PARTICLES_PER_TILE;

	/** Contains a variety of helper buffers and declarations used for GPU particle simulation. */
	struct GpuParticleHelperBuffers
	{
		static constexpr UINT32 NUM_SCRATCH_TILES = 512;
		static constexpr UINT32 NUM_SCRATCH_PARTICLES = 4096;

		GpuParticleHelperBuffers();
		
		SPtr<VertexBuffer> spriteUVs;
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
			tileId % TILE_COUNT_1D,
			tileId / TILE_COUNT_1D);
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
		injectVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 3, 0, 1); // Data UV, per instance
		injectVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD, 4, 1); // Sprite texture coordinates

		injectVertexDecl = VertexDeclaration::create(injectVertexDesc);

		// Prepare UV coordinates for rendering tiles & particles
		VERTEX_BUFFER_DESC spriteUVBufferDesc;
		spriteUVBufferDesc.numVerts = PARTICLES_PER_INSTANCE * 4;
		spriteUVBufferDesc.vertexSize = tileVertexDesc->getVertexStride();

		spriteUVs = VertexBuffer::create(spriteUVBufferDesc);

		auto* const uvs = (Vector2*)spriteUVs->lock(GBL_WRITE_ONLY_DISCARD);
		const float scale = GpuParticleResources::TILE_SIZE / (float)GpuParticleResources::TEX_SIZE;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			uvs[i * 4 + 0] = Vector2(0.0f, 0.0f) * scale;
			uvs[i * 4 + 1] = Vector2(1.0f, 0.0f) * scale;
			uvs[i * 4 + 2] = Vector2(1.0f, 1.0f) * scale;
			uvs[i * 4 + 3] = Vector2(0.0f, 1.0f) * scale;
		}

		spriteUVs->unlock();

		// Prepare indices for rendering tiles & particles
		INDEX_BUFFER_DESC spriteIndexBufferDesc;
		spriteIndexBufferDesc.indexType = IT_16BIT;
		spriteIndexBufferDesc.numIndices = PARTICLES_PER_INSTANCE * 6;

		spriteIndices = IndexBuffer::create(spriteIndexBufferDesc);

		auto* const indices = (UINT16*)spriteIndices->lock(GBL_WRITE_ONLY_DISCARD);
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			indices[i * 6 + 0] = 0; indices[i * 6 + 1] = 1; indices[i * 6 + 2] = 2;
			indices[i * 6 + 3] = 0; indices[i * 6 + 4] = 2; indices[i * 6 + 5] = 3;
		}

		spriteIndices->unlock();

		// Prepare a scratch buffer we'll use to clear tiles
		GPU_BUFFER_DESC tileScratchBufferDesc;
		tileScratchBufferDesc.type = GBT_STANDARD;
		tileScratchBufferDesc.format = BF_32X2U;
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
					newTile.id = resources.allocTile();
					newTile.lifetime = 0.0f;

					tileIdx = (UINT32)mTiles.size();
					newTiles.push_back(newTile.id);
					mTiles.push_back(newTile);

					newTilesAdded = true;
				}

				mLastAllocatedTile = tileIdx;
				tileUV = GpuParticleResources::getTileCoords(mTiles[tileIdx].id);
				mTiles[tileIdx].numFreeParticles = GpuParticleResources::PARTICLES_PER_TILE;

				mNumActiveTiles++;
			}

			GpuParticleTile& tile = mTiles[tileIdx];
			GpuParticle& particle = newParticles[i];

			const UINT32 tileParticleIdx = GpuParticleResources::PARTICLES_PER_TILE - tile.numFreeParticles;
			particle.dataUV = tileUV + GpuParticleResources::getParticleCoords(tileParticleIdx);

			tile.numFreeParticles--;
			tile.lifetime = std::max(tile.lifetime, mTime + (particle.initialLifetime - particle.lifetime));

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
		while (numFreeTiles > 0)
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
		GPU_BUFFER_DESC tilesBufferDesc;
		tilesBufferDesc.type = GBT_STANDARD;
		tilesBufferDesc.format = BF_32X2U;
		tilesBufferDesc.elementCount = numTilesToAllocates;

		mTileUVs = GpuBuffer::create(tilesBufferDesc);

		auto* tileUVs = (Vector2*)mTileUVs->lock(GBL_WRITE_ONLY_NO_OVERWRITE);
		for (UINT32 i = 0; i < numTiles; i++)
			tileUVs[i] = GpuParticleResources::getTileCoords(mTiles[i].id);

		for (UINT32 i = numTiles; i < numTilesToAllocates; i++)
			tileUVs[i] = Vector2(0.0f, 0.0f); // Out of range

		mTileUVs->unlock();

		// Particle data offsets
		const UINT32 numParticles = numTiles * GpuParticleResources::PARTICLES_PER_TILE;

		GPU_BUFFER_DESC particleUVDesc;
		particleUVDesc.type = GBT_STANDARD;
		particleUVDesc.format = BF_16X2U;
		particleUVDesc.elementCount = numParticles;

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

	struct GpuParticleSimulation::Pimpl
	{
		GpuParticleResources resources;
		GpuParticleHelperBuffers helperBuffers;
		UnorderedSet<GpuParticleSystem*> systems;
	};

	GpuParticleSimulation::GpuParticleSimulation()
		:m(bs_unique_ptr_new<Pimpl>())
	{ }

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
			const UINT32 tileEnd = std::min(numTiles, tileStart + GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

			auto* tileUVs = (Vector2*)m->helperBuffers.tileScratch->lock(GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = tileStart; j < tileEnd; j++)
				tileUVs[j] = GpuParticleResources::getTileCoords(tiles[j]);

			m->helperBuffers.tileScratch->unlock();

			const UINT32 numInstances = Math::divideAndRoundUp(tileEnd - tileStart, TILES_PER_INSTANCE);
			rapi.drawIndexed(0, PARTICLES_PER_INSTANCE * 6, 0, PARTICLES_PER_INSTANCE * 4, numInstances);

			tileStart = tileEnd;
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

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.injectScratch, m->helperBuffers.spriteUVs };
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
}}
