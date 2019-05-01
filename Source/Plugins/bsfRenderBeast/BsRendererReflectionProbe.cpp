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

	void VisibleReflProbeData::update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
	{
		mReflProbeData.clear();

		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

		// Generate refl. probe data for the visible ones
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();
		for(UINT32 i = 0; i < numProbes; i++)
		{
			if (!visibility.reflProbes[i])
				continue;

			mReflProbeData.push_back(ReflProbeData());
			sceneInfo.reflProbes[i].getParameters(mReflProbeData.back());
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
		bool supportsStructuredBuffers = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsStructuredBuffers)
		{
			UINT32 size = mNumProbes * sizeof(ReflProbeData);
			UINT32 curBufferSize;

			if (mProbeBuffer != nullptr)
				curBufferSize = mProbeBuffer->getSize();
			else
				curBufferSize = 0;

			if (size > curBufferSize || curBufferSize == 0)
			{
				// Allocate at least one block even if no probes, to avoid issues with null buffers
				UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float) REFL_PROBE_BUFFER_INCREMENT)) * REFL_PROBE_BUFFER_INCREMENT;

				GPU_BUFFER_DESC bufferDesc;
				bufferDesc.type = GBT_STRUCTURED;
				bufferDesc.elementCount = bufferSize / sizeof(ReflProbeData);
				bufferDesc.elementSize = sizeof(ReflProbeData);
				bufferDesc.format = BF_UNKNOWN;

				mProbeBuffer = GpuBuffer::create(bufferDesc);
			}

			if (size > 0)
				mProbeBuffer->writeData(0, size, mReflProbeData.data(), BWT_DISCARD);
		}
	}

	RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
		:probe(probe)
	{
		arrayIdx = -1;
		arrayDirty = true;
		errorFlagged = false;
	}

	void RendererReflectionProbe::getParameters(ReflProbeData& output) const
	{
		output.type = probe->getType() == ReflectionProbeType::Sphere ? 0 
			: probe->getType() == ReflectionProbeType::Box ? 1 : 2;
		
		const Transform& tfrm = probe->getTransform();
		output.position = tfrm.getPosition();
		output.boxExtents = probe->getExtents();

		if (probe->getType() == ReflectionProbeType::Sphere)
			output.radius = probe->getRadius();
		else
			output.radius = output.boxExtents.length();

		output.transitionDistance = probe->getTransitionDistance();
		output.cubemapIdx = arrayIdx;
		output.invBoxTransform.setInverseTRS(output.position, tfrm.getRotation(), output.boxExtents);
	}

	void ImageBasedLightingParams::populate(const SPtr<GpuParams>& params, GpuProgramType programType, bool optional, 
		bool gridIndices, bool probeArray)
	{
		// Sky
		if (!optional || params->hasTexture(programType, "gSkyReflectionTex"))
			params->getTextureParam(programType, "gSkyReflectionTex", skyReflectionsTexParam);

		// Reflections
		if (!optional || params->hasTexture(programType, "gReflProbeCubemaps"))
		{
			params->getTextureParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsTexParam);

			if(probeArray)
				params->getBufferParam(programType, "gReflectionProbes", reflectionProbesParam);
		}

		if (!optional || params->hasTexture(programType, "gPreintegratedEnvBRDF"))
			params->getTextureParam(programType, "gPreintegratedEnvBRDF", preintegratedEnvBRDFParam);

		// AO
		if (params->hasTexture(programType, "gAmbientOcclusionTex"))
			params->getTextureParam(programType, "gAmbientOcclusionTex", ambientOcclusionTexParam);

		// SSR
		if (params->hasTexture(programType, "gSSRTex"))
			params->getTextureParam(programType, "gSSRTex", ssrTexParam);

		if(gridIndices)
		{
			if (!optional || params->hasBuffer(programType, "gReflectionProbeIndices"))
				params->getBufferParam(programType, "gReflectionProbeIndices", reflectionProbeIndicesParam);
		}

		params->getParamInfo()->getBinding(
			programType,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"ReflProbeParams",
			reflProbeParamBindings
		);

		params->getParamInfo()->getBinding(
			programType,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"ReflectionProbes",
			reflProbesBinding
		);
	}

	ReflProbeParamBuffer::ReflProbeParamBuffer()
	{
		buffer = gReflProbeParamsParamDef.createBuffer();
	}

	void ReflProbeParamBuffer::populate(const Skybox* sky, UINT32 numProbes, const SPtr<Texture>& reflectionCubemaps, 
		bool capturingReflections)
	{
		float brightness = 1.0f;
		UINT32 skyReflectionsAvailable = 0;
		UINT32 numSkyMips = 0;

		if(sky != nullptr)
		{
			SPtr<Texture> filteredReflections = sky->getFilteredRadiance();
			if (filteredReflections)
			{
				numSkyMips = filteredReflections->getProperties().getNumMipmaps() + 1;
				skyReflectionsAvailable = 1;
			}

			brightness = sky->getBrightness();
		}

		gReflProbeParamsParamDef.gSkyCubemapNumMips.set(buffer, numSkyMips);
		gReflProbeParamsParamDef.gSkyCubemapAvailable.set(buffer, skyReflectionsAvailable);
		gReflProbeParamsParamDef.gNumProbes.set(buffer, numProbes);

		UINT32 numReflProbeMips = 0;
		if (reflectionCubemaps != nullptr)
			numReflProbeMips = reflectionCubemaps->getProperties().getNumMipmaps() + 1;

		gReflProbeParamsParamDef.gReflCubemapNumMips.set(buffer, numReflProbeMips);
		gReflProbeParamsParamDef.gUseReflectionMaps.set(buffer, capturingReflections ? 0 : 1);
		gReflProbeParamsParamDef.gSkyBrightness.set(buffer, brightness);
	}

	ReflProbesParamDef gReflProbesParamDef;
}}
