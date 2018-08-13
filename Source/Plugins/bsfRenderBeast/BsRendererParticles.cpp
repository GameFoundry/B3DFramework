//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererParticles.h"
#include "Particles/BsParticleManager.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMeshData.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Utility/BsGpuResourcePool.h"
#include "Utility/BsBitfield.h"

namespace bs { namespace ct
{
	const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY)
	{
		if (lockY)
		{
			switch (orient)
			{
			default:
			case ParticleOrientation::ViewPlane:
				return getParticleShaderVariation<ParticleOrientation::ViewPlane, true>();
			case ParticleOrientation::ViewPosition:
				return getParticleShaderVariation<ParticleOrientation::ViewPosition, true>();
			case ParticleOrientation::Plane:
				return getParticleShaderVariation<ParticleOrientation::Plane, true>();
			}
		}
		else
		{
			switch (orient)
			{
			default:
			case ParticleOrientation::ViewPlane:
				return getParticleShaderVariation<ParticleOrientation::ViewPlane, false>();
			case ParticleOrientation::ViewPosition:
				return getParticleShaderVariation<ParticleOrientation::ViewPosition, false>();
			case ParticleOrientation::Plane:
				return getParticleShaderVariation<ParticleOrientation::Plane, false>();
			}
		}
	}

	ParticlesParamDef gParticlesParamDef;

	ParticleTexturePool::~ParticleTexturePool()
	{
		for (auto& sizeEntry : mBufferList)
		{
			for (auto& entry : sizeEntry.second.buffers)
				mAlloc.destruct(entry);
		}
	}

	const ParticleTextures* ParticleTexturePool::alloc(const ParticleRenderData& renderData)
	{
		const UINT32 size = renderData.color.getWidth();

		const ParticleTextures* output = nullptr;
		BuffersPerSize& buffers = mBufferList[size];
		if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
		{
			output = buffers.buffers[buffers.nextFreeIdx];
			buffers.nextFreeIdx++;
		}

		if (!output)
		{
			output = createNewTextures(size);
			buffers.nextFreeIdx++;
		}

		// Populate texture contents
		// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
		// just use no-overwrite? write-discard will very likely allocate memory under the hood.
		output->positionAndRotation->writeData(renderData.positionAndRotation, 0, 0, true);
		output->color->writeData(renderData.color, 0, 0, true);
		output->sizeAndFrameIdx->writeData(renderData.sizeAndFrameIdx, 0, 0, true);

		const auto numParticles = (UINT32)renderData.indices.size();
		if(numParticles > 0)
		{
			auto* const indices = (UINT32*)output->indices->lock(GBL_WRITE_ONLY_DISCARD);
			const UINT32 numRows = Math::divideAndRoundUp(numParticles, size);

			UINT32 idx = 0;
			UINT32 y = 0;
			for (; y < numRows - 1; y++)
			{
				for (UINT32 x = 0; x < size; x++)
					indices[idx++] = (x & 0xFFFF) | (y << 16);
			}

			// Final row
			const UINT32 remainingParticles = numParticles - (numRows - 1) * size;
			for (UINT32 x = 0; x < remainingParticles; x++)
				indices[idx++] = (x & 0xFFFF) | (y << 16);

			output->indices->unlock();
		}

		return output;
	}

	void ParticleTexturePool::clear()
	{
		for(auto& buffers : mBufferList)
			buffers.second.nextFreeIdx = 0;
	}

	ParticleTextures* ParticleTexturePool::createNewTextures(UINT32 size)
	{
		ParticleTextures* output = mAlloc.construct<ParticleTextures>();

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = size;
		texDesc.height = size;
		texDesc.usage = TU_DYNAMIC;

		texDesc.format = PF_RGBA32F;
		output->positionAndRotation = Texture::create(texDesc);

		texDesc.format = PF_RGBA8;
		output->color = Texture::create(texDesc);

		texDesc.format = PF_RGBA16F;
		output->sizeAndFrameIdx = Texture::create(texDesc);

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STANDARD;
		bufferDesc.elementCount = size * size;
		bufferDesc.format = BF_16X2U;

		output->indices = GpuBuffer::create(bufferDesc);

		mBufferList[size].buffers.push_back(output);

		return output;
	}

