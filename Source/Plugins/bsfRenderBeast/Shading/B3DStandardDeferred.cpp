//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

PerLightUniformDefinition gPerLightUniformDefinition;

DeferredDirectionalLightMaterial* DeferredDirectionalLightMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
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

DeferredPointLightMaterial* DeferredPointLightMaterial::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
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

PerProbeUniformDefinition gPerProbeUniformDefinition;

void DeferredIBLSetupMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mIBLParams.Initialize(mGpuParameterSet, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSetupMaterial::Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const TShared<Texture>& ssr, const TShared<Texture>& ao, const GpuBufferSuballocation& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGpuParameterSet->SetUniformBuffer("PerCamera", perCamera);
	mGpuParameterSet->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.AmbientOcclusionTexParam.Set(ao);
	mIBLParams.SsrTexParameter.Set(ssr);
}

DeferredIBLSetupMaterial* DeferredIBLSetupMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
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

void DeferredIBLProbeMaterial::PopulateParameters(GpuDevice& gpuDevice, const TShared<GpuParameterSet>& gpuParameters, const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const RenderBeastScene& scene, const GpuBufferSuballocation& perProbeUniformBuffer, const GpuBufferSuballocation& globalProbeUniformBuffer)
{
	GBufferParameterBinding::Set(gpuDevice, gpuParameters, gBufferInput);
	ImageBasedLightingParameterBinding::SetReflectionProbeCubemaps(gpuParameters, scene.GetReflectionProbeCubemapsTex());

	gpuParameters->SetUniformBuffer("PerCamera", perCamera);
	gpuParameters->SetUniformBuffer("ReflProbeParams", globalProbeUniformBuffer);
	gpuParameters->SetUniformBuffer("PerProbe", perProbeUniformBuffer);
}

GpuBufferSuballocation DeferredIBLProbeMaterial::CreatePerProbeUniformBuffer(const ReflectioneProbeData& probeData)
{
	GpuBufferMappedScope uniforms = gPerProbeUniformDefinition.AllocateTransient().Map();

	gPerProbeUniformDefinition.gPosition.Set(uniforms, probeData.Position);

	if(probeData.Type == 1)
		gPerProbeUniformDefinition.gExtents.Set(uniforms, probeData.BoxExtents);
	else
	{
		Vector3 extents(probeData.Radius, probeData.Radius, probeData.Radius);
		gPerProbeUniformDefinition.gExtents.Set(uniforms, extents);
	}

	gPerProbeUniformDefinition.gTransitionDistance.Set(uniforms, probeData.TransitionDistance);
	gPerProbeUniformDefinition.gInvBoxTransform.Set(uniforms, probeData.InvBoxTransform);
	gPerProbeUniformDefinition.gCubemapIdx.Set(uniforms, probeData.CubemapIdx);
	gPerProbeUniformDefinition.gType.Set(uniforms, probeData.Type);

	return uniforms;
}

DeferredIBLProbeMaterial* DeferredIBLProbeMaterial::GetVariation(bool inside, bool msaa, bool singleSampleMSAA)
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

void DeferredIBLSkyMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mIBLParams.Initialize(mGpuParameterSet, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLSkyMaterial::Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const Skybox* skybox, const GpuBufferSuballocation& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGpuParameterSet->SetUniformBuffer("PerCamera", perCamera);
	mGpuParameterSet->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	if(skybox != nullptr)
		mIBLParams.SkyReflectionsTexParam.Set(skybox->GetFilteredRadiance());
}

DeferredIBLSkyMaterial* DeferredIBLSkyMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
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

void DeferredIBLFinalizeMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mGpuParameterSet->GetSampledTextureParameter("gIBLRadianceTex", mIBLRadiance);

	mIBLParams.Initialize(mGpuParameterSet, GPT_FRAGMENT_PROGRAM, true, false, false);
}

void DeferredIBLFinalizeMaterial::Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const TShared<Texture>& iblRadiance, const TShared<Texture>& preintegratedBrdf, const GpuBufferSuballocation& reflProbeParams)
{
	mGBufferParams.Bind(gBufferInput);

	mGpuParameterSet->SetUniformBuffer("PerCamera", perCamera);
	mGpuParameterSet->SetUniformBuffer("ReflProbeParams", reflProbeParams);

	mIBLParams.PreintegratedEnvBrdfParameter.Set(preintegratedBrdf);

	mIBLRadiance.Set(iblRadiance);
}

