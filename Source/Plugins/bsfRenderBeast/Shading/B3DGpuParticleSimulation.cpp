//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuParticleSimulation.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "Renderer/B3DRenderer.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "Particles/B3DVectorField.h"
#include "Particles/B3DParticleDistribution.h"
#include "Math/B3DVector3.h"
#include "RenderState/B3DParticleRenderState.h"
#include "B3DRenderBeastScene.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Utility/B3DGpuSort.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Utility/B3DBitwise.h"

namespace b3d { namespace render {

/** Contains a variety of helper buffers and declarations used for GPU particle simulation. */
struct GpuParticleHelperBuffers
{
	static constexpr u32 kNumScratchTiles = 512;
	static constexpr u32 kNumScratchParticles = 4096;

	GpuParticleHelperBuffers();

	TShared<GpuBuffer> TileUVs;
	TShared<GpuBuffer> ParticleUVs;
	TShared<GpuBuffer> SpriteIndices;
	TShared<VertexDescription> TileVertexDescription;
	TShared<VertexDescription> InjectVertexDescription;
};

GpuParticleResources::GpuParticleResources()
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	// Allocate textures
	TextureCreateInformation positionAndTimeDesc;
	positionAndTimeDesc.Name = "GPU Particles Position & Time";
	positionAndTimeDesc.Format = PF_RGBA32F;
	positionAndTimeDesc.Width = GpuParticleConstants::kTexSize;
	positionAndTimeDesc.Height = GpuParticleConstants::kTexSize;
	positionAndTimeDesc.Usage = TextureUsageFlag::RenderTarget;

	TextureCreateInformation velocityDesc;
	velocityDesc.Name = "GPU Particles Velocity";
	velocityDesc.Format = PF_RGBA16F;
	velocityDesc.Width = GpuParticleConstants::kTexSize;
	velocityDesc.Height = GpuParticleConstants::kTexSize;
	velocityDesc.Usage = TextureUsageFlag::RenderTarget;

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
	sizeAndRotationDesc.Usage = TextureUsageFlag::RenderTarget;

	mStaticTextures.SizeAndRotationTex = gpuDevice->CreateTexture(sizeAndRotationDesc);

	// Allocate the buffer containing sorted particle indices
	GpuBufferCreateInformation sortedIndicesBufferCreateInformation;
	sortedIndicesBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	sortedIndicesBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	sortedIndicesBufferCreateInformation.SimpleStorage.Format = BF_16X2U;
	sortedIndicesBufferCreateInformation.SimpleStorage.Count = GpuParticleConstants::kTexSize * GpuParticleConstants::kTexSize;

	mSortedIndices[0] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);
	mSortedIndices[1] = gpuDevice->CreateGpuBuffer(sortedIndicesBufferCreateInformation);

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

const TShared<GpuBuffer>& GpuParticleResources::GetSortedIndices() const
{
	return mSortedIndices[mSortedIndicesBufferIdx];
}

GpuParticleDrawInputs GpuParticleResources::GetDrawInputs() const
{
	GpuParticleDrawInputs inputs;
	inputs.PositionAndTime = GetCurrentState().PositionAndTimeTex;
	inputs.SizeAndRotation = mStaticTextures.SizeAndRotationTex;
	inputs.Curves = mCurveTexture.GetTexture();
	inputs.SortedIndices = GetSortedIndices();

	return inputs;
}

void GpuParticleResources::Import(FrameGraph& graph) const
{
	for(const GpuParticleStateTextures& state : mStateTextures)
	{
		graph.ImportTexture(state.PositionAndTimeTex);
		graph.ImportTexture(state.VelocityTex);
	}

	graph.ImportTexture(mStaticTextures.SizeAndRotationTex);
	graph.ImportTexture(mCurveTexture.GetTexture());

	for(const TShared<GpuBuffer>& sortedIndices : mSortedIndices)
		graph.ImportBuffer(sortedIndices);
}

void GpuParticleDrawInputs::Import(FrameGraph& graph) const
{
	graph.ImportTexture(PositionAndTime);
	graph.ImportTexture(SizeAndRotation);
	graph.ImportTexture(Curves);
	graph.ImportBuffer(SortedIndices);
}

GpuParticleHelperBuffers::GpuParticleHelperBuffers()
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

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

	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	GpuBufferUtility::Write(gpuContext, TileUVs, 0, TileUVs->GetTotalSize(), tileUVData);
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

