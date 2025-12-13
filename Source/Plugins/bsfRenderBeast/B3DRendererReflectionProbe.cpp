//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererReflectionProbe.h"
#include "Material/B3DMaterial.h"
#include "Components/B3DReflectionProbe.h"
#include "B3DRenderBeast.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"

namespace b3d { namespace render {

static const u32 kReflProbeBufferIncrement = 16 * sizeof(ReflectioneProbeData);

GlobalReflectionProbeUniformBufferDefinition gGlobalReflectionProbeUniformBufferDefinition;

void VisibleReflectionProbeData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
{
	mReflProbeData.clear();

	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	// Generate refl. probe data for the visible ones
	u32 numProbes = (u32)sceneInfo.ReflProbes.size();
	for(u32 i = 0; i < numProbes; i++)
	{
		if(!visibility.ReflProbes[i])
			continue;

		mReflProbeData.push_back(ReflectioneProbeData());
		sceneInfo.ReflProbes[i].GetParameters(mReflProbeData.back());
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

RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
	: Probe(probe)
{
	ArrayIdx = -1;
	ArrayDirty = true;
	ErrorFlagged = false;
}

void RendererReflectionProbe::GetParameters(ReflectioneProbeData& output) const
{
	output.Type = Probe->GetType() == ReflectionProbeType::Sphere ? 0
		: Probe->GetType() == ReflectionProbeType::Box			  ? 1
																  : 2;

	const Transform& tfrm = Probe->GetWorldTransform();
	output.Position = tfrm.GetPosition();
	output.BoxExtents = Probe->GetExtents();

	if(Probe->GetType() == ReflectionProbeType::Sphere)
		output.Radius = Probe->GetRadius();
	else
		output.Radius = output.BoxExtents.Length();

	output.TransitionDistance = Probe->GetTransitionDistance();
	output.CubemapIdx = ArrayIdx;
	output.InvBoxTransform.SetInverseTrs(output.Position, tfrm.GetRotation(), output.BoxExtents);
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

void RendererReflectionProbe::PopulateGlobalReflectionProbeUniformBuffer(const GpuBufferSuballocation& uniformBuffer, const Skybox* sky, u32 numProbes, const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
{
	float brightness = 1.0f;
	u32 skyReflectionsAvailable = 0;
	u32 numSkyMips = 0;

	if(sky != nullptr)
	{
		SPtr<Texture> filteredReflections = sky->GetFilteredRadiance();
		if(filteredReflections)
		{
			numSkyMips = filteredReflections->GetProperties().MipMapCount + 1;
			skyReflectionsAvailable = 1;
		}

		brightness = sky->GetBrightness();
	}

	GpuBufferMappedScope uniforms = uniformBuffer.Map();
	gGlobalReflectionProbeUniformBufferDefinition.gSkyCubemapNumMips.Set(uniforms, numSkyMips);
	gGlobalReflectionProbeUniformBufferDefinition.gSkyCubemapAvailable.Set(uniforms, skyReflectionsAvailable);
	gGlobalReflectionProbeUniformBufferDefinition.gNumProbes.Set(uniforms, numProbes);

	u32 numReflProbeMips = 0;
	if(reflectionCubemaps != nullptr)
		numReflProbeMips = reflectionCubemaps->GetProperties().MipMapCount + 1;

	gGlobalReflectionProbeUniformBufferDefinition.gReflCubemapNumMips.Set(uniforms, numReflProbeMips);
	gGlobalReflectionProbeUniformBufferDefinition.gUseReflectionMaps.Set(uniforms, capturingReflections ? 0 : 1);
	gGlobalReflectionProbeUniformBufferDefinition.gSkyBrightness.Set(uniforms, brightness);
}

ReflProbesUniformDefinition gReflProbesUniformDefinition;
}} // namespace b3d::render
