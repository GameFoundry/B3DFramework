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

namespace bs
{
	namespace ct
	{
		static const u32 CELL_XY_SIZE = 64;
		static const u32 NUM_Z_SUBDIVIDES = 32;
		static const u32 MAX_LIGHTS_PER_CELL = 32;
		static const u32 THREADGROUP_SIZE = 4;

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
			desc.ElementCount = 1;
			desc.Format = BF_UNKNOWN;
			desc.Usage = GBU_LOADSTORE;
			desc.Type = GBT_STRUCTURED;
			desc.ElementSize = 4;

			mLightsCounter = GpuBuffer::Create(desc);
			mLightsCounterParam.Set(mLightsCounter);

			mProbesCounter = GpuBuffer::Create(desc);
			mProbesCounterParam.Set(mProbesCounter);
		}

		void LightGridLLCreationMat::InitDefinesInternal(ShaderDefines& defines)
		{
			defines.Set("THREADGROUP_SIZE", THREADGROUP_SIZE);
		}

		void LightGridLLCreationMat::SetParams(const Vector3I& gridSize, const SPtr<GpuParamBlockBuffer>& gridParams, const SPtr<GpuBuffer>& lightsBuffer, const SPtr<GpuBuffer>& probesBuffer)
		{
			mGridSize = gridSize;
			u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

			if(numCells > mBufferNumCells || mBufferNumCells == 0)
			{
				GPU_BUFFER_DESC desc;
				desc.ElementCount = numCells;
				desc.Format = BF_UNKNOWN;
				desc.Usage = GBU_LOADSTORE;
				desc.Type = GBT_STRUCTURED;
				desc.ElementSize = 4;

				mLightsLLHeads = GpuBuffer::Create(desc);
				mLightsLLHeadsParam.Set(mLightsLLHeads);

				mProbesLLHeads = GpuBuffer::Create(desc);
				mProbesLLHeadsParam.Set(mProbesLLHeads);

				desc.Type = GBT_STANDARD;
				desc.Format = BF_32X4U;
				desc.ElementCount = numCells * MAX_LIGHTS_PER_CELL;
				desc.ElementSize = 0;

				mLightsLL = GpuBuffer::Create(desc);
				mLightsLLParam.Set(mLightsLL);

				desc.Format = BF_32X2U;
				mProbesLL = GpuBuffer::Create(desc);
				mProbesLLParam.Set(mProbesLL);

				mBufferNumCells = numCells;
			}

			ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(
				ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);

			clearMat->Execute(mLightsCounter);
			clearMat->Execute(mProbesCounter);

			u32 clearValue = 0xFFFFFFFF;
			Color clearColor;
			clearColor.R = *(float*)&clearValue;
			clearColor.G = *(float*)&clearValue;
			clearColor.B = *(float*)&clearValue;
			clearColor.A = *(float*)&clearValue;

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

			u32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
			u32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
			u32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

			Bind();
			RenderAPI::Instance().DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
		}

		void LightGridLLCreationMat::GetOutputs(SPtr<GpuBuffer>& lightsLLHeads, SPtr<GpuBuffer>& lightsLL, SPtr<GpuBuffer>& probesLLHeads, SPtr<GpuBuffer>& probesLL) const
		{
			lightsLLHeads = mLightsLLHeads;
			lightsLL = mLightsLL;
			probesLLHeads = mProbesLLHeads;
			probesLL = mProbesLL;
		}

		LightGridLLReductionMat::LightGridLLReductionMat()
			: mBufferNumCells(0)
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
			desc.ElementCount = 2;
			desc.Format = BF_UNKNOWN;
			desc.Usage = GBU_LOADSTORE;
			desc.Type = GBT_STRUCTURED;
			desc.ElementSize = 4;

