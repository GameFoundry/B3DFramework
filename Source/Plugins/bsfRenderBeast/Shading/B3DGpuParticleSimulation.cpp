//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuParticleSimulation.h"
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Particles/B3DVectorField.h"
#include "Particles/B3DParticleDistribution.h"
#include "Math/B3DVector3.h"
#include "B3DRendererParticles.h"
#include "B3DRenderBeastScene.h"
#include "B3DRenderBeast.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Utility/B3DGpuSort.h"

namespace b3d { namespace render {

B3D_PARAM_BLOCK_BEGIN(GpuParticleTileVertexParamsDef)
	B3D_PARAM_BLOCK_ENTRY(Vector4, gUVToNDC)
B3D_PARAM_BLOCK_END

GpuParticleTileVertexParamsDef gGpuParticleTileVertexParamsDef;

/** Material used for clearing tiles in the texture used for particle GPU simulation. */
class GpuParticleClearMat : public RendererMaterial<GpuParticleClearMat>
{
	RMAT_DEF_CUSTOMIZED("GpuParticleClear.bsl")

public:
	GpuParticleClearMat() = default;

	/** Populates GPU parameters for rendering using this material. */
	static void PopulateParameters(const SPtr<GpuParameters>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer, const SPtr<GpuBuffer>& tileUVs);
};

/** Material used for adding new particles into the particle state textures. */
class GpuParticleInjectMat : public RendererMaterial<GpuParticleInjectMat>
{
	RMAT_DEF("GpuParticleInject.bsl");

public:
	GpuParticleInjectMat() = default;

	/** Populates GPU parameters for rendering using this material. */
	static void PopulateParameters(const SPtr<GpuParameters>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer);
};

/** Material used for adding new curves into the curve texture. */
class GpuParticleCurveInjectMat : public RendererMaterial<GpuParticleCurveInjectMat>
{
	RMAT_DEF("GpuParticleCurveInject.bsl");

public:
	GpuParticleCurveInjectMat() = default;
	void Initialize() override;
};

B3D_PARAM_BLOCK_BEGIN(VectorFieldParamsDef)
	B3D_PARAM_BLOCK_ENTRY(Vector3, gFieldBounds)
	B3D_PARAM_BLOCK_ENTRY(float, gFieldIntensity)
	B3D_PARAM_BLOCK_ENTRY(Vector3, gFieldTiling)
	B3D_PARAM_BLOCK_ENTRY(float, gFieldTightness)
	B3D_PARAM_BLOCK_ENTRY(Matrix4, gWorldToField)
	B3D_PARAM_BLOCK_ENTRY(Matrix3, gFieldToWorld)
B3D_PARAM_BLOCK_END

VectorFieldParamsDef gVectorFieldParamsDef;

B3D_PARAM_BLOCK_BEGIN(GpuParticleDepthCollisionParamsDef)
	B3D_PARAM_BLOCK_ENTRY(float, gCollisionRange)
	B3D_PARAM_BLOCK_ENTRY(float, gRestitution)
	B3D_PARAM_BLOCK_ENTRY(float, gDampening)
	B3D_PARAM_BLOCK_ENTRY(float, gCollisionRadiusScale)
	B3D_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleCurveOffset)
	B3D_PARAM_BLOCK_ENTRY(Vector2, gSizeScaleCurveScale)
B3D_PARAM_BLOCK_END

GpuParticleDepthCollisionParamsDef gGpuParticleDepthCollisionParamsDef;

B3D_PARAM_BLOCK_BEGIN(GpuParticleSimulateParamsDef)
	B3D_PARAM_BLOCK_ENTRY(i32, gNumVectorFields)
	B3D_PARAM_BLOCK_ENTRY(i32, gNumIterations)
	B3D_PARAM_BLOCK_ENTRY(float, gDT)
	B3D_PARAM_BLOCK_ENTRY(float, gDrag)
	B3D_PARAM_BLOCK_ENTRY(Vector3, gAcceleration)
B3D_PARAM_BLOCK_END

GpuParticleSimulateParamsDef gGpuParticleSimulateParamsDef;

/**
 * Material used for performing GPU particle simulation. State is read from the provided input textures and output
 * into the output textures bound as render targets.
 */
class GpuParticleSimulateMat : public RendererMaterial<GpuParticleSimulateMat>
{
	RMAT_DEF_CUSTOMIZED("GpuParticleSimulate.bsl");

	/** Helper method used for initializing variations of this material. */
	template <u32 DEPTH_COLLISIONS>
	static const ShaderVariationParameters& GetVariation()
	{
		static ShaderVariationParameters variation = ShaderVariationParameters(
			{ ShaderVariationParameter("DEPTH_COLLISIONS", DEPTH_COLLISIONS) });

		return variation;
	}

public:
	GpuParticleSimulateMat() = default;

	/**
	 * Populates GPU parameters for rendering using this material.
	 *
	 * @param	gpuParameters				Parameters to populate
	 * @param	resources					GPU particle resources containing textures
	 * @param	particleVertexInputBuffer	Uniform buffer used in the particle vertex shader.
	 * @param	viewParams					Per-camera view parameters
	 * @param	depth						Depth texture for collision detection
	 * @param	normals						Normals texture for collision detection
	 * @param	tileUVs						Sets the UV offsets of individual tiles for a particular particle system that's being rendered.
	 * @param	perObjectParams				General purpose particle system parameters.
	 * @param	vectorFieldTexture			3D texture representing the vector field, or null if none.
	 * @param	supportsDepthCollisions		True if this material variation supports depth collisions
	 */
	static void PopulateParameters(const SPtr<GpuParameters>& gpuParameters, GpuParticleResources& resources, const SPtr<GpuBuffer>& particleVertexInputBuffer,
		const SPtr<GpuBuffer>& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& tileUVs,
		const SPtr<GpuBuffer>& perObjectParams, const SPtr<Texture>& vectorFieldTexture, bool supportsDepthCollisions);

	/** Returns the material variation matching the provided parameters. */
	static GpuParticleSimulateMat* GetVariation(bool depthCollisions, bool localSpace);
};

B3D_PARAM_BLOCK_BEGIN(GpuParticleBoundsParamsDef)
	B3D_PARAM_BLOCK_ENTRY(u32, gIterationsPerGroup)
	B3D_PARAM_BLOCK_ENTRY(u32, gNumExtraIterations)
	B3D_PARAM_BLOCK_ENTRY(u32, gNumParticles)
B3D_PARAM_BLOCK_END

GpuParticleBoundsParamsDef gGpuParticleBoundsParamsDef;

/** Material used for calculating particle system bounds. */
class GpuParticleBoundsMat : public RendererMaterial<GpuParticleBoundsMat>
{
	static constexpr u32 kNumThreads = 64;

	RMAT_DEF_CUSTOMIZED("GpuParticleBounds.bsl");

public:
	GpuParticleBoundsMat() = default;
	void Initialize() override;

	/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
	void Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime);

	/**
	 * Executes the material, calculating the bounds. Note that this function reads back from the GPU and should not
	 * be called at runtime.
	 *
	 * @param		commandBuffer	Command buffer to execute on.
	 * @param		indices			Buffer containing offsets into the position texture for each particle.
	 * @param		numParticles	Number of particle in the provided indices buffer.
	 */
	AABox Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& indices, u32 numParticles);

private:
	GpuParameterBuffer mParticleIndicesParam;
	GpuParameterBuffer mOutputParam;
	GpuParameterSampledTexture mPosAndTimeTexParam;
	SPtr<GpuBuffer> mInputBuffer;
};

B3D_PARAM_BLOCK_BEGIN(GpuParticleSortPrepareParamDef)
	B3D_PARAM_BLOCK_ENTRY(i32, gIterationsPerGroup)
	B3D_PARAM_BLOCK_ENTRY(i32, gNumExtraIterations)
	B3D_PARAM_BLOCK_ENTRY(i32, gNumParticles)
	B3D_PARAM_BLOCK_ENTRY(i32, gOutputOffset)
	B3D_PARAM_BLOCK_ENTRY(i32, gSystemKey)
	B3D_PARAM_BLOCK_ENTRY(Vector3, gLocalViewOrigin)
B3D_PARAM_BLOCK_END

GpuParticleSortPrepareParamDef gGpuParticleSortPrepareParamDef;

/** Material used for preparing key/values buffers used for particle sorting. */
class GpuParticleSortPrepareMat : public RendererMaterial<GpuParticleSortPrepareMat>
{
	static constexpr u32 kNumThreads = 64;

	RMAT_DEF_CUSTOMIZED("GpuParticleSortPrepare.bsl");

public:
	GpuParticleSortPrepareMat() = default;
	void Initialize() override;