	GpuBufferUtility::Write(gpuContext, ParticleUVs, 0, ParticleUVs->GetTotalSize(), particleUVData);
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

	GpuBufferUtility::Write(gpuContext, SpriteIndices, 0, SpriteIndices->GetTotalSize(), indices);
	B3DStackFree(indices);
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

				// Re-used tiles contain stale (expired) particle data, so queue them for a GPU-side clear as well
				newTiles.push_back(mTiles[tileIdx].Id);
			}
			// And finally just allocate a new tile if no room elsewhere
			else
			{
				const u32 tileId = resources.AllocTile();
				if(tileId == (u32)-1)
					return newTilesAdded; // Out of space in the texture

				GpuParticleTile newTile;
				newTile.Id = tileId;
				newTile.RemainingLifetime = 0.0f;

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
		tile.RemainingLifetime = std::max(tile.RemainingLifetime, particle.Lifetime);

		cachedTile.NumFreeParticles--;
	}

	return newTilesAdded;
}

void GpuParticleSystem::DetectInactiveTiles(float timeStep)
{
	mNumActiveTiles = 0;
	for(u32 i = 0; i < (u32)mTiles.size(); i++)
	{
		mTiles[i].RemainingLifetime -= timeStep;

		if(mTiles[i].RemainingLifetime > 0.0f)
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
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

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

		GpuBufferMappedScope mapping = mTileUVs->Map(GpuMapOption::Write);
		void* destinationMemory = mapping.GetMappedMemory();

		auto* tileUVs = (Vector2*)destinationMemory;
		for(u32 i = 0; i < tileCount; i++)
			tileUVs[i] = GpuParticleResources::GetTileCoords(mTiles[i].Id);

		for(u32 i = tileCount; i < tilesToAllocateCount; i++)
			tileUVs[i] = Vector2(2.0f, 2.0f); // Out of range
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

		GpuBufferMappedScope mapping = mParticleIndices->Map(GpuMapOption::Write);
		auto* particleIndices = (u32*)mapping.GetMappedMemory();

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
	}
}

