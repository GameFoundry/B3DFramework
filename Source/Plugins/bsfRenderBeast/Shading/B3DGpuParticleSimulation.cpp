//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuParticleSimulation.h"
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsGpuResourcePool.h"
#include "RenderAPI/BsVertexDescription.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "Particles/BsVectorField.h"
#include "Particles/BsParticleDistribution.h"
#include "Math/BsVector3.h"
#include "BsRendererParticles.h"
#include "BsRenderBeastScene.h"
#include "BsRenderBeast.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Utility/BsGpuSort.h"

namespace b3d { namespace render {

B3D_PARAM_BLOCK_BEGIN(GpuParticleTileVertexParamsDef)
	B3D_PARAM_BLOCK_ENTRY(Vector4, gUVToNDC)
B3D_PARAM_BLOCK_END

GpuParticleTileVertexParamsDef gGpuParticleTileVertexParamsDef;

/** Material used for clearing tiles in the texture used for particle GPU simulation. */
class GpuParticleClearMat : public RendererMaterial<GpuParticleClearMat>
{
	RMAT_DEF_CUSTOMIZED("GpuParticleClear.bsl");

public:
	GpuParticleClearMat() = default;
	void Initialize() override;

	/** Binds the material to the pipeline, along with the @p tileUVs buffer containing locations of tiles to clear. */
	void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& tileUVs);

private:
	GpuParameterBuffer mTileUVParam;
};

/** Material used for adding new particles into the particle state textures. */
class GpuParticleInjectMat : public RendererMaterial<GpuParticleInjectMat>
{
	RMAT_DEF("GpuParticleInject.bsl");

public:
	GpuParticleInjectMat() = default;
	void Initialize() override;
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
	void Initialize() override;

	/** Binds the material to the pipeline along with any frame-static parameters. */
	void BindGlobal(GpuCommandBuffer& commandBuffer, GpuParticleResources& resources, const SPtr<GpuBuffer>& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& simulationParams);

	/**
	 * Binds parameters that change with every material dispatch.
	 *
	 * @param	commandBuffer			Command buffer to bind the parameters on.
	 * @param	tileUVs					Sets the UV offsets of individual tiles for a particular particle system
	 *										that's being rendered.
	 * @param	perObjectParams			General purpose particle system parameters.
	 * @param	vectorFieldParams		Information about the currently bound vector field, if any.
	 * @param	vectorFieldTexture		3D texture representing the vector field, or null if none.
	 * @param	depthCollisionParams	Parameter buffer for controlling depth buffer collisions, if enabled.
	 *
	 */
	void BindPerCallParams(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& tileUVs, const SPtr<GpuBuffer>& perObjectParams, const SPtr<GpuBuffer>& vectorFieldParams, const SPtr<Texture>& vectorFieldTexture, const SPtr<GpuBuffer>& depthCollisionParams);

	/** Returns the material variation matching the provided parameters. */
	static GpuParticleSimulateMat* GetVariation(bool depthCollisions, bool localSpace);

private:
	GpuParameterBuffer mTileUVParam;
	GpuParameterSampledTexture mPosAndTimeTexParam;
	GpuParameterSampledTexture mVelocityTexParam;
	GpuParameterSampledTexture mSizeRotationTexParam;
	GpuParameterSampledTexture mCurvesTexParam;
	GpuParameterSampledTexture mDepthTexParam;
	GpuParameterSampledTexture mNormalsTexParam;
	GpuParameterBinding mParamsBinding;
	GpuParameterBinding mPerCameraBinding;
	GpuParameterBinding mPerObjectBinding;

	GpuParameterBinding mVectorFieldBinding;
	GpuParameterSampledTexture mVectorFieldTexParam;

	GpuParameterBinding mDepthCollisionBinding;

	bool mSupportsDepthCollisions;
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
	SPtr<GpuBuffer> TileScratch;
	SPtr<GpuBuffer> InjectScratch;
};

GpuParticleResources::GpuParticleResources()
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

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
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

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

	// Prepare a scratch buffer we'll use to clear tiles
	GpuBufferCreateInformation tileScratchBufferCreateInformation;
	tileScratchBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	tileScratchBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	tileScratchBufferCreateInformation.SimpleStorage.Format = BF_32X2F;
	tileScratchBufferCreateInformation.SimpleStorage.Count = kNumScratchTiles;

	TileScratch = gpuDevice->CreateGpuBuffer(tileScratchBufferCreateInformation);

