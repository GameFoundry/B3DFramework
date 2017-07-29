//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStandardDeferredLighting.h"
#include "BsRendererUtility.h"
#include "BsRendererView.h"
#include "BsGpuParamsSet.h"
#include "BsMesh.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	ShaderVariation DirectionalLightMat::VAR_MSAA = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2)
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
		variations.add(VAR_MSAA);
		variations.add(VAR_NoMSAA);
	}

	void DirectionalLightMat::bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);
	}

	void DirectionalLightMat::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->setParamBlockBuffer("PerLight", perLight, true);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	DirectionalLightMat* DirectionalLightMat::getVariation(bool msaa)
	{
		if (msaa)
			return get(VAR_MSAA);

		return get(VAR_NoMSAA);
	}

	ShaderVariation PointLightMat::VAR_MSAA_Inside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("INSIDE_GEOMETRY", true)
	});

	ShaderVariation PointLightMat::VAR_MSAA_Outside = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2)
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
		variations.add(VAR_MSAA_Inside);
		variations.add(VAR_MSAA_Outside);
		variations.add(VAR_NoMSAA_Inside);
		variations.add(VAR_NoMSAA_Outside);
	}

	void PointLightMat::bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
		const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(gBufferInput);
		mLightOcclusionTexParam.set(lightOcclusion);
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);
	}

	void PointLightMat::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->setParamBlockBuffer("PerLight", perLight, true);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	PointLightMat* PointLightMat::getVariation(bool msaa, bool inside)
	{
		if(msaa)
		{
			if (inside)
				return get(VAR_MSAA_Inside);
			else
				return get(VAR_MSAA_Outside);
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
		const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion)
	{
		const auto& viewProps = view.getProperties();

		bool isMSAA = view.getProperties().numSamples > 1;
		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		light.getParameters(mPerLightBuffer);

		if (lightType == LightType::Directional)
		{
			DirectionalLightMat* material = DirectionalLightMat::getVariation(isMSAA);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer);
			material->setPerLightParams(mPerLightBuffer);

			gRendererUtility().drawScreenQuad();
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

			PointLightMat* material = PointLightMat::getVariation(isMSAA, isInside);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer);
			material->setPerLightParams(mPerLightBuffer);

			SPtr<Mesh> stencilMesh;
			if(lightType == LightType::Radial)
				stencilMesh = RendererUtility::instance().getRadialLightStencil();
			else // Spot
				stencilMesh = RendererUtility::instance().getSpotLightStencil();

			gRendererUtility().draw(stencilMesh);
		}
	}
}}