TShared<GpuParameterSet> GpuParticleSimulation::PrepareSimulateParameters(const ParticleSystemProxy& proxy, const ParticleRenderState& renderState, const GpuParticleSystem& system, float dt)
{
	// Note: Many of those could only be updated when relevant settings change, but for simplicity we update them every frame.

	const render::ParticleGpuSimulationSettings& gpuSimulationSettings = proxy.GetGpuSimulationSettings();
	const render::ParticleSystemSettings& systemSettings = proxy.GetSettings();

	const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;
	const bool localSpace = systemSettings.SimulationSpace == ParticleSimulationSpace::Local;

	GpuParticleSimulateMaterial* const simulateMaterial = GpuParticleSimulateMaterial::GetVariation(supportsDepthCollisions, localSpace);
	TShared<GpuParameterSet> simulateParameters = simulateMaterial->CreateGpuParameterSet();

	const ParticleSystemSettings& settings = proxy.GetSettings();
	const ParticleGpuSimulationSettings& simSettings = proxy.GetGpuSimulationSettings();

	const float time = system.GetTime();
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
	TShared<Texture> vfTexture;
	if(simSettings.VectorField.VectorField)
		vfTexture = simSettings.VectorField.VectorField->GetTexture();

	if(vfTexture)
	{
		gGpuParticleSimulateUniformDefinition.gNumVectorFields.Set(simulationUniforms, 1);

		const TShared<VectorField>& vectorField = simSettings.VectorField.VectorField;
		const VECTOR_FIELD_DESC& vfDesc = vectorField->GetDesc();

		const Vector3 tiling(
			simSettings.VectorField.TilingX ? 0.0f : 1.0f,
			simSettings.VectorField.TilingY ? 0.0f : 1.0f,
			simSettings.VectorField.TilingZ ? 0.0f : 1.0f);

		gVectorFieldUniformDefinition.gFieldBounds.Set(vectorFieldUniforms, vfDesc.Bounds.GetSize());
		gVectorFieldUniformDefinition.gFieldTightness.Set(vectorFieldUniforms, simSettings.VectorField.Tightness);
		gVectorFieldUniformDefinition.gFieldTiling.Set(vectorFieldUniforms, tiling);
		gVectorFieldUniformDefinition.gFieldIntensity.Set(vectorFieldUniforms, simSettings.VectorField.Intensity);

		const Vector3 rotationRate = simSettings.VectorField.RotationRate.Evaluate(nrmTime, system.GetRandom()) * time;
		const Quaternion addedRotation(Degree(rotationRate.X), Degree(rotationRate.Y), Degree(rotationRate.Z));

		const Vector3 offset = vfDesc.Bounds.Minimum + simSettings.VectorField.Offset;
		const Quaternion rotation = simSettings.VectorField.Rotation * addedRotation;
		const Vector3 scale = vfDesc.Bounds.GetSize() * simSettings.VectorField.Scale;

		Matrix4 fieldToWorld = Matrix4::TRS(offset, rotation, scale);
		fieldToWorld = renderState.WorldTransform * fieldToWorld;

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
		Vector3 scale3D = proxy.GetWorldTransform().GetScale();
		float uniformScale = std::max(std::max(scale3D.X, scale3D.Y), scale3D.Z);

		gGpuParticleDepthCollisionUniformDefinition.gCollisionRange.Set(depthCollisionUniforms, 2.0f);
		gGpuParticleDepthCollisionUniformDefinition.gCollisionRadiusScale.Set(
			depthCollisionUniforms, depthCollisionSettings.RadiusScale * uniformScale);
		gGpuParticleDepthCollisionUniformDefinition.gDampening.Set(depthCollisionUniforms, depthCollisionSettings.Dampening);
		gGpuParticleDepthCollisionUniformDefinition.gRestitution.Set(depthCollisionUniforms, depthCollisionSettings.Restitution);

		const Vector2 sizeScaleUVOffset =
			GpuParticleCurves::GetUvOffset(renderState.SizeScaleFrameIdxCurveAlloc);
		const float sizeScaleUVScale =
			GpuParticleCurves::GetUvScale(renderState.SizeScaleFrameIdxCurveAlloc);

		gGpuParticleDepthCollisionUniformDefinition.gSizeScaleCurveOffset.Set(depthCollisionUniforms, sizeScaleUVOffset);
		gGpuParticleDepthCollisionUniformDefinition.gSizeScaleCurveScale.Set(
			depthCollisionUniforms, Vector2(sizeScaleUVScale, 0.0f));
	}

	// Bind transient uniform buffers to GPU parameters
	simulateParameters->SetUniformBuffer("Params", simulationUniforms);
	simulateParameters->SetUniformBuffer("VectorFieldParams", vectorFieldUniforms);
	simulateParameters->TrySetUniformBuffer("DepthCollisionParams", depthCollisionUniforms);

	return simulateParameters;
}

static TShared<GpuBuffer> CreateGpuParticleVertexInputBuffer()
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();

	TShared<GpuBuffer> inputBuffer = gGpuParticleTileVertexUniformDefinition.CreateBuffer(GpuBufferFlag::StoreOnGPU);

	// [0, 1] -> [-1, 1] and flip Y
	Vector4 uvToNdc(2.0f, -2.0f, -1.0f, 1.0f);

	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	// Either of these flips the Y axis, but if they're both true they cancel out
	if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
	{
		uvToNdc.Y = -uvToNdc.Y;
		uvToNdc.W = -uvToNdc.W;
	}

	// gUVToNDC is the only member of the uniform buffer, at its start
	GpuBufferUtility::Write(gpuContext, inputBuffer, 0, sizeof(uvToNdc), &uvToNdc);

	return inputBuffer;
}

struct GpuParticleSimulation::Pimpl
{
	GpuParticleResources Resources;
	GpuParticleHelperBuffers HelperBuffers;
	TShared<GpuBuffer> ParticleVertexInputBuffer;

	/** Index of the first frame that may simulate. */
	u64 NextFrameToSimulate = 0;
};

GpuParticleSimulation::GpuParticleSimulation()
	: m(B3DNew<Pimpl>())
{
	m->ParticleVertexInputBuffer = CreateGpuParticleVertexInputBuffer();
}

GpuParticleSimulation::~GpuParticleSimulation()
{
	B3DDelete(m);
}

