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

namespace bs {
namespace ct {

PerLightParamDef gPerLightParamDef;

void DeferredDirectionalLightMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
}

void DeferredDirectionalLightMat::Bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
{
	mGBufferParams.Bind(gBufferInput);
	mLightOcclusionTexParam.Set(lightOcclusion);
	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("PerLight", perLight);

	RendererMaterial::Bind();
}

DeferredDirectionalLightMat* DeferredDirectionalLightMat::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());
		else
			return Get(GetVariation<true, false>());
	}

	return Get(GetVariation<false, false>());
}

void DeferredPointLightMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
}

void DeferredPointLightMat::Bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight)
{
	mGBufferParams.Bind(gBufferInput);
	mLightOcclusionTexParam.Set(lightOcclusion);
	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("PerLight", perLight);

	RendererMaterial::Bind();
}

DeferredPointLightMat* DeferredPointLightMat::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(inside)
		{
			if(singleSampleMSAA)
				return Get(GetVariation<true, true, true>());

			return Get(GetVariation<true, true, false>());
		}
		else
		{
			if(singleSampleMSAA)
				return Get(GetVariation<false, true, true>());

			return Get(GetVariation<false, true, false>());
		}
	}
	else
	{
		if(inside)
			return Get(GetVariation<true, false, false>());
		else
			return Get(GetVariation<false, false, false>());
	}
}

PerProbeParamDef gPerProbeParamDef;

void DeferredIBLSetupMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSetupMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.AmbientOcclusionTexParam.Set(ao);
	mIBLParams.SsrTexParam.Set(ssr);

	RendererMaterial::Bind();
}

DeferredIBLSetupMat* DeferredIBLSetupMat::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		return Get(GetVariation<false, false>());
	}
}

void DeferredIBLProbeMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);

	mParamBuffer = gPerProbeParamDef.CreateBuffer();
	mGPUParameters->SetParameterBlockBuffer("PerProbe", mParamBuffer);
}

void DeferredIBLProbeMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("ReflProbeParams", reflProbeParams);

	gPerProbeParamDef.gPosition.Set(mParamBuffer, probeData.Position);

	if(probeData.Type == 1)
		gPerProbeParamDef.gExtents.Set(mParamBuffer, probeData.BoxExtents);
	else
	{
		Vector3 extents(probeData.Radius, probeData.Radius, probeData.Radius);
		gPerProbeParamDef.gExtents.Set(mParamBuffer, extents);
	}

	gPerProbeParamDef.gTransitionDistance.Set(mParamBuffer, probeData.TransitionDistance);
	gPerProbeParamDef.gInvBoxTransform.Set(mParamBuffer, probeData.InvBoxTransform);
	gPerProbeParamDef.gCubemapIdx.Set(mParamBuffer, probeData.CubemapIdx);
	gPerProbeParamDef.gType.Set(mParamBuffer, probeData.Type);

	mIBLParams.ReflectionProbeCubemapsTexParam.Set(sceneInfo.ReflProbeCubemapsTex);

	RendererMaterial::Bind();
}

DeferredIBLProbeMat* DeferredIBLProbeMat::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(inside)
		{
			if(singleSampleMSAA)
				return Get(GetVariation<true, true, true>());

			return Get(GetVariation<true, true, false>());
		}
		else
		{
			if(singleSampleMSAA)
				return Get(GetVariation<false, true, true>());

			return Get(GetVariation<false, true, false>());
		}
	}
	else
	{
		if(inside)
			return Get(GetVariation<true, false, false>());
		else
			return Get(GetVariation<false, false, false>());
	}
}

void DeferredIBLSkyMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSkyMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, const Skybox* skybox, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("ReflProbeParams", reflProbeParams);

	if(skybox != nullptr)
		mIBLParams.SkyReflectionsTexParam.Set(skybox->GetFilteredRadiance());

	RendererMaterial::Bind();
}

DeferredIBLSkyMat* DeferredIBLSkyMat::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		return Get(GetVariation<false, false>());
	}
}

void DeferredIBLFinalizeMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gIBLRadianceTex", mIBLRadiance);

	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLFinalizeMat::Bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetParameterBlockBuffer("PerCamera", perCamera);
	mGPUParameters->SetParameterBlockBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.PreintegratedEnvBrdfParam.Set(preintegratedBrdf);

	mIBLRadiance.Set(iblRadiance);

	RendererMaterial::Bind();
}

