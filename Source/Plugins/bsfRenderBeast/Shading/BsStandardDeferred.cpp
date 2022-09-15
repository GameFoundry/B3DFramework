//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsStandardDeferred.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsSkybox.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRendererScene.h"
#include "BsRendererView.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	DeferredDirectionalLightMat::DeferredDirectionalLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DeferredDirectionalLightMat::Bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.Bind(gBufferInput);
		mLightOcclusionTexParam.Set(lightOcclusion);
		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredDirectionalLightMat* DeferredDirectionalLightMat::GetVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return Get(getVariation<true, true>());
			else
				return Get(getVariation<true, false>());
		}

		return Get(getVariation<false, false>());
	}

	DeferredPointLightMat::DeferredPointLightMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	void DeferredPointLightMat::Bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.Bind(gBufferInput);
		mLightOcclusionTexParam.Set(lightOcclusion);
		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredPointLightMat* DeferredPointLightMat::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (inside)
			{
				if (singleSampleMSAA)
					return Get(getVariation<true, true, true>());

				return Get(getVariation<true, true, false>());
			}
			else
			{
				if (singleSampleMSAA)
					return Get(getVariation<false, true, true>());

				return Get(getVariation<false, true, false>());
			}
		}
		else
		{
			if (inside)
				return Get(getVariation<true, false, false>());
			else
				return Get(getVariation<false, false, false>());
		}
	}

	PerProbeParamDef gPerProbeParamDef;

	DeferredIBLSetupMat::DeferredIBLSetupMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLSetupMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		mIBLParams.ambientOcclusionTexParam.Set(ao);
		mIBLParams.ssrTexParam.Set(ssr);

		RendererMaterial::bind();
	}

	DeferredIBLSetupMat* DeferredIBLSetupMat::GetVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return Get(getVariation<true, true>());

			return Get(getVariation<true, false>());
		}
		else
		{
			return Get(getVariation<false, false>());
		}
	}

	DeferredIBLProbeMat::DeferredIBLProbeMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);

		mParamBuffer = gPerProbeParamDef.createBuffer();
		mParams->SetParamBlockBuffer("PerProbe", mParamBuffer);
	}

	void DeferredIBLProbeMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		gPerProbeParamDef.gPosition.Set(mParamBuffer, probeData.position);

		if(probeData.type == 1)
			gPerProbeParamDef.gExtents.Set(mParamBuffer, probeData.boxExtents);
		else
		{
			Vector3 extents(probeData.radius, probeData.radius, probeData.radius);
			gPerProbeParamDef.gExtents.Set(mParamBuffer, extents);
		}

		gPerProbeParamDef.gTransitionDistance.Set(mParamBuffer, probeData.transitionDistance);
		gPerProbeParamDef.gInvBoxTransform.Set(mParamBuffer, probeData.invBoxTransform);
		gPerProbeParamDef.gCubemapIdx.Set(mParamBuffer, probeData.cubemapIdx);
		gPerProbeParamDef.gType.Set(mParamBuffer, probeData.type);

		mIBLParams.reflectionProbeCubemapsTexParam.Set(sceneInfo.reflProbeCubemapsTex);

		RendererMaterial::bind();
	}

	DeferredIBLProbeMat* DeferredIBLProbeMat::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (inside)
			{
				if (singleSampleMSAA)
					return Get(getVariation<true, true, true>());

				return Get(getVariation<true, true, false>());
			}
			else
			{
				if (singleSampleMSAA)
					return Get(getVariation<false, true, true>());

				return Get(getVariation<false, true, false>());
			}
		}
		else
		{
			if (inside)
				return Get(getVariation<true, false, false>());
			else
				return Get(getVariation<false, false, false>());
		}
	}

	DeferredIBLSkyMat::DeferredIBLSkyMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLSkyMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const Skybox* skybox, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		if(skybox != nullptr)
			mIBLParams.skyReflectionsTexParam.Set(skybox->GetFilteredRadiance());

		RendererMaterial::bind();
	}

	DeferredIBLSkyMat* DeferredIBLSkyMat::GetVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return Get(getVariation<true, true>());

			return Get(getVariation<true, false>());
		}
		else
		{
			return Get(getVariation<false, false>());
		}
	}

	DeferredIBLFinalizeMat::DeferredIBLFinalizeMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gIBLRadianceTex", mIBLRadiance);

		mIBLParams.populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLFinalizeMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf,
		const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		mIBLParams.preintegratedEnvBRDFParam.Set(preintegratedBrdf);

		mIBLRadiance.Set(iblRadiance);

		RendererMaterial::bind();
	}

	DeferredIBLFinalizeMat* DeferredIBLFinalizeMat::GetVariation(bool msaa, bool singleSampleMSAA)
	{
		if(msaa)
		{
			if (singleSampleMSAA)
				return Get(getVariation<true, true>());

			return Get(getVariation<true, false>());
		}
		else
		{
			return Get(getVariation<false, false>());
		}
	}

	StandardDeferred::StandardDeferred()
	{
		mPerLightBuffer = gPerLightParamDef.createBuffer();
	}

	void StandardDeferred::RenderLight(LightType lightType, const RendererLight& light, const RendererView& view,
		const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
	{
		const auto& viewProps = view.GetProperties();

		bool isMSAA = view.GetProperties().target.numSamples > 1;
		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		light.getParameters(mPerLightBuffer);

		if (lightType == LightType::Directional)
		{
			DeferredDirectionalLightMat* material = DeferredDirectionalLightMat::GetVariation(isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			gRendererUtility().DrawScreenQuad();

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredDirectionalLightMat* msaaMaterial = DeferredDirectionalLightMat::GetVariation(true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().DrawScreenQuad();
			}
		}
		else // Radial or spot
		{
			// Check if viewer is inside the light volume
			float distSqrd = (light.internal->GetBounds().GetCenter() - viewProps.viewOrigin).squaredLength();

			// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
			// the light bounds. We need to be conservative since the material for rendering outside will not properly
			// render the inside of the light volume.
			float boundRadius = light.internal->GetBounds().GetRadius() + viewProps.nearPlane * 3.0f;

			bool isInside = distSqrd < (boundRadius * boundRadius);

			SPtr<Mesh> stencilMesh;
			if(lightType == LightType::Radial)
				stencilMesh = RendererUtility::Instance().getSphereStencil();
			else // Spot
				stencilMesh = RendererUtility::Instance().getSpotLightStencil();

			DeferredPointLightMat* material = DeferredPointLightMat::GetVariation(isInside, isMSAA, true);
			material->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
			gRendererUtility().draw(stencilMesh);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredPointLightMat* msaaMaterial = DeferredPointLightMat::GetVariation(isInside, true, false);
				msaaMaterial->bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().draw(stencilMesh);
			}
		}
	}
	void StandardDeferred::RenderReflProbe(const ReflProbeData& probeData, const RendererView& view,
		const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		const auto& viewProps = view.GetProperties();
		bool isMSAA = viewProps.target.numSamples > 1;

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
			stencilMesh = RendererUtility::Instance().getSphereStencil();
		}
		else // Box
		{
			Vector3 extents = probeData.boxExtents + radiusBuffer;
			AABox box(probeData.position - extents, probeData.position + extents);

			isInside = box.contains(viewProps.viewOrigin);
			stencilMesh = RendererUtility::Instance().getBoxStencil();
		}

		DeferredIBLProbeMat* material = DeferredIBLProbeMat::GetVariation(isInside, isMSAA, true);
		material->bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		gRendererUtility().draw(stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if (isMSAA)
		{
			DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::GetVariation(isInside, true, false);
			msaaMaterial->bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

			gRendererUtility().draw(stencilMesh);
		}
	}
}}