void GpuParticleSimulation::Simulate(FrameGraph& graph, const RenderBeastScene& scene, const EvaluatedParticleData* simData, const GpuBufferSuballocation& viewParams,
	const GBufferTextures& gbuffer, const FrameTimings& timings)
{
	if(timings.FrameIndex < m->NextFrameToSimulate)
		return;

	m->NextFrameToSimulate = timings.FrameIndex + 1;
	const float dt = timings.TimeDelta;

	const ParticleSystemObjectStorage& particleStorage = scene.GetParticleSystemStorage();

	GpuParticleResources& resources = m->Resources;
	resources.Swap();
	resources.Import(graph);
	resources.GetCurveTexture().ApplyChanges(graph);

	const Vector<PackedRendererId>& gpuSimulatedSystemIds = particleStorage.GetGpuSimulatedIds();

	Vector<u32> newTiles;
	Vector<GpuParticle> allNewParticles;
	for(PackedRendererId packedId : gpuSimulatedSystemIds)
	{
		const ParticleRenderState& renderState = particleStorage.GetParticleRenderState(packedId);
		GpuParticleSystem* entry = renderState.GpuParticleSystem;
		const ParticleSystemProxy& parentProxy = particleStorage.GetParticleSystemProxy(packedId);

		entry->DetectInactiveTiles(dt);

		bool tilesDirty = false;
		const auto iterFind = simData->GpuData.find(parentProxy.GetId());
		if(iterFind != simData->GpuData.end())
		{
			Vector<GpuParticle>& newParticles = iterFind->second->Particles;
			tilesDirty = entry->AllocateTiles(resources, newParticles, newTiles);

			allNewParticles.insert(allNewParticles.end(), newParticles.begin(), newParticles.end());
		}

		const ParticleSystemSettings& parentSettings = parentProxy.GetSettings();
		float timeStep;
		float newTime = b3d::ParticleSystem::AdvanceTime(entry->GetTime(), dt, parentSettings.Duration, parentSettings.IsLooping, timeStep);
		entry->SetTime(newTime);

		tilesDirty |= entry->FreeInactiveTiles(resources);

		if(tilesDirty)
			entry->UpdateGpuBuffers();
	}

	TArray<TileClearBatch> tileClearBatches = PrepareClearTiles(newTiles);
	TArray<ParticleInjectBatch> particleInjectBatches = PrepareInjectParticles(allNewParticles);

	// Clear the tiles of new particles and inject them into the previous state, which the simulation reads
	if(!tileClearBatches.Empty() || !particleInjectBatches.Empty())
	{
		const GpuParticleStateTextures& injectState = resources.GetPreviousState();

		FrameGraphRasterPass injectPass;
		injectPass.SetColorAttachment(0, injectState.PositionAndTimeTex, FrameGraphAttachmentLoad::Load);
		injectPass.SetColorAttachment(1, injectState.VelocityTex, FrameGraphAttachmentLoad::Load);
		injectPass.SetColorAttachment(2, resources.GetStaticTextures().SizeAndRotationTex, FrameGraphAttachmentLoad::Load);

		GpuParticleClearMaterial* const clearMaterial = GpuParticleClearMaterial::Get();
		for(TileClearBatch& batch : tileClearBatches)
			batch.GpuParameters = clearMaterial->Prepare(injectPass, m->ParticleVertexInputBuffer, batch.TileUVs);

		TShared<GpuParameterSet> injectParameters;
		if(!particleInjectBatches.Empty())
			injectParameters = GpuParticleInjectMaterial::Get()->Prepare(injectPass, m->ParticleVertexInputBuffer);

		graph.AddRasterPass("GpuParticleInject", std::move(injectPass), [this, tileClearBatches = std::move(tileClearBatches), injectParameters,
			particleInjectBatches = std::move(particleInjectBatches)](FrameGraphPassContext& context)
		{
			GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			DrawClearTiles(commandBuffer, tileClearBatches);
			DrawInjectParticles(commandBuffer, injectParameters, particleInjectBatches);
		});
	}

	// Simulate
	// TODO - Run multiple iterations for more stable simulation at lower/erratic framerates

	FrameGraphRasterPass simulatePass;
	for(PackedRendererId packedId : gpuSimulatedSystemIds)
	{
		const ParticleRenderState& renderState = particleStorage.GetParticleRenderState(packedId);
		GpuParticleSystem* entry = renderState.GpuParticleSystem;

		if(entry->GetTileCount() == 0)
			continue;

		const ParticleSystemProxy& parentProxy = particleStorage.GetParticleSystemProxy(packedId);

		// Get vector field texture if any
		TShared<Texture> vectorFieldTexture;

		const render::ParticleGpuSimulationSettings& gpuSimulationSettings = parentProxy.GetGpuSimulationSettings();
		if(gpuSimulationSettings.VectorField.VectorField)
			vectorFieldTexture = gpuSimulationSettings.VectorField.VectorField->GetTexture();

		const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;

		TShared<GpuParameterSet> systemParams = PrepareSimulateParameters(parentProxy, renderState, *entry, dt);
		entry->SetSimulateParameters(systemParams);

		// Populate remaining parameters (textures and other buffers)
		GpuParticleSimulateMaterial::PopulateParameters(systemParams, resources, m->ParticleVertexInputBuffer, viewParams,
			gbuffer.Depth, gbuffer.Normals, entry->GetTileUVs(), renderState.PerObjectSuballocation, vectorFieldTexture, supportsDepthCollisions
		);

		simulatePass.AddParameterSet(systemParams);
	}

	// Group systems by SimType for efficient material binding
	enum class SimType
	{
		Normal,
		DepthCollisionsWorld,
		DepthCollisionsLocal,
		Count
	};

	struct SimulateDraw
	{
		GpuParticleSimulateMaterial* Material;
		TShared<GpuParameterSet> Parameters;
		u32 InstanceCount;
	};

	TArray<SimulateDraw> simulateDraws;
	for(u32 simulationTypeIndex = 0; simulationTypeIndex < (u32)SimType::Count; simulationTypeIndex++)
	{
		const SimType currentType = (SimType)simulationTypeIndex;
		const bool currentDepthCollisions = currentType != SimType::Normal;
		const bool currentLocalSpace = currentType == SimType::DepthCollisionsLocal;

		// Get the material variation for this simulation type
		GpuParticleSimulateMaterial* const simulateMaterial = GpuParticleSimulateMaterial::GetVariation(currentDepthCollisions, currentLocalSpace);

		// Render all systems matching this simulation type
		for(PackedRendererId packedId : gpuSimulatedSystemIds)
		{
			const ParticleRenderState& renderState = particleStorage.GetParticleRenderState(packedId);
			GpuParticleSystem* entry = renderState.GpuParticleSystem;

			if(entry->GetTileCount() == 0)
				continue;

			const ParticleSystemProxy& parentProxy = particleStorage.GetParticleSystemProxy(packedId);
			const render::ParticleGpuSimulationSettings& gpuSimulationSettings = parentProxy.GetGpuSimulationSettings();
			const render::ParticleSystemSettings& settings = parentProxy.GetSettings();

			const bool supportsDepthCollisions = gpuSimulationSettings.DepthCollision.Enabled;
			const bool localSpace = settings.SimulationSpace == ParticleSimulationSpace::Local;

			// Skip systems that don't match this simulation type
			if(supportsDepthCollisions != currentDepthCollisions || (currentDepthCollisions && localSpace != currentLocalSpace))
				continue;

			const u32 tileCount = entry->GetTileCount();
			simulateDraws.Add({ simulateMaterial, entry->GetSimulateParameters(), Math::DivideAndRoundUp(tileCount, GpuParticleConstants::kTilesPerInstance) });
		}
	}

	if(simulateDraws.Empty())
		return;

	const GpuParticleStateTextures& simulateState = resources.GetCurrentState();
	simulatePass.SetColorAttachment(0, simulateState.PositionAndTimeTex, FrameGraphAttachmentLoad::Load);
	simulatePass.SetColorAttachment(1, simulateState.VelocityTex, FrameGraphAttachmentLoad::Load);

	graph.AddRasterPass("GpuParticleSimulate", std::move(simulatePass), [this, simulateDraws = std::move(simulateDraws)](FrameGraphPassContext& context)
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

		TShared<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
		commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
		commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
		commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

		const GpuParticleSimulateMaterial* boundMaterial = nullptr;
		for(const SimulateDraw& draw : simulateDraws)
		{
			if(draw.Material != boundMaterial)
			{
				draw.Material->Bind(commandBuffer, false);
				boundMaterial = draw.Material;
			}

			commandBuffer.SetGpuParameterSet(draw.Parameters);
			commandBuffer.DrawIndexed(0, GpuParticleConstants::kTilesPerInstance * 6, 0, GpuParticleConstants::kTilesPerInstance * 4, draw.InstanceCount);
		}
	});
}