DeferredIBLFinalizeMat* DeferredIBLFinalizeMat::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		return Get(GetVariation<false, false>());
	}
}

StandardDeferred::StandardDeferred()
{
	mPerLightBuffer = gPerLightParamDef.CreateBuffer();
}

void StandardDeferred::RenderLight(LightType lightType, const RendererLight& light, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
{
	const auto& viewProps = view.GetProperties();

	bool isMSAA = view.GetProperties().Target.NumSamples > 1;
	SPtr<GpuParamBlockBuffer> perViewBuffer = view.GetPerViewBuffer();

	light.GetParameters(mPerLightBuffer);

	if(lightType == LightType::Directional)
	{
		DeferredDirectionalLightMat* material = DeferredDirectionalLightMat::GetVariation(isMSAA, true);
		material->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

		GetRendererUtility().DrawScreenQuad();

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredDirectionalLightMat* msaaMaterial = DeferredDirectionalLightMat::GetVariation(true, false);
			msaaMaterial->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			GetRendererUtility().DrawScreenQuad();
		}
	}
	else // Radial or spot
	{
		// Check if viewer is inside the light volume
		float distSqrd = (light.Internal->GetBounds().GetCenter() - viewProps.ViewOrigin).SquaredLength();

		// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
		// the light bounds. We need to be conservative since the material for rendering outside will not properly
		// render the inside of the light volume.
		float boundRadius = light.Internal->GetBounds().GetRadius() + viewProps.NearPlane * 3.0f;

		bool isInside = distSqrd < (boundRadius * boundRadius);

		SPtr<Mesh> stencilMesh;
		if(lightType == LightType::Radial)
			stencilMesh = RendererUtility::Instance().GetSphereStencil();
		else // Spot
			stencilMesh = RendererUtility::Instance().GetSpotLightStencil();

		DeferredPointLightMat* material = DeferredPointLightMat::GetVariation(isInside, isMSAA, true);
		material->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		GetRendererUtility().Draw(stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredPointLightMat* msaaMaterial = DeferredPointLightMat::GetVariation(isInside, true, false);
			msaaMaterial->Bind(gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			GetRendererUtility().Draw(stencilMesh);
		}
	}
}

void StandardDeferred::RenderReflProbe(const ReflProbeData& probeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuParamBlockBuffer>& reflProbeParams)
{
	const auto& viewProps = view.GetProperties();
	bool isMSAA = viewProps.Target.NumSamples > 1;

	SPtr<GpuParamBlockBuffer> perViewBuffer = view.GetPerViewBuffer();

	// When checking if viewer is inside the volume extend the bounds slighty to cover the case when the viewer is
	// outside, but the near plane is intersecting the bounds. We need to be conservative since the material for
	// rendering outside will not properly render the inside of the volume.
	float radiusBuffer = viewProps.NearPlane * 3.0f;

	SPtr<Mesh> stencilMesh;
	bool isInside;
	if(probeData.Type == 0) // Sphere
	{
		// Check if viewer is inside the light volume
		float distSqrd = (probeData.Position - viewProps.ViewOrigin).SquaredLength();
		float boundRadius = probeData.Radius + radiusBuffer;

		isInside = distSqrd < (boundRadius * boundRadius);
		stencilMesh = RendererUtility::Instance().GetSphereStencil();
	}
	else // Box
	{
		Vector3 extents = probeData.BoxExtents + radiusBuffer;
		AABox box(probeData.Position - extents, probeData.Position + extents);

		isInside = box.Contains(viewProps.ViewOrigin);
		stencilMesh = RendererUtility::Instance().GetBoxStencil();
	}

	DeferredIBLProbeMat* material = DeferredIBLProbeMat::GetVariation(isInside, isMSAA, true);
	material->Bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

	// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
	GetRendererUtility().Draw(stencilMesh);

	// Draw pixels requiring per-sample evaluation
	if(isMSAA)
	{
		DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::GetVariation(isInside, true, false);
		msaaMaterial->Bind(gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		GetRendererUtility().Draw(stencilMesh);
	}
}
}} // namespace bs::ct
