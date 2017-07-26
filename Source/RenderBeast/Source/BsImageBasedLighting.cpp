//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsImageBasedLighting.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsReflectionProbe.h"
#include "BsLightProbeCache.h"
#include "BsGpuParamsSet.h"
#include "BsRenderTargets.h"
#include "BsRenderBeast.h"
#include "BsRendererUtility.h"
#include "BsSkybox.h"

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
		texture = nullptr;
		customTexture = probe->getCustomTexture() != nullptr;
		textureDirty = LightProbeCache::instance().isRadianceDirty(probe->getUUID());
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
		{
			params->getTextureParam(programType, "gSkyReflectionTex", skyReflectionsTexParam);
			params->getTextureParam(programType, "gSkyIrradianceTex", skyIrradianceTexParam);
		}

		// Reflections
		if (!optional || params->hasTexture(programType, "gReflProbeCubemaps"))
		{
			params->getTextureParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsTexParam);
			params->getBufferParam(programType, "gReflectionProbes", reflectionProbesParam);
			params->getTextureParam(programType, "gPreintegratedEnvBRDF", preintegratedEnvBRDFParam);
		}

		if(gridIndices)
		{
			if (!optional || params->hasBuffer(programType, "gReflectionProbeIndices"))
				params->getBufferParam(programType, "gReflectionProbeIndices", reflectionProbeIndicesParam);
		}

		reflProbeParamsBindingIdx = paramsSet->getParamBlockBufferIndex("ReflProbeParams");
	}

	// Note: Using larger tiles than in tiled deferred lighting since we use AABB for intersections, which is more
	// expensive to compute than frustums. This way we amortize the cost even though other parts of the shader might suffer
	// due to increased thread group load.
	const UINT32 TiledDeferredImageBasedLighting::TILE_SIZE = 32;

	TiledDeferredImageBasedLighting::TiledDeferredImageBasedLighting(const SPtr<Material>& material, 
		const SPtr<GpuParamsSet>& paramsSet, UINT32 sampleCount)
		:mSampleCount(sampleCount), mMaterial(material), mParamsSet(paramsSet)
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		if (mSampleCount > 1)
			params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);
		else
			params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);

		mImageBasedParams.populate(mParamsSet, GPT_COMPUTE_PROGRAM, false, false);

		mParamsSet->setParamBlockBuffer("ReflProbeParams", mReflProbeParamBuffer.buffer);
	}

	void TiledDeferredImageBasedLighting::execute(const RendererView& view, const SceneInfo& sceneInfo, 
		const VisibleReflProbeData& probeData, const Inputs& inputs)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledImageBasedLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		mReflProbeParamBuffer.populate(sceneInfo.sky, probeData, sceneInfo.reflProbeCubemapsTex, 
			viewProps.renderingReflections);

		mParamBuffer->flushToGPU();
		mReflProbeParamBuffer.buffer->flushToGPU();

		mGBufferA.set(inputs.gbuffer.albedo);
		mGBufferB.set(inputs.gbuffer.normals);
		mGBufferC.set(inputs.gbuffer.roughMetal);
		mGBufferDepth.set(inputs.gbuffer.depth);

		mImageBasedParams.preintegratedEnvBRDFParam.set(inputs.preIntegratedGF);
		mImageBasedParams.reflectionProbesParam.set(probeData.getProbeBuffer());
		mImageBasedParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
		mImageBasedParams.skyReflectionsTexParam.set(sceneInfo.sky.filteredReflections);
		mImageBasedParams.skyIrradianceTexParam.set(sceneInfo.sky.irradiance);

		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

		mInColorTextureParam.set(inputs.lightAccumulation);
		if (mSampleCount > 1)
			mOutputBufferParam.set(inputs.sceneColorBuffer);
		else
			mOutputTextureParam.set(inputs.sceneColorTex);

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	ReflProbeParamBuffer::ReflProbeParamBuffer()
	{
		buffer = gReflProbeParamsParamDef.createBuffer();
	}

	void ReflProbeParamBuffer::populate(const SkyInfo& sky, const VisibleReflProbeData& probeData, 
		const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
	{
		UINT32 skyReflectionsAvailable = 0;
		UINT32 numSkyMips = 0;
		if (sky.filteredReflections != nullptr)
		{
			numSkyMips = sky.filteredReflections->getProperties().getNumMipmaps() + 1;
			skyReflectionsAvailable = 1;
		}

		float brightness = 1.0f;
		if (sky.skybox != nullptr)
			brightness = sky.skybox->getBrightness();

		gReflProbeParamsParamDef.gSkyCubemapNumMips.set(buffer, numSkyMips);
		gReflProbeParamsParamDef.gSkyCubemapAvailable.set(buffer, skyReflectionsAvailable);
		gReflProbeParamsParamDef.gSkyBrightness.set(buffer, brightness);
		gReflProbeParamsParamDef.gNumProbes.set(buffer, probeData.getNumProbes());

		UINT32 numReflProbeMips = 0;
		if (reflectionCubemaps != nullptr)
			numReflProbeMips = reflectionCubemaps->getProperties().getNumMipmaps() + 1;

		gReflProbeParamsParamDef.gReflCubemapNumMips.set(buffer, numReflProbeMips);
		gReflProbeParamsParamDef.gUseReflectionMaps.set(buffer, capturingReflections ? 0 : 1);
	}

	template<int MSAA_COUNT>
	TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::TTiledDeferredImageBasedLightingMat()
		:mInternal(mMaterial, mParamsSet, MSAA_COUNT)
	{

	}

	template<int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TiledDeferredImageBasedLighting::TILE_SIZE);
		defines.set("MSAA_COUNT", MSAA_COUNT);
	}

	template <int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::execute(const RendererView& view, const SceneInfo& sceneInfo, 
		const VisibleReflProbeData& probeData, const TiledDeferredImageBasedLighting::Inputs& inputs)
	{
		mInternal.execute(view, sceneInfo, probeData, inputs);
	}

	template class TTiledDeferredImageBasedLightingMat<1>;
	template class TTiledDeferredImageBasedLightingMat<2>;
	template class TTiledDeferredImageBasedLightingMat<4>;
	template class TTiledDeferredImageBasedLightingMat<8>;
}}
