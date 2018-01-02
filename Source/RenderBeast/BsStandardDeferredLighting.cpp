//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStandardDeferredLighting.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRendererView.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	DirectionalLightMat::DirectionalLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DirectionalLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DirectionalLightMat* DirectionalLightMat::getVariation(bool msaa, bool singleSampleMSAA)
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

	PointLightMat::PointLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void PointLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParams->setParamBlockBuffer("PerCamera", perCamera);
		mParams->setParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	PointLightMat* PointLightMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
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
			DirectionalLightMat* material = DirectionalLightMat::getVariation(isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			gRendererUtility().drawScreenQuad();

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DirectionalLightMat* msaaMaterial = DirectionalLightMat::getVariation(true, false);
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
				stencilMesh = RendererUtility::instance().getRadialLightStencil();
			else // Spot
				stencilMesh = RendererUtility::instance().getSpotLightStencil();

			PointLightMat* material = PointLightMat::getVariation(isInside, isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
			gRendererUtility().draw(stencilMesh);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				PointLightMat* msaaMaterial = PointLightMat::getVariation(isInside, true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().draw(stencilMesh);
			}
		}
	}
}}