void GpuParticleSimulation::Sort(FrameGraph& graph, const RenderBeastScene& scene, const RendererView& view)
{
	const ParticleSystemObjectStorage& particleStorage = scene.GetParticleSystemStorage();

	const Vector3& viewOrigin = view.GetProperties().ViewOrigin;

	GpuParticleResources& resources = m->Resources;
	resources.Import(graph);

	struct SortedSystem
	{
		const GpuParticleSystem* System;
		u32 SystemIndex;
		Vector3 LocalViewOrigin;
		u32 Offset;
	};

	TArray<SortedSystem> sortedSystems;

	const Vector<PackedRendererId>& gpuSimIds = particleStorage.GetGpuSimulatedIds();

	u32 systemIdx = 0;
	u32 offset = 0;
	for(PackedRendererId packedId : gpuSimIds)
	{
		const ParticleRenderState& renderState = particleStorage.GetParticleRenderState(packedId);
		GpuParticleSystem* entry = renderState.GpuParticleSystem;

		if(entry->GetTileCount() == 0)
		{
			entry->SetSortInfo(false, 0);
			continue;
		}

		const ParticleSystemProxy& parentProxy = particleStorage.GetParticleSystemProxy(packedId);

		const render::ParticleSystemSettings& settings = parentProxy.GetSettings();
		if(settings.SortMode != ParticleSortMode::Distance)
		{
			entry->SetSortInfo(false, 0);
			continue;
		}

		Vector3 localViewOrigin = viewOrigin;
		if(settings.SimulationSpace == ParticleSimulationSpace::Local)
		{
			const Matrix4& worldToLocal = parentProxy.GetWorldTransform().GetInvMatrix();
			localViewOrigin = worldToLocal.MultiplyAffine(viewOrigin);
		}

		entry->SetSortInfo(true, offset);
		sortedSystems.Add({ entry, systemIdx, localViewOrigin, offset });
		offset += entry->GetParticleCount();

		systemIdx++;
	}

	const u32 numSystemsToSort = systemIdx;
	if(numSystemsToSort == 0)
		return;

	// Keys only live through the sort. The sorted indices are read by draws of later frames too.
	GpuBufferCreateInformation sortKeysBufferCreateInformation;
	sortKeysBufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	sortKeysBufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	sortKeysBufferCreateInformation.SimpleStorage.Format = BF_32X1U;
	sortKeysBufferCreateInformation.SimpleStorage.Count = GpuParticleConstants::kTexSize * GpuParticleConstants::kTexSize;

	GpuSortBuffers sortBuffers;
	sortBuffers.Keys[0] = graph.CreateBuffer("GpuParticleSortKeys", sortKeysBufferCreateInformation);
	sortBuffers.Keys[1] = graph.CreateBuffer("GpuParticleSortKeys", sortKeysBufferCreateInformation);
	sortBuffers.Values[0] = resources.mSortedIndices[0];
	sortBuffers.Values[1] = resources.mSortedIndices[1];

	// Each system writes its own range of the sort buffers. Dispatches writing the same buffer need a barrier in between, so each runs in a
	// pass of its own.
	GpuParticleSortPrepareMaterial* prepareMaterial = GpuParticleSortPrepareMaterial::Get();
	const TShared<Texture>& positionAndTime = resources.GetCurrentState().PositionAndTimeTex;
	for(const SortedSystem& sortedSystem : sortedSystems)
	{
		FrameGraphComputePass preparePass;
		const TShared<GpuParameterSet> set = prepareMaterial->Prepare(preparePass, positionAndTime, *sortedSystem.System, sortedSystem.SystemIndex,
			sortedSystem.LocalViewOrigin, sortedSystem.Offset, sortBuffers.Keys[0], sortBuffers.Values[0]);

		const GpuParticleSystem* system = sortedSystem.System;
		graph.AddComputePass("GpuParticleSortPrepare", std::move(preparePass), [prepareMaterial, set, system](FrameGraphPassContext& context)
		{
			prepareMaterial->Execute(context, set, *system);
		});
	}

	const u32 totalNumKeys = offset;
	// Keys hold the system index above the 16 bits of distance. Only the bits the largest index can use need sorting.
	const u32 keyMask = 0xFFFF | ((Bitwise::NextPow2(numSystemsToSort) - 1) << 16);
	resources.mSortedIndicesBufferIdx = GpuSort::Instance().Sort(graph, sortBuffers, totalNumKeys, keyMask);
}

