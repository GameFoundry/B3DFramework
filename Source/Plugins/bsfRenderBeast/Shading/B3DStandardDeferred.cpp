//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DStandardDeferred.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Mesh/B3DMesh.h"
#include "Components/B3DSkybox.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRenderBeastScene.h"
#include "B3DRendererView.h"

namespace b3d {
namespace render {

PerLightParamDef gPerLightParamDef;

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
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSetupMat::Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuBuffer>& reflProbeParams)
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
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);

	mParamBuffer = gPerProbeParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("PerProbe", mParamBuffer);
}

void DeferredIBLProbeMat::Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuBuffer>& reflProbeParams)
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
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSkyMat::Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const Skybox* skybox, const SPtr<GpuBuffer>& reflProbeParams)
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
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetSampledTextureParameter("gIBLRadianceTex", mIBLRadiance);

	mIBLParams.Populate(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLFinalizeMat::Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const SPtr<GpuBuffer>& reflProbeParams)
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

StandardDeferred::LightBatches StandardDeferred::PrepareLightBatches(const TArrayView<const RendererLight*>& lights, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
{
	LightBatches batches;

	const auto& viewProperties = view.GetProperties();
	const bool isMSAA = viewProperties.Target.NumSamples > 1;

	// Group lights by material variation
	for(const RendererLight* light : lights)
	{
		const LightType lightType = light->Internal->GetType();

		// Determine material variation
		MaterialVariationKey key;
		key.Type = lightType;
		key.IsMSAA = isMSAA;
		key.IsInside = false;
		key.IsSingleSampleMSAA = false;

		// For point/spot lights, determine if viewer is inside
		if(lightType != LightType::Directional)
		{
			float distSqrd = (light->Internal->GetBounds().Center - viewProperties.ViewOrigin).SquaredLength();
			float boundRadius = light->Internal->GetBounds().Radius + viewProperties.NearPlane * 3.0f;
			key.IsInside = distSqrd < (boundRadius * boundRadius);
		}

		// Add light to the appropriate group
		LightBatch& batch = batches.Batches[key];

		BatchedLightInstance instance;
		instance.Light = light;
		instance.UniformBufferOffset = 0; // Will be set later

		batch.Lights.Add(instance);
	}

	// Get GPU device for alignment calculations
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const u32 uniformBlockStride = Math::CeilToMultiple(gPerLightParamDef.GetSize(), gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

	// For each group, create instanced buffers and GPU parameters
	for(auto& [key, batch] : batches.Batches)
	{
		const u32 lightCount = (u32)batch.Lights.size();

		// Create instanced uniform buffer
		batch.PerLightUniformBuffer = gPerLightParamDef.CreateBuffer(lightCount);
		batch.UniformStride = uniformBlockStride;

		// Write light parameters to buffer
		for(u32 lightIndex = 0; lightIndex < lightCount; lightIndex++)
		{
			batch.Lights[lightIndex].Light->PopulateUniformBuffer(batch.PerLightUniformBuffer, lightIndex);
			batch.Lights[lightIndex].UniformBufferOffset = lightIndex * uniformBlockStride;
		}

		// Get material for this group
		if(key.Type == LightType::Directional)
		{
			DeferredDirectionalLightMat* const lightMaterial = DeferredDirectionalLightMat::GetVariation(key.IsMSAA, true);
			batch.GpuParameters = lightMaterial->CreateGpuParameters();
		}
		else
		{
			DeferredPointLightMat* const lightMaterial = DeferredPointLightMat::GetVariation(key.IsInside, key.IsMSAA, true);
			batch.GpuParameters = lightMaterial->CreateGpuParameters();

			// Cache stencil mesh
			if(key.Type == LightType::Radial)
				batch.StencilMesh = RendererUtility::Instance().GetSphereStencil();
			else // Spot
				batch.StencilMesh = RendererUtility::Instance().GetSpotLightStencil();
		}

		// Bind shared resources to GpuParameters
		SPtr<GpuBuffer> perViewBuffer = view.GetPerViewBuffer();

		// Set uniform buffers
		batch.GpuParameters->SetUniformBuffer("PerCamera", perViewBuffer);
		batch.GpuParameters->SetUniformBuffer("PerLight", batch.PerLightUniformBuffer);

		if(batch.GpuParameters->HasSampledTexture("gLightOcclusionTex"))
			batch.GpuParameters->SetSampledTexture("gLightOcclusionTex", lightOcclusion);

		GBufferParameterBinding::Set(*gpuDevice, batch.GpuParameters, gBufferInput);

		// Get dynamic offset index
		batch.DynamicOffsetIndex = batch.GpuParameters->GetPipelineParameterInformation()->GetDynamicOffsetIndex("PerLight");
		B3D_ENSURE(batch.DynamicOffsetIndex != ~0u);
	}

	return batches;
}

void StandardDeferred::RenderLightBatches(GpuCommandBuffer& commandBuffer, const LightBatches& batches)
{
	// Render each group
	for(const auto& [key, batch] : batches.Batches)
	{
		// Set GpuParameters once for the group
		commandBuffer.SetGpuParameters(batch.GpuParameters);

		// Render each light in the group
		for(const BatchedLightInstance& lightInstance : batch.Lights)
		{
			// Set dynamic offset to select this light's parameters
			commandBuffer.SetDynamicBufferOffset(batch.DynamicOffsetIndex, lightInstance.UniformBufferOffset);

			// Render the light
			if(key.Type == LightType::Directional)
			{
				DeferredDirectionalLightMat* const lightMaterial = DeferredDirectionalLightMat::GetVariation(key.IsMSAA, true);
				lightMaterial->Bind(commandBuffer, false);
				GetRendererUtility().DrawScreenQuad(commandBuffer);

				// MSAA second pass
				if(key.IsMSAA)
				{
					DeferredDirectionalLightMat* const multiSampleLightMaterial = DeferredDirectionalLightMat::GetVariation(true, false);
					multiSampleLightMaterial->Bind(commandBuffer, false);
					GetRendererUtility().DrawScreenQuad(commandBuffer);
				}
			}
			else // Point or Spot
			{
				DeferredPointLightMat* const lightMaterial = DeferredPointLightMat::GetVariation(key.IsInside, key.IsMSAA, true);
				lightMaterial->Bind(commandBuffer, false);
				GetRendererUtility().Draw(commandBuffer, batch.StencilMesh);

				// MSAA second pass
				if(key.IsMSAA)
				{
					DeferredPointLightMat* multiSampleLightMaterial = DeferredPointLightMat::GetVariation(key.IsInside, true, false);
					multiSampleLightMaterial->Bind(commandBuffer, false);
					GetRendererUtility().Draw(commandBuffer, batch.StencilMesh);
				}
			}
		}
	}
}

void StandardDeferred::RenderLight(GpuCommandBuffer& commandBuffer, LightType lightType, const RendererLight& light, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion)
{
	LightBatches batches = PrepareLightBatches({ &light }, view, gBufferInput, lightOcclusion);
	RenderLightBatches(commandBuffer, batches);
}

void StandardDeferred::RenderReflProbe(GpuCommandBuffer& commandBuffer, const ReflProbeData& probeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuBuffer>& reflProbeParams)
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
	GetRendererUtility().Draw(commandBuffer, stencilMesh);

	// Draw pixels requiring per-sample evaluation
	if(isMSAA)
	{
		DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::GetVariation(isInside, true, false);
		msaaMaterial->Bind(commandBuffer, gBufferInput, perViewBuffer, sceneInfo, probeData, reflProbeParams);

		GetRendererUtility().Draw(commandBuffer, stencilMesh);
	}
}
}} // namespace b3d::render
