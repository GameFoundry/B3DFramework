//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsImageBasedLighting.h"
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsReflectionProbe.h"
#include "Material/BsGpuParamsSet.h"
#include "BsRenderBeast.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"

namespace bs { namespace ct
{
	static const UINT32 BUFFER_INCREMENT = 16 * sizeof(ReflProbeData);

	ReflProbeParamsParamDef gReflProbeParamsParamDef;
	TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

	VisibleReflProbeData::VisibleReflProbeData()
		:mNumProbes(0)
	{ }

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
		UINT32 size = mNumProbes * sizeof(ReflProbeData);
		UINT32 curBufferSize;

		if (mProbeBuffer != nullptr)
			curBufferSize = mProbeBuffer->getSize();
		else
			curBufferSize = 0;

		if (size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no probes, to avoid issues with null buffers
			UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float)BUFFER_INCREMENT)) * BUFFER_INCREMENT;

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

	// Note: Using larger tiles than in tiled deferred lighting since we use AABB for intersections, which is more
	// expensive to compute than frustums. This way we amortize the cost even though other parts of the shader might suffer
	// due to increased thread group load.
	const UINT32 TiledDeferredImageBasedLightingMat::TILE_SIZE = 32;

	TiledDeferredImageBasedLightingMat::TiledDeferredImageBasedLightingMat()
	{
		mSampleCount = mVariation.getUInt("MSAA_COUNT");

		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		if (mSampleCount > 1)
		{
			mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);
			mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);
		}
		else
			mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);

		mImageBasedParams.populate(mParams, GPT_COMPUTE_PROGRAM, false, false, true);

		mParams->setParamBlockBuffer("ReflProbeParams", mReflProbeParamBuffer.buffer);
	}

	void TiledDeferredImageBasedLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredImageBasedLightingMat::execute(const RendererView& view, const SceneInfo& sceneInfo, 
		const VisibleReflProbeData& probeData, const Inputs& inputs)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledImageBasedLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		mReflProbeParamBuffer.populate(sceneInfo.skybox, probeData.getNumProbes(), sceneInfo.reflProbeCubemapsTex, 
			viewProps.capturingReflections);

		mParamBuffer->flushToGPU();
		mReflProbeParamBuffer.buffer->flushToGPU();

		mGBufferA.set(inputs.gbuffer.albedo);
		mGBufferB.set(inputs.gbuffer.normals);
		mGBufferC.set(inputs.gbuffer.roughMetal);
		mGBufferDepth.set(inputs.gbuffer.depth);

		SPtr<Texture> skyFilteredRadiance;
		if(sceneInfo.skybox)
			skyFilteredRadiance = sceneInfo.skybox->getFilteredRadiance();

		mImageBasedParams.preintegratedEnvBRDFParam.set(inputs.preIntegratedGF);
		mImageBasedParams.reflectionProbesParam.set(probeData.getProbeBuffer());
		mImageBasedParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
		mImageBasedParams.skyReflectionsTexParam.set(skyFilteredRadiance);
		mImageBasedParams.ambientOcclusionTexParam.set(inputs.ambientOcclusion);
		mImageBasedParams.ssrTexParam.set(inputs.ssr);

		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		mInColorTextureParam.set(inputs.lightAccumulation);
		if (mSampleCount > 1)
		{
			mOutputBufferParam.set(inputs.sceneColorBuffer);
			mMSAACoverageTexParam.set(inputs.msaaCoverage);
		}
		else
			mOutputTextureParam.set(inputs.sceneColorTex);

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		bind();
		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return get(getVariation<1>());
		case 2:
			return get(getVariation<2>());
		case 4:
			return get(getVariation<4>());
		case 8:
		default:
			return get(getVariation<8>());
		}
	}
}}