void GpuParticleSimulation::DrawClearTiles(GpuCommandBuffer& commandBuffer, const TArray<TileClearBatch>& batches)
{
	if(batches.Empty())
		return;

	GpuParticleClearMaterial* const clearMaterial = GpuParticleClearMaterial::Get();
	clearMaterial->Bind(commandBuffer, false);

	commandBuffer.SetVertexDescription(m->HelperBuffers.TileVertexDescription);

	TShared<GpuBuffer> buffers[] = { m->HelperBuffers.TileUVs };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const TileClearBatch& batch : batches)
	{
		commandBuffer.SetGpuParameterSet(batch.GpuParameters);
		commandBuffer.DrawIndexed(0, GpuParticleConstants::kTilesPerInstance * 6, 0, GpuParticleConstants::kTilesPerInstance * 4, batch.InstanceCount);
	}
}

void GpuParticleSimulation::DrawInjectParticles(GpuCommandBuffer& commandBuffer, const TShared<GpuParameterSet>& parameters, const TArray<ParticleInjectBatch>& batches)
{
	if(batches.Empty())
		return;

	GpuParticleInjectMaterial::Get()->Bind(commandBuffer, parameters);

	commandBuffer.SetVertexDescription(m->HelperBuffers.InjectVertexDescription);
	commandBuffer.SetIndexBuffer(m->HelperBuffers.SpriteIndices);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	for(const ParticleInjectBatch& batch : batches)
	{
		TShared<GpuBuffer> vertexBuffers[] = { batch.Particles, m->HelperBuffers.ParticleUVs };
		commandBuffer.SetVertexBuffers(0, vertexBuffers, (u32)B3DSize(vertexBuffers));
		commandBuffer.DrawIndexed(0, 6, 0, 4, batch.ParticleCount);
	}
}

