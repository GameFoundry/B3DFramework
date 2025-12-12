//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuParticleSimulation.h"
#include "Renderer/B3DGpuUniformBuffer.h"
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
	positionAndTimeDesc.Width = GpuParticleConstants::kTexSize;
	positionAndTimeDesc.Height = GpuParticleConstants::kTexSize;
	positionAndTimeDesc.Usage = TU_RENDERTARGET;

	TextureCreateInformation velocityDesc;
	velocityDesc.Name = "GPU Particles Velocity";
	velocityDesc.Format = PF_RGBA16F;
	velocityDesc.Width = GpuParticleConstants::kTexSize;
	velocityDesc.Height = GpuParticleConstants::kTexSize;
	velocityDesc.Usage = TU_RENDERTARGET;

	for(u32 i = 0; i < 2; i++)
	{
		mStateTextures[i].PositionAndTimeTex = gpuDevice->CreateTexture(positionAndTimeDesc);
		mStateTextures[i].VelocityTex = gpuDevice->CreateTexture(velocityDesc);
	}

	TextureCreateInformation sizeAndRotationDesc;
	sizeAndRotationDesc.Name = "GPU Particles Size & Rotation";
	sizeAndRotationDesc.Format = PF_RGBA16F;
	sizeAndRotationDesc.Width = GpuParticleConstants::kTexSize;
	sizeAndRotationDesc.Height = GpuParticleConstants::kTexSize;
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
	sortKeysBufferCreateInformation.SimpleStorage.Count = GpuParticleConstants::kTexSize * GpuParticleConstants::kTexSize;

	mSortBuffers.Keys[0] = gpuDevice->CreateGpuBuffer(sortKeysBufferCreateInformation);
	mSortBuffers.Keys[1] = gpuDevice->CreateGpuBuffer(sortKeysBufferCreateInformation);

	// Allocate the buffer containing sorted particle indices
	GpuBufferCreateInformation sortedIndicesBufferCreateInformation;
	sortedIndicesBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	sortedIndicesBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	sortedIndicesBufferCreateInformation.SimpleStorage.Format = BF_16X2U;
	sortedIndicesBufferCreateInformation.SimpleStorage.Count = GpuParticleConstants::kTexSize * GpuParticleConstants::kTexSize;

	mSortedIndices[0] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);
	mSortedIndices[1] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);

	mSortBuffers.Values[0] = mSortedIndices[0];
	mSortBuffers.Values[1] = mSortedIndices[1];

	// Clear the free tile linked list
	for(u32 i = 0; i < GpuParticleConstants::kTileCount; i++)
		mFreeTiles[i] = GpuParticleConstants::kTileCount - i - 1;
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
	B3D_ASSERT(tile < GpuParticleConstants::kTileCount);
	B3D_ASSERT(mNumFreeTiles < GpuParticleConstants::kTileCount);

	mFreeTiles[mNumFreeTiles] = tile;
	mNumFreeTiles++;
}

Vector2I GpuParticleResources::GetTileOffset(u32 tileId)
{
	return Vector2I(
		(tileId % GpuParticleConstants::kTileCount1D) * GpuParticleConstants::kTileSize,
		(tileId / GpuParticleConstants::kTileCount1D) * GpuParticleConstants::kTileSize);
}

Vector2 GpuParticleResources::GetTileCoords(u32 tileId)
{
	return Vector2(
		Math::Frac(tileId / (float)GpuParticleConstants::kTileCount1D),
		(u32)(tileId / GpuParticleConstants::kTileCount1D) / (float)GpuParticleConstants::kTileCount1D);
}

Vector2I GpuParticleResources::GetParticleOffset(u32 subTileId)
{
	return Vector2I(
		subTileId % GpuParticleConstants::kTileSize,
		subTileId / GpuParticleConstants::kTileSize);
}