DeferredIBLFinalizeMaterial* DeferredIBLFinalizeMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
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

StandardDeferred::LightBatches StandardDeferred::PrepareLightBatches(TArrayView<const PackedRendererId> lights, const RenderBeastScene& scene, const RendererView& view, const GBufferTextures& gBufferInput, const TShared<Texture>& lightOcclusion)
{
	LightBatches batches;

	const auto& viewProperties = view.GetProperties();
	const bool isMSAA = viewProperties.Target.NumSamples > 1;

	// Group lights by material variation
	for(PackedRendererId lightId : lights)
	{
		const LightProxy& proxy = scene.GetLightProxy(lightId);
		const LightType lightType = proxy.GetType();

		// Determine material variation
		LightMaterialVariationKey key;
		key.Type = lightType;
		key.IsMSAA = isMSAA;
		key.IsInside = false;
		key.IsSingleSampleMSAA = false;

		// For point/spot lights, determine if viewer is inside
		if(lightType != LightType::Directional)
		{
			float distSqrd = (proxy.GetBounds().Center - viewProperties.ViewOrigin).SquaredLength();
			float boundRadius = proxy.GetBounds().Radius + viewProperties.NearPlane * 3.0f;
			key.IsInside = distSqrd < (boundRadius * boundRadius);
		}

		// Add light to the appropriate group
		LightBatch& batch = batches.Batches[key];

		BatchedLightInstance instance;
		instance.LightId = lightId;
		instance.UniformBufferOffset = 0; // Will be set later

		batch.Lights.Add(instance);
	}

	// Get GPU device for alignment calculations
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const u32 uniformBlockStride = Math::CeilToMultiple(gPerLightUniformDefinition.GetSize(), gpuDevice->GetCapabilities().MinimumUniformBufferOffsetAlignment);

	// For each group, create instanced buffers and GPU parameters
	for(auto& [key, batch] : batches.Batches)
	{
		const u32 lightCount = (u32)batch.Lights.size();

		// Create instanced uniform buffer
		batch.PerLightUniformBuffer = gPerLightUniformDefinition.CreateBuffer(lightCount);
		batch.UniformStride = uniformBlockStride;

		// Write light parameters to buffer
		for(u32 lightIndex = 0; lightIndex < lightCount; lightIndex++)
		{
			const LightProxy& lightProxy = scene.GetLightProxy(batch.Lights[lightIndex].LightId);
			PopulateLightUniformBuffer(lightProxy, batch.PerLightUniformBuffer, lightIndex);
			batch.Lights[lightIndex].UniformBufferOffset = lightIndex * uniformBlockStride;
		}

		// Get material for this group
		if(key.Type == LightType::Directional)
		{
			DeferredDirectionalLightMaterial* const lightMaterial = DeferredDirectionalLightMaterial::GetVariation(key.IsMSAA, true);
			batch.GpuParameters = lightMaterial->CreateGpuParameterSet();
		}
		else
		{
			DeferredPointLightMaterial* const lightMaterial = DeferredPointLightMaterial::GetVariation(key.IsInside, key.IsMSAA, true);
			batch.GpuParameters = lightMaterial->CreateGpuParameterSet();

			// Cache stencil mesh
			if(key.Type == LightType::Radial)
				batch.StencilMesh = RendererUtility::Instance().GetSphereStencil();
			else // Spot
				batch.StencilMesh = RendererUtility::Instance().GetSpotLightStencil();
		}

		// Bind shared resources to GpuParameters
		const GpuBufferSuballocation& perViewBuffer = view.GetPerViewBuffer();

		// Set uniform buffers
		batch.GpuParameters->SetUniformBuffer("PerCamera", perViewBuffer);
		batch.GpuParameters->SetUniformBuffer("PerLight", batch.PerLightUniformBuffer);

		if(batch.GpuParameters->HasSampledTexture("gLightOcclusionTex"))
			batch.GpuParameters->SetSampledTexture("gLightOcclusionTex", lightOcclusion);

		GBufferParameterBinding::Set(*gpuDevice, batch.GpuParameters, gBufferInput);

		// Get dynamic offset index
		batch.DynamicOffsetIndex = batch.GpuParameters->GetLayout()->GetDynamicOffsetIndex("PerLight");
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
		commandBuffer.SetGpuParameterSet(batch.GpuParameters);

		// Render each light in the group
		const u32 set = batch.GpuParameters->GetSet();
		for(const BatchedLightInstance& lightInstance : batch.Lights)
		{
			// Set dynamic offset to select this light's parameters
			commandBuffer.SetDynamicBufferOffset(set, batch.DynamicOffsetIndex, lightInstance.UniformBufferOffset);

			// Render the light
			if(key.Type == LightType::Directional)
			{
				DeferredDirectionalLightMaterial* const lightMaterial = DeferredDirectionalLightMaterial::GetVariation(key.IsMSAA, true);
				lightMaterial->Bind(commandBuffer, false);
				GetRendererUtility().DrawScreenQuad(commandBuffer);

				// MSAA second pass
				if(key.IsMSAA)
				{
					DeferredDirectionalLightMaterial* const multiSampleLightMaterial = DeferredDirectionalLightMaterial::GetVariation(true, false);
					multiSampleLightMaterial->Bind(commandBuffer, false);
					GetRendererUtility().DrawScreenQuad(commandBuffer);
				}
			}
			else // Point or Spot
			{
				DeferredPointLightMaterial* const lightMaterial = DeferredPointLightMaterial::GetVariation(key.IsInside, key.IsMSAA, true);
				lightMaterial->Bind(commandBuffer, false);
				GetRendererUtility().Draw(commandBuffer, batch.StencilMesh);

				// MSAA second pass
				if(key.IsMSAA)
				{
					DeferredPointLightMaterial* multiSampleLightMaterial = DeferredPointLightMaterial::GetVariation(key.IsInside, true, false);
					multiSampleLightMaterial->Bind(commandBuffer, false);
					GetRendererUtility().Draw(commandBuffer, batch.StencilMesh);
				}
			}
		}
	}
}

