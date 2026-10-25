//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuParticleSimulationMaterials.h"
#include "B3DGpuParticleSimulation.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Renderer/B3DRenderer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Particles/B3DVectorField.h"
#include "Math/B3DVector3.h"
#include "B3DRenderBeast.h"
#include "B3DGpuParticleSimulationMaterials.h"

using namespace b3d;
using namespace b3d::render;

void GpuParticleClearMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", GpuParticleConstants::kTilesPerInstance);
}

TShared<render::GpuParameterSet> GpuParticleClearMaterial::Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& vertexInputBuffer, const TShared<GpuBuffer>& tileUVs)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
	set->SetUniformBuffer("Input", vertexInputBuffer);
	set->SetStorageBuffer("gTileUVs", tileUVs);

	return set;
}

TShared<render::GpuParameterSet> GpuParticleInjectMaterial::Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& vertexInputBuffer)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
	set->SetUniformBuffer("Input", vertexInputBuffer);

	return set;
}

TShared<render::GpuParameterSet> GpuParticleCurveInjectMaterial::Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& vertexInputBuffer)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
	set->SetUniformBuffer("Input", vertexInputBuffer);

	return set;
}

void GpuParticleSimulateMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", GpuParticleConstants::kTilesPerInstance);
}

void GpuParticleSimulateMaterial::PopulateParameters(const TShared<GpuParameterSet>& gpuParameters, GpuParticleResources& resources, const TShared<GpuBuffer>& particleVertexInputBuffer,
	const GpuBufferSuballocation& viewParams, const TShared<Texture>& depth, const TShared<Texture>& normals, const TShared<GpuBuffer>& tileUVs,
	const TShared<Texture>& vectorFieldTexture, bool supportsDepthCollisions)
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

		gpuParameters->SetSampledTexture("gSizeRotationTex", staticTextures.SizeAndRotationTex);
		gpuParameters->SetSampledTexture("gCurvesTex", curveTexture.GetTexture());
		gpuParameters->SetSampledTexture("gDepthTex", depth);
		gpuParameters->SetSampledTexture("gNormalsTex", normals);
	}
}

GpuParticleSimulateMaterial* GpuParticleSimulateMaterial::GetVariation(bool depthCollisions, bool localSpace)
{
	if(depthCollisions)
	{
		if(localSpace)
			return Get(GetVariation<2>());

		return Get(GetVariation<1>());
	}

	return Get(GetVariation<0>());
}

void GpuParticleSortPrepareMaterial::Initialize()
{
	mParameterLayout->GetUniformBufferParameter("Input", mInputUniformBufferParameter);
	mParameterLayout->GetStorageBufferParameter("gInputIndices", mInputIndicesParam);
	mParameterLayout->GetStorageBufferParameter("gOutputKeys", mOutputKeysParam);
	mParameterLayout->GetStorageBufferParameter("gOutputIndices", mOutputIndicesParam);
	mParameterLayout->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
}

void GpuParticleSortPrepareMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_THREADS", kNumThreads);
}

namespace
{
	constexpr u32 kSortPrepareMaxNumGroups = 128;
}

TShared<render::GpuParameterSet> GpuParticleSortPrepareMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& positionAndTime, const GpuParticleSystem& system,
	u32 systemIdx, const Vector3& localViewOrigin, u32 offset, const TShared<GpuBuffer>& outKeys, const TShared<GpuBuffer>& outIndices)
{
	B3D_ASSERT(systemIdx < std::pow(2, 16));

	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const u32 numParticles = system.GetParticleCount();

	const u32 numIterations = Math::DivideAndRoundUp(numParticles, kNumThreads);
	const u32 numGroups = std::min(numIterations, kSortPrepareMaxNumGroups);

	const u32 iterationsPerGroup = numIterations / numGroups;
	const u32 extraIterations = numIterations % numGroups;

	GpuBufferMappedScope inputUniforms = gGpuParticleSortPrepareUniformDefinition.AllocateTransient().Map();

	gGpuParticleSortPrepareUniformDefinition.gIterationsPerGroup.Set(inputUniforms, iterationsPerGroup);
	gGpuParticleSortPrepareUniformDefinition.gNumExtraIterations.Set(inputUniforms, extraIterations);
	gGpuParticleSortPrepareUniformDefinition.gNumParticles.Set(inputUniforms, numParticles);
	gGpuParticleSortPrepareUniformDefinition.gOutputOffset.Set(inputUniforms, offset);
	gGpuParticleSortPrepareUniformDefinition.gSystemKey.Set(inputUniforms, systemIdx << 16);
	gGpuParticleSortPrepareUniformDefinition.gLocalViewOrigin.Set(inputUniforms, localViewOrigin);

	mInputUniformBufferParameter.Set(set, inputUniforms);

	mPosAndTimeTexParam.Set(set, positionAndTime);
	mInputIndicesParam.Set(set, system.GetParticleIndices());
	mOutputKeysParam.Set(set, outKeys);
	mOutputIndicesParam.Set(set, outIndices);

	return set;
}

void GpuParticleSortPrepareMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const GpuParticleSystem& system) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

	const u32 numIterations = Math::DivideAndRoundUp(system.GetParticleCount(), kNumThreads);
	const u32 numGroups = std::min(numIterations, kSortPrepareMaxNumGroups);

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(numGroups);
}

