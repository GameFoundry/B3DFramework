//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsLightGrid.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRendererView.h"
#include "BsRendererLight.h"
#include "BsRendererReflectionProbe.h"
#include "BsTiledDeferred.h"

namespace bs { namespace ct
{
	static const UINT32 CELL_XY_SIZE = 64;
	static const UINT32 NUM_Z_SUBDIVIDES = 32;
	static const UINT32 MAX_LIGHTS_PER_CELL = 32;
	static const UINT32 THREADGROUP_SIZE = 4;

	LightGridParamDef gLightGridParamDefDef;

	LightGridLLCreationMat::LightGridLLCreationMat()
	{
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLightsCounter", mLightsCounterParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gReflectionProbes", mProbesBufferParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gProbesCounter", mProbesCounterParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 1;
		desc.format = BF_UNKNOWN;
		desc.usage = GBU_LOADSTORE;
		desc.type = GBT_STRUCTURED;
		desc.elementSize = 4;

		mLightsCounter = GpuBuffer::create(desc);
		mLightsCounterParam.Set(mLightsCounter);

		mProbesCounter = GpuBuffer::create(desc);
		mProbesCounterParam.Set(mProbesCounter);
	}

	void LightGridLLCreationMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("THREADGROUP_SIZE", THREADGROUP_SIZE);
	}

	void LightGridLLCreationMat::setParams(const Vector3I& gridSize, const SPtr<GpuParamBlockBuffer>& gridParams,
		const SPtr<GpuBuffer>& lightsBuffer, const SPtr<GpuBuffer>& probesBuffer)
	{
		mGridSize = gridSize;
		UINT32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

		if(numCells > mBufferNumCells || mBufferNumCells == 0)
		{
			GPU_BUFFER_DESC desc;
			desc.elementCount = numCells;
			desc.format = BF_UNKNOWN;
			desc.usage = GBU_LOADSTORE;
			desc.type = GBT_STRUCTURED;
			desc.elementSize = 4;

			mLightsLLHeads = GpuBuffer::create(desc);
			mLightsLLHeadsParam.Set(mLightsLLHeads);

			mProbesLLHeads = GpuBuffer::create(desc);
			mProbesLLHeadsParam.Set(mProbesLLHeads);

			desc.type = GBT_STANDARD;
			desc.format = BF_32X4U;
			desc.elementCount = numCells * MAX_LIGHTS_PER_CELL;
			desc.elementSize = 0;

			mLightsLL = GpuBuffer::create(desc);
			mLightsLLParam.Set(mLightsLL);

			desc.format = BF_32X2U;
			mProbesLL = GpuBuffer::create(desc);
			mProbesLLParam.Set(mProbesLL);

			mBufferNumCells = numCells;
		}

		ClearLoadStoreMat* clearMat = ClearLoadStoreMat::getVariation(
			ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1
		);

		clearMat->Execute(mLightsCounter);
		clearMat->Execute(mProbesCounter);

		UINT32 clearValue = 0xFFFFFFFF;
		Color clearColor;
		clearColor.r = *(float*) &clearValue;
		clearColor.g = *(float*) &clearValue;
		clearColor.b = *(float*) &clearValue;
		clearColor.a = *(float*) &clearValue;

		clearMat->Execute(mLightsLLHeads, clearColor);
		clearMat->Execute(mProbesLLHeads, clearColor);

		mParams->SetParamBlockBuffer("GridParams", gridParams);
		mLightBufferParam.Set(lightsBuffer);
		mProbesBufferParam.Set(probesBuffer);
	}

