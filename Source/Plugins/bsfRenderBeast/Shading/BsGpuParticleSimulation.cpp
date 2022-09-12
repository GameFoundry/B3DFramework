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
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Particles/BsVectorField.h"
#include "Particles/BsParticleDistribution.h"
#include "Math/BsVector3.h"
#include "BsRendererParticles.h"
#include "BsRendererScene.h"
#include "BsRenderBeast.h"
#include "Utility/BsGpuSort.h"

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
		void Bind(const SPtr<GpuBuffer>& tileUVs);

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

	/** Material used for adding new curves into the curve texture. */
	class GpuParticleCurveInjectMat : public RendererMaterial<GpuParticleCurveInjectMat>
	{
		RMAT_DEF("GpuParticleCurveInject.bsl");

	public:
		GpuParticleCurveInjectMat();
	};

	BS_PARAM_BLOCK_BEGIN(VectorFieldParamsDef)
		BS_PARAM_BLOCK_ENTRY(Vector3, gFieldBounds)
		BS_PARAM_BLOCK_ENTRY(float, gFieldIntensity)
		BS_PARAM_BLOCK_ENTRY(Vector3, gFieldTiling)
		BS_PARAM_BLOCK_ENTRY(float, gFieldTightness)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gWorldToField)
		BS_PARAM_BLOCK_ENTRY(Matrix3, gFieldToWorld)
	BS_PARAM_BLOCK_END

	VectorFieldParamsDef gVectorFieldParamsDef;

	BS_PARAM_BLOCK_BEGIN(GpuParticleDepthCollisionParamsDef)
		BS_PARAM_BLOCK_ENTRY(float, gCollisionRange)
		BS_PARAM_BLOCK_ENTRY(float, gRestitution)
		BS_PARAM_BLOCK_ENTRY(float, gDampening)
		BS_PARAM_BLOCK_ENTRY(float, gCollisionRadiusScale)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleCurveOffset)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleCurveScale)
	BS_PARAM_BLOCK_END

	GpuParticleDepthCollisionParamsDef gGpuParticleDepthCollisionParamsDef;

	BS_PARAM_BLOCK_BEGIN(GpuParticleSimulateParamsDef)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumVectorFields)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumIterations)
		BS_PARAM_BLOCK_ENTRY(float, gDT)
		BS_PARAM_BLOCK_ENTRY(float, gDrag)
		BS_PARAM_BLOCK_ENTRY(Vector3, gAcceleration)
	BS_PARAM_BLOCK_END

	GpuParticleSimulateParamsDef gGpuParticleSimulateParamsDef;

	/**
	 * Material used for performing GPU particle simulation. State is read from the provided input textures and output
	 * into the output textures bound as render targets.
	 */
	class GpuParticleSimulateMat : public RendererMaterial<GpuParticleSimulateMat>
	{
		RMAT_DEF_CUSTOMIZED("GpuParticleSimulate.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 DEPTH_COLLISIONS>
		static const ShaderVariation& GetVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("DEPTH_COLLISIONS", DEPTH_COLLISIONS)
			});

			return variation;
		}
	public:
		GpuParticleSimulateMat();

		/** Binds the material to the pipeline along with any frame-static parameters. */
		void bindGlobal(GpuParticleResources& resources, const SPtr<GpuParamBlockBuffer>& viewParams,
			const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuParamBlockBuffer>& simulationParams);

		/**
		 * Binds parameters that change with every material dispatch.
		 *
		 * @param[in]	tileUVs					Sets the UV offsets of individual tiles for a particular particle system
		 *										that's being rendered.
		 * @param[in]	perObjectParams			General purpose particle system parameters.
		 * @param[in]	vectorFieldParams		Information about the currently bound vector field, if any.
		 * @param[in]	vectorFieldTexture		3D texture representing the vector field, or null if none.
		 * @param[in]	depthCollisionParams	Parameter buffer for controlling depth buffer collisions, if enabled.
		 *
		 */
		void bindPerCallParams(const SPtr<GpuBuffer>& tileUVs, const SPtr<GpuParamBlockBuffer>& perObjectParams,
			const SPtr<GpuParamBlockBuffer>& vectorFieldParams, const SPtr<Texture>& vectorFieldTexture,
			const SPtr<GpuParamBlockBuffer>& depthCollisionParams);

		/** Returns the material variation matching the provided parameters. */
		static GpuParticleSimulateMat* getVariation(bool depthCollisions, bool localSpace);
	private:
		GpuParamBuffer mTileUVParam;
		GpuParamTexture mPosAndTimeTexParam;
		GpuParamTexture mVelocityTexParam;
		GpuParamTexture mSizeRotationTexParam;
		GpuParamTexture mCurvesTexParam;
		GpuParamTexture mDepthTexParam;
		GpuParamTexture mNormalsTexParam;
		GpuParamBinding mParamsBinding;
		GpuParamBinding mPerCameraBinding;
		GpuParamBinding mPerObjectBinding;

		GpuParamBinding mVectorFieldBinding;
		GpuParamTexture mVectorFieldTexParam;

		GpuParamBinding mDepthCollisionBinding;

		bool mSupportsDepthCollisions;
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
		void Bind(const SPtr<Texture>& positionAndTime);

		/**
		 * Executes the material, calculating the bounds. Note that this function reads back from the GPU and should not
		 * be called at runtime.
		 *
		 * @param[in]	indices			Buffer containing offsets into the position texture for each particle.
		 * @param[in]	numParticles	Number of particle in the provided indices buffer.
		 */
		AABox Execute(const SPtr<GpuBuffer>& indices, UINT32 numParticles);

	private:
		GpuParamBuffer mParticleIndicesParam;
		GpuParamBuffer mOutputParam;
		GpuParamTexture mPosAndTimeTexParam;
		SPtr<GpuParamBlockBuffer> mInputBuffer;
	};

	BS_PARAM_BLOCK_BEGIN(GpuParticleSortPrepareParamDef)
		BS_PARAM_BLOCK_ENTRY(INT32, gIterationsPerGroup)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumExtraIterations)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumParticles)
		BS_PARAM_BLOCK_ENTRY(INT32, gOutputOffset)
		BS_PARAM_BLOCK_ENTRY(INT32, gSystemKey)
		BS_PARAM_BLOCK_ENTRY(Vector3, gLocalViewOrigin)
	BS_PARAM_BLOCK_END

	GpuParticleSortPrepareParamDef gGpuParticleSortPrepareParamDef;

	/** Material used for preparing key/values buffers used for particle sorting. */
	class GpuParticleSortPrepareMat : public RendererMaterial<GpuParticleSortPrepareMat>
	{
		static constexpr UINT32 NUM_THREADS = 64;

		RMAT_DEF_CUSTOMIZED("GpuParticleSortPrepare.bsl");

	public:
		GpuParticleSortPrepareMat();

		/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
		void Bind(const SPtr<Texture>& positionAndTime);

		/**
		 * Executes the material, generating sort data for a particular particle system and injecting it into the specified
		 * location in the key and index buffers.
		 *
		 * @param[in]	system			System whose particles to insert into the sort key/index buffers.
		 * @param[in]	systemIdx		Sequential index of the system to insert into the sort buffers.
		 * @param[in]	offset			Offset into the key/index buffer at which to insert the sort data.
		 * @param[in]	viewOrigin		View origin to use for determining sorting keys, in world space.
		 * @param[out]	outKeys			Pre-allocated buffer that will receive the keys used for sorting. The buffer must
		 *								be GPU writable and use a 1x 32-bit integer format.
		 * @param[out]	outIndices		Pre-allocated buffer that will receive the indices to be sorted. The buffer must
		 *								be GPU writable and use a 2x 16-bit integer format. Must have the same capacity
		 *								as @p outKeys.
		 * @return						Number of particle that were written to the buffers.
		 */
		UINT32 execute(const GpuParticleSystem& system, UINT32 systemIdx, const Vector3& viewOrigin, UINT32 offset,
			const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices);

	private:
		GpuParamBuffer mInputIndicesParam;
		GpuParamBuffer mOutputKeysParam;
		GpuParamBuffer mOutputIndicesParam;
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
		
		SPtr<VertexBuffer> tileUVs;
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
		TEXTURE_DESC positionAndTimeDesc;
		positionAndTimeDesc.format = PF_RGBA32F;
		positionAndTimeDesc.width = TEX_SIZE;
		positionAndTimeDesc.height = TEX_SIZE;
		positionAndTimeDesc.usage = TU_RENDERTARGET;

		TEXTURE_DESC velocityDesc;
		velocityDesc.format = PF_RGBA16F;
		velocityDesc.width = TEX_SIZE;
		velocityDesc.height = TEX_SIZE;
		velocityDesc.usage = TU_RENDERTARGET;

		for (UINT32 i = 0; i < 2; i++)
		{
			mStateTextures[i].positionAndTimeTex = Texture::create(positionAndTimeDesc);
			mStateTextures[i].velocityTex = Texture::create(velocityDesc);
		}

		TEXTURE_DESC sizeAndRotationDesc;
		sizeAndRotationDesc.format = PF_RGBA16F;
		sizeAndRotationDesc.width = TEX_SIZE;
		sizeAndRotationDesc.height = TEX_SIZE;
		sizeAndRotationDesc.usage = TU_RENDERTARGET;

		mStaticTextures.sizeAndRotationTex = Texture::create(sizeAndRotationDesc);

		RENDER_TEXTURE_DESC staticRtDesc;
		staticRtDesc.colorSurfaces[0].texture = mStaticTextures.sizeAndRotationTex;

		for (UINT32 i = 0; i < 2; i++)
		{
			RENDER_TEXTURE_DESC simulationRTDesc;
			simulationRTDesc.colorSurfaces[0].texture = mStateTextures[i].positionAndTimeTex;
			simulationRTDesc.colorSurfaces[1].texture = mStateTextures[i].velocityTex;

			mSimulateRT[i] = RenderTexture::create(simulationRTDesc);

			RENDER_TEXTURE_DESC injectRTDesc;
			injectRTDesc.colorSurfaces[0].texture = mStateTextures[i].positionAndTimeTex;
			injectRTDesc.colorSurfaces[1].texture = mStateTextures[i].velocityTex;
			injectRTDesc.colorSurfaces[2].texture = mStaticTextures.sizeAndRotationTex;
			mInjectRT[i] = RenderTexture::create(injectRTDesc);
		}

		// Allocate the buffer containing keys used for sorting
		GPU_BUFFER_DESC sortKeysBufferDesc;
		sortKeysBufferDesc.type = GBT_STANDARD;
		sortKeysBufferDesc.format = BF_32X1U;
		sortKeysBufferDesc.elementCount = TEX_SIZE * TEX_SIZE;
		sortKeysBufferDesc.usage = GBU_LOADSTORE;

		mSortBuffers.keys[0] = GpuBuffer::create(sortKeysBufferDesc);
		mSortBuffers.keys[1] = GpuBuffer::create(sortKeysBufferDesc);

		// Allocate the buffer containing sorted particle indices
		GPU_BUFFER_DESC sortedIndicesBufferDesc;
		sortedIndicesBufferDesc.type = GBT_STANDARD;
		sortedIndicesBufferDesc.format = BF_16X2U;
		sortedIndicesBufferDesc.elementCount = TEX_SIZE * TEX_SIZE;
		sortedIndicesBufferDesc.usage = GBU_LOADSTORE;

		mSortedIndices[0] = GpuBuffer::create(sortedIndicesBufferDesc);
		mSortedIndices[1] = GpuBuffer::create(sortedIndicesBufferDesc);

		mSortBuffers.values[0] = mSortedIndices[0]->GetView(GBT_STANDARD, BF_32X1U);
		mSortBuffers.values[1] = mSortedIndices[1]->GetView(GBT_STANDARD, BF_32X1U);

		// Clear the free tile linked list
		for (UINT32 i = 0; i < TILE_COUNT; i++)
			mFreeTiles[i] = TILE_COUNT - i - 1;
	}

	UINT32 GpuParticleResources::AllocTile()
	{
		if (mNumFreeTiles > 0)
		{
			mNumFreeTiles--;
			return mFreeTiles[mNumFreeTiles];
		}

		return (UINT32)-1;
	}

	void GpuParticleResources::FreeTile(UINT32 tile)
	{
		assert(tile < TILE_COUNT);
		assert(mNumFreeTiles < TILE_COUNT);

		mFreeTiles[mNumFreeTiles] = tile;
		mNumFreeTiles++;
	}

	Vector2I GpuParticleResources::GetTileOffset(UINT32 tileId)
	{
		return Vector2I(
			(tileId % TILE_COUNT_1D) * TILE_SIZE,
			(tileId / TILE_COUNT_1D) * TILE_SIZE);
	}

	Vector2 GpuParticleResources::GetTileCoords(UINT32 tileId)
	{
		return Vector2(
			Math::frac(tileId / (float)TILE_COUNT_1D),
			(UINT32)(tileId / TILE_COUNT_1D) / (float)TILE_COUNT_1D);
	}

	Vector2I GpuParticleResources::GetParticleOffset(UINT32 subTileId)
	{
		return Vector2I(
			subTileId % TILE_SIZE,
			subTileId / TILE_SIZE);
	}

	Vector2 GpuParticleResources::GetParticleCoords(UINT32 subTileId)
	{
		const Vector2I tileOffset = getParticleOffset(subTileId);
		return tileOffset / (float)TEX_SIZE;
	}

	const SPtr<GpuBuffer>& GpuParticleResources::GetSortedIndices() const
	{
		return mSortedIndices[mSortedIndicesBufferIdx];
	}

	GpuParticleHelperBuffers::GpuParticleHelperBuffers()
	{
		// Prepare vertex declaration for rendering tiles
		SPtr<VertexDataDesc> tileVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		tileVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);

		tileVertexDecl = VertexDeclaration::create(tileVertexDesc);

		// Prepare vertex declaration for injecting new particles
		SPtr<VertexDataDesc> injectVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		injectVertexDesc->AddVertElem(VET_FLOAT4, VES_TEXCOORD, 0, 0, 1); // Position & time, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT4, VES_TEXCOORD, 1, 0, 1); // Velocity, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 2, 0, 1); // Size, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT1, VES_TEXCOORD, 3, 0, 1); // Rotation, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 4, 0, 1); // Data UV, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 5, 1); // Sprite texture coordinates

		injectVertexDecl = VertexDeclaration::create(injectVertexDesc);

		// Prepare UV coordinates for rendering tiles
		VERTEX_BUFFER_DESC tileUVBufferDesc;
		tileUVBufferDesc.numVerts = PARTICLES_PER_INSTANCE * 4;
		tileUVBufferDesc.vertexSize = tileVertexDesc->GetVertexStride();

		tileUVs = VertexBuffer::create(tileUVBufferDesc);

		auto* const tileUVData = (Vector2*)tileUVs->Lock(GBL_WRITE_ONLY_DISCARD);
		const float tileUVScale = GpuParticleResources::TILE_SIZE / (float)GpuParticleResources::TEX_SIZE;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			tileUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * tileUVScale;
			tileUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * tileUVScale;
			tileUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * tileUVScale;
			tileUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * tileUVScale;
		}

		tileUVs->Unlock();

		// Prepare UV coordinates for rendering particles
		VERTEX_BUFFER_DESC particleUVBufferDesc;
		particleUVBufferDesc.numVerts = PARTICLES_PER_INSTANCE * 4;
		particleUVBufferDesc.vertexSize = tileVertexDesc->GetVertexStride();

		particleUVs = VertexBuffer::create(particleUVBufferDesc);

		auto* const particleUVData = (Vector2*)particleUVs->Lock(GBL_WRITE_ONLY_DISCARD);
		const float particleUVScale = 1.0f / (float)GpuParticleResources::TEX_SIZE;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			particleUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * particleUVScale;
			particleUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * particleUVScale;
			particleUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * particleUVScale;
			particleUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * particleUVScale;
		}

		particleUVs->Unlock();

		// Prepare indices for rendering tiles & particles
		INDEX_BUFFER_DESC spriteIndexBufferDesc;
		spriteIndexBufferDesc.indexType = IT_16BIT;
		spriteIndexBufferDesc.numIndices = PARTICLES_PER_INSTANCE * 6;

		spriteIndices = IndexBuffer::create(spriteIndexBufferDesc);

		auto* const indices = (UINT16*)spriteIndices->Lock(GBL_WRITE_ONLY_DISCARD);

		const Conventions& rapiConventions = gCaps().conventions;
		for (UINT32 i = 0; i < PARTICLES_PER_INSTANCE; i++)
		{
			// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
			// get culled.
			if (rapiConventions.uvYAxis == Conventions::Axis::Up)
			{
				indices[i * 6 + 0] = i * 4 + 2; indices[i * 6 + 1] = i * 4 + 1; indices[i * 6 + 2] = i * 4 + 0;
				indices[i * 6 + 3] = i * 4 + 3; indices[i * 6 + 4] = i * 4 + 2; indices[i * 6 + 5] = i * 4 + 0;
			}
			else
			{
				indices[i * 6 + 0] = i * 4 + 0; indices[i * 6 + 1] = i * 4 + 1; indices[i * 6 + 2] = i * 4 + 2;
				indices[i * 6 + 3] = i * 4 + 0; indices[i * 6 + 4] = i * 4 + 2; indices[i * 6 + 5] = i * 4 + 3;
			}
		}

		spriteIndices->Unlock();

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
		injectScratchBufferDesc.vertexSize = injectVertexDesc->GetVertexStride(0);
		injectScratchBufferDesc.usage = GBU_DYNAMIC;

		injectScratch = VertexBuffer::create(injectScratchBufferDesc);
	}

	GpuParticleSystem::GpuParticleSystem(ParticleSystem* parent)
		:mParent(parent)
	{
		GpuParticleSimulation::instance().AddSystem(this);
	}

	GpuParticleSystem::~GpuParticleSystem()
	{
		GpuParticleSimulation::instance().RemoveSystem(this);
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
					const UINT32 tileId = resources.AllocTile();
					if (tileId == (UINT32)-1)
						return newTilesAdded; // Out of space in the texture

					GpuParticleTile newTile;
					newTile.id = tileId;
					newTile.lifetime = 0.0f;

					tileIdx = (UINT32)mTiles.size();
					newTiles.push_back(newTile.id);
					mTiles.push_back(newTile);
					mActiveTiles.Add(true);

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

	void GpuParticleSystem::DetectInactiveTiles()
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

	bool GpuParticleSystem::FreeInactiveTiles(GpuParticleResources& resources)
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

			resources.FreeTile(mTiles[lastIdx].id);

			mTiles.erase(mTiles.end() - 1);
			mActiveTiles.Remove(lastIdx);
		}

		// Tile order changed so this might no longer be valid
		if (numFreeTiles > 0)
			mLastAllocatedTile = (UINT32)-1;

		return numFreeTiles > 0;
	}

	void GpuParticleSystem::UpdateGpuBuffers()
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

			auto* tileUVs = (Vector2*)mTileUVs->Lock(GBL_WRITE_ONLY_NO_OVERWRITE);
			for (UINT32 i = 0; i < numTiles; i++)
				tileUVs[i] = GpuParticleResources::getTileCoords(mTiles[i].id);

			for (UINT32 i = numTiles; i < numTilesToAllocates; i++)
				tileUVs[i] = Vector2(2.0f, 2.0f); // Out of range

			mTileUVs->Unlock();
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
			auto* particleIndices = (UINT32*)mParticleIndices->Lock(GBL_WRITE_ONLY_NO_OVERWRITE);

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

			mParticleIndices->Unlock();
		}
	}
	
	void GpuParticleSystem::AdvanceTime(float dt)
	{
		const ParticleSystemSettings& settings = mParent->GetSettings();

		float timeStep;
		mTime = bs::ParticleSystem::_advanceTime(mTime, dt, settings.duration, settings.isLooping, timeStep);
	}

	AABox GpuParticleSystem::GetBounds() const
	{
		const ParticleSystemSettings& settings = mParent->GetSettings();

		if(settings.useAutomaticBounds)
			return AABox::INF_BOX;

		return settings.customBounds;
	}

	struct GpuParticleSimulation::Pimpl
	{
		GpuParticleResources resources;
		GpuParticleHelperBuffers helperBuffers;
		SPtr<GpuParamBlockBuffer> vectorFieldParams;
		SPtr<GpuParamBlockBuffer> depthCollisionParams;
		SPtr<GpuParamBlockBuffer> simulationParams;
		UnorderedSet<GpuParticleSystem*> systems;
	};

	GpuParticleSimulation::GpuParticleSimulation()
		:m(bs_new<Pimpl>())
	{
		m->vectorFieldParams = gVectorFieldParamsDef.CreateBuffer();
		m->depthCollisionParams = gGpuParticleDepthCollisionParamsDef.CreateBuffer();
		m->simulationParams = gGpuParticleSimulateParamsDef.CreateBuffer();
	}

	GpuParticleSimulation::~GpuParticleSimulation()
	{
		bs_delete(m);
	}

	void GpuParticleSimulation::AddSystem(GpuParticleSystem* system)
	{
		m->systems.Insert(system);
	}

	void GpuParticleSimulation::RemoveSystem(GpuParticleSystem* system)
	{
		m->systems.erase(system);
	}

	void GpuParticleSimulation::simulate(const SceneInfo& sceneInfo, const ParticlePerFrameData* simData,
		const SPtr<GpuParamBlockBuffer>& viewParams, const GBufferTextures& gbuffer, float dt)
	{
		m->resources.Swap();
		m->resources.GetCurveTexture().applyChanges();

		Vector<UINT32> newTiles;
		Vector<GpuParticle> allNewParticles;
		for (auto& entry : m->systems)
		{
			entry->DetectInactiveTiles();

			bool tilesDirty = false;
			const auto iterFind = simData->gpuData.find(entry->GetParent()->getId());
			if(iterFind != simData->gpuData.end())
			{
				Vector<GpuParticle>& newParticles = iterFind->second->particles;
				tilesDirty = entry->AllocateTiles(m->resources, newParticles, newTiles);

				allNewParticles.Insert(allNewParticles.end(), newParticles.begin(), newParticles.end());
			}

			entry->AdvanceTime(dt);
			tilesDirty |= entry->FreeInactiveTiles(m->resources);

			if (tilesDirty)
				entry->UpdateGpuBuffers();
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(m->resources.getInjectTarget());

		clearTiles(newTiles);
		injectParticles(allNewParticles);

		// Simulate
		// TODO - Run multiple iterations for more stable simulation at lower/erratic framerates
		gGpuParticleSimulateParamsDef.gDT.Set(m->simulationParams, dt);
		gGpuParticleSimulateParamsDef.gNumIterations.Set(m->simulationParams, 1);

		rapi.SetRenderTarget(m->resources.getSimulationTarget());
		rapi.SetVertexDeclaration(m->helperBuffers.tileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.tileUVs };
		rapi.SetVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.SetIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

		enum class SimType { Normal, DepthCollisionsWorld, DepthCollisionsLocal, Count };

		for(UINT32 i = 0; i < (UINT32)SimType::Count; i++)
		{
			const SimType type = (SimType)i;
			const bool simulateDepthCollisions = type == SimType::DepthCollisionsWorld ||
				type == SimType::DepthCollisionsLocal;
			const bool localSpace = type == SimType::DepthCollisionsLocal;

			GpuParticleSimulateMat* simulateMat = GpuParticleSimulateMat::getVariation(simulateDepthCollisions, localSpace);
			simulateMat->BindGlobal(m->resources, viewParams, gbuffer.depth, gbuffer.normals, m->simulationParams);

			for (auto& entry : m->systems)
			{
				if (entry->GetNumTiles() == 0)
					continue;

				ParticleSystem* parentSystem = entry->GetParent();

				const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();
				if(simSettings.depthCollision.enabled != simulateDepthCollisions)
					continue;

				if(simulateDepthCollisions)
				{
					const ParticleSystemSettings& settings = parentSystem->GetSettings();
					bool isLocal = settings.simulationSpace == ParticleSimulationSpace::Local;
					if(isLocal != localSpace)
						continue;
				}

				const RendererParticles& rendererParticles = sceneInfo.particleSystems[parentSystem->GetRendererId()];

				prepareBuffers(entry, rendererParticles);

				SPtr<Texture> vfTexture;
				if (simSettings.vectorField.vectorField)
					vfTexture = simSettings.vectorField.vectorField->GetTexture();

				simulateMat->BindPerCallParams(entry->getTileUVs(), rendererParticles.perObjectParamBuffer,
					m->vectorFieldParams, vfTexture, m->depthCollisionParams);

				const UINT32 tileCount = entry->GetNumTiles();
				const UINT32 numInstances = Math::divideAndRoundUp(tileCount, TILES_PER_INSTANCE);
				rapi.DrawIndexed(0, TILES_PER_INSTANCE * 6, 0, TILES_PER_INSTANCE * 4, numInstances);
			}
		}
	}

	void GpuParticleSimulation::Sort(const RendererView& view)
	{
		const bool supportsCompute = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(!supportsCompute)
			return;

		// Make sure that the position texture isn't bound for rendering
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(nullptr);

		const Vector3& viewOrigin = view.GetProperties().viewOrigin;

		GpuParticleSortPrepareMat* prepareMat = GpuParticleSortPrepareMat::get();
		prepareMat->Bind(m->resources.GetCurrentState().positionAndTimeTex);

		UINT32 systemIdx = 0;
		UINT32 offset = 0;
		for (auto& entry : m->systems)
		{
			if (entry->GetNumTiles() == 0)
			{
				entry->SetSortInfo(false, 0);
				continue;
			}

			ParticleSystem* parentSystem = entry->GetParent();

			const ParticleSystemSettings& settings = parentSystem->GetSettings();
			if (settings.sortMode != ParticleSortMode::Distance)
			{
				entry->SetSortInfo(false, 0);
				continue;
			}

			entry->SetSortInfo(true, offset);

			offset += prepareMat->execute(*entry, systemIdx, viewOrigin, offset,
				m->resources.mSortBuffers.keys[0],
				m->resources.mSortedIndices[0]);

			systemIdx++;
		}

		const UINT32 numSystemsToSort = systemIdx;
		if(numSystemsToSort == 0)
			return;

		const UINT32 totalNumKeys = offset;
		const UINT32 keyMask = 0xFFFF | (Math::ceilToInt(Math::log2((float)(numSystemsToSort + 1))) << 16);
		const UINT32 outputBufferIdx = GpuSort::instance().Sort(m->resources.mSortBuffers, totalNumKeys, keyMask);

		m->resources.mSortedIndicesBufferIdx = outputBufferIdx;
	}

	void GpuParticleSimulation::PrepareBuffers(const GpuParticleSystem* system, const RendererParticles& rendererInfo)
	{
		ParticleSystem* parentSystem = system->GetParent();

		const ParticleSystemSettings& settings = parentSystem->GetSettings();
		const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();

		const Random& random = system->GetRandom();
		const float time = system->GetTime();
		const float nrmTime = time / settings.duration;

		gGpuParticleSimulateParamsDef.gDrag.Set(m->simulationParams, simSettings.drag);
		gGpuParticleSimulateParamsDef.gAcceleration.Set(m->simulationParams, simSettings.acceleration);

		SPtr<Texture> vfTexture;
		if(simSettings.vectorField.vectorField)
			vfTexture = simSettings.vectorField.vectorField->GetTexture();

		if(vfTexture)
		{
			gGpuParticleSimulateParamsDef.gNumVectorFields.Set(m->simulationParams, 1);
				
			const SPtr<VectorField>& vectorField = simSettings.vectorField.vectorField;
			const VECTOR_FIELD_DESC& vfDesc = vectorField->GetDesc();

			const Vector3 tiling(
				simSettings.vectorField.tilingX ? 0.0f : 1.0f,
				simSettings.vectorField.tilingY ? 0.0f : 1.0f,
				simSettings.vectorField.tilingZ ? 0.0f : 1.0f
			);

			gVectorFieldParamsDef.gFieldBounds.Set(m->vectorFieldParams, vfDesc.bounds.getSize());
			gVectorFieldParamsDef.gFieldTightness.Set(m->vectorFieldParams, simSettings.vectorField.tightness);
			gVectorFieldParamsDef.gFieldTiling.Set(m->vectorFieldParams, tiling);
			gVectorFieldParamsDef.gFieldIntensity.Set(m->vectorFieldParams, simSettings.vectorField.intensity);

			const Vector3 rotationRate = simSettings.vectorField.rotationRate.Evaluate(nrmTime, random) * time;
			const Quaternion AddedRotation(Degree(rotationRate.x), Degree(rotationRate.y), Degree(rotationRate.z));

			const Vector3 offset = vfDesc.bounds.GetMin() + simSettings.vectorField.offset;
			const Quaternion rotation = simSettings.vectorField.rotation * addedRotation;
			const Vector3 scale = vfDesc.bounds.GetSize() * simSettings.vectorField.scale;

			Matrix4 fieldToWorld = Matrix4::TRS(offset, rotation, scale);
			fieldToWorld = rendererInfo.localToWorld * fieldToWorld;

			const Matrix3 fieldToWorld3x3 = fieldToWorld.Get3x3();

			gVectorFieldParamsDef.gFieldToWorld.Set(m->vectorFieldParams, fieldToWorld3x3);
			gVectorFieldParamsDef.gWorldToField.Set(m->vectorFieldParams, fieldToWorld.inverseAffine());
		}
		else
			gGpuParticleSimulateParamsDef.gNumVectorFields.Set(m->simulationParams, 0);

		const ParticleDepthCollisionSettings& depthCollisionSettings = simSettings.depthCollision;
		if(depthCollisionSettings.enabled)
		{
			Vector3 scale3D = rendererInfo.particleSystem->GetTransform().GetScale();
			float uniformScale = std::max(std::max(scale3D.x, scale3D.y), scale3D.z);

			gGpuParticleDepthCollisionParamsDef.gCollisionRange.Set(m->depthCollisionParams, 2.0f);
			gGpuParticleDepthCollisionParamsDef.gCollisionRadiusScale.set(m->depthCollisionParams,
				depthCollisionSettings.radiusScale * uniformScale);
			gGpuParticleDepthCollisionParamsDef.gDampening.set(m->depthCollisionParams,
				depthCollisionSettings.dampening);
			gGpuParticleDepthCollisionParamsDef.gRestitution.set(m->depthCollisionParams,
				depthCollisionSettings.restitution);

			const Vector2 sizeScaleUVOffset =
					GpuParticleCurves::getUVOffset(rendererInfo.sizeScaleFrameIdxCurveAlloc);
			const float sizeScaleUVScale =
					GpuParticleCurves::getUVScale(rendererInfo.sizeScaleFrameIdxCurveAlloc);

			gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveOffset.Set(m->depthCollisionParams, sizeScaleUVOffset);
			gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveScale.Set(m->depthCollisionParams, Vector2(sizeScaleUVScale, 0.0f));
		}
	}

	void GpuParticleSimulation::ClearTiles(const Vector<UINT32>& tiles)
	{
		const auto numTiles = (UINT32)tiles.size();
		if(numTiles == 0)
			return;

		const UINT32 numIterations = Math::divideAndRoundUp(numTiles, GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

		GpuParticleClearMat* clearMat = GpuParticleClearMat::get();
		clearMat->Bind(m->helperBuffers.tileScratch);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetVertexDeclaration(m->helperBuffers.tileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.tileUVs };
		rapi.SetVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.SetIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

		UINT32 tileStart = 0;
		for (UINT32 i = 0; i < numIterations; i++)
		{
			static_assert(GpuParticleHelperBuffers::NUM_SCRATCH_TILES % TILES_PER_INSTANCE == 0,
				"Tile scratch buffer size must be divisble with number of tiles per instance.");

			const UINT32 tileEnd = std::min(numTiles, tileStart + GpuParticleHelperBuffers::NUM_SCRATCH_TILES);

			auto* tileUVs = (Vector2*)m->helperBuffers.tileScratch->Lock(GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = tileStart; j < tileEnd; j++)
				tileUVs[j - tileStart] = GpuParticleResources::getTileCoords(tiles[j]);

			const UINT32 alignedTileEnd = Math::divideAndRoundUp(tileEnd, TILES_PER_INSTANCE) * TILES_PER_INSTANCE;
			for (UINT32 j = tileEnd; j < alignedTileEnd; j++)
				tileUVs[j - tileEnd] = Vector2(2.0f, 2.0f); // Out of bounds (we don't want to accidentaly clear used tiles)

			m->helperBuffers.tileScratch->Unlock();

			const UINT32 numInstances = (alignedTileEnd - tileStart) / TILES_PER_INSTANCE;
			rapi.DrawIndexed(0, TILES_PER_INSTANCE * 6, 0, TILES_PER_INSTANCE * 4, numInstances);

			tileStart = alignedTileEnd;
		}
	}

	void GpuParticleSimulation::InjectParticles(const Vector<GpuParticle>& particles)
	{
		const auto numParticles = (UINT32)particles.size();
		const UINT32 numIterations = Math::divideAndRoundUp(numParticles, GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

		GpuParticleInjectMat* injectMat = GpuParticleInjectMat::get();
		injectMat->Bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetVertexDeclaration(m->helperBuffers.injectVertexDecl);

		SPtr<VertexBuffer> buffers[] = { m->helperBuffers.injectScratch, m->helperBuffers.particleUVs };
		rapi.SetVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.SetIndexBuffer(m->helperBuffers.spriteIndices);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

		UINT32 particleStart = 0;
		for (UINT32 i = 0; i < numIterations; i++)
		{
			const UINT32 particleEnd = std::min(numParticles, particleStart + GpuParticleHelperBuffers::NUM_SCRATCH_PARTICLES);

			auto* particleData = (GpuParticleVertex*)m->helperBuffers.injectScratch->Lock(GBL_WRITE_ONLY_DISCARD);
			for (UINT32 j = particleStart; j < particleEnd; j++)
				particleData[j - particleStart] = particles[j].GetVertex();

			m->helperBuffers.injectScratch->Unlock();

			rapi.DrawIndexed(0, 6, 0, 4, particleEnd - particleStart);
			particleStart = particleEnd;
		}
	}

	GpuParticleResources& GpuParticleSimulation::GetResources() const
	{
		return m->resources;
	}

	SPtr<GpuParamBlockBuffer> CreateGpuParticleVertexInputBuffer()
	{
		SPtr<GpuParamBlockBuffer> inputBuffer = gGpuParticleTileVertexParamsDef.CreateBuffer();

		// [0, 1] -> [-1, 1] and flip Y
		Vector4 UvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

		const Conventions& rapiConventions = gCaps().conventions;

		// Either of these flips the Y axis, but if they're both true they cancel out
		if ((rapiConventions.uvYAxis == Conventions::Axis::Up) ^ (rapiConventions.ndcYAxis == Conventions::Axis::Down))
		{
			uvToNdc.y = -uvToNdc.y;
			uvToNdc.w = -uvToNdc.w;
		}

		gGpuParticleTileVertexParamsDef.gUVToNDC.Set(inputBuffer, uvToNdc);

		return inputBuffer;
	}

	GpuParticleClearMat::GpuParticleClearMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();

		mParams->SetParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
		mParams->GetBufferParam(GPT_VERTEX_PROGRAM, "gTileUVs", mTileUVParam);
	}

	void GpuParticleClearMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILES_PER_INSTANCE", TILES_PER_INSTANCE);
	}

	void GpuParticleClearMat::Bind(const SPtr<GpuBuffer>& tileUVs)
	{
		mTileUVParam.Set(tileUVs);

		RendererMaterial::bind();
	}

	GpuParticleInjectMat::GpuParticleInjectMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();
		mParams->SetParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
	}

	GpuParticleCurveInjectMat::GpuParticleCurveInjectMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();
		mParams->SetParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);
	}

	GpuParticleSimulateMat::GpuParticleSimulateMat()
	{
		const SPtr<GpuParamBlockBuffer> inputBuffer = createGpuParticleVertexInputBuffer();
		mParams->SetParamBlockBuffer(GPT_VERTEX_PROGRAM, "Input", inputBuffer);

		mParams->GetParamInfo()->getBinding(
			GPT_FRAGMENT_PROGRAM,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"Params",
			mParamsBinding
		);
		
		mParams->GetParamInfo()->getBinding(
			GPT_FRAGMENT_PROGRAM,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"VectorFieldParams",
			mVectorFieldBinding
		);
		
		mParams->GetBufferParam(GPT_VERTEX_PROGRAM, "gTileUVs", mTileUVParam);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gPosAndTimeTex", mPosAndTimeTexParam);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gVelocityTex", mVelocityTexParam);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gVectorFieldTex", mVectorFieldTexParam);

		mSupportsDepthCollisions = mVariation.GetUInt("DEPTH_COLLISIONS") > 0;
		if(mSupportsDepthCollisions)
		{
			mParams->GetParamInfo()->getBinding(
				GPT_FRAGMENT_PROGRAM,
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"PerCamera",
				mPerCameraBinding
			);

			mParams->GetParamInfo()->getBinding(
				GPT_FRAGMENT_PROGRAM,
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"PerObject",
				mPerObjectBinding
			);

			mParams->GetParamInfo()->getBinding(
				GPT_FRAGMENT_PROGRAM,
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"DepthCollisionParams",
				mDepthCollisionBinding
			);
		
			mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSizeRotationTex", mSizeRotationTexParam);
			mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gCurvesTex", mCurvesTexParam);
			mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexParam);
			mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexParam);
		}
	}

	void GpuParticleSimulateMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILES_PER_INSTANCE", TILES_PER_INSTANCE);
	}

	void GpuParticleSimulateMat::bindGlobal(GpuParticleResources& resources, const SPtr<GpuParamBlockBuffer>& viewParams,
		const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuParamBlockBuffer>& simulationParams)
	{
		GpuParticleStateTextures& prevState = resources.GetPreviousState();
		const GpuParticleStaticTextures& staticTextures = resources.GetStaticTextures();
		GpuParticleCurves& curveTexture = resources.GetCurveTexture();

		mParams->SetParamBlockBuffer(mParamsBinding.set, mParamsBinding.slot, simulationParams);

		mPosAndTimeTexParam.Set(prevState.positionAndTimeTex);
		mVelocityTexParam.Set(prevState.velocityTex);

		if(mSupportsDepthCollisions)
		{
			mParams->SetParamBlockBuffer(mPerCameraBinding.set, mPerCameraBinding.slot, viewParams);

			mSizeRotationTexParam.Set(staticTextures.sizeAndRotationTex);
			mCurvesTexParam.Set(curveTexture.getTexture());
			mDepthTexParam.Set(depth);
			mNormalsTexParam.Set(normals);
		}

		RendererMaterial::bind(false);
	}

	void GpuParticleSimulateMat::bindPerCallParams(const SPtr<GpuBuffer>& tileUVs,
		const SPtr<GpuParamBlockBuffer>& perObjectParams, const SPtr<GpuParamBlockBuffer>& vectorFieldParams,
		const SPtr<Texture>& vectorFieldTexture, const SPtr<GpuParamBlockBuffer>& depthCollisionParams)
	{
		mTileUVParam.Set(tileUVs);
		mParams->SetParamBlockBuffer(mVectorFieldBinding.set, mVectorFieldBinding.slot, vectorFieldParams);
		mVectorFieldTexParam.Set(vectorFieldTexture);

		if(mSupportsDepthCollisions)
		{
			mParams->SetParamBlockBuffer(mPerObjectBinding.set, mPerObjectBinding.slot, perObjectParams);
			mParams->SetParamBlockBuffer(mDepthCollisionBinding.set, mDepthCollisionBinding.slot, depthCollisionParams);
		}

		bindParams();
	}

	GpuParticleSimulateMat* GpuParticleSimulateMat::getVariation(bool depthCollisions, bool localSpace)
	{
		if(depthCollisions)
		{
			if(localSpace)
				return Get(getVariation<2>());

			return Get(getVariation<1>());
		}

		return Get(getVariation<0>());
	}

	GpuParticleBoundsMat::GpuParticleBoundsMat()
	{
		mInputBuffer = gGpuParticleBoundsParamsDef.CreateBuffer();
		mParams->SetParamBlockBuffer(GPT_COMPUTE_PROGRAM, "Input", mInputBuffer);
		
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gParticleIndices", mParticleIndicesParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputParam);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gPosAndTimeTex", mPosAndTimeTexParam);
	}

	void GpuParticleBoundsMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("NUM_THREADS", NUM_THREADS);
	}

	void GpuParticleBoundsMat::Bind(const SPtr<Texture>& positionAndTime)
	{
		mPosAndTimeTexParam.Set(positionAndTime);

		RendererMaterial::bind();
	}

	AABox GpuParticleBoundsMat::Execute(const SPtr<GpuBuffer>& indices, UINT32 numParticles)
	{
		static constexpr UINT32 MAX_NUM_GROUPS = 128;

		const UINT32 numIterations = Math::divideAndRoundUp(numParticles, NUM_THREADS);
		const UINT32 numGroups = std::min(numIterations, MAX_NUM_GROUPS);

		const UINT32 iterationsPerGroup = numIterations / numGroups;
		const UINT32 extraIterations = numIterations % numGroups;

		gGpuParticleBoundsParamsDef.gIterationsPerGroup.Set(mInputBuffer, iterationsPerGroup);
		gGpuParticleBoundsParamsDef.gNumExtraIterations.Set(mInputBuffer, extraIterations);
		gGpuParticleBoundsParamsDef.gNumParticles.Set(mInputBuffer, numParticles);

		GPU_BUFFER_DESC outputDesc;
		outputDesc.type = GBT_STANDARD;
		outputDesc.format = BF_32X2U;
		outputDesc.elementCount = numGroups * 2;
		outputDesc.usage = GBU_DYNAMIC;

		SPtr<GpuBuffer> output = GpuBuffer::create(outputDesc);

		mParticleIndicesParam.Set(indices);
		mOutputParam.Set(output);

		RenderAPI::instance().DispatchCompute(numGroups);

		Vector3 min = Vector3::INF;
		Vector3 max = -Vector3::INF;

		const Vector3* data = (Vector3*)output->Lock(GBL_READ_ONLY);
		for(UINT32 i = 0; i < numGroups; i++)
		{
			min = Vector3::min(min, data[i * 2 + 0]);
			max = Vector3::min(max, data[i * 2 + 1]);
		}

		output->Unlock();

		return AABox(min, max);
	}

	GpuParticleSortPrepareMat::GpuParticleSortPrepareMat()
	{
		mInputBuffer = gGpuParticleSortPrepareParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer(GPT_COMPUTE_PROGRAM, "Input", mInputBuffer);
		
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gInputIndices", mInputIndicesParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutputKeys", mOutputKeysParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutputIndices", mOutputIndicesParam);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gPosAndTimeTex", mPosAndTimeTexParam);
	}

	void GpuParticleSortPrepareMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("NUM_THREADS", NUM_THREADS);
	}

	void GpuParticleSortPrepareMat::Bind(const SPtr<Texture>& positionAndTime)
	{
		mPosAndTimeTexParam.Set(positionAndTime);

		RendererMaterial::bind(false);
	}

	UINT32 GpuParticleSortPrepareMat::execute(const GpuParticleSystem& system, UINT32 systemIdx, const Vector3& viewOrigin,
		UINT32 offset, const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices)
	{
		static constexpr UINT32 MAX_NUM_GROUPS = 128;

		assert(systemIdx < std::pow(2, 16));

		const UINT32 numParticles = system.GetNumTiles() * GpuParticleResources::PARTICLES_PER_TILE;

		const UINT32 numIterations = Math::divideAndRoundUp(numParticles, NUM_THREADS);
		const UINT32 numGroups = std::min(numIterations, MAX_NUM_GROUPS);

		const UINT32 iterationsPerGroup = numIterations / numGroups;
		const UINT32 extraIterations = numIterations % numGroups;

		Vector3 localViewOrigin;
		ParticleSystem* parentSystem = system.GetParent();
		if(parentSystem->GetSettings().simulationSpace == ParticleSimulationSpace::Local)
		{
			const Matrix4& worldToLocal = parentSystem->GetTransform().GetInvMatrix();
			localViewOrigin = worldToLocal.MultiplyAffine(viewOrigin);
		}
		else
			localViewOrigin = viewOrigin;

		gGpuParticleSortPrepareParamDef.gIterationsPerGroup.Set(mInputBuffer, iterationsPerGroup);
		gGpuParticleSortPrepareParamDef.gNumExtraIterations.Set(mInputBuffer, extraIterations);
		gGpuParticleSortPrepareParamDef.gNumParticles.Set(mInputBuffer, numParticles);
		gGpuParticleSortPrepareParamDef.gOutputOffset.Set(mInputBuffer, offset);
		gGpuParticleSortPrepareParamDef.gSystemKey.Set(mInputBuffer, systemIdx << 16);
		gGpuParticleSortPrepareParamDef.gLocalViewOrigin.Set(mInputBuffer, localViewOrigin);

		mInputIndicesParam.Set(system.getParticleIndices());
		mOutputKeysParam.Set(outKeys);
		mOutputIndicesParam.Set(outIndices);

		bindParams();
		RenderAPI::instance().DispatchCompute(numGroups);
		return numParticles;
	}

	struct GpuParticleCurveInject
	{
		Color color;
		Vector2 dataUV;
	};

	GpuParticleCurves::GpuParticleCurves()
	{
		TEXTURE_DESC textureDesc;
		textureDesc.format = PF_RGBA16F;
		textureDesc.width = TEX_SIZE;
		textureDesc.height = TEX_SIZE;
		textureDesc.usage = TU_RENDERTARGET;

		mCurveTexture = Texture::create(textureDesc);

		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.colorSurfaces[0].texture = mCurveTexture;

		mRT = RenderTexture::create(rtDesc);

		// Prepare vertex declaration for injecting new curves
		SPtr<VertexDataDesc> injectVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		injectVertexDesc->AddVertElem(VET_FLOAT4, VES_TEXCOORD, 0, 0, 1); // Color, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 1, 0, 1); // Data UV, per instance
		injectVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD, 2, 1); // Pixel texture coordinates

		mInjectVertexDecl = VertexDeclaration::create(injectVertexDesc);

		// Prepare UV coordinates for injecting curves
		VERTEX_BUFFER_DESC injectUVBufferDesc;
		injectUVBufferDesc.numVerts = 4;
		injectUVBufferDesc.vertexSize = injectVertexDesc->GetVertexStride(1);

		mInjectUV = VertexBuffer::create(injectUVBufferDesc);

		auto* const tileUVData = (Vector2*)mInjectUV->Lock(GBL_WRITE_ONLY_DISCARD);
		const float tileUVScale = 1.0f / (float)TEX_SIZE;
		tileUVData[0] = Vector2(0.0f, 0.0f) * tileUVScale;
		tileUVData[1] = Vector2(1.0f, 0.0f) * tileUVScale;
		tileUVData[2] = Vector2(1.0f, 1.0f) * tileUVScale;
		tileUVData[3] = Vector2(0.0f, 1.0f) * tileUVScale;

		mInjectUV->Unlock();

		// Prepare indices for injecting curves
		INDEX_BUFFER_DESC injectIndexBufferDesc;
		injectIndexBufferDesc.indexType = IT_16BIT;
		injectIndexBufferDesc.numIndices = 6;

		mInjectIndices = IndexBuffer::create(injectIndexBufferDesc);

		const Conventions& rapiConventions = gCaps().conventions;

		auto* const indices = (UINT16*)mInjectIndices->Lock(GBL_WRITE_ONLY_DISCARD);

		// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
		// get culled.
		if (rapiConventions.uvYAxis == Conventions::Axis::Up)
		{
			indices[0] = 2; indices[1] = 1; indices[2] = 0;
			indices[3] = 3; indices[4] = 2; indices[5] = 0;
		}
		else
		{
			indices[0] = 0; indices[1] = 1; indices[2] = 2;
			indices[3] = 0; indices[4] = 2; indices[5] = 3;
		}

		mInjectIndices->Unlock();

		// Prepare a scratch buffer we'll use to inject new curves
		VERTEX_BUFFER_DESC injectScratchBufferDesc;
		injectScratchBufferDesc.numVerts = SCRATCH_NUM_VERTICES;
		injectScratchBufferDesc.vertexSize = injectVertexDesc->GetVertexStride(0);
		injectScratchBufferDesc.usage = GBU_DYNAMIC;

		mInjectScratch = VertexBuffer::create(injectScratchBufferDesc);
	}

	GpuParticleCurves::~GpuParticleCurves()
	{
		for(auto& entry : mPendingAllocations)
			mPendingAllocator.Free(entry.pixels);

		mPendingAllocator.Clear();
	}

	TextureRowAllocation GpuParticleCurves::Alloc(Color* pixels, uint32_t count)
	{
		PendingAllocation pendingAlloc;
		pendingAlloc.allocation = mRowAllocator.Alloc(count);

		if(pendingAlloc.allocation.length == 0)
			return pendingAlloc.allocation;

		pendingAlloc.pixels = (Color*)mPendingAllocator.Alloc(sizeof(Color) * count);
		memcpy(pendingAlloc.pixels, pixels, sizeof(Color) * count);

		mPendingAllocations.push_back(pendingAlloc);
		return pendingAlloc.allocation;
	}

	void GpuParticleCurves::Free(const TextureRowAllocation& alloc)
	{
		mRowAllocator.Free(alloc);
	}

	void GpuParticleCurves::ApplyChanges()
	{
		const auto numCurves = (UINT32)mPendingAllocations.size();
		if(numCurves == 0)
			return;

		GpuParticleCurveInjectMat* injectMat = GpuParticleCurveInjectMat::get();
		injectMat->Bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(mRT);
		rapi.SetVertexDeclaration(mInjectVertexDecl);

		SPtr<VertexBuffer> buffers[] = { mInjectScratch, mInjectUV };
		rapi.SetVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.SetIndexBuffer(mInjectIndices);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

		UINT32 curveIdx = 0;

		auto* data = (GpuParticleCurveInject*)mInjectScratch->Lock(GBL_WRITE_ONLY_DISCARD);
		while(curveIdx < numCurves)
		{
			UINT32 count = 0;
			for(; curveIdx < numCurves; curveIdx++)
			{
				const PendingAllocation& pendingAlloc = mPendingAllocations[curveIdx];

				const UINT32 entryCount = pendingAlloc.allocation.length;
				if((count + entryCount) > SCRATCH_NUM_VERTICES)
					break;

				for(UINT32 i = 0; i < entryCount; i++)
				{
					data[count].color = pendingAlloc.pixels[i];
					data[count].dataUV = Vector2(
						(pendingAlloc.allocation.x + i) / (float)TEX_SIZE,
						pendingAlloc.allocation.y / (float)TEX_SIZE);
					
					count++;
				}
			}

			mInjectScratch->Unlock();
			rapi.DrawIndexed(0, 6, 0, 4, count);

			data = (GpuParticleCurveInject*)mInjectScratch->Lock(GBL_WRITE_ONLY_DISCARD);
		}

		mInjectScratch->Unlock();

		for(auto& entry : mPendingAllocations)
			mPendingAllocator.Free(entry.pixels);

		mPendingAllocations.Clear();
		mPendingAllocator.Clear();
	}

	Vector2 GpuParticleCurves::GetUVOffset(const TextureRowAllocation& alloc)
	{
		return Vector2(
			((float)alloc.x + 0.5f) / TEX_SIZE,
			((float)alloc.y + 0.5f) / TEX_SIZE
		);
	}

	float GpuParticleCurves::GetUVScale(const TextureRowAllocation& alloc)
	{
		if(alloc.length == 0)
			return 0.0f;

		return (alloc.length - 1) / (float)TEX_SIZE;
	}
}}
