//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererReflectionProbe.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Renderer/BsReflectionProbe.h"
#include "BsRenderBeast.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"

namespace bs { namespace ct
{
	static const UINT32 REFL_PROBE_BUFFER_INCREMENT = 16 * sizeof(ReflProbeData);
	
	ReflProbeParamsParamDef gReflProbeParamsParamDef;

	void VisibleReflProbeData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
	{
		mReflProbeData.clear();

		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		// Generate refl. probe data for the visible ones
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();
		for(UINT32 i = 0; i < numProbes; i++)
		{
			if (!visibility.reflProbes[i])
				continue;

			mReflProbeData.push_back(ReflProbeData());
			sceneInfo.reflProbes[i].GetParameters(mReflProbeData.back());
		}

		// Sort probes so bigger ones get accessed first, this way we overlay smaller ones on top of biggers ones when
		// rendering
		auto sorter = [](const ReflProbeData& lhs, const ReflProbeData& rhs)
		{
			return rhs.radius < lhs.radius;
		};

		std::sort(mReflProbeData.begin(), mReflProbeData.end(), sorter);

		mNumProbes = (UINT32)mReflProbeData.size();

		// Move refl. probe data into a GPU buffer
		bool supportsStructuredBuffers = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsStructuredBuffers)
		{
			UINT32 size = mNumProbes * sizeof(ReflProbeData);
			UINT32 curBufferSize;

			if (mProbeBuffer != nullptr)
				curBufferSize = mProbeBuffer->GetSize();
			else
				curBufferSize = 0;

			if (size > curBufferSize || curBufferSize == 0)
			{
				// Allocate at least one block even if no probes, to avoid issues with null buffers
				UINT32 bufferSize = std::max(1, Math::CeilToInt(size / (float) REFL_PROBE_BUFFER_INCREMENT)) * REFL_PROBE_BUFFER_INCREMENT;

				GPU_BUFFER_DESC bufferDesc;
				bufferDesc.type = GBT_STRUCTURED;
				bufferDesc.elementCount = bufferSize / sizeof(ReflProbeData);
				bufferDesc.elementSize = sizeof(ReflProbeData);
				bufferDesc.format = BF_UNKNOWN;

				mProbeBuffer = GpuBuffer::Create(bufferDesc);
			}

			if (size > 0)
				mProbeBuffer->WriteData(0, size, mReflProbeData.data(), BWT_DISCARD);
		}
	}

	RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
		:probe(probe)
	{
		arrayIdx = -1;
		arrayDirty = true;
		errorFlagged = false;
	}

	void RendererReflectionProbe::GetParameters(ReflProbeData& output) const
	{
		output.type = probe->GetType() == ReflectionProbeType::Sphere ? 0
			: probe->GetType() == ReflectionProbeType::Box ? 1 : 2;
		
		const Transform& tfrm = probe->GetTransform();
		output.position = tfrm.GetPosition();
		output.boxExtents = probe->GetExtents();

		if (probe->GetType() == ReflectionProbeType::Sphere)
			output.radius = probe->GetRadius();
		else
			output.radius = output.boxExtents.Length();

		output.transitionDistance = probe->GetTransitionDistance();
		output.cubemapIdx = arrayIdx;
		output.invBoxTransform.SetInverseTrs(output.position, tfrm.GetRotation(), output.boxExtents);
	}

	void ImageBasedLightingParams::Populate(const SPtr<GpuParams>& params, GpuProgramType programType, bool optional,
		bool gridIndices, bool probeArray)
	{
		// Sky
		if (!optional || params->HasTexture(programType, "gSkyReflectionTex"))
			params->GetTextureParam(programType, "gSkyReflectionTex", skyReflectionsTexParam);

		// Reflections
		if (!optional || params->HasTexture(programType, "gReflProbeCubemaps"))
		{
			params->GetTextureParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsTexParam);

			if(probeArray)
				params->GetBufferParam(programType, "gReflectionProbes", reflectionProbesParam);
		}

		if (!optional || params->HasTexture(programType, "gPreintegratedEnvBRDF"))
			params->GetTextureParam(programType, "gPreintegratedEnvBRDF", preintegratedEnvBRDFParam);

		// AO
		if (params->HasTexture(programType, "gAmbientOcclusionTex"))
			params->GetTextureParam(programType, "gAmbientOcclusionTex", ambientOcclusionTexParam);

		// SSR
		if (params->HasTexture(programType, "gSSRTex"))
			params->GetTextureParam(programType, "gSSRTex", ssrTexParam);

		if(gridIndices)
		{
			if (!optional || params->HasBuffer(programType, "gReflectionProbeIndices"))
				params->GetBufferParam(programType, "gReflectionProbeIndices", reflectionProbeIndicesParam);
		}

		params->GetParamInfo()->GetBinding(
			programType,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"ReflProbeParams",
			reflProbeParamBindings
		);

		params->GetParamInfo()->GetBinding(
			programType,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"ReflectionProbes",
			reflProbesBinding
		);
	}

	ReflProbeParamBuffer::ReflProbeParamBuffer()
	{
		buffer = gReflProbeParamsParamDef.CreateBuffer();
	}

	void ReflProbeParamBuffer::Populate(const Skybox* sky, UINT32 numProbes, const SPtr<Texture>& reflectionCubemaps,
		bool capturingReflections)
	{
		float brightness = 1.0f;
		UINT32 skyReflectionsAvailable = 0;
		UINT32 numSkyMips = 0;

		if(sky != nullptr)
		{
			SPtr<Texture> filteredReflections = sky->GetFilteredRadiance();
			if (filteredReflections)
			{
				numSkyMips = filteredReflections->GetProperties().GetNumMipmaps() + 1;
				skyReflectionsAvailable = 1;
			}

			brightness = sky->GetBrightness();
		}

		gReflProbeParamsParamDef.gSkyCubemapNumMips.Set(buffer, numSkyMips);
		gReflProbeParamsParamDef.gSkyCubemapAvailable.Set(buffer, skyReflectionsAvailable);
		gReflProbeParamsParamDef.gNumProbes.Set(buffer, numProbes);

		UINT32 numReflProbeMips = 0;
		if (reflectionCubemaps != nullptr)
			numReflProbeMips = reflectionCubemaps->GetProperties().GetNumMipmaps() + 1;

		gReflProbeParamsParamDef.gReflCubemapNumMips.Set(buffer, numReflProbeMips);
		gReflProbeParamsParamDef.gUseReflectionMaps.Set(buffer, capturingReflections ? 0 : 1);
		gReflProbeParamsParamDef.gSkyBrightness.Set(buffer, brightness);
	}

	ReflProbesParamDef gReflProbesParamDef;
}}
