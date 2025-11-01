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
	mIBLParams.Initialize(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSetupMat::Prepare(const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.AmbientOcclusionTexParam.Set(ao);
	mIBLParams.SsrTexParam.Set(ssr);
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

void DeferredIBLProbeMat::PopulateParameters(GpuDevice& gpuDevice, const SPtr<GpuParameters>& gpuParameters, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SceneInfo& sceneInfo, const SPtr<GpuBuffer>& perProbeUniformBuffer, const SPtr<GpuBuffer>& globalProbeUniformBuffer)
{
	GBufferParameterBinding::Set(gpuDevice, gpuParameters, gBufferInput); 
	ImageBasedLightingParameterBinding::SetReflectionProbeCubemaps(gpuParameters, sceneInfo.ReflProbeCubemapsTex);

	gpuParameters->SetUniformBuffer("PerCamera", perCamera);
	gpuParameters->SetUniformBuffer("ReflProbeParams", globalProbeUniformBuffer);
	gpuParameters->SetUniformBuffer("PerProbe", perProbeUniformBuffer);
}

SPtr<GpuBuffer> DeferredIBLProbeMat::CreatePerProbeUniformBuffer(const ReflectioneProbeData& probeData)
{
	SPtr<GpuBuffer> buffer = gPerProbeParamDef.CreateBuffer();
	
	gPerProbeParamDef.gPosition.Set(buffer, probeData.Position);

	if(probeData.Type == 1)
		gPerProbeParamDef.gExtents.Set(buffer, probeData.BoxExtents);
	else
	{
		Vector3 extents(probeData.Radius, probeData.Radius, probeData.Radius);
		gPerProbeParamDef.gExtents.Set(buffer, extents);
	}

	gPerProbeParamDef.gTransitionDistance.Set(buffer, probeData.TransitionDistance);
	gPerProbeParamDef.gInvBoxTransform.Set(buffer, probeData.InvBoxTransform);
	gPerProbeParamDef.gCubemapIdx.Set(buffer, probeData.CubemapIdx);
	gPerProbeParamDef.gType.Set(buffer, probeData.Type);

	return buffer;
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
	mIBLParams.Initialize(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSkyMat::Prepare(const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const Skybox* skybox, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	if(skybox != nullptr)
		mIBLParams.SkyReflectionsTexParam.Set(skybox->GetFilteredRadiance());
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

	mIBLParams.Initialize(mGPUParameters, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLFinalizeMat::Prepare(const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const SPtr<GpuBuffer>& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);
	mGPUParameters->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.PreintegratedEnvBrdfParam.Set(preintegratedBrdf);

	mIBLRadiance.Set(iblRadiance);
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
		LightMaterialVariationKey key;
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

TArray<StandardDeferred::ReflectionProbeRenderInformation> StandardDeferred::PrepareReflectionProbes(GpuDevice& device, const VisibleReflectionProbeData& visibleReflectionProbeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuBuffer>& globalReflectionProbeUniformBuffer)
{
	TArray<ReflectionProbeRenderInformation> output;

	const auto& viewProperties = view.GetProperties();
	const SPtr<GpuBuffer>& perViewBuffer = view.GetPerViewBuffer();
	const bool isMSAA = viewProperties.Target.NumSamples > 1;

	const u32 probeCount = visibleReflectionProbeData.GetProbeCount();
	for(u32 probeIndex = 0; probeIndex < probeCount; probeIndex++)
	{
		const ReflectioneProbeData& probeData = visibleReflectionProbeData.GetProbeData(probeIndex);

		// When checking if viewer is inside the volume extend the bounds slighty to cover the case when the viewer is
		// outside, but the near plane is intersecting the bounds. We need to be conservative since the material for
		// rendering outside will not properly render the inside of the volume.
		float radiusBuffer = viewProperties.NearPlane * 3.0f;

		ReflectionProbeRenderInformation renderInformation;
		renderInformation.Type = probeData.Type;
		if(probeData.Type == 0) // Sphere
		{
			// Check if viewer is inside the light volume
			float distSqrd = (probeData.Position - viewProperties.ViewOrigin).SquaredLength();
			float boundRadius = probeData.Radius + radiusBuffer;

			renderInformation.IsViewerInside = distSqrd < (boundRadius * boundRadius);
		}
		else // Box
		{
			Vector3 extents = probeData.BoxExtents + radiusBuffer;
			AABox box(probeData.Position - extents, probeData.Position + extents);

			renderInformation.IsViewerInside = box.Contains(viewProperties.ViewOrigin);
		}

		SPtr<GpuBuffer> perProbeBuffer = DeferredIBLProbeMat::CreatePerProbeUniformBuffer(probeData);

		DeferredIBLProbeMat* material = DeferredIBLProbeMat::GetVariation(renderInformation.IsViewerInside, isMSAA, true);
		renderInformation.GpuParameters = material->CreateGpuParameters();
		DeferredIBLProbeMat::PopulateParameters(device, renderInformation.GpuParameters, gBufferInput, perViewBuffer, sceneInfo, perProbeBuffer, globalReflectionProbeUniformBuffer);

		output.Add(renderInformation);
	}

	return output;
}

void StandardDeferred::RenderReflectionProbes(GpuCommandBuffer& commandBuffer, const TArray<ReflectionProbeRenderInformation>& probeRenderInformation, const RendererView& view)
{
	const auto& viewProperties = view.GetProperties();
	const bool isMSAA = viewProperties.Target.NumSamples > 1;

	for(const auto& entry : probeRenderInformation)
	{
		SPtr<Mesh> stencilMesh;
		if(entry.Type == 0) // Sphere
			stencilMesh = RendererUtility::Instance().GetSphereStencil();
		else // Box
			stencilMesh = RendererUtility::Instance().GetBoxStencil();

		DeferredIBLProbeMat* const material = DeferredIBLProbeMat::GetVariation(entry.IsViewerInside, isMSAA, true);

		commandBuffer.SetGpuParameters(entry.GpuParameters);
		material->Bind(commandBuffer, false);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		GetRendererUtility().Draw(commandBuffer, stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredIBLProbeMat* msaaMaterial = DeferredIBLProbeMat::GetVariation(entry.IsViewerInside, true, false);
			msaaMaterial->Bind(commandBuffer, false);

			GetRendererUtility().Draw(commandBuffer, stencilMesh);
		}
	}
}
}} // namespace b3d::render