	/** Binds the material to the pipeline along with the global input texture containing particle positions and times. */
	void Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime);

	/**
	 * Executes the material, generating sort data for a particular particle system and injecting it into the specified
	 * location in the key and index buffers.
	 *
	 * @param	commandBuffer	Command buffer to execute on.
	 * @param	system			System whose particles to insert into the sort key/index buffers.
	 * @param	systemIdx		Sequential index of the system to insert into the sort buffers.
	 * @param	offset			Offset into the key/index buffer at which to insert the sort data.
	 * @param	viewOrigin		View origin to use for determining sorting keys, in world space.
	 * @param	outKeys			Pre-allocated buffer that will receive the keys used for sorting. The buffer must
	 *								be GPU writable and use a 1x 32-bit integer format.
	 * @param	outIndices		Pre-allocated buffer that will receive the indices to be sorted. The buffer must
	 *								be GPU writable and use a 2x 16-bit integer format. Must have the same capacity
	 *								as @p outKeys.
	 * @return						Number of particle that were written to the buffers.
	 */
	u32 Execute(GpuCommandBuffer& commandBuffer, const GpuParticleSystem& system, u32 systemIdx, const Vector3& viewOrigin, u32 offset, const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices);

private:
	GpuParameterBuffer mInputIndicesParam;
	GpuParameterBuffer mOutputKeysParam;
	GpuParameterBuffer mOutputIndicesParam;
	GpuParameterSampledTexture mPosAndTimeTexParam;
	SPtr<GpuBuffer> mInputBuffer;
};

static constexpr u32 kTilesPerInstance = 8;
static constexpr u32 kParticlesPerInstance = kTilesPerInstance * GpuParticleResources::kParticlesPerTile;

/** Contains a variety of helper buffers and declarations used for GPU particle simulation. */
struct GpuParticleHelperBuffers
{
	static constexpr u32 kNumScratchTiles = 512;
	static constexpr u32 kNumScratchParticles = 4096;

	GpuParticleHelperBuffers();

	SPtr<GpuBuffer> TileUVs;
	SPtr<GpuBuffer> ParticleUVs;
	SPtr<GpuBuffer> SpriteIndices;
	SPtr<VertexDescription> TileVertexDescription;
	SPtr<VertexDescription> InjectVertexDescription;
};

GpuParticleResources::GpuParticleResources()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	// Allocate textures
	TextureCreateInformation positionAndTimeDesc;
	positionAndTimeDesc.Name = "GPU Particles Position & Time";
	positionAndTimeDesc.Format = PF_RGBA32F;
	positionAndTimeDesc.Width = kTexSize;
	positionAndTimeDesc.Height = kTexSize;
	positionAndTimeDesc.Usage = TU_RENDERTARGET;

	TextureCreateInformation velocityDesc;
	velocityDesc.Name = "GPU Particles Velocity";
	velocityDesc.Format = PF_RGBA16F;
	velocityDesc.Width = kTexSize;
	velocityDesc.Height = kTexSize;
	velocityDesc.Usage = TU_RENDERTARGET;

	for(u32 i = 0; i < 2; i++)
	{
		mStateTextures[i].PositionAndTimeTex = gpuDevice->CreateTexture(positionAndTimeDesc);
		mStateTextures[i].VelocityTex = gpuDevice->CreateTexture(velocityDesc);
	}

	TextureCreateInformation sizeAndRotationDesc;
	sizeAndRotationDesc.Name = "GPU Particles Size & Rotation";
	sizeAndRotationDesc.Format = PF_RGBA16F;
	sizeAndRotationDesc.Width = kTexSize;
	sizeAndRotationDesc.Height = kTexSize;
	sizeAndRotationDesc.Usage = TU_RENDERTARGET;

	mStaticTextures.SizeAndRotationTex = gpuDevice->CreateTexture(sizeAndRotationDesc);

	RenderTextureCreateInformation staticRtDesc;
	staticRtDesc.ColorSurfaces[0].Texture = mStaticTextures.SizeAndRotationTex;

	for(u32 i = 0; i < 2; i++)
	{
		RenderTextureCreateInformation simulationRTDesc;
		simulationRTDesc.ColorSurfaces[0].Texture = mStateTextures[i].PositionAndTimeTex;
		simulationRTDesc.ColorSurfaces[1].Texture = mStateTextures[i].VelocityTex;

		mSimulateRT[i] = RenderTexture::Create(simulationRTDesc);

		RenderTextureCreateInformation injectRTDesc;
		injectRTDesc.ColorSurfaces[0].Texture = mStateTextures[i].PositionAndTimeTex;
		injectRTDesc.ColorSurfaces[1].Texture = mStateTextures[i].VelocityTex;
		injectRTDesc.ColorSurfaces[2].Texture = mStaticTextures.SizeAndRotationTex;
		mInjectRT[i] = RenderTexture::Create(injectRTDesc);
	}

	// Allocate the buffer containing keys used for sorting
	GpuBufferCreateInformation sortKeysBufferCreateInformation;
	sortKeysBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	sortKeysBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	sortKeysBufferCreateInformation.SimpleStorage.Format = BF_32X1U;
	sortKeysBufferCreateInformation.SimpleStorage.Count = kTexSize * kTexSize;

	mSortBuffers.Keys[0] = gpuDevice->CreateGpuBuffer(sortKeysBufferCreateInformation);
	mSortBuffers.Keys[1] = gpuDevice->CreateGpuBuffer(sortKeysBufferCreateInformation);

	// Allocate the buffer containing sorted particle indices
	GpuBufferCreateInformation sortedIndicesBufferCreateInformation;
	sortedIndicesBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	sortedIndicesBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	sortedIndicesBufferCreateInformation.SimpleStorage.Format = BF_16X2U;
	sortedIndicesBufferCreateInformation.SimpleStorage.Count = kTexSize * kTexSize;

	mSortedIndices[0] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);
	mSortedIndices[1] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);

	mSortBuffers.Values[0] = mSortedIndices[0];
	mSortBuffers.Values[1] = mSortedIndices[1];

	// Clear the free tile linked list
	for(u32 i = 0; i < kTileCount; i++)
		mFreeTiles[i] = kTileCount - i - 1;
}

u32 GpuParticleResources::AllocTile()
{
	if(mNumFreeTiles > 0)
	{
		mNumFreeTiles--;
		return mFreeTiles[mNumFreeTiles];
	}

	return (u32)-1;
}

void GpuParticleResources::FreeTile(u32 tile)
{
	B3D_ASSERT(tile < kTileCount);
	B3D_ASSERT(mNumFreeTiles < kTileCount);

	mFreeTiles[mNumFreeTiles] = tile;
	mNumFreeTiles++;
}

Vector2I GpuParticleResources::GetTileOffset(u32 tileId)
{
	return Vector2I(
		(tileId % kTileCount1D) * kTileSize,
		(tileId / kTileCount1D) * kTileSize);
}

Vector2 GpuParticleResources::GetTileCoords(u32 tileId)
{
	return Vector2(
		Math::Frac(tileId / (float)kTileCount1D),
		(u32)(tileId / kTileCount1D) / (float)kTileCount1D);
}

Vector2I GpuParticleResources::GetParticleOffset(u32 subTileId)
{
	return Vector2I(
		subTileId % kTileSize,
		subTileId / kTileSize);
}

Vector2 GpuParticleResources::GetParticleCoords(u32 subTileId)
{
	const Vector2 tileOffset = GetParticleOffset(subTileId).To<float>();
	return tileOffset / (float)kTexSize;
}

const SPtr<GpuBuffer>& GpuParticleResources::GetSortedIndices() const
{
	return mSortedIndices[mSortedIndicesBufferIdx];
}