	struct ParticleRenderer::Members
	{
		SPtr<VertexBuffer> billboardVB;
		SPtr<VertexDeclaration> billboardVD;
	};

	ParticleRenderer::ParticleRenderer()
		:m(bs_new<Members>())
	{
		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		m->billboardVD = VertexDeclaration::create(vertexDesc);

		VERTEX_BUFFER_DESC vbDesc;
		vbDesc.numVerts = 4;
		vbDesc.vertexSize = m->billboardVD->getProperties().getVertexSize(0);
		m->billboardVB = VertexBuffer::create(vbDesc);

		MeshData meshData(4, 0, vertexDesc);
		auto vecIter = meshData.getVec3DataIter(VES_POSITION);
		vecIter.addValue(Vector3(-0.5f, -0.5f, 0.0f));
		vecIter.addValue(Vector3(-0.5f, 0.5f, 0.0f));
		vecIter.addValue(Vector3(0.5f, -0.5f, 0.0f));
		vecIter.addValue(Vector3(0.5f, 0.5f, 0.0f));

		auto uvIter = meshData.getVec2DataIter(VES_TEXCOORD);
		uvIter.addValue(Vector2(0.0f, 1.0f));
		uvIter.addValue(Vector2(0.0f, 0.0f));
		uvIter.addValue(Vector2(1.0f, 1.0f));
		uvIter.addValue(Vector2(1.0f, 0.0f));

		m->billboardVB->writeData(0, meshData.getStreamSize(0), meshData.getStreamData(0), BWT_DISCARD);
	}

	ParticleRenderer::~ParticleRenderer()
	{
		bs_delete(m);
	}

	void ParticleRenderer::drawBillboards(UINT32 count)
	{
		SPtr<VertexBuffer> vertexBuffers[] = { m->billboardVB };

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setVertexDeclaration(m->billboardVD);
		rapi.setVertexBuffers(0, vertexBuffers, 1);
		rapi.setDrawOperation(DOT_TRIANGLE_STRIP);
		rapi.draw(0, 4, count);
	}

	// TODO - Doc
	struct GpuParticleTile
	{
		UINT32 id = (UINT32)-1;
		UINT32 numFreeParticles = 0;
		float lifetime = 0.0f;
	};

	// TODO - Doc
	struct GpuParticle
	{
		Vector3 position;
		float lifetime;
		Vector3 velocity;
		Vector2 dataUV;
	};

	struct GpuParticleStateTextures
	{
		SPtr<PooledRenderTexture> positionAndTimeTex;
		SPtr<PooledRenderTexture> velocityTex;
		SPtr<RenderTexture> renderTarget;
	};

	class GpuParticleResources;

	BS_PARAM_BLOCK_BEGIN(GpuParticleTileVertexParamsDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gUVToNDC)
	BS_PARAM_BLOCK_END

	GpuParticleTileVertexParamsDef gGpuParticleTileVertexParamsDef;
	
	class GpuParticleClearMat : public RendererMaterial<GpuParticleClearMat>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleClear.bsl");

	public:
		GpuParticleClearMat();

		// Renders to currently bound render target
		void bind(const SPtr<GpuBuffer>& tileUVs);

