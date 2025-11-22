//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLightGrid.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRendererView.h"
#include "B3DRendererLight.h"
#include "B3DRendererReflectionProbe.h"
#include "B3DTiledDeferred.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d {
namespace render {

static const u32 kCellXySize = 64;
static const u32 kNumZSubdivides = 32;
static const u32 kMaxLightsPerCell = 32;
static const u32 kThreadgroupSize = 4;

LightGridUniformDefinition gLightGridUniformDefinition;

void LightGridLLCreationMaterial::Initialize()
{
	mGPUParameters->GetStorageBufferParameter("gLights", mLightBufferParam);
	mGPUParameters->GetStorageBufferParameter("gLightsCounter", mLightsCounterParam);
	mGPUParameters->GetStorageBufferParameter("gLightsLLHeads", mLightsLLHeadsParam);
	mGPUParameters->GetStorageBufferParameter("gLightsLL", mLightsLLParam);

	mGPUParameters->GetStorageBufferParameter("gReflectionProbes", mProbesBufferParam);
	mGPUParameters->GetStorageBufferParameter("gProbesCounter", mProbesCounterParam);
	mGPUParameters->GetStorageBufferParameter("gProbesLLHeads", mProbesLLHeadsParam);
	mGPUParameters->GetStorageBufferParameter("gProbesLL", mProbesLLParam);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	bufferCreateInformation.StructuredStorage.ElementSize = 4;
	bufferCreateInformation.StructuredStorage.Count = 1;

	mLightsCounter = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
	mLightsCounterParam.Set(mLightsCounter);

	mProbesCounter = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
	mProbesCounterParam.Set(mProbesCounter);
}

void LightGridLLCreationMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

void LightGridLLCreationMaterial::SetParams(GpuCommandBuffer& commandBuffer, const Vector3I& gridSize, const GpuBufferSuballocation& gridUniformBuffer, const SPtr<GpuBuffer>& lightsBuffer, const SPtr<GpuBuffer>& probesBuffer)
{
	mGridSize = gridSize;
	u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

	if(numCells > mBufferCellCount || mBufferCellCount == 0)
	{
		GpuBufferCreateInformation bufferCreateInformation;
		bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
		bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		bufferCreateInformation.StructuredStorage.ElementSize = 4;
		bufferCreateInformation.StructuredStorage.Count = numCells;

		mLightsLLHeads = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mLightsLLHeads->SetName("LightsLLHeads");

		mLightsLLHeadsParam.Set(mLightsLLHeads);

		mProbesLLHeads = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mProbesLLHeads->SetName("ProbesLLHeads");

		mProbesLLHeadsParam.Set(mProbesLLHeads);

		bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		bufferCreateInformation.SimpleStorage.Format = BF_32X4U;
		bufferCreateInformation.SimpleStorage.Count = numCells * kMaxLightsPerCell;

		mLightsLL = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mLightsLL->SetName("LightsLL");

		mLightsLLParam.Set(mLightsLL);

		bufferCreateInformation.SimpleStorage.Format = BF_32X2U;
		mProbesLL = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mProbesLL->SetName("ProbesLL");

		mProbesLLParam.Set(mProbesLL);

		mBufferCellCount = numCells;
	}

	ClearLoadStoreMaterial* clearMat = ClearLoadStoreMaterial::GetVariation(
		ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);

	clearMat->Execute(commandBuffer, mLightsCounter);
	clearMat->Execute(commandBuffer, mProbesCounter);

	u32 clearValue = 0xFFFFFFFF;
	Color clearColor;
	clearColor.R = *(float*)&clearValue;
	clearColor.G = *(float*)&clearValue;
	clearColor.B = *(float*)&clearValue;
	clearColor.A = *(float*)&clearValue;

	clearMat->Execute(commandBuffer, mLightsLLHeads, clearColor);
	clearMat->Execute(commandBuffer, mProbesLLHeads, clearColor);

	mGPUParameters->SetUniformBuffer("GridParams", gridUniformBuffer);
	mLightBufferParam.Set(lightsBuffer);
	mProbesBufferParam.Set(probesBuffer);
}

void LightGridLLCreationMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGPUParameters->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	u32 numGroupsX = (mGridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (mGridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (mGridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void LightGridLLCreationMaterial::GetOutputs(SPtr<GpuBuffer>& lightsLLHeads, SPtr<GpuBuffer>& lightsLL, SPtr<GpuBuffer>& probesLLHeads, SPtr<GpuBuffer>& probesLL) const
{
	lightsLLHeads = mLightsLLHeads;
	lightsLL = mLightsLL;
	probesLLHeads = mProbesLLHeads;
	probesLL = mProbesLL;
}

void LightGridLLReductionMaterial::Initialize()
{
	mBufferCellCount = 0;

	mGPUParameters->GetStorageBufferParameter("gLightsLLHeads", mLightsLLHeadsParam);
	mGPUParameters->GetStorageBufferParameter("gLightsLL", mLightsLLParam);

	mGPUParameters->GetStorageBufferParameter("gProbesLLHeads", mProbesLLHeadsParam);
	mGPUParameters->GetStorageBufferParameter("gProbesLL", mProbesLLParam);

	mGPUParameters->GetStorageBufferParameter("gGridDataCounter", mGridDataCounterParam);

	mGPUParameters->GetStorageBufferParameter("gGridLightOffsetAndSize", mGridLightOffsetAndSizeParam);
	mGPUParameters->GetStorageBufferParameter("gGridLightIndices", mGridLightIndicesParam);

	mGPUParameters->GetStorageBufferParameter("gGridProbeOffsetAndSize", mGridProbeOffsetAndSizeParam);
	mGPUParameters->GetStorageBufferParameter("gGridProbeIndices", mGridProbeIndicesParam);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	bufferCreateInformation.StructuredStorage.Count = 2;
	bufferCreateInformation.StructuredStorage.ElementSize = 4;

	mGridDataCounter = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
	mGridDataCounter->SetName("GridDataCounter");

	mGridDataCounterParam.Set(mGridDataCounter);
}

void LightGridLLReductionMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

void LightGridLLReductionMaterial::SetParams(GpuCommandBuffer& commandBuffer, const Vector3I& gridSize, const GpuBufferSuballocation& gridUniformBuffer, const SPtr<GpuBuffer>& lightsLLHeads, const SPtr<GpuBuffer>& lightsLL, const SPtr<GpuBuffer>& probeLLHeads, const SPtr<GpuBuffer>& probeLL)
{
	mGridSize = gridSize;
	u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

	if(numCells > mBufferCellCount || mBufferCellCount == 0)
	{
		GpuBufferCreateInformation bufferCreateInformation;
		bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		bufferCreateInformation.SimpleStorage.Count = numCells;
		bufferCreateInformation.SimpleStorage.Format = BF_32X4U;

		mGridLightOffsetAndSize = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridLightOffsetAndSize->SetName("GridLightOffsetAndSize");

		mGridLightOffsetAndSizeParam.Set(mGridLightOffsetAndSize);

		bufferCreateInformation.SimpleStorage.Format = BF_32X2U;

		mGridProbeOffsetAndSize = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridProbeOffsetAndSize->SetName("GridProbeOffsetAndSize");

		mGridProbeOffsetAndSizeParam.Set(mGridProbeOffsetAndSize);

		bufferCreateInformation.SimpleStorage.Format = BF_32X1U;
		bufferCreateInformation.SimpleStorage.Count = numCells * kMaxLightsPerCell;
		mGridLightIndices = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridLightIndices->SetName("GridLightIndices");

		mGridLightIndicesParam.Set(mGridLightIndices);

		mGridProbeIndices = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridProbeIndices->SetName("GridProbeIndices");

		mGridProbeIndicesParam.Set(mGridProbeIndices);

		mBufferCellCount = numCells;
	}

	ClearLoadStoreMaterial* clearMat = ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);
	clearMat->Execute(commandBuffer, mGridDataCounter);

	mGPUParameters->SetUniformBuffer("GridParams", gridUniformBuffer);

	mLightsLLHeadsParam.Set(lightsLLHeads);
	mLightsLLParam.Set(lightsLL);

	mProbesLLHeadsParam.Set(probeLLHeads);
	mProbesLLParam.Set(probeLL);
}

void LightGridLLReductionMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGPUParameters->TrySetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	u32 numGroupsX = (mGridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (mGridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (mGridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void LightGridLLReductionMaterial::GetOutputs(SPtr<GpuBuffer>& gridLightOffsetsAndSize, SPtr<GpuBuffer>& gridLightIndices, SPtr<GpuBuffer>& gridProbeOffsetsAndSize, SPtr<GpuBuffer>& gridProbeIndices) const
{
	gridLightOffsetsAndSize = mGridLightOffsetAndSize;
	gridLightIndices = mGridLightIndices;
	gridProbeOffsetsAndSize = mGridProbeOffsetAndSize;
	gridProbeIndices = mGridProbeIndices;
}

void LightGrid::UpdateGrid(GpuCommandBuffer& commandBuffer, const RendererView& view, const VisibleLightData& lightData, const VisibleReflectionProbeData& probeData, bool noLighting)
{
	const RendererViewProperties& viewProps = view.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	Vector3I gridSize;
	gridSize[0] = (width + kCellXySize - 1) / kCellXySize;
	gridSize[1] = (height + kCellXySize - 1) / kCellXySize;
	gridSize[2] = kNumZSubdivides;

	Vector4I lightCount;
	Vector2I lightStrides;
	if(!noLighting)
	{
		lightCount[0] = lightData.GetLightCount(LightType::Directional);
		lightCount[1] = lightData.GetLightCount(LightType::Radial);
		lightCount[2] = lightData.GetLightCount(LightType::Spot);
		lightCount[3] = lightCount[0] + lightCount[1] + lightCount[2];

		lightStrides[0] = lightCount[0];
		lightStrides[1] = lightStrides[0] + lightCount[1];
	}
	else
	{
		lightCount[0] = 0;
		lightCount[1] = 0;
		lightCount[2] = 0;
		lightCount[3] = 0;

		lightStrides[0] = 0;
		lightStrides[1] = 0;
	}

	u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

	mGridUniformBuffer = gLightGridUniformDefinition.AllocateTransient();
	gLightGridUniformDefinition.gLightCounts.Set(mGridUniformBuffer, lightCount);
	gLightGridUniformDefinition.gLightStrides.Set(mGridUniformBuffer, lightStrides);
	gLightGridUniformDefinition.gNumReflProbes.Set(mGridUniformBuffer, probeData.GetProbeCount());
	gLightGridUniformDefinition.gNumCells.Set(mGridUniformBuffer, numCells);
	gLightGridUniformDefinition.gGridSize.Set(mGridUniformBuffer, gridSize);
	gLightGridUniformDefinition.gMaxNumLightsPerCell.Set(mGridUniformBuffer, kMaxLightsPerCell);
	gLightGridUniformDefinition.gGridPixelSize.Set(mGridUniformBuffer, Vector2I(kCellXySize, kCellXySize));

	LightGridLLCreationMaterial* creationMat = LightGridLLCreationMaterial::Get();
	creationMat->SetParams(commandBuffer, gridSize, mGridUniformBuffer, lightData.GetLightBuffer(), probeData.GetProbeBuffer());
	creationMat->Execute(commandBuffer, view);

	SPtr<GpuBuffer> lightLLHeads;
	SPtr<GpuBuffer> lightLL;
	SPtr<GpuBuffer> probeLLHeads;
	SPtr<GpuBuffer> probeLL;
	creationMat->GetOutputs(lightLLHeads, lightLL, probeLLHeads, probeLL);

	LightGridLLReductionMaterial* reductionMat = LightGridLLReductionMaterial::Get();
	reductionMat->SetParams(commandBuffer, gridSize, mGridUniformBuffer, lightLLHeads, lightLL, probeLLHeads, probeLL);
	reductionMat->Execute(commandBuffer, view);
}

LightGridOutputs LightGrid::GetOutputs() const
{
	LightGridOutputs outputs;

	LightGridLLReductionMaterial* reductionMat = LightGridLLReductionMaterial::Get();
	reductionMat->GetOutputs(
		outputs.GridLightOffsetsAndSize,
		outputs.GridLightIndices,
		outputs.GridProbeOffsetsAndSize,
		outputs.GridProbeIndices);

	outputs.UniformBuffer = mGridUniformBuffer;

	return outputs;
}
}} // namespace b3d::render