GpuParticleHelperBuffers::GpuParticleHelperBuffers()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	// Prepare vertex declaration for rendering tiles
	TInlineArray<VertexElement, 8> tileVertexElements;
	tileVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

	TileVertexDescription = B3DMakeShared<VertexDescription>(tileVertexElements);

	// Prepare vertex declaration for injecting new particles
	TInlineArray<VertexElement, 8> injectVertexElements;
	injectVertexElements.Add(VertexElement(VET_FLOAT4, VES_TEXCOORD, 0, 0, 1)); // Position & time, per instance
	injectVertexElements.Add(VertexElement(VET_FLOAT4, VES_TEXCOORD, 1, 0, 1)); // Velocity, per instance
	injectVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 2, 0, 1)); // Size, per instance
	injectVertexElements.Add(VertexElement(VET_FLOAT1, VES_TEXCOORD, 3, 0, 1)); // Rotation, per instance
	injectVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 4, 0, 1)); // Data UV, per instance
	injectVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 5, 1)); // Sprite texture coordinates

	InjectVertexDescription = B3DMakeShared<VertexDescription>(injectVertexElements);

	// Prepare UV coordinates for rendering tiles
	GpuBufferCreateInformation tileUVBufferCreateInformation;
	tileUVBufferCreateInformation.Type = GpuBufferType::Vertex;
	tileUVBufferCreateInformation.Vertex.Count = kParticlesPerInstance * 4;
	tileUVBufferCreateInformation.Vertex.ElementSize = TileVertexDescription->GetVertexStride();

	TileUVs = gpuDevice->CreateGpuBuffer(tileUVBufferCreateInformation);

	auto* const tileUVData = (Vector2*)B3DStackAllocate(TileUVs->GetTotalSize());
	const float tileUVScale = GpuParticleResources::kTileSize / (float)GpuParticleResources::kTexSize;
	for(u32 i = 0; i < kParticlesPerInstance; i++)
	{
		tileUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * tileUVScale;
		tileUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * tileUVScale;
		tileUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * tileUVScale;
		tileUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * tileUVScale;
	}

	TileUVs->WriteData(0, TileUVs->GetTotalSize(), tileUVData);
	B3DStackFree(tileUVData);

	// Prepare UV coordinates for rendering particles
	GpuBufferCreateInformation particleUVBufferCreateInformation;
	particleUVBufferCreateInformation.Type = GpuBufferType::Vertex;
	particleUVBufferCreateInformation.Vertex.Count = kParticlesPerInstance * 4;
	particleUVBufferCreateInformation.Vertex.ElementSize = TileVertexDescription->GetVertexStride();

	ParticleUVs = gpuDevice->CreateGpuBuffer(particleUVBufferCreateInformation);

	auto* const particleUVData = (Vector2*)B3DStackAllocate(ParticleUVs->GetTotalSize());
	const float particleUVScale = 1.0f / (float)GpuParticleResources::kTexSize;
	for(u32 i = 0; i < kParticlesPerInstance; i++)
	{
		particleUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * particleUVScale;
		particleUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * particleUVScale;
		particleUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * particleUVScale;
		particleUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * particleUVScale;
	}

	ParticleUVs->WriteData(0, ParticleUVs->GetTotalSize(), particleUVData);
	B3DStackFree(particleUVData);

	// Prepare indices for rendering tiles & particles
	GpuBufferCreateInformation spriteIndexBufferCreateInformation;
	spriteIndexBufferCreateInformation.Type = GpuBufferType::Index;
	spriteIndexBufferCreateInformation.Index.Type = IT_16BIT;
	spriteIndexBufferCreateInformation.Index.Count = kParticlesPerInstance * 6;

	SpriteIndices = gpuDevice->CreateGpuBuffer(spriteIndexBufferCreateInformation);

	auto* const indices = (u16*)B3DStackAllocate(SpriteIndices->GetTotalSize());

	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
	for(u32 i = 0; i < kParticlesPerInstance; i++)
	{
		// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
		// get culled.
		if(gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up)
		{
			indices[i * 6 + 0] = i * 4 + 2;
			indices[i * 6 + 1] = i * 4 + 1;
			indices[i * 6 + 2] = i * 4 + 0;
			indices[i * 6 + 3] = i * 4 + 3;
			indices[i * 6 + 4] = i * 4 + 2;
			indices[i * 6 + 5] = i * 4 + 0;
		}
		else
		{
			indices[i * 6 + 0] = i * 4 + 0;
			indices[i * 6 + 1] = i * 4 + 1;
			indices[i * 6 + 2] = i * 4 + 2;
			indices[i * 6 + 3] = i * 4 + 0;
			indices[i * 6 + 4] = i * 4 + 2;
			indices[i * 6 + 5] = i * 4 + 3;
		}
	}

	SpriteIndices->WriteData(0, SpriteIndices->GetTotalSize(), indices);
	B3DStackFree(indices);
}

GpuParticleSystem::GpuParticleSystem(ParticleSystem* parent)
	: mParent(parent)
{
	GpuParticleSimulation::Instance().AddSystem(this);
}

GpuParticleSystem::~GpuParticleSystem()
{
	GpuParticleSimulation::Instance().RemoveSystem(this);
}

bool GpuParticleSystem::AllocateTiles(GpuParticleResources& resources, Vector<GpuParticle>& newParticles, Vector<u32>& newTiles)
{
	GpuParticleTile cachedTile = mLastAllocatedTile == (u32)-1 ? GpuParticleTile() : mTiles[mLastAllocatedTile];
	Vector2 tileUV = GpuParticleResources::GetTileCoords(cachedTile.Id);

	bool newTilesAdded = false;
	for(u32 i = 0; i < (u32)newParticles.size(); i++)
	{
		u32 tileIdx;

		// Use the last allocated tile if there's room
		if(cachedTile.NumFreeParticles > 0)
			tileIdx = mLastAllocatedTile;
		else
		{
			// Otherwise try to find an inactive tile
			if(mNumActiveTiles < (u32)mTiles.size())
			{
				tileIdx = (u32)mActiveTiles.Find(false);
				mActiveTiles[tileIdx] = true;
			}
			// And finally just allocate a new tile if no room elsewhere
			else
			{
				const u32 tileId = resources.AllocTile();
				if(tileId == (u32)-1)
					return newTilesAdded; // Out of space in the texture

				GpuParticleTile newTile;
				newTile.Id = tileId;
				newTile.Lifetime = 0.0f;

				tileIdx = (u32)mTiles.size();
				newTiles.push_back(newTile.Id);
				mTiles.push_back(newTile);
				mActiveTiles.Add(true);

				newTilesAdded = true;
			}

			mLastAllocatedTile = tileIdx;
			tileUV = GpuParticleResources::GetTileCoords(mTiles[tileIdx].Id);
			mTiles[tileIdx].NumFreeParticles = GpuParticleResources::kParticlesPerTile;

			cachedTile = mTiles[tileIdx];
			mNumActiveTiles++;
		}

		GpuParticleTile& tile = mTiles[tileIdx];
		GpuParticle& particle = newParticles[i];

		const u32 tileParticleIdx = GpuParticleResources::kParticlesPerTile - tile.NumFreeParticles;
		particle.DataUv = tileUV + GpuParticleResources::GetParticleCoords(tileParticleIdx);

		tile.NumFreeParticles--;
		tile.Lifetime = std::max(tile.Lifetime, mTime + particle.Lifetime);

		cachedTile.NumFreeParticles--;
	}

	return newTilesAdded;
}

void GpuParticleSystem::DetectInactiveTiles()
{
	mNumActiveTiles = 0;
	for(u32 i = 0; i < (u32)mTiles.size(); i++)
	{
		if(mTiles[i].Lifetime >= mTime)
		{
			mNumActiveTiles++;
			continue;
		}

		mActiveTiles[i] = false;

		if(mLastAllocatedTile == i)
			mLastAllocatedTile = (u32)-1;
	}
}

bool GpuParticleSystem::FreeInactiveTiles(GpuParticleResources& resources)
{
	const u32 numFreeTiles = (u32)mTiles.size() - mNumActiveTiles;
	for(u32 i = 0; i < numFreeTiles; i++)
	{
		const u32 freeIdx = (u32)mActiveTiles.Find(false);
		B3D_ASSERT(freeIdx != (u32)-1);

		const u32 lastIdx = (u32)mTiles.size() - 1;

		if(freeIdx != lastIdx)
		{
			std::swap(mTiles[freeIdx], mTiles[lastIdx]);
			std::Swap(mActiveTiles[freeIdx], mActiveTiles[lastIdx]);
		}

		resources.FreeTile(mTiles[lastIdx].Id);

		mTiles.erase(mTiles.end() - 1);
		mActiveTiles.Remove(lastIdx);
	}

	// Tile order changed so this might no longer be valid
	if(numFreeTiles > 0)
		mLastAllocatedTile = (u32)-1;

	return numFreeTiles > 0;
}