	private:
		GpuParamBuffer mTileUVParam;
	};

	class GpuParticleInjectMat : public RendererMaterial<GpuParticleInjectMat>
	{
		RMAT_DEF("GpuParticleInject.bsl");

	public:
		GpuParticleInjectMat();
	};

	class GpuParticleSimulateMat : public RendererMaterial<GpuParticleSimulateMat>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleSimulate.bsl");

	public:
		GpuParticleSimulateMat();

		// Renders to currently bound render target
		void bind(GpuParticleResources& resources);

		void setTileUVs(const SPtr<GpuBuffer>& tileUVs);

	private:
		GpuParamBuffer mTileUVParam;
		GpuParamTexture mPosAndTimeTexParam;
		GpuParamTexture mVelocityParam;
	};

	static constexpr UINT32 PARTICLE_TEX_SIZE = 1024;
	static constexpr UINT32 PARTICLE_TILE_SIZE = 4;
	static constexpr UINT32 PARTICLES_PER_TILE = PARTICLE_TILE_SIZE * PARTICLE_TILE_SIZE;
	static constexpr UINT32 TILE_COUNT_1D = PARTICLE_TEX_SIZE / PARTICLE_TILE_SIZE;
	static constexpr UINT32 TILE_COUNT = TILE_COUNT_1D * TILE_COUNT_1D;
	static constexpr UINT32 TILES_PER_INSTANCE = 8;
	static constexpr UINT32 PARTICLES_PER_INSTANCE = TILES_PER_INSTANCE * PARTICLES_PER_TILE;

	static_assert((PARTICLE_TEX_SIZE & (PARTICLE_TEX_SIZE - 1)) == 0, "Particle texture size not a power of two");
	static_assert((PARTICLE_TILE_SIZE & (PARTICLE_TILE_SIZE - 1)) == 0, "Particle tile size not a power of two");

	// TODO - Doc
	class GpuParticleResources
	{
	public:
		GpuParticleResources()
		{
			// Allocate textures
			const POOLED_RENDER_TEXTURE_DESC positionAndTimeDesc = 
				POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA32F, PARTICLE_TEX_SIZE, PARTICLE_TEX_SIZE, TU_RENDERTARGET);

			const POOLED_RENDER_TEXTURE_DESC velocityDesc =
				POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, PARTICLE_TEX_SIZE, PARTICLE_TEX_SIZE, TU_RENDERTARGET);

			for(UINT32 i = 0; i < 2; i++)
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
			for(UINT32 i = 0; i < TILE_COUNT; i++)
				mFreeTiles[i] = TILE_COUNT - i - 1;
		}

		void swap()
		{
			mWriteBufferIdx ^= 0x1;
		}

		GpuParticleStateTextures& getReadState()
		{
			return mStateTextures[mWriteBufferIdx ^ 0x1];
		}

		GpuParticleStateTextures& getWriteState()
		{
			return mStateTextures[mWriteBufferIdx];
		}

		// Returns - 1 if alloc failed
		UINT32 allocTile()
		{
			if(mNumFreeTiles > 0)
			{
				mNumFreeTiles--;
				return mFreeTiles[mNumFreeTiles];
			}

			return (UINT32)-1;
		}

		void freeTile(UINT32 tile)
		{
			assert(tile < TILE_COUNT);
			assert(mNumFreeTiles < TILE_COUNT);

			mFreeTiles[mNumFreeTiles] = tile;
			mNumFreeTiles++;
		}

		static Vector2 getTileCoords(UINT32 tileId)
		{
			return Vector2(
				Math::frac(tileId / (float)TILE_COUNT_1D),
				(UINT32)(tileId / TILE_COUNT_1D) / (float)TILE_COUNT_1D);
		}

		// Returns particle coordinates relative to a tile
		static Vector2 getParticleCoords(UINT32 subTileIdx)
		{
			const Vector2I tilePos(subTileIdx % PARTICLE_TILE_SIZE, subTileIdx / PARTICLE_TILE_SIZE);
			return tilePos / (float)PARTICLE_TEX_SIZE;
		}

	private:
		GpuParticleStateTextures mStateTextures[2];

		UINT32 mWriteBufferIdx = 0;

		UINT32 mFreeTiles[TILE_COUNT];
		UINT32 mNumFreeTiles = TILE_COUNT;
	};

	struct GpuParticleHelperBuffers
	{
		static constexpr UINT32 NUM_SCRATCH_TILES = 512;
		static constexpr UINT32 NUM_SCRATCH_PARTICLES = 4096;

		GpuParticleHelperBuffers()
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

			auto* const uvs = (Vector2*)spriteUVs->lock(GBL_WRITE_ONLY_NO_OVERWRITE);
			const float scale = PARTICLE_TILE_SIZE / (float)PARTICLE_TEX_SIZE;
			for(UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
			{
				uvs[i*4+0] = Vector2(0.0f, 0.0f) * scale;
				uvs[i*4+1] = Vector2(1.0f, 0.0f) * scale;
				uvs[i*4+2] = Vector2(1.0f, 1.0f) * scale;
				uvs[i*4+3] = Vector2(0.0f, 1.0f) * scale;
			}

			spriteUVs->unlock();

			// Prepare indices for rendering tiles & particles
			INDEX_BUFFER_DESC spriteIndexBufferDesc;
			spriteIndexBufferDesc.indexType = IT_16BIT;
			spriteIndexBufferDesc.numIndices = PARTICLES_PER_INSTANCE * 6;

			spriteIndices = IndexBuffer::create(spriteIndexBufferDesc);

			auto* const indices = (UINT16*)spriteIndices->lock(GBL_WRITE_ONLY_NO_OVERWRITE);
			for(UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
			{
				indices[i*6+0] = 0; indices[i*6+1] = 1; indices[i*6+2] = 2;
				indices[i*6+3] = 0; indices[i*6+4] = 2; indices[i*6+5] = 3;
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
		
		SPtr<VertexBuffer> spriteUVs;
		SPtr<IndexBuffer> spriteIndices;
		SPtr<VertexDeclaration> tileVertexDecl;
		SPtr<VertexDeclaration> injectVertexDecl;
		SPtr<GpuBuffer> tileScratch;
		SPtr<VertexBuffer> injectScratch;
	};

	class GpuParticleSystem
	{
	public:
		bool allocateTiles(GpuParticleResources& resources, Vector<GpuParticle>& newParticles, Vector<UINT32>& newTiles)
		{
			GpuParticleTile cachedTile = mLastAllocatedTile == (UINT32)-1 ? GpuParticleTile() : mTiles[mLastAllocatedTile];
			Vector2 tileUV = GpuParticleResources::getTileCoords(cachedTile.id);

			bool newTilesAdded = false;
			for(UINT32 i = 0; i < (UINT32)newParticles.size(); i++)
			{
				UINT32 tileIdx;

				// Use the last allocated tile if there's room
				if(cachedTile.numFreeParticles > 0)
					tileIdx = mLastAllocatedTile;
				else
				{
					// Otherwise try to find an inactive tile
					if(mNumActiveTiles < (UINT32)mTiles.size())
					{
						tileIdx = mActiveTiles.find(false);
						mActiveTiles[tileIdx] = true;
					}
					// And finally just allocate a new tile if no room elsewhere
					else
					{
						const UINT32 tileId = resources.allocTile();
						if(tileId == (UINT32)-1)
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
					mTiles[tileIdx].numFreeParticles = PARTICLES_PER_TILE;

					mNumActiveTiles++;
				}

				GpuParticleTile& tile = mTiles[tileIdx];
				GpuParticle& particle = newParticles[i];

				const UINT32 tileParticleIdx = PARTICLES_PER_TILE - tile.numFreeParticles;
				particle.dataUV = tileUV + GpuParticleResources::getParticleCoords(tileParticleIdx);

				tile.numFreeParticles--;
				tile.lifetime = std::max(tile.lifetime, particle.lifetime);

				cachedTile.numFreeParticles--;
			}

			return newTilesAdded;
		}

		void detectInactiveTiles(float time)
		{
			mNumActiveTiles = 0;
			for(UINT32 i = 0; i < (UINT32)mTiles.size(); i++)
			{
				if(mTiles[i].lifetime >= time)
				{
					mNumActiveTiles++;
					continue;
				}

				mActiveTiles[i] = false;

				if(mLastAllocatedTile == i)
					mLastAllocatedTile = (UINT32)-1;
			}
		}

		bool freeInactiveTiles(GpuParticleResources& resources)
		{
			const UINT32 numFreeTiles = (UINT32)mTiles.size() - mNumActiveTiles;
			while(numFreeTiles > 0)
			{
				const UINT32 freeIdx = mActiveTiles.find(false);
				assert(freeIdx != (UINT32)-1);

				const UINT32 lastIdx = (UINT32)mTiles.size() - 1;

				if(freeIdx != lastIdx)
				{
					std::swap(mTiles[freeIdx], mTiles[lastIdx]);
					std::swap(mActiveTiles[freeIdx], mActiveTiles[lastIdx]);
				}

				resources.freeTile(mTiles[lastIdx].id);

				mTiles.erase(mTiles.end() - 1);
				mActiveTiles.remove(lastIdx);
			}

			// Tile order changed so this might no longer be valid
			if(numFreeTiles > 0)
				mLastAllocatedTile = (UINT32)-1;

			return numFreeTiles > 0;
		}

		SPtr<GpuBuffer> getTileUVs() const { return mTileOffsets; }
		UINT32 getNumTiles() const { return (UINT32)mTiles.size(); }

		void updateGpuBuffers()
		{
			const auto numTiles = (UINT32)mTiles.size();
			const UINT32 numTilesToAllocates = Math::divideAndRoundUp(numTiles, TILES_PER_INSTANCE) * TILES_PER_INSTANCE;

			// Tile offsets buffer
			GPU_BUFFER_DESC tilesBufferDesc;
			tilesBufferDesc.type = GBT_STANDARD;
			tilesBufferDesc.format = BF_32X2U;
			tilesBufferDesc.elementCount = numTilesToAllocates;

			mTileOffsets = GpuBuffer::create(tilesBufferDesc);

			auto* tileUVs = (Vector2*)mTileOffsets->lock(GBL_WRITE_ONLY_NO_OVERWRITE);
			for(UINT32 i = 0; i < numTiles; i++)
				tileUVs[i] = GpuParticleResources::getTileCoords(mTiles[i].id);

			for(UINT32 i = numTiles; i < numTilesToAllocates; i++)
				tileUVs[i] = Vector2(0.0f, 0.0f); // Out of range

			mTileOffsets->unlock();

			// Particle data UV
			const UINT32 numParticles = numTiles * PARTICLES_PER_TILE;

			VERTEX_BUFFER_DESC particleUVDesc;
			particleUVDesc.numVerts = numParticles;
			particleUVDesc.vertexSize = sizeof(UINT32);

			mParticleUVs = VertexBuffer::create(particleUVDesc);
			auto* particleUVs = (UINT32*)mParticleUVs->lock(GBL_WRITE_ONLY_NO_OVERWRITE);

			const Vector2 halfPixel = Vector2::ONE * (0.5f / (float)PARTICLE_TEX_SIZE);
			for(UINT32 i = 0; i < numTiles; i++)
			{
				const Vector2 tileUV = GpuParticleResources::getTileCoords(mTiles[i].id);

				for(UINT32 y = 0; y < PARTICLE_TILE_SIZE; y++)
				{
					for(UINT32 x = 0; x < PARTICLE_TILE_SIZE; x++)
					{
						Vector2 particleUV(
							x / (float)PARTICLE_TEX_SIZE,
							y / (float)PARTICLE_TEX_SIZE);

						particleUV += tileUV + halfPixel;

						UINT32 encodedUV = Bitwise::floatToHalf(particleUV.x);
						encodedUV |= Bitwise::floatToHalf(particleUV.y) << 16;

						particleUVs[i] = encodedUV;
					}
				}
			}

			mParticleUVs->unlock();
		}

	private:
		Vector<GpuParticleTile> mTiles;
		Bitfield mActiveTiles;
		UINT32 mNumActiveTiles = 0;
		UINT32 mLastAllocatedTile = (UINT32)-1;

		SPtr<GpuBuffer> mTileOffsets;
		SPtr<VertexBuffer> mParticleUVs;
	};

	class GpuParticles
	{
	public:
		void addSystem(GpuParticleSystem* system)
		{
			// TODO
		}

		void removeSystem(GpuParticleSystem* system)
		{
			// TODO
		}

		void simulate(float dt)
		{
			mResources.swap();

			Vector<UINT32> newTiles;
			Vector<GpuParticle> allNewParticles;
			for(auto& entry : mSystems)
			{
				entry->detectInactiveTiles(dt);

				// TODO - Convert from ParticleSet to a list of GpuParticle's
				// TODO - Particle time must be mapped to [0, 1] range
				Vector<GpuParticle> newParticles;

				bool tilesDirty = entry->allocateTiles(mResources, newParticles, newTiles);
				tilesDirty |= entry->freeInactiveTiles(mResources);

				if(tilesDirty)
					entry->updateGpuBuffers();

				allNewParticles.insert(allNewParticles.end(), newParticles.begin(), newParticles.end());
			}

			GpuParticleStateTextures& readState = mResources.getReadState();
			GpuParticleStateTextures& writeState = mResources.getWriteState();

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(readState.renderTarget);

			clearTiles(newTiles);
			injectParticles(allNewParticles);

			// Simulate
			rapi.setRenderTarget(writeState.renderTarget);

			GpuParticleSimulateMat* simulateMat = GpuParticleSimulateMat::get();
			simulateMat->bind(mResources);

			rapi.setVertexDeclaration(mHelperBuffers.tileVertexDecl);

			SPtr<VertexBuffer> buffers[] = { mHelperBuffers.spriteUVs };
			rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
			rapi.setIndexBuffer(mHelperBuffers.spriteIndices);
			rapi.setDrawOperation(DOT_TRIANGLE_LIST);

			for(auto& entry : mSystems)
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

		void clearTiles(const Vector<UINT32>& tiles)
		{
			const auto numTiles = (UINT32)tiles.size();
			const UINT32 numIterations = Math::divideAndRoundUp(numTiles, GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

			GpuParticleClearMat* clearMat = GpuParticleClearMat::get();
			clearMat->bind(mHelperBuffers.tileScratch);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setVertexDeclaration(mHelperBuffers.tileVertexDecl);

			SPtr<VertexBuffer> buffers[] = { mHelperBuffers.spriteUVs };
			rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
			rapi.setIndexBuffer(mHelperBuffers.spriteIndices);
			rapi.setDrawOperation(DOT_TRIANGLE_LIST);

			UINT32 tileStart = 0;
			for(UINT32 i = 0; i < numIterations; i++)
			{
				const UINT32 tileEnd = std::min(numTiles, tileStart + GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

				auto* tileUVs = (Vector2*)mHelperBuffers.tileScratch->lock(GBL_WRITE_ONLY_DISCARD);
				for(UINT32 j = tileStart; j < tileEnd; j++)
					tileUVs[j] = GpuParticleResources::getTileCoords(tiles[j]);

				mHelperBuffers.tileScratch->unlock();

				const UINT32 numInstances = Math::divideAndRoundUp(tileEnd - tileStart, TILES_PER_INSTANCE);
				rapi.drawIndexed(0, PARTICLES_PER_INSTANCE * 6, 0, PARTICLES_PER_INSTANCE * 4, numInstances);

				tileStart = tileEnd;
			}
		}

		void injectParticles(const Vector<GpuParticle>& particles)
		{
			const auto numParticles = (UINT32)particles.size();
			const UINT32 numIterations = Math::divideAndRoundUp(numParticles, GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

			GpuParticleInjectMat* injectMat = GpuParticleInjectMat::get();
			injectMat->bind();

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setVertexDeclaration(mHelperBuffers.injectVertexDecl);

			SPtr<VertexBuffer> buffers[] = { mHelperBuffers.injectScratch, mHelperBuffers.spriteUVs };
			rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
			rapi.setIndexBuffer(mHelperBuffers.spriteIndices);
			rapi.setDrawOperation(DOT_TRIANGLE_LIST);

			UINT32 particleStart = 0;
			for(UINT32 i = 0; i < numIterations; i++)
			{
				const UINT32 particleEnd = std::min(numParticles, particleStart + GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

				auto* particleData = (GpuParticle*)mHelperBuffers.injectScratch->lock(GBL_WRITE_ONLY_DISCARD);
				for(UINT32 j = particleStart; j < particleEnd; j++)
					particleData[j] = particles[j];

				mHelperBuffers.injectScratch->unlock();

				rapi.drawIndexed(0, 6, 0, 4, particleEnd - particleStart);
				particleStart = particleEnd;
			}
		}

	private:
		GpuParticleResources mResources;
		GpuParticleHelperBuffers mHelperBuffers;
		Vector<GpuParticleSystem*> mSystems;
	};

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
