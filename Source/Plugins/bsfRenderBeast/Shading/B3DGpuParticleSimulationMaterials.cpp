//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuParticleSimulationMaterials.h"
#include "B3DGpuParticleSimulation.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
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

void GpuParticleClearMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer, const SPtr<GpuBuffer>& tileUVs)
{
	gpuParameters->SetUniformBuffer("Input", vertexInputBuffer);
	gpuParameters->SetStorageBuffer("gTileUVs", tileUVs);
}

void GpuParticleInjectMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const SPtr<GpuBuffer>& vertexInputBuffer)
{
	gpuParameters->SetUniformBuffer("Input", vertexInputBuffer);
}

void GpuParticleCurveInjectMaterial::Prepare(const SPtr<GpuBuffer>& vertexInputBuffer)
{
	mGpuParameterSet->SetUniformBuffer("Input", vertexInputBuffer);
}

void GpuParticleSimulateMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILES_PER_INSTANCE", GpuParticleConstants::kTilesPerInstance);
}

void GpuParticleSimulateMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, GpuParticleResources& resources, const SPtr<GpuBuffer>& particleVertexInputBuffer,
	const GpuBufferSuballocation& viewParams, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<GpuBuffer>& tileUVs,
	const GpuBufferSuballocation& perObjectParams, const SPtr<Texture>& vectorFieldTexture, bool supportsDepthCollisions)
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

void GpuParticleBoundsMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mInputUniformBufferParameter);
	mGpuParameterSet->GetStorageBufferParameter("gParticleIndices", mParticleIndicesParam);
	mGpuParameterSet->GetStorageBufferParameter("gOutput", mOutputParam);
	mGpuParameterSet->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
}

void GpuParticleBoundsMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_THREADS", kNumThreads);
}

void GpuParticleBoundsMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime)
{
	mPosAndTimeTexParam.Set(positionAndTime);

	RendererMaterial::Bind(commandBuffer);
}

AABox GpuParticleBoundsMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& indices, u32 numParticles)
{
	static constexpr u32 kMaxNumGroups = 128;

	const u32 numIterations = Math::DivideAndRoundUp(numParticles, kNumThreads);
	const u32 numGroups = std::min(numIterations, kMaxNumGroups);

	const u32 iterationsPerGroup = numIterations / numGroups;
	const u32 extraIterations = numIterations % numGroups;

	GpuBufferMappedScope inputUniforms = gGpuParticleBoundsUniformDefinition.AllocateTransient().Map();

	gGpuParticleBoundsUniformDefinition.gIterationsPerGroup.Set(inputUniforms, iterationsPerGroup);
	gGpuParticleBoundsUniformDefinition.gNumExtraIterations.Set(inputUniforms, extraIterations);
	gGpuParticleBoundsUniformDefinition.gNumParticles.Set(inputUniforms, numParticles);

	mInputUniformBufferParameter.Set(inputUniforms);

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
	GpuBufferUtility::Read(output, 0, output->GetTotalSize(), data);

	for(u32 i = 0; i < numGroups; i++)
	{
		min = Vector3::Min(min, data[i * 2 + 0]);
		max = Vector3::Min(max, data[i * 2 + 1]);
	}

	B3DStackFree(data);

	return AABox(min, max);
}

void GpuParticleSortPrepareMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mInputUniformBufferParameter);
	mGpuParameterSet->GetStorageBufferParameter("gInputIndices", mInputIndicesParam);
	mGpuParameterSet->GetStorageBufferParameter("gOutputKeys", mOutputKeysParam);
	mGpuParameterSet->GetStorageBufferParameter("gOutputIndices", mOutputIndicesParam);
	mGpuParameterSet->GetSampledTextureParameter("gPosAndTimeTex", mPosAndTimeTexParam);
}

void GpuParticleSortPrepareMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_THREADS", kNumThreads);
}

void GpuParticleSortPrepareMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& positionAndTime)
{
	mPosAndTimeTexParam.Set(positionAndTime);

	RendererMaterial::Bind(commandBuffer, false);
}

u32 GpuParticleSortPrepareMaterial::Execute(GpuCommandBuffer& commandBuffer, const GpuParticleSystem& system, u32 systemIdx, const Vector3& localViewOrigin, u32 offset, const SPtr<GpuBuffer>& outKeys, const SPtr<GpuBuffer>& outIndices)
{
	static constexpr u32 kMaxNumGroups = 128;

	B3D_ASSERT(systemIdx < std::pow(2, 16));

	const u32 numParticles = system.GetTileCount() * GpuParticleConstants::kParticlesPerTile;

	const u32 numIterations = Math::DivideAndRoundUp(numParticles, kNumThreads);
	const u32 numGroups = std::min(numIterations, kMaxNumGroups);

	const u32 iterationsPerGroup = numIterations / numGroups;
	const u32 extraIterations = numIterations % numGroups;

	GpuBufferMappedScope inputUniforms = gGpuParticleSortPrepareUniformDefinition.AllocateTransient().Map();

	gGpuParticleSortPrepareUniformDefinition.gIterationsPerGroup.Set(inputUniforms, iterationsPerGroup);
	gGpuParticleSortPrepareUniformDefinition.gNumExtraIterations.Set(inputUniforms, extraIterations);
	gGpuParticleSortPrepareUniformDefinition.gNumParticles.Set(inputUniforms, numParticles);
	gGpuParticleSortPrepareUniformDefinition.gOutputOffset.Set(inputUniforms, offset);
	gGpuParticleSortPrepareUniformDefinition.gSystemKey.Set(inputUniforms, systemIdx << 16);
	gGpuParticleSortPrepareUniformDefinition.gLocalViewOrigin.Set(inputUniforms, localViewOrigin);

	mInputUniformBufferParameter.Set(inputUniforms);

	mInputIndicesParam.Set(system.GetParticleIndices());
	mOutputKeysParam.Set(outKeys);
	mOutputIndicesParam.Set(outIndices);

	BindParameters(commandBuffer);
	commandBuffer.DispatchCompute(numGroups);
	return numParticles;
}