TArray<StandardDeferred::ReflectionProbeRenderInformation> StandardDeferred::PrepareReflectionProbes(GpuDevice& device, const VisibleReflectionProbeData& visibleReflectionProbeData, const RendererView& view, const GBufferTextures& gBufferInput, const RenderBeastScene& scene, const GpuBufferSuballocation& globalReflectionProbeUniformBuffer)
{
	TArray<ReflectionProbeRenderInformation> output;

	const auto& viewProperties = view.GetProperties();
	const GpuBufferSuballocation& perViewBuffer = view.GetPerViewBuffer();
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

		GpuBufferSuballocation perProbeBuffer = DeferredIBLProbeMaterial::CreatePerProbeUniformBuffer(probeData);

		DeferredIBLProbeMaterial* material = DeferredIBLProbeMaterial::GetVariation(renderInformation.IsViewerInside, isMSAA, true);
		renderInformation.GpuParameters = material->CreateGpuParameterSet();
		DeferredIBLProbeMaterial::PopulateParameters(device, renderInformation.GpuParameters, gBufferInput, perViewBuffer, scene, perProbeBuffer, globalReflectionProbeUniformBuffer);

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
		TShared<Mesh> stencilMesh;
		if(entry.Type == 0) // Sphere
			stencilMesh = RendererUtility::Instance().GetSphereStencil();
		else // Box
			stencilMesh = RendererUtility::Instance().GetBoxStencil();

		DeferredIBLProbeMaterial* const material = DeferredIBLProbeMaterial::GetVariation(entry.IsViewerInside, isMSAA, true);

		commandBuffer.SetGpuParameterSet(entry.GpuParameters);
		material->Bind(commandBuffer, false);

		// Note: If MSAA is enabled this will be rendered multisampled (on polygon edges), see if this can be avoided
		GetRendererUtility().Draw(commandBuffer, stencilMesh);

		// Draw pixels requiring per-sample evaluation
		if(isMSAA)
		{
			DeferredIBLProbeMaterial* msaaMaterial = DeferredIBLProbeMaterial::GetVariation(entry.IsViewerInside, true, false);
			msaaMaterial->Bind(commandBuffer, false);

			GetRendererUtility().Draw(commandBuffer, stencilMesh);
		}
	}
}
}} // namespace b3d::render