	// Prepare a scratch buffer we'll use to inject new particles
	GpuBufferCreateInformation injectScratchBufferCreateInformation;
	injectScratchBufferCreateInformation.Type = GpuBufferType::Vertex;
	injectScratchBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	injectScratchBufferCreateInformation.Vertex.Count = kNumScratchParticles;
	injectScratchBufferCreateInformation.Vertex.ElementSize = InjectVertexDescription->GetVertexStride(0);

	InjectScratch = gpuDevice->CreateGpuBuffer(injectScratchBufferCreateInformation);
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
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

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

struct GpuParticleSimulation::Pimpl
{
	GpuParticleResources Resources;
	GpuParticleHelperBuffers HelperBuffers;
	SPtr<GpuBuffer> VectorFieldParams;
	SPtr<GpuBuffer> DepthCollisionParams;
	SPtr<GpuBuffer> SimulationParams;
	UnorderedSet<GpuParticleSystem*> Systems;
};

GpuParticleSimulation::GpuParticleSimulation()
	: m(B3DNew<Pimpl>())
{
	m->VectorFieldParams = gVectorFieldParamsDef.CreateBuffer();
	m->DepthCollisionParams = gGpuParticleDepthCollisionParamsDef.CreateBuffer();
	m->SimulationParams = gGpuParticleSimulateParamsDef.CreateBuffer();
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

	commandBuffer.SetRenderTarget(m->Resources.GetInjectTarget(), 0, RT_ALL);

	ClearTiles(commandBuffer, newTiles);
	InjectParticles(commandBuffer, allNewParticles);

	// Simulate
	// TODO - Run multiple iterations for more stable simulation at lower/erratic framerates
	gGpuParticleSimulateParamsDef.gDT.Set(m->SimulationParams, dt);
	gGpuParticleSimulateParamsDef.gNumIterations.Set(m->SimulationParams, 1);

	commandBuffer.SetRenderTarget(m->Resources.GetSimulationTarget());
	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	enum class SimType
	{
		Normal,
		DepthCollisionsWorld,
		DepthCollisionsLocal,
		Count
	};

	for(u32 i = 0; i < (u32)SimType::Count; i++)
	{
		const SimType type = (SimType)i;
		const bool simulateDepthCollisions = type == SimType::DepthCollisionsWorld ||
			type == SimType::DepthCollisionsLocal;
		const bool localSpace = type == SimType::DepthCollisionsLocal;

		GpuParticleSimulateMat* simulateMat = GpuParticleSimulateMat::GetVariation(simulateDepthCollisions, localSpace);
		simulateMat->BindGlobal(commandBuffer, m->Resources, viewParams, gbuffer.Depth, gbuffer.Normals, m->SimulationParams);

		for(auto& entry : m->Systems)
		{
			if(entry->GetNumTiles() == 0)
				continue;

			ParticleSystem* parentSystem = entry->GetParent();

			const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();
			if(simSettings.DepthCollision.Enabled != simulateDepthCollisions)
				continue;

			if(simulateDepthCollisions)
			{
				const ParticleSystemSettings& settings = parentSystem->GetSettings();
				bool isLocal = settings.SimulationSpace == ParticleSimulationSpace::Local;
				if(isLocal != localSpace)
					continue;
			}

			const RendererParticles& rendererParticles = sceneInfo.ParticleSystems[parentSystem->GetRendererId()];

			PrepareBuffers(entry, rendererParticles);

			SPtr<Texture> vfTexture;
			if(simSettings.VectorField.VectorField)
				vfTexture = simSettings.VectorField.VectorField->GetTexture();

			simulateMat->BindPerCallParams(commandBuffer, entry->GetTileUVs(), rendererParticles.PerObjectParamBuffer, m->VectorFieldParams, vfTexture, m->DepthCollisionParams);

			const u32 tileCount = entry->GetNumTiles();
			const u32 numInstances = Math::DivideAndRoundUp(tileCount, kTilesPerInstance);
			commandBuffer.DrawIndexed(0, kTilesPerInstance * 6, 0, kTilesPerInstance * 4, numInstances);
		}
	}
}

void GpuParticleSimulation::Sort(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	const bool supportsCompute = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(!supportsCompute)
		return;

	// Make sure that the position texture isn't bound for rendering
	commandBuffer.SetRenderTarget(nullptr);

	const Vector3& viewOrigin = view.GetProperties().ViewOrigin;

	GpuParticleSortPrepareMat* prepareMat = GpuParticleSortPrepareMat::Get();
	prepareMat->Bind(commandBuffer, m->Resources.GetCurrentState().PositionAndTimeTex);

	u32 systemIdx = 0;
	u32 offset = 0;
	for(auto& entry : m->Systems)
	{
		if(entry->GetNumTiles() == 0)
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

void GpuParticleSimulation::PrepareBuffers(const GpuParticleSystem* system, const RendererParticles& rendererInfo)
{
	ParticleSystem* parentSystem = system->GetParent();

	const ParticleSystemSettings& settings = parentSystem->GetSettings();
	const ParticleGpuSimulationSettings& simSettings = parentSystem->GetGpuSimulationSettings();

	const Random& random = system->GetRandom();
	const float time = system->GetTime();
	const float nrmTime = time / settings.Duration;

	gGpuParticleSimulateParamsDef.gDrag.Set(m->SimulationParams, simSettings.Drag);
	gGpuParticleSimulateParamsDef.gAcceleration.Set(m->SimulationParams, simSettings.Acceleration);

	SPtr<Texture> vfTexture;
	if(simSettings.VectorField.VectorField)
		vfTexture = simSettings.VectorField.VectorField->GetTexture();

	if(vfTexture)
	{
		gGpuParticleSimulateParamsDef.gNumVectorFields.Set(m->SimulationParams, 1);

		const SPtr<VectorField>& vectorField = simSettings.VectorField.VectorField;
		const VECTOR_FIELD_DESC& vfDesc = vectorField->GetDesc();

		const Vector3 tiling(
			simSettings.VectorField.TilingX ? 0.0f : 1.0f,
			simSettings.VectorField.TilingY ? 0.0f : 1.0f,
			simSettings.VectorField.TilingZ ? 0.0f : 1.0f);

		gVectorFieldParamsDef.gFieldBounds.Set(m->VectorFieldParams, vfDesc.Bounds.GetSize());
		gVectorFieldParamsDef.gFieldTightness.Set(m->VectorFieldParams, simSettings.VectorField.Tightness);
		gVectorFieldParamsDef.gFieldTiling.Set(m->VectorFieldParams, tiling);
		gVectorFieldParamsDef.gFieldIntensity.Set(m->VectorFieldParams, simSettings.VectorField.Intensity);

		const Vector3 rotationRate = simSettings.VectorField.RotationRate.Evaluate(nrmTime, random) * time;
		const Quaternion addedRotation(Degree(rotationRate.X), Degree(rotationRate.Y), Degree(rotationRate.Z));

		const Vector3 offset = vfDesc.Bounds.Minimum + simSettings.VectorField.Offset;
		const Quaternion rotation = simSettings.VectorField.Rotation * addedRotation;
		const Vector3 scale = vfDesc.Bounds.GetSize() * simSettings.VectorField.Scale;

		Matrix4 fieldToWorld = Matrix4::TRS(offset, rotation, scale);
		fieldToWorld = rendererInfo.LocalToWorld * fieldToWorld;

		const Matrix3 fieldToWorld3x3 = fieldToWorld.Get3x3();

		gVectorFieldParamsDef.gFieldToWorld.Set(m->VectorFieldParams, fieldToWorld3x3);
		gVectorFieldParamsDef.gWorldToField.Set(m->VectorFieldParams, fieldToWorld.InverseAffine());
	}
	else
		gGpuParticleSimulateParamsDef.gNumVectorFields.Set(m->SimulationParams, 0);

	const ParticleDepthCollisionSettings& depthCollisionSettings = simSettings.DepthCollision;
	if(depthCollisionSettings.Enabled)
	{
		Vector3 scale3D = rendererInfo.ParticleSystem->GetWorldTransform().GetScale();
		float uniformScale = std::max(std::max(scale3D.X, scale3D.Y), scale3D.Z);

		gGpuParticleDepthCollisionParamsDef.gCollisionRange.Set(m->DepthCollisionParams, 2.0f);
		gGpuParticleDepthCollisionParamsDef.gCollisionRadiusScale.Set(m->DepthCollisionParams, depthCollisionSettings.RadiusScale * uniformScale);
		gGpuParticleDepthCollisionParamsDef.gDampening.Set(m->DepthCollisionParams, depthCollisionSettings.Dampening);
		gGpuParticleDepthCollisionParamsDef.gRestitution.Set(m->DepthCollisionParams, depthCollisionSettings.Restitution);

		const Vector2 sizeScaleUVOffset =
			GpuParticleCurves::GetUvOffset(rendererInfo.SizeScaleFrameIdxCurveAlloc);
		const float sizeScaleUVScale =
			GpuParticleCurves::GetUvScale(rendererInfo.SizeScaleFrameIdxCurveAlloc);

		gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveOffset.Set(m->DepthCollisionParams, sizeScaleUVOffset);
		gGpuParticleDepthCollisionParamsDef.gSizeScaleCurveScale.Set(m->DepthCollisionParams, Vector2(sizeScaleUVScale, 0.0f));
	}
}

void GpuParticleSimulation::ClearTiles(GpuCommandBuffer& commandBuffer, const Vector<u32>& tiles)
{
	const auto numTiles = (u32)tiles.size();
	if(numTiles == 0)
		return;

	const u32 numIterations = Math::DivideAndRoundUp(numTiles, GpuParticleHelperBuffers::kNumScratchTiles);

	GpuParticleClearMat* clearMat = GpuParticleClearMat::Get();
	clearMat->Bind(commandBuffer, m->HelperBuffers.TileScratch);

	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	u32 tileStart = 0;
	for(u32 i = 0; i < numIterations; i++)
	{
		static_assert(GpuParticleHelperBuffers::kNumScratchTiles % kTilesPerInstance == 0, "Tile scratch buffer size must be divisible with number of tiles per instance.");

		const u32 tileEnd = std::min(numTiles, tileStart + GpuParticleHelperBuffers::kNumScratchTiles);

		auto* tileUVs = (Vector2*)B3DStackAllocate(m->HelperBuffers.TileScratch->GetTotalSize());
		for(u32 j = tileStart; j < tileEnd; j++)
			tileUVs[j - tileStart] = GpuParticleResources::GetTileCoords(tiles[j]);

		const u32 alignedTileEnd = Math::DivideAndRoundUp(tileEnd, kTilesPerInstance) * kTilesPerInstance;
		for(u32 j = tileEnd; j < alignedTileEnd; j++)
			tileUVs[j] = Vector2(2.0f, 2.0f); // Out of bounds (we don't want to accidentaly clear used tiles)

		m->HelperBuffers.TileScratch->WriteData(0, m->HelperBuffers.TileScratch->GetTotalSize(), tileUVs, BWT_DISCARD); // TODO - Write using the command buffer below? It wouldn't require discard.
		B3DStackFree(tileUVs);

		const u32 numInstances = (alignedTileEnd - tileStart) / kTilesPerInstance;
		commandBuffer.DrawIndexed(0, kTilesPerInstance * 6, 0, kTilesPerInstance * 4, numInstances);

		tileStart = alignedTileEnd;
	}
}

void GpuParticleSimulation::InjectParticles(GpuCommandBuffer& commandBuffer, const Vector<GpuParticle>& particles)
{
	const auto numParticles = (u32)particles.size();
	const u32 numIterations = Math::DivideAndRoundUp(numParticles, GpuParticleHelperBuffers::kNumScratchParticles);

	GpuParticleInjectMat* injectMat = GpuParticleInjectMat::Get();
	injectMat->Bind(commandBuffer);

	commandBuffer.SetVertexDescription(m->HelperBuffers.InjectVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.InjectScratch, m->HelperBuffers.ParticleUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	u32 particleStart = 0;
	for(u32 i = 0; i < numIterations; i++)
	{
		const u32 particleEnd = std::min(numParticles, particleStart + GpuParticleHelperBuffers::kNumScratchParticles);

		auto* particleData = (GpuParticleVertex*)B3DStackAllocate(m->HelperBuffers.InjectScratch->GetTotalSize());
		for(u32 j = particleStart; j < particleEnd; j++)
			particleData[j - particleStart] = particles[j].GetVertex();

		m->HelperBuffers.InjectScratch->WriteData(0, m->HelperBuffers.InjectScratch->GetTotalSize(), particleData, BWT_DISCARD); // TODO - Write using the command buffer below? It wouldn't require discard.
		B3DStackFree(particleData);

		commandBuffer.DrawIndexed(0, 6, 0, 4, particleEnd - particleStart);
		particleStart = particleEnd;
	}
}

GpuParticleResources& GpuParticleSimulation::GetResources() const
{
	return m->Resources;
}

SPtr<GpuBuffer> CreateGpuParticleVertexInputBuffer()
{
	SPtr<GpuBuffer> inputBuffer = gGpuParticleTileVertexParamsDef.CreateBuffer();

	// [0, 1] -> [-1, 1] and flip Y
	Vector4 uvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
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

void GpuParticleClearMat::Initialize()
{
	const SPtr<GpuBuffer> inputBuffer = CreateGpuParticleVertexInputBuffer();

	mGPUParameters->SetUniformBuffer("Input", inputBuffer);
	mGPUParameters->GetStorageBufferParameter("gTileUVs", mTileUVParam);
}

void GpuParticleClearMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", kTilesPerInstance);
}

void GpuParticleClearMat::Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& tileUVs)
{
	mTileUVParam.Set(tileUVs);

	RendererMaterial::Bind(commandBuffer);
}

void GpuParticleInjectMat::Initialize()
{
	const SPtr<GpuBuffer> inputBuffer = CreateGpuParticleVertexInputBuffer();
	mGPUParameters->SetUniformBuffer("Input", inputBuffer);
}

void GpuParticleCurveInjectMat::Initialize()
{
	const SPtr<GpuBuffer> inputBuffer = CreateGpuParticleVertexInputBuffer();
	mGPUParameters->SetUniformBuffer("Input", inputBuffer);
}

void GpuParticleSimulateMat::Initialize()
{
	const SPtr<GpuBuffer> inputBuffer = CreateGpuParticleVertexInputBuffer();
	mGPUParameters->SetUniformBuffer("Input", inputBuffer);
	mGPUParameters->GetPipelineParameterInformation()->GetBinding("Params", mParamsBinding);

	mGPUParameters->GetPipelineParameterInformation()->GetBinding("VectorFieldParams", mVectorFieldBinding);

	mGPUParameters->GetStorageBufferParameter("gTileUVs", mTileUVParam);
	mGPUParameters->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
	mGPUParameters->GetSampledTextureParameter("gVelocityTex", mVelocityTexParam);
	mGPUParameters->GetSampledTextureParameter("gVectorFieldTex", mVectorFieldTexParam);

	mSupportsDepthCollisions = mVariationParameters.GetUI32("DEPTH_COLLISIONS") > 0;
	if(mSupportsDepthCollisions)
	{
		mGPUParameters->GetPipelineParameterInformation()->GetBinding("PerCamera", mPerCameraBinding);
		mGPUParameters->GetPipelineParameterInformation()->GetBinding("PerObject", mPerObjectBinding);
		mGPUParameters->GetPipelineParameterInformation()->GetBinding("DepthCollisionParams", mDepthCollisionBinding);

		mGPUParameters->GetSampledTextureParameter("gSizeRotationTex", mSizeRotationTexParam);
		mGPUParameters->GetSampledTextureParameter("gCurvesTex", mCurvesTexParam);
		mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexParam);
		mGPUParameters->GetSampledTextureParameter("gNormalsTex", mNormalsTexParam);
	}
}

void GpuParticleSimulateMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", kTilesPerInstance);
}

void GpuParticleSimulateMat::BindGlobal(GpuCommandBuffer& commandBuffer, GpuParticleResources& resources, const SPtr<GpuBuffer>& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& simulationParams)
{
	GpuParticleStateTextures& prevState = resources.GetPreviousState();
	const GpuParticleStaticTextures& staticTextures = resources.GetStaticTextures();
	GpuParticleCurves& curveTexture = resources.GetCurveTexture();

	mGPUParameters->SetUniformBuffer(mParamsBinding.Set, mParamsBinding.Slot, simulationParams);

	mPosAndTimeTexParam.Set(prevState.PositionAndTimeTex);
	mVelocityTexParam.Set(prevState.VelocityTex);

	if(mSupportsDepthCollisions)
	{
		mGPUParameters->SetUniformBuffer(mPerCameraBinding.Set, mPerCameraBinding.Slot, viewParams);

		mSizeRotationTexParam.Set(staticTextures.SizeAndRotationTex);
		mCurvesTexParam.Set(curveTexture.GetTexture());
		mDepthTexParam.Set(depth);
		mNormalsTexParam.Set(normals);
	}

	RendererMaterial::Bind(commandBuffer,false);
}

void GpuParticleSimulateMat::BindPerCallParams(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& tileUVs, const SPtr<GpuBuffer>& perObjectParams, const SPtr<GpuBuffer>& vectorFieldParams, const SPtr<Texture>& vectorFieldTexture, const SPtr<GpuBuffer>& depthCollisionParams)
{
	mTileUVParam.Set(tileUVs);
	mGPUParameters->SetUniformBuffer(mVectorFieldBinding.Set, mVectorFieldBinding.Slot, vectorFieldParams);
	mVectorFieldTexParam.Set(vectorFieldTexture);

	if(mSupportsDepthCollisions)
	{
		mGPUParameters->SetUniformBuffer(mPerObjectBinding.Set, mPerObjectBinding.Slot, perObjectParams);
		mGPUParameters->SetUniformBuffer(mDepthCollisionBinding.Set, mDepthCollisionBinding.Slot, depthCollisionParams);
	}

	BindParameters(commandBuffer);
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

	const u32 numParticles = system.GetNumTiles() * GpuParticleResources::kParticlesPerTile;

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
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

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

	commandBuffer.SetRenderTarget(mRT, 0, RT_ALL);
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
