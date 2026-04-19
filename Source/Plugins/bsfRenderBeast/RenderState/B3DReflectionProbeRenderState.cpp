//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DReflectionProbeRenderState.h"
#include "B3DRenderBeastScene.h"
#include "Material/B3DMaterial.h"
#include "Components/B3DReflectionProbe.h"
#include "B3DRenderBeast.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"

namespace b3d { namespace render {

static const u32 kReflProbeBufferIncrement = 16 * sizeof(ReflectioneProbeData);

GlobalReflectionProbeUniformBufferDefinition gGlobalReflectionProbeUniformBufferDefinition;

void VisibleReflectionProbeData::Update(const RenderBeastScene& scene, const RendererViewGroup& viewGroup)
{
	mReflProbeData.clear();

	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	// Generate refl. probe data for the visible ones
	u32 numProbes = scene.GetReflectionProbeCount();
	for(u32 i = 0; i < numProbes; i++)
	{
		if(!visibility.ReflProbes[i])
			continue;

		mReflProbeData.push_back(ReflectioneProbeData());
		const ReflectionProbeProxy& proxy = scene.GetReflectionProbeProxy(i);
		scene.GetReflectionProbeRenderState(i).GetParameters(proxy, mReflProbeData.back());
	}

	// Sort probes so bigger ones get accessed first, this way we overlay smaller ones on top of biggers ones when
	// rendering
	auto sorter = [](const ReflectioneProbeData& lhs, const ReflectioneProbeData& rhs)
	{
		return rhs.Radius < lhs.Radius;
	};

	std::sort(mReflProbeData.begin(), mReflProbeData.end(), sorter);

	mNumProbes = (u32)mReflProbeData.size();

	// Move refl. probe data into a GPU buffer
	bool supportsStructuredBuffers = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsStructuredBuffers)
	{
		u32 size = mNumProbes * sizeof(ReflectioneProbeData);
		u32 curBufferSize;

		if(mProbeBuffer != nullptr)
			curBufferSize = mProbeBuffer->GetTotalSize();
		else
			curBufferSize = 0;

		if(size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no probes, to avoid issues with null buffers
			u32 bufferSize = std::max(1, Math::CeilToInt(size / (float)kReflProbeBufferIncrement)) * kReflProbeBufferIncrement;

			GpuBufferCreateInformation bufferCreateInformation;
			bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
			bufferCreateInformation.StructuredStorage.Count = bufferSize / sizeof(ReflectioneProbeData);
			bufferCreateInformation.StructuredStorage.ElementSize = sizeof(ReflectioneProbeData);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			mProbeBuffer = gpuDevice->CreateGpuBuffer(bufferCreateInformation);
		}

		if(size > 0)
			GpuBufferUtility::Write(mProbeBuffer, 0, size, mReflProbeData.data(), GpuBufferWriteFlag::Discard);
	}
}

ReflectionProbeRenderState::ReflectionProbeRenderState()
{
	ArrayIdx = -1;
	ArrayDirty = true;
	ErrorFlagged = false;
}

void ReflectionProbeRenderState::GetParameters(const ReflectionProbeProxy& proxy, ReflectioneProbeData& output) const
{
	output.Type = proxy.GetType() == ReflectionProbeType::Sphere ? 0
		: proxy.GetType() == ReflectionProbeType::Box			? 1
																: 2;

	output.Position = proxy.GetWorldTransform().GetPosition();
	output.BoxExtents = proxy.GetWorldExtents();

	if(proxy.GetType() == ReflectionProbeType::Sphere)
		output.Radius = proxy.GetWorldRadius();
	else
		output.Radius = output.BoxExtents.Length();

	output.TransitionDistance = proxy.GetTransitionDistance();
	output.CubemapIdx = ArrayIdx;
	output.InvBoxTransform.SetInverseTrs(output.Position, proxy.GetWorldTransform().GetRotation(), output.BoxExtents);
}

void ImageBasedLightingParameterBinding::Initialize(const SPtr<GpuParameterSet>& parameters, GpuProgramType programType, bool optional, bool gridIndices, bool probeArray)
{
	// Sky
	if(!optional || parameters->HasSampledTexture(kSkyReflectionTextureName))
		parameters->GetSampledTextureParameter(kSkyReflectionTextureName, SkyReflectionsTexParam);

	// Reflections
	if(!optional || parameters->HasSampledTexture(kReflectionProbeCubemapsTextureName))
	{
		parameters->GetSampledTextureParameter(kReflectionProbeCubemapsTextureName, ReflectionProbeCubemapsTexParameter);

		if(probeArray)
			parameters->GetStorageBufferParameter(kReflectionProbesBufferName, ReflectionProbesParameter);
	}

	if(!optional || parameters->HasSampledTexture(kPreintegratedEnvBRDFTextureName))
		parameters->GetSampledTextureParameter(kPreintegratedEnvBRDFTextureName, PreintegratedEnvBrdfParameter);

	// AO
	if(parameters->HasSampledTexture(kAmbientOcclusionTextureName))
		parameters->GetSampledTextureParameter(kAmbientOcclusionTextureName, AmbientOcclusionTexParam);

	// SSR
	if(parameters->HasSampledTexture(kSSRTexName))
		parameters->GetSampledTextureParameter(kSSRTexName, SsrTexParameter);

	if(gridIndices)
	{
		if(!optional || parameters->HasStorageBuffer(kReflectionProbeIndicesBufferName))
			parameters->GetStorageBufferParameter(kReflectionProbeIndicesBufferName, ReflectionProbeIndicesParameter);
	}

	parameters->TryGetUniformBufferParameter(kPerProbeUniformBufferName, ReflectionProbeUniformBufferParameter);
	parameters->TryGetUniformBufferParameter(kGlobalReflectionProbeUniformBufferName, ReflectionProbesUniformBufferParameter);
}

void ImageBasedLightingParameterBinding::SetReflectionProbeCubemaps(const SPtr<GpuParameterSet>& parameters, const SPtr<Texture>& cubemaps, bool optional)
{
	if(!optional || parameters->HasSampledTexture(kReflectionProbeCubemapsTextureName))
		parameters->SetSampledTexture(kReflectionProbeCubemapsTextureName, cubemaps);
}

void ReflectionProbeRenderState::PopulateGlobalReflectionProbeUniformBuffer(const GpuBufferSuballocation& uniformBuffer, const Skybox* sky, u32 probeCount, const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
{
	float brightness = 1.0f;
	u32 skyReflectionsAvailable = 0;
	u32 skyMipCount = 0;

	if(sky != nullptr)
	{
		SPtr<Texture> filteredReflections = sky->GetFilteredRadiance();
		if(filteredReflections)
		{
			skyMipCount = filteredReflections->GetProperties().MipMapCount + 1;
			skyReflectionsAvailable = 1;
		}

		brightness = sky->GetBrightness();
	}

	GpuBufferMappedScope uniforms = uniformBuffer.Map();
	gGlobalReflectionProbeUniformBufferDefinition.gSkyCubemapNumMips.Set(uniforms, skyMipCount);
	gGlobalReflectionProbeUniformBufferDefinition.gSkyCubemapAvailable.Set(uniforms, skyReflectionsAvailable);
	gGlobalReflectionProbeUniformBufferDefinition.gNumProbes.Set(uniforms, probeCount);

	u32 reflecionProbeMipCount = 0;
	if(reflectionCubemaps != nullptr)
		reflecionProbeMipCount = reflectionCubemaps->GetProperties().MipMapCount + 1;

	gGlobalReflectionProbeUniformBufferDefinition.gReflCubemapNumMips.Set(uniforms, reflecionProbeMipCount);
	gGlobalReflectionProbeUniformBufferDefinition.gUseReflectionMaps.Set(uniforms, capturingReflections ? 0 : 1);
	gGlobalReflectionProbeUniformBufferDefinition.gSkyBrightness.Set(uniforms, brightness);
}

ReflProbesUniformDefinition gReflProbesUniformDefinition;
}} // namespace b3d::render