Vector2 GpuParticleResources::GetParticleCoords(u32 subTileId)
{
	const Vector2 tileOffset = GetParticleOffset(subTileId).To<float>();
	return tileOffset / (float)GpuParticleConstants::kTexSize;
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
	tileUVBufferCreateInformation.Vertex.Count = GpuParticleConstants::kParticlesPerInstance * 4;
	tileUVBufferCreateInformation.Vertex.ElementSize = TileVertexDescription->GetVertexStride();

	TileUVs = gpuDevice->CreateGpuBuffer(tileUVBufferCreateInformation);

	auto* const tileUVData = (Vector2*)B3DStackAllocate(TileUVs->GetTotalSize());
	const float tileUVScale = GpuParticleConstants::kTileSize / (float)GpuParticleConstants::kTexSize;
	for(u32 i = 0; i < GpuParticleConstants::kParticlesPerInstance; i++)
	{
		tileUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * tileUVScale;
		tileUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * tileUVScale;
		tileUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * tileUVScale;
		tileUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * tileUVScale;
	}

	GpuBufferUtility::Write(TileUVs, 0, TileUVs->GetTotalSize(), tileUVData);
	B3DStackFree(tileUVData);

	// Prepare UV coordinates for rendering particles
	GpuBufferCreateInformation particleUVBufferCreateInformation;
	particleUVBufferCreateInformation.Type = GpuBufferType::Vertex;
	particleUVBufferCreateInformation.Vertex.Count = GpuParticleConstants::kParticlesPerInstance * 4;
	particleUVBufferCreateInformation.Vertex.ElementSize = TileVertexDescription->GetVertexStride();

	ParticleUVs = gpuDevice->CreateGpuBuffer(particleUVBufferCreateInformation);

	auto* const particleUVData = (Vector2*)B3DStackAllocate(ParticleUVs->GetTotalSize());
	const float particleUVScale = 1.0f / (float)GpuParticleConstants::kTexSize;
	for(u32 i = 0; i < GpuParticleConstants::kParticlesPerInstance; i++)
	{
		particleUVData[i * 4 + 0] = Vector2(0.0f, 0.0f) * particleUVScale;
		particleUVData[i * 4 + 1] = Vector2(1.0f, 0.0f) * particleUVScale;
		particleUVData[i * 4 + 2] = Vector2(1.0f, 1.0f) * particleUVScale;
		particleUVData[i * 4 + 3] = Vector2(0.0f, 1.0f) * particleUVScale;
	}

	GpuBufferUtility::Write(ParticleUVs, 0, ParticleUVs->GetTotalSize(), particleUVData);
	B3DStackFree(particleUVData);

	// Prepare indices for rendering tiles & particles
	GpuBufferCreateInformation spriteIndexBufferCreateInformation;
	spriteIndexBufferCreateInformation.Type = GpuBufferType::Index;
	spriteIndexBufferCreateInformation.Index.Type = IT_16BIT;
	spriteIndexBufferCreateInformation.Index.Count = GpuParticleConstants::kParticlesPerInstance * 6;

	SpriteIndices = gpuDevice->CreateGpuBuffer(spriteIndexBufferCreateInformation);

	auto* const indices = (u16*)B3DStackAllocate(SpriteIndices->GetTotalSize());

	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;
	for(u32 i = 0; i < GpuParticleConstants::kParticlesPerInstance; i++)
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

	GpuBufferUtility::Write(SpriteIndices, 0, SpriteIndices->GetTotalSize(), indices);
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
			mTiles[tileIdx].NumFreeParticles = GpuParticleConstants::kParticlesPerTile;

			cachedTile = mTiles[tileIdx];
			mNumActiveTiles++;
		}

		GpuParticleTile& tile = mTiles[tileIdx];
		GpuParticle& particle = newParticles[i];

		const u32 tileParticleIdx = GpuParticleConstants::kParticlesPerTile - tile.NumFreeParticles;
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

	const auto tileCount = (u32)mTiles.size();
	const u32 tilesToAllocateCount = Math::DivideAndRoundUp(tileCount, GpuParticleConstants::kTilesPerInstance) * GpuParticleConstants::kTilesPerInstance;

	// Tile offsets buffer
	if(tileCount > 0)
	{
		GpuBufferCreateInformation tilesBufferCreateInformation;
		tilesBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		tilesBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		tilesBufferCreateInformation.SimpleStorage.Format = BF_32X2F;
		tilesBufferCreateInformation.SimpleStorage.Count = tilesToAllocateCount;

		mTileUVs = gpuDevice->CreateGpuBuffer(tilesBufferCreateInformation);

		void* destinationMemory = mTileUVs->Lock(0, mTileUVs->GetTotalSize(), GBL_WRITE_ONLY);

		auto* tileUVs = (Vector2*)destinationMemory;
		for(u32 i = 0; i < tileCount; i++)
			tileUVs[i] = GpuParticleResources::GetTileCoords(mTiles[i].Id);

		for(u32 i = tileCount; i < tilesToAllocateCount; i++)
			tileUVs[i] = Vector2(2.0f, 2.0f); // Out of range

		mTileUVs->Unlock();
	}

	// Particle data offsets
	const u32 particleCOunt = tileCount * GpuParticleConstants::kParticlesPerTile;

	if(particleCOunt > 0)
	{
		GpuBufferCreateInformation particleUVCreateInformation;
		particleUVCreateInformation.Type = GpuBufferType::SimpleStorage;
		particleUVCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		particleUVCreateInformation.SimpleStorage.Format = BF_16X2U;
		particleUVCreateInformation.SimpleStorage.Count = particleCOunt;

		mParticleIndices = gpuDevice->CreateGpuBuffer(particleUVCreateInformation);

		void* destinationMemory = mParticleIndices->Lock(0, mParticleIndices->GetTotalSize(), GBL_WRITE_ONLY);
		auto* particleIndices = (u32*)destinationMemory;

		u32 particleIndex = 0;
		for(u32 tileIndex = 0; tileIndex < tileCount; tileIndex++)
		{
			const Vector2I tileOffset = GpuParticleResources::GetTileOffset(mTiles[tileIndex].Id);
			for(u32 y = 0; y < GpuParticleConstants::kTileSize; y++)
			{
				for(u32 x = 0; x < GpuParticleConstants::kTileSize; x++)
				{
					const Vector2I offset = tileOffset + Vector2I(x, y);
					particleIndices[particleIndex++] = (offset.X & 0xFFFF) | (offset.Y << 16);
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
	mTime = b3d::ParticleSystem::AdvanceTime(mTime, dt, settings.Duration, settings.IsLooping, timeStep);
}

AABox GpuParticleSystem::GetBounds() const
{
	const ParticleSystemSettings& settings = mParent->GetSettings();

	if(settings.UseAutomaticBounds)
		return AABox(-(float)kMaximumSceneExtent, (float)kMaximumSceneExtent);

	return settings.CustomBounds;
}

const SPtr<GpuParameterSet>& GpuParticleSystem::PrepareSimulateParameters(const RendererParticles& rendererInfo, float dt)
{
	// Note: Many of those could only be updated when relevant settings change, but for simplicity we update them every frame.

	ParticleSystem* const parentSystem = GetParent();

	const ParticleGpuSimulationSettings& gpuSimulationSettings = parentSystem->GetGpuSimulationSettings();
	const ParticleSystemSettings& systemSettings = parentSystem->GetSettings();

	const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;
	const bool localSpace = systemSettings.SimulationSpace == ParticleSimulationSpace::Local;

	GpuParticleSimulateMaterial* const simulateMaterial = GpuParticleSimulateMaterial::GetVariation(supportsDepthCollisions, localSpace);
	mSimulateParameters = simulateMaterial->CreateGpuParameterSet();

	// Get parent system settings
	const ParticleSystemSettings& settings = mParent->GetSettings();
	const ParticleGpuSimulationSettings& simSettings = mParent->GetGpuSimulationSettings();

	const float time = GetTime();
	const float nrmTime = time / settings.Duration;

	// Allocate transient uniform buffers
	GpuBufferMappedScope simulationUniforms = gGpuParticleSimulateUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope vectorFieldUniforms = gVectorFieldUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope depthCollisionUniforms = gGpuParticleDepthCollisionUniformDefinition.AllocateTransient().Map();

	// Write simulation parameters
	gGpuParticleSimulateUniformDefinition.gDT.Set(simulationUniforms, dt);
	gGpuParticleSimulateUniformDefinition.gNumIterations.Set(simulationUniforms, 1);
	gGpuParticleSimulateUniformDefinition.gDrag.Set(simulationUniforms, simSettings.Drag);
	gGpuParticleSimulateUniformDefinition.gAcceleration.Set(simulationUniforms, simSettings.Acceleration);

	// Write vector field parameters
	SPtr<Texture> vfTexture;
	if(simSettings.VectorField.VectorField)
		vfTexture = simSettings.VectorField.VectorField->GetTexture();

	if(vfTexture)
	{
		gGpuParticleSimulateUniformDefinition.gNumVectorFields.Set(simulationUniforms, 1);

		const SPtr<VectorField>& vectorField = simSettings.VectorField.VectorField;
		const VECTOR_FIELD_DESC& vfDesc = vectorField->GetDesc();

		const Vector3 tiling(
			simSettings.VectorField.TilingX ? 0.0f : 1.0f,
			simSettings.VectorField.TilingY ? 0.0f : 1.0f,
			simSettings.VectorField.TilingZ ? 0.0f : 1.0f);

		gVectorFieldUniformDefinition.gFieldBounds.Set(vectorFieldUniforms, vfDesc.Bounds.GetSize());
		gVectorFieldUniformDefinition.gFieldTightness.Set(vectorFieldUniforms, simSettings.VectorField.Tightness);
		gVectorFieldUniformDefinition.gFieldTiling.Set(vectorFieldUniforms, tiling);
		gVectorFieldUniformDefinition.gFieldIntensity.Set(vectorFieldUniforms, simSettings.VectorField.Intensity);

		const Vector3 rotationRate = simSettings.VectorField.RotationRate.Evaluate(nrmTime, mRandom) * time;
		const Quaternion addedRotation(Degree(rotationRate.X), Degree(rotationRate.Y), Degree(rotationRate.Z));

		const Vector3 offset = vfDesc.Bounds.Minimum + simSettings.VectorField.Offset;
		const Quaternion rotation = simSettings.VectorField.Rotation * addedRotation;
		const Vector3 scale = vfDesc.Bounds.GetSize() * simSettings.VectorField.Scale;

		Matrix4 fieldToWorld = Matrix4::TRS(offset, rotation, scale);
		fieldToWorld = rendererInfo.WorldTransform * fieldToWorld;

		const Matrix3 fieldToWorld3x3 = fieldToWorld.Get3x3();

		gVectorFieldUniformDefinition.gFieldToWorld.Set(vectorFieldUniforms, fieldToWorld3x3);
		gVectorFieldUniformDefinition.gWorldToField.Set(vectorFieldUniforms, fieldToWorld.InverseAffine());
	}
	else
	{
		gGpuParticleSimulateUniformDefinition.gNumVectorFields.Set(simulationUniforms, 0);
	}

	// Write depth collision parameters
	const ParticleDepthCollisionSettings& depthCollisionSettings = simSettings.DepthCollision;
	if(depthCollisionSettings.Enabled)
	{
		Vector3 scale3D = rendererInfo.ParticleSystem->GetWorldTransform().GetScale();
		float uniformScale = std::max(std::max(scale3D.X, scale3D.Y), scale3D.Z);

		gGpuParticleDepthCollisionUniformDefinition.gCollisionRange.Set(depthCollisionUniforms, 2.0f);
		gGpuParticleDepthCollisionUniformDefinition.gCollisionRadiusScale.Set(
			depthCollisionUniforms, depthCollisionSettings.RadiusScale * uniformScale);
		gGpuParticleDepthCollisionUniformDefinition.gDampening.Set(depthCollisionUniforms, depthCollisionSettings.Dampening);
		gGpuParticleDepthCollisionUniformDefinition.gRestitution.Set(depthCollisionUniforms, depthCollisionSettings.Restitution);

		const Vector2 sizeScaleUVOffset =
			GpuParticleCurves::GetUvOffset(rendererInfo.SizeScaleFrameIdxCurveAlloc);
		const float sizeScaleUVScale =
			GpuParticleCurves::GetUvScale(rendererInfo.SizeScaleFrameIdxCurveAlloc);

		gGpuParticleDepthCollisionUniformDefinition.gSizeScaleCurveOffset.Set(depthCollisionUniforms, sizeScaleUVOffset);
		gGpuParticleDepthCollisionUniformDefinition.gSizeScaleCurveScale.Set(
			depthCollisionUniforms, Vector2(sizeScaleUVScale, 0.0f));
	}

	// Bind transient uniform buffers to GPU parameters
	mSimulateParameters->SetUniformBuffer("Params", simulationUniforms);
	mSimulateParameters->SetUniformBuffer("VectorFieldParams", vectorFieldUniforms);
	mSimulateParameters->SetUniformBuffer("DepthCollisionParams", depthCollisionUniforms);

	return mSimulateParameters;
}

static SPtr<GpuBuffer> CreateGpuParticleVertexInputBuffer()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const u32 size = gGpuParticleTileVertexUniformDefinition.GetSize();
	SPtr<GpuBuffer> stagingBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(size));
	SPtr<GpuBuffer> inputBuffer = gGpuParticleTileVertexUniformDefinition.CreateBuffer(GpuBufferFlag::StoreOnGPU);

	// [0, 1] -> [-1, 1] and flip Y
	Vector4 uvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	// Either of these flips the Y axis, but if they're both true they cancel out
	if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
	{
		uvToNdc.Y = -uvToNdc.Y;
		uvToNdc.W = -uvToNdc.W;
	}

	gGpuParticleTileVertexUniformDefinition.gUVToNDC.Set(stagingBuffer, uvToNdc);

	//stagingBuffer->FlushCache(); // TODO - Call regular flush here
	gpuDevice->GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer()->CopyBufferToBuffer(stagingBuffer, inputBuffer, 0, 0, size);

	return inputBuffer;
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

	RenderPassCreateInformation simulatePass(m->Resources.GetSimulationTarget(), RT_NONE, RT_ALL);
	for(auto& entry : m->Systems)
	{
		if(entry->GetTileCount() == 0)
			continue;

		ParticleSystem* parentSystem = entry->GetParent();
		const RendererParticles& rendererParticles = sceneInfo.ParticleSystems[parentSystem->GetRendererId()];

		// Get vector field texture if any
		SPtr<Texture> vectorFieldTexture;

		const ParticleGpuSimulationSettings& gpuSimulationSettings = parentSystem->GetGpuSimulationSettings();
		if(gpuSimulationSettings.VectorField.VectorField)
			vectorFieldTexture = gpuSimulationSettings.VectorField.VectorField->GetTexture();

		const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;

		const SPtr<GpuParameterSet>& systemParams = entry->PrepareSimulateParameters(rendererParticles, dt);

		// Populate remaining parameters (textures and other buffers)
		GpuParticleSimulateMaterial::PopulateParameters(systemParams, m->Resources, m->ParticleVertexInputBuffer, viewParams,
			gbuffer.Depth, gbuffer.Normals, entry->GetTileUVs(), rendererParticles.PerObjectSuballocation, vectorFieldTexture, supportsDepthCollisions
		);

		simulatePass.Parameters.Add(systemParams);
	}

	commandBuffer.BeginRenderPass(simulatePass);
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
		GpuParticleSimulateMaterial* const simulateMaterial = GpuParticleSimulateMaterial::GetVariation(currentDepthCollisions, currentLocalSpace);
		simulateMaterial->Bind(commandBuffer, false);

		// Render all systems matching this simulation type
		for(auto& entry : m->Systems)
		{
			if(entry->GetTileCount() == 0)
				continue;

			ParticleSystem* parentSystem = entry->GetParent();
			const ParticleGpuSimulationSettings& gpuSimulationSettings = parentSystem->GetGpuSimulationSettings();
			const ParticleSystemSettings& settings = parentSystem->GetSettings();

			const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;
			const bool localSpace = settings.SimulationSpace == ParticleSimulationSpace::Local;

			// Skip systems that don't match this simulation type
			if(supportsDepthCollisions != currentDepthCollisions || (currentDepthCollisions && localSpace != currentLocalSpace))
				continue;

			// Get the pre-populated parameters
			SPtr<GpuParameterSet> systemParams = entry->GetSimulateParameters();

			// Bind parameters and draw
			commandBuffer.SetGpuParameterSet(systemParams);

			const u32 tileCount = entry->GetTileCount();
			const u32 numInstances = Math::DivideAndRoundUp(tileCount, GpuParticleConstants::kTilesPerInstance);
			commandBuffer.DrawIndexed(0, GpuParticleConstants::kTilesPerInstance * 6, 0, GpuParticleConstants::kTilesPerInstance * 4, numInstances);
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

	GpuParticleSortPrepareMaterial* prepareMat = GpuParticleSortPrepareMaterial::Get();
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

	GpuParticleClearMaterial* const clearMaterial = GpuParticleClearMaterial::Get();
	clearMaterial->Bind(commandBuffer, false);

	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const TileClearBatch& batch : m->PreparedTileClearBatches)
	{
		commandBuffer.SetGpuParameterSet(batch.Parameters.GpuParameters);
		commandBuffer.DrawIndexed(0, GpuParticleConstants::kTilesPerInstance * 6, 0, GpuParticleConstants::kTilesPerInstance * 4, batch.InstanceCount);
	}
}

void GpuParticleSimulation::DrawInjectParticles(GpuCommandBuffer& commandBuffer)
{
	if(m->PreparedParticleInjectBatches.Empty())
		return;

	GpuParticleInjectMaterial* const injectMaterial = GpuParticleInjectMaterial::Get();
	injectMaterial->Bind(commandBuffer, false);

	commandBuffer.SetVertexDescription(m->HelperBuffers.InjectVertexDescription);
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const ParticleInjectBatch& batch : m->PreparedParticleInjectBatches)
	{
		commandBuffer.SetGpuParameterSet(batch.Parameters.GpuParameters);

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

	output.GpuParameters = GpuParticleClearMaterial::Get()->CreateGpuParameterSet();
	GpuParticleClearMaterial::PopulateParameters(output.GpuParameters, m->ParticleVertexInputBuffer, output.ScratchBuffer);

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

	output.GpuParameters = GpuParticleInjectMaterial::Get()->CreateGpuParameterSet();
	GpuParticleInjectMaterial::PopulateParameters(output.GpuParameters, m->ParticleVertexInputBuffer);

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
		static_assert(GpuParticleHelperBuffers::kNumScratchTiles % GpuParticleConstants::kTilesPerInstance == 0,
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
		const u32 alignedTileEnd = Math::DivideAndRoundUp(tileEnd, GpuParticleConstants::kTilesPerInstance) * GpuParticleConstants::kTilesPerInstance;
		for(u32 tileIndex = tileEnd; tileIndex < alignedTileEnd; ++tileIndex)
			tileUVs[tileIndex] = Vector2(2.0f, 2.0f); // Out of bounds

		// Write data to buffer (BEFORE render pass)
		GpuBufferUtility::Write(parameters.ScratchBuffer, 0, parameters.ScratchBuffer->GetTotalSize(), tileUVs);

		// Free temporary array
		B3DStackFree(tileUVs);

		// Store batch information for rendering
		TileClearBatch batch;
		batch.Parameters = parameters;
		batch.InstanceCount = (alignedTileEnd - tileStart) / GpuParticleConstants::kTilesPerInstance;
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
		GpuBufferUtility::Write(parameters.ScratchBuffer, 0, parameters.ScratchBuffer->GetTotalSize(), particleData, GpuBufferWriteFlag::Discard);

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

	GpuBufferUtility::Write(mInjectUV, 0, mInjectUV->GetTotalSize(), tileUVData);
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

	GpuBufferUtility::Write(mInjectIndices, 0, mInjectIndices->GetTotalSize(), indices);
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

	pendingAlloc.Pixels = (Color*)mPendingAllocator.Allocate(sizeof(Color) * count);
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

	GpuParticleCurveInjectMaterial* injectMat = GpuParticleCurveInjectMaterial::Get();
	injectMat->Prepare(CreateGpuParticleVertexInputBuffer());
	injectMat->Bind(commandBuffer);

	RenderPassCreateInformation renderPassCreateInformation(mRT, injectMat->GetGpuParameterSet(), RT_NONE, RT_ALL);

	commandBuffer.BeginRenderPass(renderPassCreateInformation);
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

		GpuBufferUtility::Write(mInjectScratch, 0, mInjectScratch->GetTotalSize(), data, GpuBufferWriteFlag::Discard); // TODO - Write using the command buffer below? It wouldn't require discard.

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
