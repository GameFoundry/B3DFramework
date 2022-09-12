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

	void DeferredDirectionalLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.Bind(gBufferInput);
		mLightOcclusionTexParam.Set(lightOcclusion);
		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredDirectionalLightMat* DeferredDirectionalLightMat::getVariation(bool msaa, bool singleSampleMSAA)
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

	void DeferredPointLightMat::bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mGBufferParams.Bind(gBufferInput);
		mLightOcclusionTexParam.Set(lightOcclusion);
		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("PerLight", perLight);

		RendererMaterial::bind();
	}

	DeferredPointLightMat* DeferredPointLightMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
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
		mIBLParams.Populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLSetupMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		mIBLParams.ambientOcclusionTexParam.Set(ao);
		mIBLParams.ssrTexParam.Set(ssr);

		RendererMaterial::bind();
	}

	DeferredIBLSetupMat* DeferredIBLSetupMat::getVariation(bool msaa, bool singleSampleMSAA)
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
		mIBLParams.Populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);

		mParamBuffer = gPerProbeParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("PerProbe", mParamBuffer);
	}

	void DeferredIBLProbeMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
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
			Vector3 Extents(probeData.radius, probeData.radius, probeData.radius);
			gPerProbeParamDef.gExtents.Set(mParamBuffer, extents);
		}

		gPerProbeParamDef.gTransitionDistance.Set(mParamBuffer, probeData.transitionDistance);
		gPerProbeParamDef.gInvBoxTransform.Set(mParamBuffer, probeData.invBoxTransform);
		gPerProbeParamDef.gCubemapIdx.Set(mParamBuffer, probeData.cubemapIdx);
		gPerProbeParamDef.gType.Set(mParamBuffer, probeData.type);

		mIBLParams.reflectionProbeCubemapsTexParam.Set(sceneInfo.reflProbeCubemapsTex);

		RendererMaterial::bind();
	}

	DeferredIBLProbeMat* DeferredIBLProbeMat::getVariation(bool inside, bool msaa, bool singleSampleMSAA)
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
		mIBLParams.Populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLSkyMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
		const Skybox* skybox, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		mGBufferParams.Bind(gBufferInput);

		mParams->SetParamBlockBuffer("PerCamera", perCamera);
		mParams->SetParamBlockBuffer("ReflProbeParams", reflProbeParams);

		if(skybox != nullptr)
			mIBLParams.skyReflectionsTexParam.Set(skybox->GetFilteredRadiance());

		RendererMaterial::bind();
	}

	DeferredIBLSkyMat* DeferredIBLSkyMat::getVariation(bool msaa, bool singleSampleMSAA)
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

		mIBLParams.Populate(mParams, GPT_FRAGMENT_PROGRAM, true, false, false);
	}

	void DeferredIBLFinalizeMat::bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
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

	DeferredIBLFinalizeMat* DeferredIBLFinalizeMat::getVariation(bool msaa, bool singleSampleMSAA)
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
		mPerLightBuffer = gPerLightParamDef.CreateBuffer();
	}

	void StandardDeferred::renderLight(LightType lightType, const RendererLight& light, const RendererView& view,
		const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
	{
		const auto& viewProps = view.GetProperties();

		bool isMSAA = view.GetProperties().target.numSamples > 1;
		SPtr<GpuParamBlockBuffer> perViewBuffer = view.GetPerViewBuffer();

		light.GetParameters(mPerLightBuffer);

		if (lightType == LightType::Directional)
		{
			DeferredDirectionalLightMat* material = DeferredDirectionalLightMat::getVariation(isMSAA, true);
			material->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			gRendererUtility().DrawScreenQuad();

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredDirectionalLightMat* msaaMaterial = DeferredDirectionalLightMat::getVariation(true, false);
				msaaMaterial->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

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
				stencilMesh = RendererUtility::instance().GetSphereStencil();
			else // Spot
				stencilMesh = RendererUtility::instance().GetSpotLightStencil();

			DeferredPointLightMat* material = DeferredPointLightMat::getVariation(isInside, isMSAA, true);
			material->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
			gRendererUtility().Draw(stencilMesh);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredPointLightMat* msaaMaterial = DeferredPointLightMat::getVariation(isInside, true, false);
				msaaMaterial->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

				gRendererUtility().Draw(stencilMesh);
			}
		}
	}
	void StandardDeferred::renderReflProbe(const ReflProbeData& probeData, const RendererView& view,
		const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
	{
		const auto& viewProps = view.GetProperties();
		bool isMSAA = viewProps.target.numSamples > 1;

		SPtr<GpuParamBlockBuffer> perViewBuffer = view.GetPerViewBuffer();

		// When checking if viewer is inside the volume extend the bounds slighty to cover the case when the viewer is
		// outside, but the near plane is intersecting the bounds. We need to be conservative since the material for
		// rendering outside will not properly render the inside of the volume.
		float radiusBuffer = viewProps.nearPlane * 3.0f;

		SPtr<Mesh> stencilMesh;
		bool isInside;
		if(probeData.type == 0) // Sphere
		{
			// Check if viewer is inside the light volume
			float distSqrd = (probeData.position - viewProps.viewOrigin).SquaredLength();
			float boundRadius = probeData.radius + radiusBuffer;
			
			isInside = distSqrd < (boundRadius * boundRadius);
			stencilMesh = RendererUtility::instance().GetSphereStencil();
		}
		else // Box
		{
			Vector3 extents = probeData.boxExtents + radiusBuffer;
			AABox Box(probeData.position - extents, probeData.position + extents);

			isInside = box.Contains(viewProps.viewOrigin);
			stencilMesh = RendererUtility::instance().GetBoxStencil();
		}

		DeferredIBLProbeMat* material = DeferredIBLProbeMat::getVariation(isInside, isMSAA, true);
		material->Bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		gRendererUtility().Draw(stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if (isMSAA)
		{
			DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::getVariation(isInside, true, false);
			msaaMaterial->Bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

			gRendererUtility().Draw(stencilMesh);
		}
	}
}}
