//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLightGrid.h"
#include "Material/B3DGpuParamsSet.h"
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

LightGridParamDef gLightGridParamDefDef;

void LightGridLLCreationMat::Initialize()
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

void LightGridLLCreationMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

void LightGridLLCreationMat::SetParams(GpuCommandBuffer& commandBuffer, const Vector3I& gridSize, const SPtr<GpuBuffer>& gridParams, const SPtr<GpuBuffer>& lightsBuffer, const SPtr<GpuBuffer>& probesBuffer)
{
	mGridSize = gridSize;
	u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

	if(numCells > mBufferNumCells || mBufferNumCells == 0)
	{
		GpuBufferCreateInformation bufferCreateInformation;
		bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
		bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		bufferCreateInformation.StructuredStorage.ElementSize = 4;
		bufferCreateInformation.StructuredStorage.Count = numCells;

		mLightsLLHeads = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mLightsLLHeadsParam.Set(mLightsLLHeads);

		mProbesLLHeads = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mProbesLLHeadsParam.Set(mProbesLLHeads);

		bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		bufferCreateInformation.SimpleStorage.Format = BF_32X4U;
		bufferCreateInformation.SimpleStorage.Count = numCells * kMaxLightsPerCell;

		mLightsLL = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mLightsLLParam.Set(mLightsLL);

		bufferCreateInformation.SimpleStorage.Format = BF_32X2U;
		mProbesLL = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mProbesLLParam.Set(mProbesLL);

		mBufferNumCells = numCells;
	}

	ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(
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

	// Ensure we can safely write to the light & probe LL head buffers
	commandBuffer.IssueBarrier(GpuResourceUseFlag::Shader, GpuAccessFlag::Write, GpuResourceUseFlag::Shader, GpuAccessFlag::Write);

	mGPUParameters->SetUniformBuffer("GridParams", gridParams);
	mLightBufferParam.Set(lightsBuffer);
	mProbesBufferParam.Set(probesBuffer);
}

void LightGridLLCreationMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGPUParameters->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	u32 numGroupsX = (mGridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (mGridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (mGridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void LightGridLLCreationMat::GetOutputs(SPtr<GpuBuffer>& lightsLLHeads, SPtr<GpuBuffer>& lightsLL, SPtr<GpuBuffer>& probesLLHeads, SPtr<GpuBuffer>& probesLL) const
{
	lightsLLHeads = mLightsLLHeads;
	lightsLL = mLightsLL;
	probesLLHeads = mProbesLLHeads;
	probesLL = mProbesLL;
}

void LightGridLLReductionMat::Initialize()
{
	mBufferNumCells = 0;

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
	mGridDataCounterParam.Set(mGridDataCounter);
}

void LightGridLLReductionMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

void LightGridLLReductionMat::SetParams(GpuCommandBuffer& commandBuffer, const Vector3I& gridSize, const SPtr<GpuBuffer>& gridParams, const SPtr<GpuBuffer>& lightsLLHeads, const SPtr<GpuBuffer>& lightsLL, const SPtr<GpuBuffer>& probeLLHeads, const SPtr<GpuBuffer>& probeLL)
{
	mGridSize = gridSize;
	u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

	if(numCells > mBufferNumCells || mBufferNumCells == 0)
	{
		GpuBufferCreateInformation bufferCreateInformation;
		bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		bufferCreateInformation.SimpleStorage.Count = numCells;
		bufferCreateInformation.SimpleStorage.Format = BF_32X4U;

		mGridLightOffsetAndSize = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridLightOffsetAndSizeParam.Set(mGridLightOffsetAndSize);

		bufferCreateInformation.SimpleStorage.Format = BF_32X2U;

		mGridProbeOffsetAndSize = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridProbeOffsetAndSizeParam.Set(mGridProbeOffsetAndSize);

		bufferCreateInformation.SimpleStorage.Format = BF_32X1U;
		bufferCreateInformation.SimpleStorage.Count = numCells * kMaxLightsPerCell;
		mGridLightIndices = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridLightIndicesParam.Set(mGridLightIndices);

		mGridProbeIndices = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
		mGridProbeIndicesParam.Set(mGridProbeIndices);

		mBufferNumCells = numCells;
	}

	ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);
	clearMat->Execute(commandBuffer, mGridDataCounter);

	// Ensure we can safely write to the grid data counter buffer
	commandBuffer.IssueBarrier(GpuResourceUseFlag::Shader, GpuAccessFlag::Write, GpuResourceUseFlag::Shader, GpuAccessFlag::Write);

	mGPUParameters->SetUniformBuffer("GridParams", gridParams);

	mLightsLLHeadsParam.Set(lightsLLHeads);
	mLightsLLParam.Set(lightsLL);

	mProbesLLHeadsParam.Set(probeLLHeads);
	mProbesLLParam.Set(probeLL);
}

void LightGridLLReductionMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGPUParameters->TrySetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	u32 numGroupsX = (mGridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (mGridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (mGridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void LightGridLLReductionMat::GetOutputs(SPtr<GpuBuffer>& gridLightOffsetsAndSize, SPtr<GpuBuffer>& gridLightIndices, SPtr<GpuBuffer>& gridProbeOffsetsAndSize, SPtr<GpuBuffer>& gridProbeIndices) const
{
	gridLightOffsetsAndSize = mGridLightOffsetAndSize;
	gridLightIndices = mGridLightIndices;
	gridProbeOffsetsAndSize = mGridProbeOffsetAndSize;
	gridProbeIndices = mGridProbeIndices;
}

LightGrid::LightGrid()
{
	mGridParamBuffer = gLightGridParamDefDef.CreateBuffer();
}

void LightGrid::UpdateGrid(GpuCommandBuffer& commandBuffer, const RendererView& view, const VisibleLightData& lightData, const VisibleReflProbeData& probeData, bool noLighting)
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
		lightCount[0] = lightData.GetNumLights(LightType::Directional);
		lightCount[1] = lightData.GetNumLights(LightType::Radial);
		lightCount[2] = lightData.GetNumLights(LightType::Spot);
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

	gLightGridParamDefDef.gLightCounts.Set(mGridParamBuffer, lightCount);
	gLightGridParamDefDef.gLightStrides.Set(mGridParamBuffer, lightStrides);
	gLightGridParamDefDef.gNumReflProbes.Set(mGridParamBuffer, probeData.GetNumProbes());
	gLightGridParamDefDef.gNumCells.Set(mGridParamBuffer, numCells);
	gLightGridParamDefDef.gGridSize.Set(mGridParamBuffer, gridSize);
	gLightGridParamDefDef.gMaxNumLightsPerCell.Set(mGridParamBuffer, kMaxLightsPerCell);
	gLightGridParamDefDef.gGridPixelSize.Set(mGridParamBuffer, Vector2I(kCellXySize, kCellXySize));

	LightGridLLCreationMat* creationMat = LightGridLLCreationMat::Get();
	creationMat->SetParams(commandBuffer, gridSize, mGridParamBuffer, lightData.GetLightBuffer(), probeData.GetProbeBuffer());
	creationMat->Execute(commandBuffer, view);

	SPtr<GpuBuffer> lightLLHeads;
	SPtr<GpuBuffer> lightLL;
	SPtr<GpuBuffer> probeLLHeads;
	SPtr<GpuBuffer> probeLL;
	creationMat->GetOutputs(lightLLHeads, lightLL, probeLLHeads, probeLL);

	LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::Get();
	reductionMat->SetParams(commandBuffer, gridSize, mGridParamBuffer, lightLLHeads, lightLL, probeLLHeads, probeLL);
	reductionMat->Execute(commandBuffer, view);
}

LightGridOutputs LightGrid::GetOutputs() const
{
	LightGridOutputs outputs;

	LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::Get();
	reductionMat->GetOutputs(
		outputs.GridLightOffsetsAndSize,
		outputs.GridLightIndices,
		outputs.GridProbeOffsetsAndSize,
		outputs.GridProbeIndices);

	outputs.GridParams = mGridParamBuffer;

	return outputs;
}
}} // namespace b3d::render