void GpuParticleSystem::UpdateGpuBuffers()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const auto numTiles = (u32)mTiles.size();
	const u32 numTilesToAllocates = Math::DivideAndRoundUp(numTiles, kTilesPerInstance) * kTilesPerInstance;

	// Tile offsets buffer
	if(numTiles > 0)
	{
		GpuBufferCreateInformation tilesBufferCreateInformation;
		tilesBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		tilesBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		tilesBufferCreateInformation.SimpleStorage.Format = BF_32X2F;
		tilesBufferCreateInformation.SimpleStorage.Count = numTilesToAllocates;

		mTileUVs = gpuDevice->CreateGpuBuffer(tilesBufferCreateInformation);

		auto* tileUVs = (Vector2*)B3DStackAllocate(mTileUVs->GetTotalSize());
		for(u32 i = 0; i < numTiles; i++)
			tileUVs[i] = GpuParticleResources::GetTileCoords(mTiles[i].Id);

		for(u32 i = numTiles; i < numTilesToAllocates; i++)
			tileUVs[i] = Vector2(2.0f, 2.0f); // Out of range

		mTileUVs->WriteData(0, mTileUVs->GetTotalSize(), tileUVs, BWT_NO_OVERWRITE);
		B3DStackFree(tileUVs);
	}

	// Particle data offsets
	const u32 numParticles = numTiles * GpuParticleResources::kParticlesPerTile;

	if(numParticles > 0)
	{
		GpuBufferCreateInformation particleUVCreateInformation;
		particleUVCreateInformation.Type = GpuBufferType::SimpleStorage;
		particleUVCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		particleUVCreateInformation.SimpleStorage.Format = BF_16X2U;
		particleUVCreateInformation.SimpleStorage.Count = numParticles;

		mParticleIndices = gpuDevice->CreateGpuBuffer(particleUVCreateInformation);
		auto* particleIndices = (u32*)B3DStackAllocate(mParticleIndices->GetTotalSize());

		u32 idx = 0;
		for(u32 i = 0; i < numTiles; i++)
		{
			const Vector2I tileOffset = GpuParticleResources::GetTileOffset(mTiles[i].Id);
			for(u32 y = 0; y < GpuParticleResources::kTileSize; y++)
			{
				for(u32 x = 0; x < GpuParticleResources::kTileSize; x++)
				{
					const Vector2I offset = tileOffset + Vector2I(x, y);
					particleIndices[idx++] = (offset.X & 0xFFFF) | (offset.Y << 16);
				}
			}
		}

		mParticleIndices->WriteData(0, mParticleIndices->GetTotalSize(), particleIndices, BWT_NO_OVERWRITE);
		B3DStackFree(particleIndices);
	}
}

void GpuParticleSystem::AdvanceTime(float dt)
{
	const ParticleSystemSettings& settings = mParent->GetSettings();

	float timeStep;
	mTime = b3d::ParticleSystem::AdvanceTime(mTime, dt, settings.Duration, settings.IsLooping, timeStep);
}

AABox GpuParticleSystem::GetBounds() const
{
	const ParticleSystemSettings& settings = mParent->GetSettings();

	if(settings.UseAutomaticBounds)
		return AABox(-(float)kMaximumSceneExtent, (float)kMaximumSceneExtent);

	return settings.CustomBounds;
}

// TODO - Move GpuParticleSimulateMat to the header so I don't need to pass the material as the parameter. Ideally create simulate parameters during construction.
SPtr<GpuParameters> GpuParticleSystem::GetOrCreateSimulateParameters(GpuParticleSimulateMat* material)
{
	if(!mSimulateParameters)
		mSimulateParameters = material->CreateGpuParameters();

	return mSimulateParameters;
}

void GpuParticleSystem::PrepareSimulationBuffers(const RendererParticles& rendererInfo, float dt, const SPtr<GpuParameters>& gpuParameters)
{
	// Create buffers on first use
	if(!mSimulationParams)
		mSimulationParams = gGpuParticleSimulateParamsDef.CreateBuffer();

	if(!mVectorFieldParams)
		mVectorFieldParams = gVectorFieldParamsDef.CreateBuffer();

	if(!mDepthCollisionParams)
		mDepthCollisionParams = gGpuParticleDepthCollisionParamsDef.CreateBuffer();

	// Get parent system settings
	const ParticleSystemSettings& settings = mParent->GetSettings();
	const ParticleGpuSimulationSettings& simSettings = mParent->GetGpuSimulationSettings();

	const float time = GetTime();
	const float nrmTime = time / settings.Duration;

	// Write simulation parameters
	gGpuParticleSimulateParamsDef.gDT.Set(mSimulationParams, dt);
	gGpuParticleSimulateParamsDef.gNumIterations.Set(mSimulationParams, 1);
	gGpuParticleSimulateParamsDef.gDrag.Set(mSimulationParams, simSettings.Drag);
	gGpuParticleSimulateParamsDef.gAcceleration.Set(mSimulationParams, simSettings.Acceleration);

	// Write vector field parameters
	SPtr<Texture> vfTexture;
	if(simSettings.VectorField.VectorField)
		vfTexture = simSettings.VectorField.VectorField->GetTexture();

	if(vfTexture)
	{
		gGpuParticleSimulateParamsDef.gNumVectorFields.Set(mSimulationParams, 1);

		const SPtr<VectorField>& vectorField = simSettings.VectorField.VectorField;
		const VECTOR_FIELD_DESC& vfDesc = vectorField->GetDesc();

		const Vector3 tiling(
			simSettings.VectorField.TilingX ? 0.0f : 1.0f,
			simSettings.VectorField.TilingY ? 0.0f : 1.0f,
			simSettings.VectorField.TilingZ ? 0.0f : 1.0f);

		gVectorFieldParamsDef.gFieldBounds.Set(mVectorFieldParams, vfDesc.Bounds.GetSize());
		gVectorFieldParamsDef.gFieldTightness.Set(mVectorFieldParams, simSettings.VectorField.Tightness);
		gVectorFieldParamsDef.gFieldTiling.Set(mVectorFieldParams, tiling);
		gVectorFieldParamsDef.gFieldIntensity.Set(mVectorFieldParams, simSettings.VectorField.Intensity);

		const Vector3 rotationRate = simSettings.VectorField.RotationRate.Evaluate(nrmTime, mRandom) * time;
		const Quaternion addedRotation(Degree(rotationRate.X), Degree(rotationRate.Y), Degree(rotationRate.Z));

		const Vector3 offset = vfDesc.Bounds.Minimum + simSettings.VectorField.Offset;
		const Quaternion rotation = simSettings.VectorField.Rotation * addedRotation;
		const Vector3 scale = vfDesc.Bounds.GetSize() * simSettings.VectorField.Scale;

		Matrix4 fieldToWorld = Matrix4::TRS(offset, rotation, scale);
		fieldToWorld = rendererInfo.LocalToWorld * fieldToWorld;

		const Matrix3 fieldToWorld3x3 = fieldToWorld.Get3x3();

		gVectorFieldParamsDef.gFieldToWorld.Set(mVectorFieldParams, fieldToWorld3x3);
		gVectorFieldParamsDef.gWorldToField.Set(mVectorFieldParams, fieldToWorld.InverseAffine());
	}
	else
	{
		gGpuParticleSimulateParamsDef.gNumVectorFields.Set(mSimulationParams, 0);
	}

	// Write depth collision parameters
	const ParticleDepthCollisionSettings& depthCollisionSettings = simSettings.DepthCollision;
	if(depthCollisionSettings.Enabled)
	{
		Vector3 scale3D = rendererInfo.ParticleSystem->GetWorldTransform().GetScale();
		float uniformScale = std::max(std::max(scale3D.X, scale3D.Y), scale3D.Z);

		gGpuParticleDepthCollisionParamsDef.gCollisionRange.Set(mDepthCollisionParams, 2.0f);
		gGpuParticleDepthCollisionParamsDef.gCollisionRadiusScale.Set(
			mDepthCollisionParams, depthCollisionSettings.RadiusScale * uniformScale);
		gGpuParticleDepthCollisionParamsDef.gDampening.Set(mDepthCollisionParams, depthCollisionSettings.Dampening);
		gGpuParticleDepthCollisionParamsDef.gRestitution.Set(mDepthCollisionParams, depthCollisionSettings.Restitution);

		const Vector2 sizeScaleUVOffset =
			GpuParticleCurves::GetUvOffset(rendererInfo.SizeScaleFrameIdxCurveAlloc);
		const float sizeScaleUVScale =
			GpuParticleCurves::GetUvScale(rendererInfo.SizeScaleFrameIdxCurveAlloc);

		gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveOffset.Set(mDepthCollisionParams, sizeScaleUVOffset);
		gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveScale.Set(
			mDepthCollisionParams, Vector2(sizeScaleUVScale, 0.0f));
	}

	// Bind uniform buffers to GPU parameters
	gpuParameters->SetUniformBuffer("Params", mSimulationParams);
	gpuParameters->SetUniformBuffer("VectorFieldParams", mVectorFieldParams);
	gpuParameters->SetUniformBuffer("DepthCollisionParams", mDepthCollisionParams);
}