GpuParticleResources& GpuParticleSimulation::GetResources() const
{
	return m->Resources;
}

TArray<GpuParticleSimulation::TileClearBatch> GpuParticleSimulation::PrepareClearTiles(const Vector<u32>& tiles)
{
	TArray<TileClearBatch> batches;

	const auto tileCount = (u32)tiles.size();
	if(tileCount == 0)
		return batches;

	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	const u32 batchCount = Math::DivideAndRoundUp(tileCount, GpuParticleHelperBuffers::kNumScratchTiles);

	u32 tileStart = 0;
	for(u32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
	{
		static_assert(GpuParticleHelperBuffers::kNumScratchTiles % GpuParticleConstants::kTilesPerInstance == 0,
			"Tile scratch buffer size must be divisible with number of tiles per instance.");

		const u32 tileEnd = std::min(tileCount, tileStart + GpuParticleHelperBuffers::kNumScratchTiles);
		const u32 alignedTileEnd = Math::DivideAndRoundUp(tileEnd, GpuParticleConstants::kTilesPerInstance) * GpuParticleConstants::kTilesPerInstance;

		TileClearBatch batch;
		batch.TileUVs = gpuContext.CreateScratchGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X2F, alignedTileEnd - tileStart, GpuBufferFlag::StoreOnCPUWithGPUAccess));
		batch.InstanceCount = (alignedTileEnd - tileStart) / GpuParticleConstants::kTilesPerInstance;

		GpuBufferMappedScope mapping = batch.TileUVs->Map(GpuMapOption::Write);
		auto* const tileUVs = (Vector2*)mapping.GetMappedMemory();

		for(u32 tileIndex = tileStart; tileIndex < tileEnd; ++tileIndex)
			tileUVs[tileIndex - tileStart] = GpuParticleResources::GetTileCoords(tiles[tileIndex]);

		// Fill remaining slots with out-of-bounds values
		for(u32 tileIndex = tileEnd; tileIndex < alignedTileEnd; ++tileIndex)
			tileUVs[tileIndex - tileStart] = Vector2(2.0f, 2.0f);

		batches.Add(batch);
		tileStart = alignedTileEnd;
	}

	return batches;
}