			mGridDataCounter = GpuBuffer::Create(desc);
			mGridDataCounterParam.Set(mGridDataCounter);
		}

		void LightGridLLReductionMat::InitDefinesInternal(ShaderDefines& defines)
		{
			defines.Set("THREADGROUP_SIZE", THREADGROUP_SIZE);
		}

		void LightGridLLReductionMat::SetParams(const Vector3I& gridSize, const SPtr<GpuParamBlockBuffer>& gridParams, const SPtr<GpuBuffer>& lightsLLHeads, const SPtr<GpuBuffer>& lightsLL, const SPtr<GpuBuffer>& probeLLHeads, const SPtr<GpuBuffer>& probeLL)
		{
			mGridSize = gridSize;
			u32 numCells = gridSize[0] * gridSize[1] * gridSize[2];

			if(numCells > mBufferNumCells || mBufferNumCells == 0)
			{
				GPU_BUFFER_DESC desc;
				desc.ElementCount = numCells;
				desc.Format = BF_32X4U;
				desc.Usage = GBU_LOADSTORE;
				desc.Type = GBT_STANDARD;
				desc.ElementSize = 0;

				mGridLightOffsetAndSize = GpuBuffer::Create(desc);
				mGridLightOffsetAndSizeParam.Set(mGridLightOffsetAndSize);

				desc.Format = BF_32X2U;

				mGridProbeOffsetAndSize = GpuBuffer::Create(desc);
				mGridProbeOffsetAndSizeParam.Set(mGridProbeOffsetAndSize);

				desc.Format = BF_32X1U;
				desc.ElementCount = numCells * MAX_LIGHTS_PER_CELL;
				mGridLightIndices = GpuBuffer::Create(desc);
				mGridLightIndicesParam.Set(mGridLightIndices);

				mGridProbeIndices = GpuBuffer::Create(desc);
				mGridProbeIndicesParam.Set(mGridProbeIndices);

				mBufferNumCells = numCells;
			}

			ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(
				ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int, 1);
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

			u32 numGroupsX = (mGridSize[0] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
			u32 numGroupsY = (mGridSize[1] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;
			u32 numGroupsZ = (mGridSize[2] + THREADGROUP_SIZE - 1) / THREADGROUP_SIZE;

			Bind();
			RenderAPI::Instance().DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
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

		void LightGrid::UpdateGrid(const RendererView& view, const VisibleLightData& lightData, const VisibleReflProbeData& probeData, bool noLighting)
		{
			const RendererViewProperties& viewProps = view.GetProperties();

			u32 width = viewProps.Target.ViewRect.Width;
			u32 height = viewProps.Target.ViewRect.Height;

			Vector3I gridSize;
			gridSize[0] = (width + CELL_XY_SIZE - 1) / CELL_XY_SIZE;
			gridSize[1] = (height + CELL_XY_SIZE - 1) / CELL_XY_SIZE;
			gridSize[2] = NUM_Z_SUBDIVIDES;

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
			gLightGridParamDefDef.gMaxNumLightsPerCell.Set(mGridParamBuffer, MAX_LIGHTS_PER_CELL);
			gLightGridParamDefDef.gGridPixelSize.Set(mGridParamBuffer, Vector2I(CELL_XY_SIZE, CELL_XY_SIZE));

			LightGridLLCreationMat* creationMat = LightGridLLCreationMat::Get();
			creationMat->SetParams(gridSize, mGridParamBuffer, lightData.GetLightBuffer(), probeData.GetProbeBuffer());
			creationMat->Execute(view);

			SPtr<GpuBuffer> lightLLHeads;
			SPtr<GpuBuffer> lightLL;
			SPtr<GpuBuffer> probeLLHeads;
			SPtr<GpuBuffer> probeLL;
			creationMat->GetOutputs(lightLLHeads, lightLL, probeLLHeads, probeLL);

			LightGridLLReductionMat* reductionMat = LightGridLLReductionMat::Get();
			reductionMat->SetParams(gridSize, mGridParamBuffer, lightLLHeads, lightLL, probeLLHeads, probeLL);
			reductionMat->Execute(view);
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
	} // namespace ct
} // namespace bs