static SPtr<GpuBuffer> CreateGpuParticleVertexInputBuffer()
{
	SPtr<GpuBuffer> inputBuffer = gGpuParticleTileVertexParamsDef.CreateBuffer();

	// [0, 1] -> [-1, 1] and flip Y
	Vector4 uvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	// Either of these flips the Y axis, but if they're both true they cancel out
	if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
	{
		uvToNdc.Y = -uvToNdc.Y;
		uvToNdc.W = -uvToNdc.W;
	}

	gGpuParticleTileVertexParamsDef.gUVToNDC.Set(inputBuffer, uvToNdc);

	return inputBuffer;
}

/** Helper structure for caching system classification and renderer info during simulation preparation. */
struct SystemSimulationInfo
{
	GpuParticleSystem* System;
	const RendererParticles* RendererInfo;
	SPtr<Texture> VectorFieldTexture;

	enum class SimType
	{
		Normal,
		DepthCollisionsWorld,
		DepthCollisionsLocal
	};
	SimType Type;

	bool SupportsDepthCollisions() const
	{
		return Type != SimType::Normal;
	}

	bool IsLocalSpace() const
	{
		return Type == SimType::DepthCollisionsLocal;
	}
};

/** Helper function to classify a system by its simulation type. */
static SystemSimulationInfo::SimType ClassifySystem(const GpuParticleSystem* system)
{
	ParticleSystem* parentSystem = system->GetParent();
	const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();

	if(!simSettings.DepthCollision.Enabled)
		return SystemSimulationInfo::SimType::Normal;

	const ParticleSystemSettings& settings = parentSystem->GetSettings();
	bool isLocal = settings.SimulationSpace == ParticleSimulationSpace::Local;

	return isLocal ? SystemSimulationInfo::SimType::DepthCollisionsLocal
	               : SystemSimulationInfo::SimType::DepthCollisionsWorld;
}

struct GpuParticleSimulation::Pimpl
{
	GpuParticleResources Resources;
	GpuParticleHelperBuffers HelperBuffers;
	UnorderedSet<GpuParticleSystem*> Systems;
	SPtr<GpuBuffer> ParticleVertexInputBuffer;

	TArray<TileClearParameters> TileClearParameterPool;
	TArray<ParticleInjectParameters> ParticleInjectParameterPool;

	TArray<TileClearBatch> PreparedTileClearBatches;
	TArray<ParticleInjectBatch> PreparedParticleInjectBatches;
};

GpuParticleSimulation::GpuParticleSimulation()
	: m(B3DNew<Pimpl>())
{
	m->ParticleVertexInputBuffer = CreateGpuParticleVertexInputBuffer();

	m->TileClearParameterPool.Add(CreateTileClearParameters());
	m->ParticleInjectParameterPool.Add(CreateParticleInjectParameters());
}

GpuParticleSimulation::~GpuParticleSimulation()
{
	B3DDelete(m);
}

void GpuParticleSimulation::AddSystem(GpuParticleSystem* system)
{
	m->Systems.insert(system);
}

void GpuParticleSimulation::RemoveSystem(GpuParticleSystem* system)
{
	m->Systems.erase(system);
}

void GpuParticleSimulation::Simulate(GpuCommandBuffer& commandBuffer, const SceneInfo& sceneInfo, const EvaluatedParticleData* simData, const SPtr<GpuBuffer>& viewParams, const GBufferTextures& gbuffer, float dt)
{
	m->Resources.Swap();
	m->Resources.GetCurveTexture().ApplyChanges(commandBuffer);

	Vector<u32> newTiles;
	Vector<GpuParticle> allNewParticles;
	for(auto& entry : m->Systems)
	{
		entry->DetectInactiveTiles();

		bool tilesDirty = false;
		const auto iterFind = simData->GpuData.find(entry->GetParent()->GetId());
		if(iterFind != simData->GpuData.end())
		{
			Vector<GpuParticle>& newParticles = iterFind->second->Particles;
			tilesDirty = entry->AllocateTiles(m->Resources, newParticles, newTiles);

			allNewParticles.insert(allNewParticles.end(), newParticles.begin(), newParticles.end());
		}

		entry->AdvanceTime(dt);
		tilesDirty |= entry->FreeInactiveTiles(m->Resources);

		if(tilesDirty)
			entry->UpdateGpuBuffers();
	}

	RenderPassCreateInformation clearAndInjectParticlesPass(m->Resources.GetInjectTarget(), RT_NONE, RT_ALL);

	PrepareClearTiles(newTiles);
	PrepareInjectParticles(allNewParticles);

	for(const auto& batch : m->PreparedTileClearBatches)
		clearAndInjectParticlesPass.Parameters.Add(batch.Parameters.GpuParameters);

	for(const auto& batch : m->PreparedParticleInjectBatches)
		clearAndInjectParticlesPass.Parameters.Add(batch.Parameters.GpuParameters);
	
	commandBuffer.BeginRenderPass(clearAndInjectParticlesPass);

	DrawClearTiles(commandBuffer);
	DrawInjectParticles(commandBuffer);

	commandBuffer.EndRenderPass();

	// Simulate
	// TODO - Run multiple iterations for more stable simulation at lower/erratic framerates

	// PHASE 1: Prepare all parameters BEFORE BeginRenderPass
	// - Classify each system by simulation type
	// - Prepare uniform buffers (simulation params, vector fields, collision params)
	// - Create/retrieve GpuParameters for each system
	// - Populate GpuParameters with all inputs (textures, buffers, uniforms)
	Vector<SystemSimulationInfo> systemsToSimulate;
	systemsToSimulate.reserve(m->Systems.size());

	for(auto& entry : m->Systems)
	{
		if(entry->GetTileCount() == 0)
			continue;

		ParticleSystem* parentSystem = entry->GetParent();
		const RendererParticles& rendererParticles = sceneInfo.ParticleSystems[parentSystem->GetRendererId()];

		// Classify system
		SystemSimulationInfo info;
		info.System = entry;
		info.RendererInfo = &rendererParticles;
		info.Type = ClassifySystem(entry);

		// Get vector field texture if any
		const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();
		if(simSettings.VectorField.VectorField)
			info.VectorFieldTexture = simSettings.VectorField.VectorField->GetTexture();

		// Determine which material variation to use
		const bool supportsDepthCollisions = info.SupportsDepthCollisions();
		const bool localSpace = info.IsLocalSpace();
		GpuParticleSimulateMat* const simulateMaterial = GpuParticleSimulateMat::GetVariation(supportsDepthCollisions, localSpace);

		// Get or create per-system GPU parameters
		SPtr<GpuParameters> systemParams = entry->GetOrCreateSimulateParameters(simulateMaterial);

		// Prepare per-system uniform buffers and bind them (gDT, gNumIterations, gDrag, gAcceleration, vector field params, depth collision params)
		entry->PrepareSimulationBuffers(rendererParticles, dt, systemParams);

		// Populate remaining parameters (textures and other buffers)
		GpuParticleSimulateMat::PopulateParameters(
			systemParams,
			m->Resources,
			m->ParticleVertexInputBuffer,
			viewParams,
			gbuffer.Depth,
			gbuffer.Normals,
			entry->GetTileUVs(),              // Storage buffer (tile UV offsets)
			rendererParticles.PerObjectParamBuffer,  // Uniform buffer (per-object params)
			info.VectorFieldTexture,          // Texture (vector field 3D texture)
			supportsDepthCollisions
		);

		systemsToSimulate.push_back(info);
	}

	// PHASE 2: Create render pass with all parameters pre-declared
	// The render pass now knows about all GpuParameters upfront
	RenderPassCreateInformation simulatePass(m->Resources.GetSimulationTarget(), RT_NONE, RT_ALL);

	for(const SystemSimulationInfo& info : systemsToSimulate)
	{
		SPtr<GpuParameters> systemParams = info.System->GetOrCreateSimulateParameters(
			GpuParticleSimulateMat::GetVariation(info.SupportsDepthCollisions(), info.IsLocalSpace())
		);
		simulatePass.Parameters.Add(systemParams);
	}

	commandBuffer.BeginRenderPass(simulatePass);

	// PHASE 3: Render loop - bind materials and draw
	// All parameters are already prepared and bound to the render pass
	// Just need to bind the correct material variation and issue draw calls
	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	// Group systems by SimType for efficient material binding
	enum class SimType
	{
		Normal,
		DepthCollisionsWorld,
		DepthCollisionsLocal,
		Count
	};

	for(u32 simulationTypeIndex = 0; simulationTypeIndex < (u32)SimType::Count; simulationTypeIndex++)
	{
		const SimType currentType = (SimType)simulationTypeIndex;
		const bool currentDepthCollisions = currentType != SimType::Normal;
		const bool currentLocalSpace = currentType == SimType::DepthCollisionsLocal;

		// Get the material variation for this simulation type
		GpuParticleSimulateMat* const simulateMaterial = GpuParticleSimulateMat::GetVariation(currentDepthCollisions, currentLocalSpace);
		simulateMaterial->Bind(commandBuffer, false);

		// Render all systems matching this simulation type
		for(const SystemSimulationInfo& info : systemsToSimulate)
		{
			// Skip systems that don't match this simulation type
			if(static_cast<u32>(info.Type) != simulationTypeIndex)
				continue;

			// Get the pre-populated parameters
			SPtr<GpuParameters> systemParams = info.System->GetOrCreateSimulateParameters(simulateMaterial);

			// Bind parameters and draw
			commandBuffer.SetGpuParameters(systemParams);

			const u32 tileCount = info.System->GetTileCount();
			const u32 numInstances = Math::DivideAndRoundUp(tileCount, kTilesPerInstance);
			commandBuffer.DrawIndexed(0, kTilesPerInstance * 6, 0, kTilesPerInstance * 4, numInstances);
		}
	}

	commandBuffer.EndRenderPass();
}

