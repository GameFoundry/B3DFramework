//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStandardDeferredLighting.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRendererView.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	ShaderVariation DirectionalLightMat::VAR_FullMSAA = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true)
	});

	ShaderVariation DirectionalLightMat::VAR_SingleMSAA = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("MSAA_RESOLVE_0TH", true)
	});

	ShaderVariation DirectionalLightMat::VAR_NoMSAA = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 1)
	});

	DirectionalLightMat::DirectionalLightMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		const GpuParams& gpuParams = *mParamsSet->getGpuParams();
		gpuParams.getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DirectionalLightMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_FullMSAA);
		variations.add(VAR_SingleMSAA);
		variations.add(VAR_NoMSAA);
	}

	void DirectionalLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParamsSet->getGpuParams()->setParamBlockBuffer("PerCamera", perCamera);
	}

	void DirectionalLightMat::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->getGpuParams()->setParamBlockBuffer("PerLight", perLight);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	DirectionalLightMat* DirectionalLightMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return get(VAR_SingleMSAA);
			else
				return get(VAR_FullMSAA);
		}

		return get(VAR_NoMSAA);
	}

	ShaderVariation PointLightMat::VAR_FullMSAA_Inside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("INSIDE_GEOMETRY", true)
	});

	ShaderVariation PointLightMat::VAR_SingleMSAA_Inside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("INSIDE_GEOMETRY", true),
		ShaderVariation::Param("MSAA_RESOLVE_0TH", true)
	});

	ShaderVariation PointLightMat::VAR_FullMSAA_Outside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true)
	});

	ShaderVariation PointLightMat::VAR_SingleMSAA_Outside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("MSAA_RESOLVE_0TH", true)
	});

	ShaderVariation PointLightMat::VAR_NoMSAA_Inside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 1),
		ShaderVariation::Param("INSIDE_GEOMETRY", true)
	});

	ShaderVariation PointLightMat::VAR_NoMSAA_Outside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 1)
	});

	PointLightMat::PointLightMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		const GpuParams& gpuParams = *mParamsSet->getGpuParams();
		gpuParams.getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void PointLightMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_FullMSAA_Inside);
		variations.add(VAR_SingleMSAA_Inside);
		variations.add(VAR_FullMSAA_Outside);
		variations.add(VAR_SingleMSAA_Outside);
		variations.add(VAR_NoMSAA_Inside);
		variations.add(VAR_NoMSAA_Outside);
	}

	void PointLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParamsSet->getGpuParams()->setParamBlockBuffer("PerCamera", perCamera);
	}

	void PointLightMat::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->getGpuParams()->setParamBlockBuffer("PerLight", perLight);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	PointLightMat* PointLightMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (inside)
			{
				if (singleSampleMSAA)
					return get(VAR_SingleMSAA_Inside);

				return get(VAR_FullMSAA_Inside);
			}
			else
			{
				if (singleSampleMSAA)
					return get(VAR_SingleMSAA_Outside);

				return get(VAR_FullMSAA_Outside);
			}
		}
		else
		{
			if (inside)
				return get(VAR_NoMSAA_Inside);
			else
				return get(VAR_NoMSAA_Outside);
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
			material->bind(gBufferInput, lightOcclusion, perViewBuffer);
			material->setPerLightParams(mPerLightBuffer);

			gRendererUtility().drawScreenQuad();

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DirectionalLightMat* msaaMaterial = DirectionalLightMat::getVariation(true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer);
				msaaMaterial->setPerLightParams(mPerLightBuffer);

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
			material->bind(gBufferInput, lightOcclusion, perViewBuffer);
			material->setPerLightParams(mPerLightBuffer);

			// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
			gRendererUtility().draw(stencilMesh);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				PointLightMat* msaaMaterial = PointLightMat::getVariation(isInside, true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer);
				msaaMaterial->setPerLightParams(mPerLightBuffer);

				gRendererUtility().draw(stencilMesh);
			}
		}
	}
}}
