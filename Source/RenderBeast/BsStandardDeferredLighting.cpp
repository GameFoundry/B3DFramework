//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStandardDeferredLighting.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRendererView.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsSkybox.h"
#include "BsRendererScene.h"
#include "Renderer/BsReflectionProbe.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	DeferredDirectionalLightMat::DeferredDirectionalLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DeferredDirectionalLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredDirectionalLightMat* DeferredDirectionalLightMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());
			else
				return get(getVariation<true, false>());
		}

		return get(getVariation<false, false>());
	}

	DeferredPointLightMat::DeferredPointLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DeferredPointLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredPointLightMat* DeferredPointLightMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (inside)
			{
				if (singleSampleMSAA)
					return get(getVariation<true, true, true>());

				return get(getVariation<true, true, false>());
			}
			else
			{
				if (singleSampleMSAA)
					return get(getVariation<false, true, true>());

				return get(getVariation<false, true, false>());
			}
		}
		else
		{
			if (inside)
				return get(getVariation<true, false, false>());
			else
				return get(getVariation<false, false, false>());
		}
	}

	PerProbeParamDef gPerProbeParamDef;

	DeferredIBLSetupMat::DeferredIBLSetupMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::create(desc);
		mIBLParams.ssrSampParam.set(samplerState);
		mIBLParams.ambientOcclusionSampParam.set(samplerState);
	}

	void DeferredIBLSetupMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
		const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.bind(gBufferInput);

		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("ReflProbeParams", reflProbeParams);

		mIBLParams.ambientOcclusionTexParam.set(ao);
		mIBLParams.ssrTexParam.set(ssr);

		RendererMaterial::bind();
	}

	DeferredIBLSetupMat* DeferredIBLSetupMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			return get(getVariation<false, false>());
		}
	}

	DeferredIBLProbeMat::DeferredIBLProbeMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);

		mParamBuffer = gPerProbeParamDef.createBuffer();
		mParams->setParamBlockBuffer("PerProbe", mParamBuffer);
	}

	void DeferredIBLProbeMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
		const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.bind(gBufferInput);

		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("ReflProbeParams", reflProbeParams);

		gPerProbeParamDef.gPosition.set(mParamBuffer, probeData.position);

		if(probeData.type == 1)
			gPerProbeParamDef.gExtents.set(mParamBuffer, probeData.boxExtents);
		else
		{
			Vector3 extents(probeData.radius, probeData.radius, probeData.radius);
			gPerProbeParamDef.gExtents.set(mParamBuffer, extents);
		}

		gPerProbeParamDef.gTransitionDistance.set(mParamBuffer, probeData.transitionDistance);
		gPerProbeParamDef.gInvBoxTransform.set(mParamBuffer, probeData.invBoxTransform);
		gPerProbeParamDef.gCubemapIdx.set(mParamBuffer, probeData.cubemapIdx);
		gPerProbeParamDef.gType.set(mParamBuffer, probeData.type);

		mIBLParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);

		RendererMaterial::bind();
	}

	DeferredIBLProbeMat* DeferredIBLProbeMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (inside)
			{
				if (singleSampleMSAA)
					return get(getVariation<true, true, true>());

				return get(getVariation<true, true, false>());
			}
			else
			{
				if (singleSampleMSAA)
					return get(getVariation<false, true, true>());

				return get(getVariation<false, true, false>());
			}
		}
		else
		{
			if (inside)
				return get(getVariation<true, false, false>());
			else
				return get(getVariation<false, false, false>());
		}
	}

	DeferredIBLSkyMat::DeferredIBLSkyMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::create(desc);
		mIBLParams.ssrSampParam.set(samplerState);
		mIBLParams.ambientOcclusionSampParam.set(samplerState);
	}

	void DeferredIBLSkyMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
		const Skybox* skybox, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.bind(gBufferInput);

		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("ReflProbeParams", reflProbeParams);

		if(skybox != nullptr)
			mIBLParams.skyReflectionsTexParam.set(skybox->getFilteredRadiance());

		RendererMaterial::bind();
	}

	DeferredIBLSkyMat* DeferredIBLSkyMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			return get(getVariation<false, false>());
		}
	}

	DeferredIBLFinalizeMat::DeferredIBLFinalizeMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gIBLRadianceTex", mIBLRadiance);

		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLFinalizeMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
		const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, 
		const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.bind(gBufferInput);

		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("ReflProbeParams", reflProbeParams);

		mIBLParams.preintegratedEnvBRDFParam.set(preintegratedBrdf);

		mIBLRadiance.set(iblRadiance);

		RendererMaterial::bind();
	}

	DeferredIBLFinalizeMat* DeferredIBLFinalizeMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			return get(getVariation<false, false>());
		}
	}

	StandardDeferred::StandardDeferred()
	{
		mPerLightBuffer = gPerLightParamDef.createBuffer();
	}

	void StandardDeferred::renderLight(LightType lightType, const RendererLight& light, const RendererView& view, 
		const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
	{
		const auto& viewProps = view.getProperties();

		bool isMSAA = view.getProperties().numSamples > 1;
		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		light.getParameters(mPerLightBuffer);

		if (lightType == LightType::Directional)
		{
			DeferredDirectionalLightMat* material = DeferredDirectionalLightMat::getVariation(isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			gRendererUtility().drawScreenQuad();

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredDirectionalLightMat* msaaMaterial = DeferredDirectionalLightMat::getVariation(true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().drawScreenQuad();
			}
		}
		else // Radial or spot
		{
			// Check if viewer is inside the light volume
			float distSqrd = (light.internal->getBounds().getCenter() - viewProps.viewOrigin).squaredLength();

			// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
			// the light bounds. We need to be conservative since the material for rendering outside will not properly
			// render the inside of the light volume.
			float boundRadius = light.internal->getBounds().getRadius() + viewProps.nearPlane * 3.0f;

			bool isInside = distSqrd < (boundRadius * boundRadius);

			SPtr<Mesh> stencilMesh;
			if(lightType == LightType::Radial)
				stencilMesh = RendererUtility::instance().getSphereStencil();
			else // Spot
				stencilMesh = RendererUtility::instance().getSpotLightStencil();

			DeferredPointLightMat* material = DeferredPointLightMat::getVariation(isInside, isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
			gRendererUtility().draw(stencilMesh);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredPointLightMat* msaaMaterial = DeferredPointLightMat::getVariation(isInside, true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().draw(stencilMesh);
			}
		}
	}
	void StandardDeferred::renderReflProbe(const ReflProbeData& probeData, const RendererView& view,
		const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		const auto& viewProps = view.getProperties();
		bool isMSAA = viewProps.numSamples > 1;

		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		// When checking if viewer is inside the volume extend the bounds slighty to cover the case when the viewer is
		// outside, but the near plane is intersecting the bounds. We need to be conservative since the material for
		// rendering outside will not properly render the inside of the volume.
		float radiusBuffer = viewProps.nearPlane * 3.0f;

		SPtr<Mesh> stencilMesh;
		bool isInside;
		if(probeData.type == 0) // Sphere
		{
			// Check if viewer is inside the light volume
			float distSqrd = (probeData.position - viewProps.viewOrigin).squaredLength();
			float boundRadius = probeData.radius + radiusBuffer;
			
			isInside = distSqrd < (boundRadius * boundRadius);
			stencilMesh = RendererUtility::instance().getSphereStencil();
		} 
		else // Box
		{
			Vector3 extents = probeData.boxExtents + radiusBuffer;
			AABox box(probeData.position - extents, probeData.position + extents);

			isInside = box.contains(viewProps.viewOrigin);
			stencilMesh = RendererUtility::instance().getBoxStencil();
		}

		DeferredIBLProbeMat* material = DeferredIBLProbeMat::getVariation(isInside, isMSAA, true);
		material->bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		gRendererUtility().draw(stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if (isMSAA)
		{
			DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::getVariation(isInside, true, false);
			msaaMaterial->bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

			gRendererUtility().draw(stencilMesh);
		}
	}
}}