void GpuParticleSimulation::Sort(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	const bool supportsCompute = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(!supportsCompute)
		return;

	const Vector3& viewOrigin = view.GetProperties().ViewOrigin;

	GpuParticleSortPrepareMat* prepareMat = GpuParticleSortPrepareMat::Get();
	prepareMat->Bind(commandBuffer, m->Resources.GetCurrentState().PositionAndTimeTex);

	u32 systemIdx = 0;
	u32 offset = 0;
	for(auto& entry : m->Systems)
	{
		if(entry->GetTileCount() == 0)
		{
			entry->SetSortInfo(false, 0);
			continue;
		}

		ParticleSystem* parentSystem = entry->GetParent();

		const ParticleSystemSettings& settings = parentSystem->GetSettings();
		if(settings.SortMode != ParticleSortMode::Distance)
		{
			entry->SetSortInfo(false, 0);
			continue;
		}

		entry->SetSortInfo(true, offset);

		if(systemIdx > 0)
		{
			// Write-after-write for sort buffer keys and sorted indices
			commandBuffer.IssueBarriers({ {
					GpuBufferBarrier(m->Resources.mSortBuffers.Keys[0], GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write),
					GpuBufferBarrier(m->Resources.mSortedIndices[0], GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageComputeShader, GpuAccessFlag::Write)
				}});
		}

		offset += prepareMat->Execute(commandBuffer, *entry, systemIdx, viewOrigin, offset, m->Resources.mSortBuffers.Keys[0], m->Resources.mSortedIndices[0]);

		systemIdx++;
	}

	const u32 numSystemsToSort = systemIdx;
	if(numSystemsToSort == 0)
		return;

	const u32 totalNumKeys = offset;
	const u32 keyMask = 0xFFFF | (Math::CeilToInt(Math::Log2((float)(numSystemsToSort + 1))) << 16);
	const u32 outputBufferIdx = GpuSort::Instance().Sort(commandBuffer, m->Resources.mSortBuffers, totalNumKeys, keyMask);

	m->Resources.mSortedIndicesBufferIdx = outputBufferIdx;
}

void GpuParticleSimulation::DrawClearTiles(GpuCommandBuffer& commandBuffer)
{
	if(m->PreparedTileClearBatches.Empty())
		return;

	GpuParticleClearMat* const clearMaterial = GpuParticleClearMat::Get();
	clearMaterial->Bind(commandBuffer, false);

	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const TileClearBatch& batch : m->PreparedTileClearBatches)
	{
		commandBuffer.SetGpuParameters(batch.Parameters.GpuParameters);
		commandBuffer.DrawIndexed(0, kTilesPerInstance * 6, 0, kTilesPerInstance * 4, batch.InstanceCount);
	}
}

void GpuParticleSimulation::DrawInjectParticles(GpuCommandBuffer& commandBuffer)
{
	if(m->PreparedParticleInjectBatches.Empty())
		return;

	GpuParticleInjectMat* const injectMaterial = GpuParticleInjectMat::Get();
	injectMaterial->Bind(commandBuffer, false);

	commandBuffer.SetVertexDescription(m->HelperBuffers.InjectVertexDescription);
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const ParticleInjectBatch& batch : m->PreparedParticleInjectBatches)
	{
		commandBuffer.SetGpuParameters(batch.Parameters.GpuParameters);

		SPtr<GpuBuffer> vertexBuffers[] = { batch.Parameters.ScratchBuffer, m->HelperBuffers.ParticleUVs };
		commandBuffer.SetVertexBuffers(0, vertexBuffers, (u32)B3DSize(vertexBuffers));
		commandBuffer.DrawIndexed(0, 6, 0, 4, batch.ParticleCount);
	}
}

GpuParticleResources& GpuParticleSimulation::GetResources() const
{
	return m->Resources;
}

GpuParticleSimulation::TileClearParameters GpuParticleSimulation::CreateTileClearParameters()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	GpuBufferCreateInformation tileScratchBufferCreateInformation;
	tileScratchBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	tileScratchBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	tileScratchBufferCreateInformation.SimpleStorage.Format = BF_32X2F;
	tileScratchBufferCreateInformation.SimpleStorage.Count = GpuParticleHelperBuffers::kNumScratchTiles;

	TileClearParameters output;
	output.ScratchBuffer = gpuDevice->CreateGpuBuffer(tileScratchBufferCreateInformation);

	output.GpuParameters = GpuParticleClearMat::Get()->CreateGpuParameters();
	GpuParticleClearMat::PopulateParameters(output.GpuParameters, m->ParticleVertexInputBuffer, output.ScratchBuffer);

	return output;
}

GpuParticleSimulation::ParticleInjectParameters GpuParticleSimulation::CreateParticleInjectParameters()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	GpuBufferCreateInformation injectScratchBufferCreateInformation;
	injectScratchBufferCreateInformation.Type = GpuBufferType::Vertex;
	injectScratchBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	injectScratchBufferCreateInformation.Vertex.Count = GpuParticleHelperBuffers::kNumScratchParticles;
	injectScratchBufferCreateInformation.Vertex.ElementSize = m->HelperBuffers.InjectVertexDescription->GetVertexStride(0);

	ParticleInjectParameters output;
	output.ScratchBuffer = gpuDevice->CreateGpuBuffer(injectScratchBufferCreateInformation);

	output.GpuParameters = GpuParticleInjectMat::Get()->CreateGpuParameters();
	GpuParticleInjectMat::PopulateParameters(output.GpuParameters, m->ParticleVertexInputBuffer);

	return output;
}

GpuParticleSimulation::TileClearParameters& GpuParticleSimulation::FindOrCreateTileClearParameters()
{
	for(auto& entry : m->TileClearParameterPool)
	{
		if(entry.IsAvailable())
			return entry;
	}

	m->TileClearParameterPool.Add(CreateTileClearParameters());
	return m->TileClearParameterPool.back();
}

GpuParticleSimulation::ParticleInjectParameters& GpuParticleSimulation::FindOrCreateParticleInjectParameters()
{
	for(auto& entry : m->ParticleInjectParameterPool)
	{
		if(entry.IsAvailable())
			return entry;
	}

	m->ParticleInjectParameterPool.Add(CreateParticleInjectParameters());
	return m->ParticleInjectParameterPool.back();
}

