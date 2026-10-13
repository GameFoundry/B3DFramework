//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DStandardDeferred.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Mesh/B3DMesh.h"
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
}} // namespace b3d::render