	void LightGridLLCreationMat::Execute(const RendererView& view)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->SetParamBlockBuffer("PerCamera", view.GetPerViewBuffer());

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		bind();
		RenderAPI::instance().DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	}

	void LightGridLLCreationMat::getOutputs(SPtr<GpuBuffer>& lightsLLHeads, SPtr<GpuBuffer>& lightsLL,
		SPtr<GpuBuffer>& probesLLHeads, SPtr<GpuBuffer>& probesLL) const
	{
		lightsLLHeads = mLightsLLHeads;
		lightsLL = mLightsLL;
		probesLLHeads = mProbesLLHeads;
		probesLL = mProbesLL;
	}

	LightGridLLReductionMat::LightGridLLReductionMat()
		:mBufferNumCells(0)
	{
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gGridDataCounter", mGridDataCounterParam);

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightOffsetAndSize", mGridLightOffsetAndSizeParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightIndices", mGridLightIndicesParam);

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeOffsetAndSize", mGridProbeOffsetAndSizeParam);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeIndices", mGridProbeIndicesParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 2;
		desc.format = BF_UNKNOWN;
		desc.usage = GBU_LOADSTORE;
		desc.type = GBT_STRUCTURED;
		desc.elementSize = 4;

		mGridDataCounter = GpuBuffer::create(desc);
		mGridDataCounterParam.Set(mGridDataCounter);
	}

	void LightGridLLReductionMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("THREADGROUP_SIZE", THREADGROUP_SIZE);
	}

	void LightGridLLReductionMat::setParams(const Vector3I& gridSize, const SPtr<GpuParamBlockBuffer>& gridParams,
		const SPtr<GpuBuffer>& lightsLLHeads, const SPtr<GpuBuffer>& lightsLL,
		const SPtr<GpuBuffer>& probeLLHeads, const SPtr<GpuBuffer>& probeLL)
	{
		mGridSize = gridSize;
		UINT32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

		if (numCells > mBufferNumCells || mBufferNumCells == 0)
		{
			GPU_BUFFER_DESC desc;
			desc.elementCount = numCells;
			desc.format = BF_32X4U;
			desc.usage = GBU_LOADSTORE;
			desc.type = GBT_STANDARD;
			desc.elementSize = 0;

			mGridLightOffsetAndSize = GpuBuffer::create(desc);
			mGridLightOffsetAndSizeParam.Set(mGridLightOffsetAndSize);

			desc.format = BF_32X2U;

			mGridProbeOffsetAndSize = GpuBuffer::create(desc);
			mGridProbeOffsetAndSizeParam.Set(mGridProbeOffsetAndSize);

			desc.format = BF_32X1U;
			desc.elementCount = numCells * MAX_LIGHTS_PER_CELL;
			mGridLightIndices = GpuBuffer::create(desc);
			mGridLightIndicesParam.Set(mGridLightIndices);

			mGridProbeIndices = GpuBuffer::create(desc);
			mGridProbeIndicesParam.Set(mGridProbeIndices);

			mBufferNumCells = numCells;
		}

		ClearLoadStoreMat* clearMat = ClearLoadStoreMat::getVariation(
			ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1
		);
		clearMat->Execute(mGridDataCounter);

		mParams->SetParamBlockBuffer("GridParams", gridParams);

		mLightsLLHeadsParam.Set(lightsLLHeads);
		mLightsLLParam.Set(lightsLL);

		mProbesLLHeadsParam.Set(probeLLHeads);
		mProbesLLParam.Set(probeLL);
	}

	void LightGridLLReductionMat::Execute(const RendererView& view)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->SetParamBlockBuffer("PerCamera", view.GetPerViewBuffer());

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		bind();
		RenderAPI::instance().DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	}

	void LightGridLLReductionMat::getOutputs(SPtr<GpuBuffer>& gridLightOffsetsAndSize, SPtr<GpuBuffer>& gridLightIndices,
		SPtr<GpuBuffer>& gridProbeOffsetsAndSize, SPtr<GpuBuffer>& gridProbeIndices) const
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

	void LightGrid::updateGrid(const RendererView& view, const VisibleLightData& lightData, const VisibleReflProbeData& probeData,
		bool noLighting)
	{
		const RendererViewProperties& viewProps = view.GetProperties();

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		Vector3I gridSize;
		gridSize[0] = (width + CELL_XY_SIZE - 1) / CELL_XY_SIZE;
		gridSize[1] = (height + CELL_XY_SIZE - 1) / CELL_XY_SIZE;
		gridSize[2] = NUM_Z_SUBDIVIDES;

		Vector4I lightCount;
		Vector2I lightStrides;
		if (!noLighting)
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

		UINT32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

		gLightGridParamDefDef.gLightCounts.Set(mGridParamBuffer, lightCount);
		gLightGridParamDefDef.gLightStrides.Set(mGridParamBuffer, lightStrides);
		gLightGridParamDefDef.gNumReflProbes.Set(mGridParamBuffer, probeData.getNumProbes());
		gLightGridParamDefDef.gNumCells.Set(mGridParamBuffer, numCells);
		gLightGridParamDefDef.gGridSize.Set(mGridParamBuffer, gridSize);
		gLightGridParamDefDef.gMaxNumLightsPerCell.Set(mGridParamBuffer, MAX_LIGHTS_PER_CELL);
		gLightGridParamDefDef.gGridPixelSize.Set(mGridParamBuffer, Vector2I(CELL_XY_SIZE, CELL_XY_SIZE));

		LightGridLLCreationMat* creationMat = LightGridLLCreationMat::get();
		creationMat->SetParams(gridSize, mGridParamBuffer, lightData.GetLightBuffer(), probeData.getProbeBuffer());
		creationMat->Execute(view);

		SPtr<GpuBuffer> lightLLHeads;
		SPtr<GpuBuffer> lightLL;
		SPtr<GpuBuffer> probeLLHeads;
		SPtr<GpuBuffer> probeLL;
		creationMat->GetOutputs(lightLLHeads, lightLL, probeLLHeads, probeLL);

		LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::get();
		reductionMat->SetParams(gridSize, mGridParamBuffer, lightLLHeads, lightLL, probeLLHeads, probeLL);
		reductionMat->Execute(view);
	}

	LightGridOutputs LightGrid::GetOutputs() const
	{
		LightGridOutputs outputs;

		LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::get();
		reductionMat->getOutputs(
			outputs.gridLightOffsetsAndSize,
			outputs.gridLightIndices,
			outputs.gridProbeOffsetsAndSize,
			outputs.gridProbeIndices
		);

		outputs.gridParams = mGridParamBuffer;

		return outputs;
	}
}}
