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
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
}

void DeferredDirectionalLightMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, const SPtr<GpuBuffer>& perCamera, const SPtr<GpuBuffer>& perLight)
{
	mGBufferParams.Bind(gBufferInput);
	mLightOcclusionTexParam.Set(lightOcclusion);
	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("PerLight", perLight);

	RendererMaterial::Bind(commandBuffer);
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
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
}

void DeferredPointLightMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, const SPtr<GpuBuffer>& perCamera, const SPtr<GpuBuffer>& perLight)
{
	mGBufferParams.Bind(gBufferInput);
	mLightOcclusionTexParam.Set(lightOcclusion);
	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("PerLight", perLight);

	RendererMaterial::Bind(commandBuffer);
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

void DeferredIBLSetupMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.AmbientOcclusionTexParam.Set(ao);
	mIBLParams.SsrTexParam.Set(ssr);

	RendererMaterial::Bind(commandBuffer);
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
	mGPUParameters->SetUniformBuffer("PerProbe", mParamBuffer);
}

void DeferredIBLProbeMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

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

	RendererMaterial::Bind(commandBuffer);
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

void DeferredIBLSkyMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const Skybox* skybox, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	if(skybox != nullptr)
		mIBLParams.SkyReflectionsTexParam.Set(skybox->GetFilteredRadiance());

	RendererMaterial::Bind(commandBuffer);
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
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gIBLRadianceTex", mIBLRadiance);

	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLFinalizeMat::Bind(CommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.PreintegratedEnvBrdfParam.Set(preintegratedBrdf);

	mIBLRadiance.Set(iblRadiance);

	RendererMaterial::Bind(commandBuffer);
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

void StandardDeferred::RenderLight(CommandBuffer& commandBuffer, LightType lightType, const RendererLight& light, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
{
	const auto& viewProps = view.GetProperties();

	bool isMSAA = view.GetProperties().Target.NumSamples > 1;
	SPtr<GpuBuffer> perViewBuffer = view.GetPerViewBuffer();

	light.GetParameters(mPerLightBuffer);

	if(lightType == LightType::Directional)
	{
		DeferredDirectionalLightMat* material = DeferredDirectionalLightMat::GetVariation(isMSAA, true);
		material->Bind(commandBuffer, gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

		GetRendererUtility().DrawScreenQuad();

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredDirectionalLightMat* msaaMaterial = DeferredDirectionalLightMat::GetVariation(true, false);
			msaaMaterial->Bind(commandBuffer, gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

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
		material->Bind(commandBuffer, gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		GetRendererUtility().Draw(stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredPointLightMat* msaaMaterial = DeferredPointLightMat::GetVariation(isInside, true, false);
			msaaMaterial->Bind(commandBuffer, gBufferInput, lightOcclusion, perViewBuffer, mPerLightBuffer);

			GetRendererUtility().Draw(stencilMesh);
		}
	}
}

void StandardDeferred::RenderReflProbe(CommandBuffer& commandBuffer, const ReflProbeData& probeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuBuffer>& reflProbeParams)
{
	const auto& viewProps = view.GetProperties();
	bool isMSAA = viewProps.Target.NumSamples > 1;

	SPtr<GpuBuffer> perViewBuffer = view.GetPerViewBuffer();

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
	material->Bind(commandBuffer, gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

	// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
	GetRendererUtility().Draw(stencilMesh);

	// Draw pixels requiring per-sample evaluation
	if(isMSAA)
	{
		DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::GetVariation(isInside, true, false);
		msaaMaterial->Bind(commandBuffer, gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		GetRendererUtility().Draw(stencilMesh);
	}
}
}} // namespace bs::ct