TArray<GpuParticleSimulation::ParticleInjectBatch> GpuParticleSimulation::PrepareInjectParticles(const Vector<GpuParticle>& particles)
{
	TArray<ParticleInjectBatch> batches;

	const auto particleCount = (u32)particles.size();
	if(particleCount == 0)
		return batches;

	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	const u32 vertexStride = m->HelperBuffers.InjectVertexDescription->GetVertexStride(0);
	const u32 batchCount = Math::DivideAndRoundUp(particleCount, GpuParticleHelperBuffers::kNumScratchParticles);

	u32 particleStart = 0;
	for(u32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
	{
		const u32 particleEnd = std::min(particleCount, particleStart + GpuParticleHelperBuffers::kNumScratchParticles);

		ParticleInjectBatch batch;
		batch.ParticleCount = particleEnd - particleStart;
		batch.Particles = gpuContext.CreateScratchGpuBuffer(GpuBufferCreateInformation::CreateVertex(vertexStride, batch.ParticleCount, GpuBufferFlag::StoreOnCPUWithGPUAccess));

		GpuBufferMappedScope mapping = batch.Particles->Map(GpuMapOption::Write);
		auto* const particleData = (GpuParticleVertex*)mapping.GetMappedMemory();

		for(u32 particleIndex = particleStart; particleIndex < particleEnd; ++particleIndex)
			particleData[particleIndex - particleStart] = particles[particleIndex].GetVertex();

		batches.Add(batch);
		particleStart = particleEnd;
	}

	return batches;
}

struct GpuParticleCurveInject
{
	Color Color;
	Vector2 DataUv;
};

GpuParticleCurves::GpuParticleCurves()
{
	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Name = "GPU Particles Curves";
	textureCreateInformation.Format = PF_RGBA16F;
	textureCreateInformation.Width = kTexSize;
	textureCreateInformation.Height = kTexSize;
	textureCreateInformation.Usage = TextureUsageFlag::RenderTarget;

	mCurveTexture = gpuDevice->CreateTexture(textureCreateInformation);

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

	GpuBufferUtility::Write(gpuContext, mInjectUV, 0, mInjectUV->GetTotalSize(), tileUVData);
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

	GpuBufferUtility::Write(gpuContext, mInjectIndices, 0, mInjectIndices->GetTotalSize(), indices);
	B3DStackFree(indices);
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

void GpuParticleCurves::ApplyChanges(FrameGraph& graph)
{
	const auto numCurves = (u32)mPendingAllocations.size();
	if(numCurves == 0)
		return;

	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	const GpuBufferCreateInformation scratchBufferCreateInformation = GpuBufferCreateInformation::CreateVertex(mInjectVertexDescription->GetVertexStride(0),
		kScratchNumVertices, GpuBufferFlag::StoreOnCPUWithGPUAccess);

	struct InjectBatch
	{
		TShared<GpuBuffer> ScratchBuffer;
		u32 VertexCount;
	};

	// The draws run after every batch is written, so each batch has a scratch buffer of its own
	TArray<InjectBatch> batches;

	u32 curveIdx = 0;
	while(curveIdx < numCurves)
	{
		const TShared<GpuBuffer> scratchBuffer = gpuContext.CreateScratchGpuBuffer(scratchBufferCreateInformation);

		GpuBufferMappedScope mapping = scratchBuffer->Map(GpuMapOption::Write);
		auto* const data = (GpuParticleCurveInject*)mapping.GetMappedMemory();

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

		batches.Add({ scratchBuffer, count });
	}

	for(auto& entry : mPendingAllocations)
		mPendingAllocator.Free(entry.Pixels);

	mPendingAllocations.clear();
	mPendingAllocator.Clear();

	GpuParticleCurveInjectMaterial* injectMaterial = GpuParticleCurveInjectMaterial::Get();

	graph.ImportTexture(mCurveTexture);

	FrameGraphRasterPass injectPass;
	injectPass.SetColorAttachment(0, mCurveTexture, FrameGraphAttachmentLoad::Load);
	const TShared<GpuParameterSet> set = injectMaterial->Prepare(injectPass, CreateGpuParticleVertexInputBuffer());

	graph.AddRasterPass("GpuParticleCurves", std::move(injectPass), [this, injectMaterial, set, batches = std::move(batches)](FrameGraphPassContext& context)
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		injectMaterial->Bind(commandBuffer, set);

		commandBuffer.SetVertexDescription(mInjectVertexDescription);
		commandBuffer.SetIndexBuffer(mInjectIndices);
		commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

		for(const InjectBatch& batch : batches)
		{
			TShared<GpuBuffer> buffers[] = { batch.ScratchBuffer, mInjectUV };
			commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
			commandBuffer.DrawIndexed(0, 6, 0, 4, batch.VertexCount);
		}
	});
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
