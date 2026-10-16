//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLightGrid.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRendererView.h"
#include "RenderState/B3DLightRenderState.h"
#include "RenderState/B3DReflectionProbeRenderState.h"
#include "B3DTiledDeferred.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "FrameGraph/B3DFrameGraph.h"

namespace b3d {
namespace render {

static const u32 kCellXySize = 64;
static const u32 kNumZSubdivides = 32;
static const u32 kMaxLightsPerCell = 32;
static const u32 kThreadgroupSize = 4;

LightGridUniformDefinition gLightGridUniformDefinition;

void LightGridLLCreationMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("GridParams", mGridParamsParameter);
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);

	mGpuParameterSet->GetStorageBufferParameter("gLights", mLightBufferParam);
	mGpuParameterSet->GetStorageBufferParameter("gLightsCounter", mLightsCounterParam);
	mGpuParameterSet->GetStorageBufferParameter("gLightsLLHeads", mLightsLLHeadsParam);
	mGpuParameterSet->GetStorageBufferParameter("gLightsLL", mLightsLLParam);

	mGpuParameterSet->GetStorageBufferParameter("gReflectionProbes", mProbesBufferParam);
	mGpuParameterSet->GetStorageBufferParameter("gProbesCounter", mProbesCounterParam);
	mGpuParameterSet->GetStorageBufferParameter("gProbesLLHeads", mProbesLLHeadsParam);
	mGpuParameterSet->GetStorageBufferParameter("gProbesLL", mProbesLLParam);
}

void LightGridLLCreationMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

TShared<GpuParameterSet> LightGridLLCreationMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const GpuBufferSuballocation& gridUniformBuffer,
	const TShared<GpuBuffer>& lightsBuffer, const TShared<GpuBuffer>& probesBuffer, const LightGridLinkedLists& linkedLists)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	mGridParamsParameter.Set(set, gridUniformBuffer);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	mLightBufferParam.Set(set, lightsBuffer);
	mLightsCounterParam.Set(set, linkedLists.LightsCounter);
	mLightsLLHeadsParam.Set(set, linkedLists.LightsHeads);
	mLightsLLParam.Set(set, linkedLists.Lights);

	mProbesBufferParam.Set(set, probesBuffer);
	mProbesCounterParam.Set(set, linkedLists.ProbesCounter);
	mProbesLLHeadsParam.Set(set, linkedLists.ProbesHeads);
	mProbesLLParam.Set(set, linkedLists.Probes);

	return set;
}

void LightGridLLCreationMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector3I& gridSize) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	u32 numGroupsX = (gridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (gridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (gridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void LightGridLLReductionMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("GridParams", mGridParamsParameter);
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	mGpuParameterSet->GetStorageBufferParameter("gLightsLLHeads", mLightsLLHeadsParam);
	mGpuParameterSet->GetStorageBufferParameter("gLightsLL", mLightsLLParam);

	mGpuParameterSet->GetStorageBufferParameter("gProbesLLHeads", mProbesLLHeadsParam);
	mGpuParameterSet->GetStorageBufferParameter("gProbesLL", mProbesLLParam);

	mGpuParameterSet->GetStorageBufferParameter("gGridDataCounter", mGridDataCounterParam);

	mGpuParameterSet->GetStorageBufferParameter("gGridLightOffsetAndSize", mGridLightOffsetAndSizeParam);
	mGpuParameterSet->GetStorageBufferParameter("gGridLightIndices", mGridLightIndicesParam);

	mGpuParameterSet->GetStorageBufferParameter("gGridProbeOffsetAndSize", mGridProbeOffsetAndSizeParam);
	mGpuParameterSet->GetStorageBufferParameter("gGridProbeIndices", mGridProbeIndicesParam);
}

void LightGridLLReductionMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE", kThreadgroupSize);
}

TShared<GpuParameterSet> LightGridLLReductionMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const LightGridLinkedLists& linkedLists,
	const TShared<GpuBuffer>& gridDataCounter, const LightGridOutputs& outputs)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	mGridParamsParameter.Set(set, outputs.UniformBuffer);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	mLightsLLHeadsParam.Set(set, linkedLists.LightsHeads);
	mLightsLLParam.Set(set, linkedLists.Lights);
	mProbesLLHeadsParam.Set(set, linkedLists.ProbesHeads);
	mProbesLLParam.Set(set, linkedLists.Probes);

	mGridDataCounterParam.Set(set, gridDataCounter);

	mGridLightOffsetAndSizeParam.Set(set, outputs.GridLightOffsetsAndSize);
	mGridLightIndicesParam.Set(set, outputs.GridLightIndices);
	mGridProbeOffsetAndSizeParam.Set(set, outputs.GridProbeOffsetsAndSize);
	mGridProbeIndicesParam.Set(set, outputs.GridProbeIndices);

	return set;
}

void LightGridLLReductionMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector3I& gridSize) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	u32 numGroupsX = (gridSize[0] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsY = (gridSize[1] + kThreadgroupSize - 1) / kThreadgroupSize;
	u32 numGroupsZ = (gridSize[2] + kThreadgroupSize - 1) / kThreadgroupSize;

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

namespace
{
	/** Creates a transient buffer of @p elementCount elements of @p elementSize bytes, structured by @p elementSize or typed by @p format. */
	TShared<GpuBuffer> CreateGridBuffer(FrameGraph& graph, const char* name, u32 elementCount, u32 elementSize, GpuBufferFormat format = BF_UNKNOWN)
	{
		GpuBufferCreateInformation information;
		information.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		if(format == BF_UNKNOWN)
		{
			information.Type = GpuBufferType::StructuredStorage;
			information.StructuredStorage.ElementSize = elementSize;
			information.StructuredStorage.Count = elementCount;
		}
		else
		{
			information.Type = GpuBufferType::SimpleStorage;
			information.SimpleStorage.Format = format;
			information.SimpleStorage.Count = elementCount;
		}

		return graph.CreateBuffer(name, information);
	}
}

LightGridOutputs AddLightGridPasses(FrameGraph& graph, const RendererView& view, const VisibleLightData& lightData, const VisibleReflectionProbeData& probeData,
	bool noLighting)
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

	LightGridOutputs outputs;
	outputs.UniformBuffer = gLightGridUniformDefinition.AllocateTransient();

	{
		GpuBufferMappedScope lightGridUniforms = outputs.UniformBuffer.Map();
		gLightGridUniformDefinition.gLightCounts.Set(lightGridUniforms, lightCount);
		gLightGridUniformDefinition.gLightStrides.Set(lightGridUniforms, lightStrides);
		gLightGridUniformDefinition.gNumReflProbes.Set(lightGridUniforms, probeData.GetProbeCount());
		gLightGridUniformDefinition.gNumCells.Set(lightGridUniforms, numCells);
		gLightGridUniformDefinition.gGridSize.Set(lightGridUniforms, gridSize);
		gLightGridUniformDefinition.gMaxNumLightsPerCell.Set(lightGridUniforms, kMaxLightsPerCell);
		gLightGridUniformDefinition.gGridPixelSize.Set(lightGridUniforms, Vector2I(kCellXySize, kCellXySize));
	}

	LightGridLinkedLists linkedLists;
	linkedLists.LightsCounter = CreateGridBuffer(graph, "LightGridLightsCounter", 1, 4);
	linkedLists.LightsHeads = CreateGridBuffer(graph, "LightGridLightsHeads", numCells, 4);
	linkedLists.Lights = CreateGridBuffer(graph, "LightGridLights", numCells * kMaxLightsPerCell, 0, BF_32X4U);
	linkedLists.ProbesCounter = CreateGridBuffer(graph, "LightGridProbesCounter", 1, 4);
	linkedLists.ProbesHeads = CreateGridBuffer(graph, "LightGridProbesHeads", numCells, 4);
	linkedLists.Probes = CreateGridBuffer(graph, "LightGridProbes", numCells * kMaxLightsPerCell, 0, BF_32X2U);

	const TShared<GpuBuffer> gridDataCounter = CreateGridBuffer(graph, "LightGridDataCounter", 2, 4);

	outputs.GridLightOffsetsAndSize = CreateGridBuffer(graph, "GridLightOffsetAndSize", numCells, 0, BF_32X4U);
	outputs.GridProbeOffsetsAndSize = CreateGridBuffer(graph, "GridProbeOffsetAndSize", numCells, 0, BF_32X2U);
	outputs.GridLightIndices = CreateGridBuffer(graph, "GridLightIndices", numCells * kMaxLightsPerCell, 0, BF_32X1U);
	outputs.GridProbeIndices = CreateGridBuffer(graph, "GridProbeIndices", numCells * kMaxLightsPerCell, 0, BF_32X1U);

	// Counters start at zero and list heads at the end-of-list marker
	{
		ClearLoadStoreMaterial* clearMaterial = ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);

		u32 endOfList = 0xFFFFFFFF;
		Color endOfListValue;
		endOfListValue.R = *(float*)&endOfList;
		endOfListValue.G = *(float*)&endOfList;
		endOfListValue.B = *(float*)&endOfList;
		endOfListValue.A = *(float*)&endOfList;

		struct Clear
		{
			TShared<GpuParameterSet> Set;
			u32 ElementCount;
		};

		FrameGraphComputePass clearPass;
		const Array<Clear, 5> clears =
		{{
			{ clearMaterial->Prepare(clearPass, linkedLists.LightsCounter), 1 },
			{ clearMaterial->Prepare(clearPass, linkedLists.ProbesCounter), 1 },
			{ clearMaterial->Prepare(clearPass, linkedLists.LightsHeads, endOfListValue), numCells },
			{ clearMaterial->Prepare(clearPass, linkedLists.ProbesHeads, endOfListValue), numCells },
			{ clearMaterial->Prepare(clearPass, gridDataCounter), 2 },
		}};

		graph.AddComputePass("LightGridClear", std::move(clearPass), [clearMaterial, clears](FrameGraphPassContext& context)
		{
			for(const Clear& clear : clears)
				clearMaterial->Execute(context, clear.Set, clear.ElementCount);
		});
	}

	{
		LightGridLLCreationMaterial* creationMaterial = LightGridLLCreationMaterial::Get();

		FrameGraphComputePass creationPass;
		const TShared<GpuParameterSet> set = creationMaterial->Prepare(creationPass, view, outputs.UniformBuffer, lightData.GetLightBuffer(), probeData.GetProbeBuffer(), linkedLists);

		graph.AddComputePass("LightGridCreation", std::move(creationPass), [creationMaterial, set, gridSize](FrameGraphPassContext& context)
		{
			creationMaterial->Execute(context, set, gridSize);
		});
	}

	{
		LightGridLLReductionMaterial* reductionMaterial = LightGridLLReductionMaterial::Get();

		FrameGraphComputePass reductionPass;
		const TShared<GpuParameterSet> set = reductionMaterial->Prepare(reductionPass, view, linkedLists, gridDataCounter, outputs);

		graph.AddComputePass("LightGridReduction", std::move(reductionPass), [reductionMaterial, set, gridSize](FrameGraphPassContext& context)
		{
			reductionMaterial->Execute(context, set, gridSize);
		});
	}

	return outputs;
}
}} // namespace b3d::render
