//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightGrid.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsRendererUtility.h"
#include "BsRendererView.h"
#include "BsRenderTargets.h"
#include "BsLightRendering.h"
#include "BsImageBasedLighting.h"

namespace bs { namespace ct
{
	static const UINT32 CELL_XY_SIZE = 64;
	static const UINT32 NUM_Z_SUBDIVIDES = 32;
	static const UINT32 MAX_LIGHTS_PER_CELL = 32;
	static const UINT32 THREADGROUP_SIZE = 4;

	LightGridParamDef gLightGridParamDefDef;

	LightGridLLCreationMat::LightGridLLCreationMat()
		:mBufferNumCells(0)
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsCounter", mLightsCounterParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gReflectionProbes", mProbesBufferParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesCounter", mProbesCounterParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 1;
		desc.format = BF_32X1U;
		desc.randomGpuWrite = true;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;

		mLightsCounter = GpuBuffer::create(desc);
		mLightsCounterParam.set(mLightsCounter);

		mProbesCounter = GpuBuffer::create(desc);
		mProbesCounterParam.set(mProbesCounter);
	}

	void LightGridLLCreationMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("THREADGROUP_SIZE", THREADGROUP_SIZE)
		});

		variations.add(variation);
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
			desc.randomGpuWrite = true;
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

		UINT32 zero = 0;
		mLightsCounter->writeData(0, sizeof(UINT32), &zero, BWT_DISCARD);
		mProbesCounter->writeData(0, sizeof(UINT32), &zero, BWT_DISCARD);

		// Note: Add a method to clear buffer data directly? e.g. GpuBuffer->clear(value);
		UINT32* headsClearData = (UINT32*)bs_stack_alloc(mLightsLLHeads->getSize());
		memset(headsClearData, 0xFFFFFFFF, mLightsLLHeads->getSize());

		mLightsLLHeads->writeData(0, mLightsLLHeads->getSize(), headsClearData, BWT_DISCARD);
		bs_stack_free(headsClearData);

		headsClearData = (UINT32*)bs_stack_alloc(mProbesLLHeads->getSize());
		memset(headsClearData, 0xFFFFFFFF, mProbesLLHeads->getSize());

		mProbesLLHeads->writeData(0, mProbesLLHeads->getSize(), headsClearData, BWT_DISCARD);
		bs_stack_free(headsClearData);

		mParamsSet->setParamBlockBuffer("GridParams", gridParams, true);
		mLightBufferParam.set(lightsBuffer);
		mProbesBufferParam.set(probesBuffer);
	}

	void LightGridLLCreationMat::execute(const RendererView& view)
	{
		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

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
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLLHeads", mLightsLLHeadsParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLightsLL", mLightsLLParam);

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLLHeads", mProbesLLHeadsParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gProbesLL", mProbesLLParam);

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridDataCounter", mGridDataCounterParam);

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightOffsetAndSize", mGridLightOffsetAndSizeParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridLightIndices", mGridLightIndicesParam);

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeOffsetAndSize", mGridProbeOffsetAndSizeParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gGridProbeIndices", mGridProbeIndicesParam);

		GPU_BUFFER_DESC desc;
		desc.elementCount = 2;
		desc.format = BF_32X1U;
		desc.randomGpuWrite = true;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;

		mGridDataCounter = GpuBuffer::create(desc);
		mGridDataCounterParam.set(mGridDataCounter);
	}

	void LightGridLLReductionMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("THREADGROUP_SIZE", THREADGROUP_SIZE)
		});

		variations.add(variation);
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
			desc.randomGpuWrite = true;
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

		// Note: Add a method to clear buffer data directly? e.g. GpuBuffer->clear(value);
		UINT32 zeros[] = { 0, 0 };
		mGridDataCounter->writeData(0, sizeof(UINT32) * 2, zeros, BWT_DISCARD);

		mParamsSet->setParamBlockBuffer("GridParams", gridParams, true);

		mLightsLLHeadsParam.set(lightsLLHeads);
		mLightsLLParam.set(lightsLL);

		mProbesLLHeadsParam.set(probeLLHeads);
		mProbesLLParam.set(probeLL);
	}

	void LightGridLLReductionMat::execute(const RendererView& view)
	{
		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

		UINT32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
		UINT32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

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

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

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

	void LightGrid::getOutputs(SPtr<GpuBuffer>& gridLightOffsetsAndSize, SPtr<GpuBuffer>& gridLightIndices,
		SPtr<GpuBuffer>& gridProbeOffsetsAndSize, SPtr<GpuBuffer>& gridProbeIndices, 
		SPtr<GpuParamBlockBuffer>& gridParams) const
	{
		LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::get();
		reductionMat->getOutputs(gridLightOffsetsAndSize, gridLightIndices, gridProbeOffsetsAndSize, gridProbeIndices);
		gridParams = mGridParamBuffer;
	}
}}