void GpuParticleSimulation::PrepareClearTiles(const Vector<u32>& tiles)
{
	// Clear previous frame's batches
	m->PreparedTileClearBatches.Clear();

	const auto tileCount = (u32)tiles.size();
	if(tileCount == 0)
		return;

	const u32 batchCount = Math::DivideAndRoundUp(tileCount, GpuParticleHelperBuffers::kNumScratchTiles);

	u32 tileStart = 0;
	for(u32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
	{
		static_assert(GpuParticleHelperBuffers::kNumScratchTiles % kTilesPerInstance == 0,
			"Tile scratch buffer size must be divisible with number of tiles per instance.");

		const u32 tileEnd = std::min(tileCount, tileStart + GpuParticleHelperBuffers::kNumScratchTiles);

		// Get parameters from the pool
		TileClearParameters& parameters = FindOrCreateTileClearParameters();

		// Allocate temporary array for tile UVs
		auto* tileUVs = (Vector2*)B3DStackAllocate(parameters.ScratchBuffer->GetTotalSize());

		// Populate tile UVs
		for(u32 tileIndex = tileStart; tileIndex < tileEnd; ++tileIndex)
			tileUVs[tileIndex - tileStart] = GpuParticleResources::GetTileCoords(tiles[tileIndex]);

		// Fill remaining slots with out-of-bounds values
		const u32 alignedTileEnd = Math::DivideAndRoundUp(tileEnd, kTilesPerInstance) * kTilesPerInstance;
		for(u32 tileIndex = tileEnd; tileIndex < alignedTileEnd; ++tileIndex)
			tileUVs[tileIndex] = Vector2(2.0f, 2.0f); // Out of bounds

		// Write data to buffer (BEFORE render pass)
		parameters.ScratchBuffer->WriteData(0, parameters.ScratchBuffer->GetTotalSize(), tileUVs);

		// Free temporary array
		B3DStackFree(tileUVs);

		// Store batch information for rendering
		TileClearBatch batch;
		batch.Parameters = parameters;
		batch.InstanceCount = (alignedTileEnd - tileStart) / kTilesPerInstance;
		m->PreparedTileClearBatches.Add(batch);

		tileStart = alignedTileEnd;
	}
}

void GpuParticleSimulation::PrepareInjectParticles(const Vector<GpuParticle>& particles)
{
	// Clear previous frame's batches
	m->PreparedParticleInjectBatches.Clear();

	const auto particleCount = (u32)particles.size();
	if(particleCount == 0)
		return;

	const u32 batchCount = Math::DivideAndRoundUp(particleCount, GpuParticleHelperBuffers::kNumScratchParticles);

	u32 particleStart = 0;
	for(u32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
	{
		const u32 particleEnd = std::min(particleCount, particleStart + GpuParticleHelperBuffers::kNumScratchParticles);

		// Get parameters from the pool
		ParticleInjectParameters& parameters = FindOrCreateParticleInjectParameters();

		// Allocate temporary array for particle data
		auto* particleData = (GpuParticleVertex*)B3DStackAllocate(parameters.ScratchBuffer->GetTotalSize());

		// Populate particle data
		for(u32 particleIndex = particleStart; particleIndex < particleEnd; ++particleIndex)
			particleData[particleIndex - particleStart] = particles[particleIndex].GetVertex();

		// Write data to buffer (BEFORE render pass)
		parameters.ScratchBuffer->WriteData(0, parameters.ScratchBuffer->GetTotalSize(), particleData, BWT_DISCARD);

		// Free temporary array
		B3DStackFree(particleData);

		// Store batch information for rendering
		ParticleInjectBatch batch;
		batch.Parameters = parameters;
		batch.ParticleCount = particleEnd - particleStart;
		m->PreparedParticleInjectBatches.Add(batch);

		particleStart = particleEnd;
	}
}

void GpuParticleClearMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", kTilesPerInstance);
}

void GpuParticleClearMat::PopulateParameters(const SPtr<GpuParameters>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer, const SPtr<GpuBuffer>& tileUVs)
{
	gpuParameters->SetUniformBuffer("Input", vertexInputBuffer);
	gpuParameters->SetStorageBuffer("gTileUVs", tileUVs);
}

void GpuParticleInjectMat::PopulateParameters(const SPtr<GpuParameters>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer)
{
	gpuParameters->SetUniformBuffer("Input", vertexInputBuffer);
}

void GpuParticleCurveInjectMat::Initialize()
{
	const SPtr<GpuBuffer> inputBuffer = CreateGpuParticleVertexInputBuffer();
	mGPUParameters->SetUniformBuffer("Input", inputBuffer);
}

void GpuParticleSimulateMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", kTilesPerInstance);
}

void GpuParticleSimulateMat::PopulateParameters(const SPtr<GpuParameters>& gpuParameters, GpuParticleResources& resources, const SPtr<GpuBuffer>& particleVertexInputBuffer,
	const SPtr<GpuBuffer>& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& tileUVs,
	const SPtr<GpuBuffer>& perObjectParams, const SPtr<Texture>& vectorFieldTexture, bool supportsDepthCollisions)
{
	GpuParticleStateTextures& prevState = resources.GetPreviousState();
	const GpuParticleStaticTextures& staticTextures = resources.GetStaticTextures();
	GpuParticleCurves& curveTexture = resources.GetCurveTexture();

	// Set uniform buffers
	gpuParameters->SetUniformBuffer("Input", particleVertexInputBuffer);

	// Set textures and buffers
	gpuParameters->SetSampledTexture("gPosAndTimeTex", prevState.PositionAndTimeTex);
	gpuParameters->SetSampledTexture("gVelocityTex", prevState.VelocityTex);
	gpuParameters->SetSampledTexture("gVectorFieldTex", vectorFieldTexture);
	gpuParameters->SetStorageBuffer("gTileUVs", tileUVs);

	if(supportsDepthCollisions)
	{
		gpuParameters->SetUniformBuffer("PerCamera", viewParams);
		gpuParameters->SetUniformBuffer("PerObject", perObjectParams);

		gpuParameters->SetSampledTexture("gSizeRotationTex", staticTextures.SizeAndRotationTex);
		gpuParameters->SetSampledTexture("gCurvesTex", curveTexture.GetTexture());
		gpuParameters->SetSampledTexture("gDepthTex", depth);
		gpuParameters->SetSampledTexture("gNormalsTex", normals);
	}
}

GpuParticleSimulateMat* GpuParticleSimulateMat::GetVariation(bool depthCollisions, bool localSpace)
{
	if(depthCollisions)
	{
		if(localSpace)
			return Get(GetVariation<2>());

		return Get(GetVariation<1>());
	}

	return Get(GetVariation<0>());
}

void GpuParticleBoundsMat::Initialize()
{
	mInputBuffer = gGpuParticleBoundsParamsDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Input", mInputBuffer);

	mGPUParameters->GetStorageBufferParameter("gParticleIndices", mParticleIndicesParam);
	mGPUParameters->GetStorageBufferParameter("gOutput", mOutputParam);
	mGPUParameters->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
}

void GpuParticleBoundsMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_THREADS", kNumThreads);
}

void GpuParticleBoundsMat::Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime)
{
	mPosAndTimeTexParam.Set(positionAndTime);

	RendererMaterial::Bind(commandBuffer);
}

AABox GpuParticleBoundsMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& indices, u32 numParticles)
{
	static constexpr u32 kMaxNumGroups = 128;

	const u32 numIterations = Math::DivideAndRoundUp(numParticles, kNumThreads);
	const u32 numGroups = std::min(numIterations, kMaxNumGroups);

	const u32 iterationsPerGroup = numIterations / numGroups;
	const u32 extraIterations = numIterations % numGroups;

	gGpuParticleBoundsParamsDef.gIterationsPerGroup.Set(mInputBuffer, iterationsPerGroup);
	gGpuParticleBoundsParamsDef.gNumExtraIterations.Set(mInputBuffer, extraIterations);
	gGpuParticleBoundsParamsDef.gNumParticles.Set(mInputBuffer, numParticles);

	GpuBufferCreateInformation outputBufferCreateInformation;
	outputBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	outputBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	outputBufferCreateInformation.SimpleStorage.Format = BF_32X2U;
	outputBufferCreateInformation.SimpleStorage.Count = numGroups * 2;

	SPtr<GpuBuffer> output = mGpuDevice->CreateGpuBuffer(outputBufferCreateInformation);

	mParticleIndicesParam.Set(indices);
	mOutputParam.Set(output);

	commandBuffer.DispatchCompute(numGroups);

	Vector3 min = Vector3::kInfinite;
	Vector3 max = -Vector3::kInfinite;

	Vector3* data = (Vector3*)B3DStackAllocate(output->GetTotalSize());
	output->ReadData(0, output->GetTotalSize(), data);

	for(u32 i = 0; i < numGroups; i++)
	{
		min = Vector3::Min(min, data[i * 2 + 0]);
		max = Vector3::Min(max, data[i * 2 + 1]);
	}

	B3DStackFree(data);

	return AABox(min, max);
}

void GpuParticleSortPrepareMat::Initialize()
{
	mInputBuffer = gGpuParticleSortPrepareParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Input", mInputBuffer);

	mGPUParameters->GetStorageBufferParameter("gInputIndices", mInputIndicesParam);
	mGPUParameters->GetStorageBufferParameter("gOutputKeys", mOutputKeysParam);
	mGPUParameters->GetStorageBufferParameter("gOutputIndices", mOutputIndicesParam);
	mGPUParameters->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
}

void GpuParticleSortPrepareMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_THREADS", kNumThreads);
}

