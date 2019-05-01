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
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsCounter", mLightsCounterParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gReflectionProbes", mProbesBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesCounter", mProbesCounterParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 1;
		desc.format = BF_32X1U;
		desc.usage = GBU_LOADSTORE;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;

		mLightsCounter = GpuBuffer::create(desc);
		mLightsCounterParam.set(mLightsCounter);

		mProbesCounter = GpuBuffer::create(desc);
		mProbesCounterParam.set(mProbesCounter);
	}

	void LightGridLLCreationMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE", THREADGROUP_SIZE);
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
			desc.format = BF_32X1U;
			desc.usage = GBU_LOADSTORE;
			desc.type = GBT_STANDARD;
			desc.elementSize = 0;

			mLightsLLHeads = GpuBuffer::create(desc);
			mLightsLLHeadsParam.set(mLightsLLHeads);

			mProbesLLHeads = GpuBuffer::create(desc);
			mProbesLLHeadsParam.set(mProbesLLHeads);

			desc.format = BF_32X4U;
			desc.elementCount = numCells * MAX_LIGHTS_PER_CELL;

			mLightsLL = GpuBuffer::create(desc);
			mLightsLLParam.set(mLightsLL);

			desc.format = BF_32X2U;
			mProbesLL = GpuBuffer::create(desc);
			mProbesLLParam.set(mProbesLL);

			mBufferNumCells = numCells;
		}

		ClearLoadStoreMat* clearMat = ClearLoadStoreMat::getVariation(
			ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Int, 1
		);

		clearMat->execute(mLightsCounter);
		clearMat->execute(mProbesCounter);

		UINT32 clearValue = 0xFFFFFFFF;
		Color clearColor;
		clearColor.r = *(float*) &clearValue;
		clearColor.g = *(float*) &clearValue;
		clearColor.b = *(float*) &clearValue;
		clearColor.a = *(float*) &clearValue;

		clearMat->execute(mLightsLLHeads, clearColor);
		clearMat->execute(mProbesLLHeads, clearColor);

		mParams->setParamBlockBuffer("GridParams", gridParams);
		mLightBufferParam.set(lightsBuffer);
		mProbesBufferParam.set(probesBuffer);
	}

	void LightGridLLCreationMat::execute(const RendererView& view)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		bind();
		RenderAPI::instance().dispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
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
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridDataCounter", mGridDataCounterParam);

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightOffsetAndSize", mGridLightOffsetAndSizeParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightIndices", mGridLightIndicesParam);

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeOffsetAndSize", mGridProbeOffsetAndSizeParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeIndices", mGridProbeIndicesParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 2;
		desc.format = BF_32X1U;
		desc.usage = GBU_LOADSTORE;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;

		mGridDataCounter = GpuBuffer::create(desc);
		mGridDataCounterParam.set(mGridDataCounter);
	}

	void LightGridLLReductionMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE", THREADGROUP_SIZE);
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
			mGridLightOffsetAndSizeParam.set(mGridLightOffsetAndSize);

			desc.format = BF_32X2U;

			mGridProbeOffsetAndSize = GpuBuffer::create(desc);
			mGridProbeOffsetAndSizeParam.set(mGridProbeOffsetAndSize);

			desc.format = BF_32X1U;
			desc.elementCount = numCells * MAX_LIGHTS_PER_CELL;
			mGridLightIndices = GpuBuffer::create(desc);
			mGridLightIndicesParam.set(mGridLightIndices);

			mGridProbeIndices = GpuBuffer::create(desc);
			mGridProbeIndicesParam.set(mGridProbeIndices);

			mBufferNumCells = numCells;
		}

		ClearLoadStoreMat* clearMat = ClearLoadStoreMat::getVariation(
			ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Int, 1
		);
		clearMat->execute(mGridDataCounter);

		mParams->setParamBlockBuffer("GridParams", gridParams);

		mLightsLLHeadsParam.set(lightsLLHeads);
		mLightsLLParam.set(lightsLL);

		mProbesLLHeadsParam.set(probeLLHeads);
		mProbesLLParam.set(probeLL);
	}

	void LightGridLLReductionMat::execute(const RendererView& view)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		bind();
		RenderAPI::instance().dispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
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
		mGridParamBuffer = gLightGridParamDefDef.createBuffer();
	}

	void LightGrid::updateGrid(const RendererView& view, const VisibleLightData& lightData, const VisibleReflProbeData& probeData,
		bool noLighting)
	{
		const RendererViewProperties& viewProps = view.getProperties();

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
			lightCount[0] = lightData.getNumLights(LightType::Directional);
			lightCount[1] = lightData.getNumLights(LightType::Radial);
			lightCount[2] = lightData.getNumLights(LightType::Spot);
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

		gLightGridParamDefDef.gLightCounts.set(mGridParamBuffer, lightCount);
		gLightGridParamDefDef.gLightStrides.set(mGridParamBuffer, lightStrides);
		gLightGridParamDefDef.gNumReflProbes.set(mGridParamBuffer, probeData.getNumProbes());
		gLightGridParamDefDef.gNumCells.set(mGridParamBuffer, numCells);
		gLightGridParamDefDef.gGridSize.set(mGridParamBuffer, gridSize);
		gLightGridParamDefDef.gMaxNumLightsPerCell.set(mGridParamBuffer, MAX_LIGHTS_PER_CELL);
		gLightGridParamDefDef.gGridPixelSize.set(mGridParamBuffer, Vector2I(CELL_XY_SIZE, CELL_XY_SIZE));

		LightGridLLCreationMat* creationMat = LightGridLLCreationMat::get();
		creationMat->setParams(gridSize, mGridParamBuffer, lightData.getLightBuffer(), probeData.getProbeBuffer());
		creationMat->execute(view);

		SPtr<GpuBuffer> lightLLHeads;
		SPtr<GpuBuffer> lightLL;
		SPtr<GpuBuffer> probeLLHeads;
		SPtr<GpuBuffer> probeLL;
		creationMat->getOutputs(lightLLHeads, lightLL, probeLLHeads, probeLL);

		LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::get();
		reductionMat->setParams(gridSize, mGridParamBuffer, lightLLHeads, lightLL, probeLLHeads, probeLL);
		reductionMat->execute(view);
	}

	LightGridOutputs LightGrid::getOutputs() const
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
