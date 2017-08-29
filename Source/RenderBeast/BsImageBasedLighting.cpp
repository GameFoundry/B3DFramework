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
		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

		// Generate refl. probe data for the visible ones
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();
		for(UINT32 i = 0; i < numProbes; i++)
		{
			if (!visibility.reflProbes[i])
				continue;

			mReflProbeDataTemp.push_back(ReflProbeData());
			sceneInfo.reflProbes[i].getParameters(mReflProbeDataTemp.back());
		}

		// Sort probes so bigger ones get accessed first, this way we overlay smaller ones on top of biggers ones when
		// rendering
		auto sorter = [](const ReflProbeData& lhs, const ReflProbeData& rhs)
		{
			return rhs.radius < lhs.radius;
		};

		std::sort(mReflProbeDataTemp.begin(), mReflProbeDataTemp.end(), sorter);

		mNumProbes = (UINT32)mReflProbeDataTemp.size();

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
			mProbeBuffer->writeData(0, size, mReflProbeDataTemp.data(), BWT_DISCARD);

		mReflProbeDataTemp.clear();
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
		
		output.position = probe->getPosition();
		output.boxExtents = probe->getExtents();

		if (probe->getType() == ReflectionProbeType::Sphere)
			output.radius = probe->getRadius();
		else
			output.radius = output.boxExtents.length();

		output.transitionDistance = probe->getTransitionDistance();
		output.cubemapIdx = arrayIdx;
		output.invBoxTransform.setInverseTRS(output.position, probe->getRotation(), output.boxExtents);
	}

	void ImageBasedLightingParams::populate(const SPtr<GpuParamsSet>& paramsSet, GpuProgramType programType, bool optional, 
		bool gridIndices)
	{
		SPtr<GpuParams> params = paramsSet->getGpuParams();

		// Sky
		if (!optional || params->hasTexture(programType, "gSkyReflectionTex"))
			params->getTextureParam(programType, "gSkyReflectionTex", skyReflectionsTexParam);

		// Reflections
		if (!optional || params->hasTexture(programType, "gReflProbeCubemaps"))
		{
			params->getTextureParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsTexParam);
			params->getBufferParam(programType, "gReflectionProbes", reflectionProbesParam);
			params->getTextureParam(programType, "gPreintegratedEnvBRDF", preintegratedEnvBRDFParam);
		}

		// AO
		if(params->hasTexture(programType, "gAmbientOcclusionTex"))
			params->getTextureParam(programType, "gAmbientOcclusionTex", ambientOcclusionTexParam);

		// SSR
		if(params->hasTexture(programType, "gSSRTex"))
			params->getTextureParam(programType, "gSSRTex", ssrTexParam);

		if(gridIndices)
		{
			if (!optional || params->hasBuffer(programType, "gReflectionProbeIndices"))
				params->getBufferParam(programType, "gReflectionProbeIndices", reflectionProbeIndicesParam);
		}

		reflProbeParamsBindingIdx = paramsSet->getParamBlockBufferIndex("ReflProbeParams");
	}

	ReflProbeParamBuffer::ReflProbeParamBuffer()
	{
		buffer = gReflProbeParamsParamDef.createBuffer();
	}

	void ReflProbeParamBuffer::populate(const Skybox* sky, const VisibleReflProbeData& probeData, 
		const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
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
		gReflProbeParamsParamDef.gNumProbes.set(buffer, probeData.getNumProbes());

		UINT32 numReflProbeMips = 0;
		if (reflectionCubemaps != nullptr)
			numReflProbeMips = reflectionCubemaps->getProperties().getNumMipmaps() + 1;

		gReflProbeParamsParamDef.gReflCubemapNumMips.set(buffer, numReflProbeMips);
		gReflProbeParamsParamDef.gUseReflectionMaps.set(buffer, capturingReflections ? 0 : 1);
		gReflProbeParamsParamDef.gSkyBrightness.set(buffer, brightness);
	}

	ShaderVariation TiledDeferredImageBasedLightingMat::VAR_1MSAA = ShaderVariation({
		ShaderVariation::Param("TILE_SIZE", TILE_SIZE),
		ShaderVariation::Param("MSAA_COUNT", 1)
	});

	ShaderVariation TiledDeferredImageBasedLightingMat::VAR_2MSAA = ShaderVariation({
		ShaderVariation::Param("TILE_SIZE", TILE_SIZE),
		ShaderVariation::Param("MSAA_COUNT", 2)
	});

	ShaderVariation TiledDeferredImageBasedLightingMat::VAR_4MSAA = ShaderVariation({
		ShaderVariation::Param("TILE_SIZE", TILE_SIZE),
		ShaderVariation::Param("MSAA_COUNT", 4)
	});

	ShaderVariation TiledDeferredImageBasedLightingMat::VAR_8MSAA = ShaderVariation({
		ShaderVariation::Param("TILE_SIZE", TILE_SIZE),
		ShaderVariation::Param("MSAA_COUNT", 8)
	});

	// Note: Using larger tiles than in tiled deferred lighting since we use AABB for intersections, which is more
	// expensive to compute than frustums. This way we amortize the cost even though other parts of the shader might suffer
	// due to increased thread group load.
	const UINT32 TiledDeferredImageBasedLightingMat::TILE_SIZE = 32;

	TiledDeferredImageBasedLightingMat::TiledDeferredImageBasedLightingMat()
	{
		mSampleCount = mVariation.getUInt("MSAA_COUNT");

		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		if (mSampleCount > 1)
		{
			params->getTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);
			params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);
		}
		else
			params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);

		mImageBasedParams.populate(mParamsSet, GPT_COMPUTE_PROGRAM, false, false);

		mParamsSet->setParamBlockBuffer("ReflProbeParams", mReflProbeParamBuffer.buffer);
	}

	void TiledDeferredImageBasedLightingMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_1MSAA);
		variations.add(VAR_2MSAA);
		variations.add(VAR_4MSAA);
		variations.add(VAR_8MSAA);
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

		mReflProbeParamBuffer.populate(sceneInfo.skybox, probeData, sceneInfo.reflProbeCubemapsTex, 
			viewProps.renderingReflections);

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

		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

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

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return get(VAR_1MSAA);
		case 2:
			return get(VAR_2MSAA);
		case 4:
			return get(VAR_4MSAA);
		case 8:
		default:
			return get(VAR_8MSAA);
		}
	}
}}