void GpuParticleSortPrepareMat::Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime)
{
	mPosAndTimeTexParam.Set(positionAndTime);

	RendererMaterial::Bind(commandBuffer, false);
}

u32 GpuParticleSortPrepareMat::Execute(GpuCommandBuffer& commandBuffer, const GpuParticleSystem& system, u32 systemIdx, const Vector3& viewOrigin, u32 offset, const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices)
{
	static constexpr u32 kMaxNumGroups = 128;

	B3D_ASSERT(systemIdx < std::pow(2, 16));

	const u32 numParticles = system.GetTileCount() * GpuParticleResources::kParticlesPerTile;

	const u32 numIterations = Math::DivideAndRoundUp(numParticles, kNumThreads);
	const u32 numGroups = std::min(numIterations, kMaxNumGroups);

	const u32 iterationsPerGroup = numIterations / numGroups;
	const u32 extraIterations = numIterations % numGroups;

	Vector3 localViewOrigin;
	ParticleSystem* parentSystem = system.GetParent();
	if(parentSystem->GetSettings().SimulationSpace == ParticleSimulationSpace::Local)
	{
		const Matrix4& worldToLocal = parentSystem->GetWorldTransform().GetInvMatrix();
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

	mInputIndicesParam.Set(system.GetParticleIndices());
	mOutputKeysParam.Set(outKeys);
	mOutputIndicesParam.Set(outIndices);

	BindParameters(commandBuffer);
	commandBuffer.DispatchCompute(numGroups);
	return numParticles;
}

struct GpuParticleCurveInject
{
	Color Color;
	Vector2 DataUv;
};

GpuParticleCurves::GpuParticleCurves()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Name = "GPU Particles Curves";
	textureCreateInformation.Format = PF_RGBA16F;
	textureCreateInformation.Width = kTexSize;
	textureCreateInformation.Height = kTexSize;
	textureCreateInformation.Usage = TU_RENDERTARGET;

	mCurveTexture = gpuDevice->CreateTexture(textureCreateInformation);

	RenderTextureCreateInformation rtDesc;
	rtDesc.ColorSurfaces[0].Texture = mCurveTexture;

	mRT = RenderTexture::Create(rtDesc);

	// Prepare vertex declaration for injecting new curves
	TInlineArray<VertexElement, 8> injectVertexElement;
	injectVertexElement.Add(VertexElement(VET_FLOAT4, VES_TEXCOORD, 0, 0, 1)); // Color, per instance
	injectVertexElement.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 1, 0, 1)); // Data UV, per instance
	injectVertexElement.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 2, 1)); // Pixel texture coordinates

	mInjectVertexDescription = B3DMakeShared<VertexDescription>(injectVertexElement);

	// Prepare UV coordinates for injecting curves
	GpuBufferCreateInformation injectUVBufferCreateInformation;
	injectUVBufferCreateInformation.Type = GpuBufferType::Vertex;
	injectUVBufferCreateInformation.Vertex.Count = 4;
	injectUVBufferCreateInformation.Vertex.ElementSize = mInjectVertexDescription->GetVertexStride(1);

	mInjectUV = gpuDevice->CreateGpuBuffer(injectUVBufferCreateInformation);

	auto* const tileUVData = (Vector2*)B3DStackAllocate(mInjectUV->GetTotalSize());
	const float tileUVScale = 1.0f / (float)kTexSize;
	tileUVData[0] = Vector2(0.0f, 0.0f) * tileUVScale;
	tileUVData[1] = Vector2(1.0f, 0.0f) * tileUVScale;
	tileUVData[2] = Vector2(1.0f, 1.0f) * tileUVScale;
	tileUVData[3] = Vector2(0.0f, 1.0f) * tileUVScale;

	mInjectUV->WriteData(0, mInjectUV->GetTotalSize(), tileUVData);
	B3DStackFree(tileUVData);

	// Prepare indices for injecting curves
	GpuBufferCreateInformation injectIndexBufferCreateInformation;
	injectIndexBufferCreateInformation.Type = GpuBufferType::Index;
	injectIndexBufferCreateInformation.Index.Type = IT_16BIT;
	injectIndexBufferCreateInformation.Index.Count = 6;

	mInjectIndices = gpuDevice->CreateGpuBuffer(injectIndexBufferCreateInformation);

	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	auto* const indices = (u16*)B3DStackAllocate(mInjectIndices->GetTotalSize());

	// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
	// get culled.
	if(gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up)
	{
		indices[0] = 2;
		indices[1] = 1;
		indices[2] = 0;
		indices[3] = 3;
		indices[4] = 2;
		indices[5] = 0;
	}
	else
	{
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
	}

	mInjectIndices->WriteData(0, mInjectIndices->GetTotalSize(), indices);
	B3DStackFree(indices);

	// Prepare a scratch buffer we'll use to inject new curves
	GpuBufferCreateInformation injectScratchBufferCreateInformation;
	injectScratchBufferCreateInformation.Type = GpuBufferType::Vertex;
	injectScratchBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	injectScratchBufferCreateInformation.Vertex.Count = kScratchNumVertices;
	injectScratchBufferCreateInformation.Vertex.ElementSize = mInjectVertexDescription->GetVertexStride(0);

	mInjectScratch = gpuDevice->CreateGpuBuffer(injectScratchBufferCreateInformation);
}

GpuParticleCurves::~GpuParticleCurves()
{
	for(auto& entry : mPendingAllocations)
		mPendingAllocator.Free(entry.Pixels);

	mPendingAllocator.Clear();
}

TextureRowAllocation GpuParticleCurves::Alloc(Color* pixels, uint32_t count)
{
	PendingAllocation pendingAlloc;
	pendingAlloc.Allocation = mRowAllocator.Alloc(count);

	if(pendingAlloc.Allocation.Length == 0)
		return pendingAlloc.Allocation;

	pendingAlloc.Pixels = (Color*)mPendingAllocator.Alloc(sizeof(Color) * count);
	memcpy(pendingAlloc.Pixels, pixels, sizeof(Color) * count);

	mPendingAllocations.push_back(pendingAlloc);
	return pendingAlloc.Allocation;
}

void GpuParticleCurves::Free(const TextureRowAllocation& alloc)
{
	mRowAllocator.Free(alloc);
}

void GpuParticleCurves::ApplyChanges(GpuCommandBuffer& commandBuffer)
{
	const auto numCurves = (u32)mPendingAllocations.size();
	if(numCurves == 0)
		return;

	GpuParticleCurveInjectMat* injectMat = GpuParticleCurveInjectMat::Get();
	injectMat->Bind(commandBuffer);

	commandBuffer.BeginRenderPass(mRT, RT_NONE, RT_ALL);
	commandBuffer.SetVertexDescription(mInjectVertexDescription);

	SPtr<GpuBuffer> buffers[] = { mInjectScratch, mInjectUV };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(mInjectIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	u32 curveIdx = 0;

	while(curveIdx < numCurves)
	{
		auto* data = (GpuParticleCurveInject*)B3DStackAllocate(mInjectScratch->GetTotalSize());

		u32 count = 0;
		for(; curveIdx < numCurves; curveIdx++)
		{
			const PendingAllocation& pendingAlloc = mPendingAllocations[curveIdx];

			const u32 entryCount = pendingAlloc.Allocation.Length;
			if((count + entryCount) > kScratchNumVertices)
				break;

			for(u32 i = 0; i < entryCount; i++)
			{
				data[count].Color = pendingAlloc.Pixels[i];
				data[count].DataUv = Vector2(
					(pendingAlloc.Allocation.X + i) / (float)kTexSize,
					pendingAlloc.Allocation.Y / (float)kTexSize);

				count++;
			}
		}

		mInjectScratch->WriteData(0, mInjectScratch->GetTotalSize(), data, BWT_DISCARD); // TODO - Write using the command buffer below? It wouldn't require discard.

		B3DStackFree(data);
		commandBuffer.DrawIndexed(0, 6, 0, 4, count);
	}

	commandBuffer.EndRenderPass();

	for(auto& entry : mPendingAllocations)
		mPendingAllocator.Free(entry.Pixels);

	mPendingAllocations.clear();
	mPendingAllocator.Clear();
}

Vector2 GpuParticleCurves::GetUvOffset(const TextureRowAllocation& alloc)
{
	return Vector2(
		((float)alloc.X + 0.5f) / kTexSize,
		((float)alloc.Y + 0.5f) / kTexSize);
}

float GpuParticleCurves::GetUvScale(const TextureRowAllocation& alloc)
{
	if(alloc.Length == 0)
		return 0.0f;

	return (alloc.Length - 1) / (float)kTexSize;
}
}}
