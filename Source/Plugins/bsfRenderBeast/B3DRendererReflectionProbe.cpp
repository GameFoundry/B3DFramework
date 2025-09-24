//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererReflectionProbe.h"
#include "Material/B3DMaterial.h"
#include "Components/B3DReflectionProbe.h"
#include "B3DRenderBeast.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"

namespace b3d { namespace render {

static const u32 kReflProbeBufferIncrement = 16 * sizeof(ReflProbeData);

ReflProbeParamsParamDef gReflProbeParamsParamDef;

void VisibleReflProbeData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
{
	mReflProbeData.clear();

	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	// Generate refl. probe data for the visible ones
	u32 numProbes = (u32)sceneInfo.ReflProbes.size();
	for(u32 i = 0; i < numProbes; i++)
	{
		if(!visibility.ReflProbes[i])
			continue;

		mReflProbeData.push_back(ReflProbeData());
		sceneInfo.ReflProbes[i].GetParameters(mReflProbeData.back());
	}

	// Sort probes so bigger ones get accessed first, this way we overlay smaller ones on top of biggers ones when
	// rendering
	auto sorter = [](const ReflProbeData& lhs, const ReflProbeData& rhs)
	{
		return rhs.Radius < lhs.Radius;
	};

	std::sort(mReflProbeData.begin(), mReflProbeData.end(), sorter);

	mNumProbes = (u32)mReflProbeData.size();

	// Move refl. probe data into a GPU buffer
	bool supportsStructuredBuffers = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsStructuredBuffers)
	{
		u32 size = mNumProbes * sizeof(ReflProbeData);
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
			bufferCreateInformation.StructuredStorage.Count = bufferSize / sizeof(ReflProbeData);
			bufferCreateInformation.StructuredStorage.ElementSize = sizeof(ReflProbeData);

			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			mProbeBuffer = gpuDevice->CreateGpuBuffer(bufferCreateInformation);
		}

		if(size > 0)
			mProbeBuffer->WriteData(0, size, mReflProbeData.data(), BWT_DISCARD);
	}
}

RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
	: Probe(probe)
{
	ArrayIdx = -1;
	ArrayDirty = true;
	ErrorFlagged = false;
}

void RendererReflectionProbe::GetParameters(ReflProbeData& output) const
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

void ImageBasedLightingParams::Populate(const SPtr<GpuParameters>& params, GpuProgramType programType, bool optional, bool gridIndices, bool probeArray)
{
	// Sky
	if(!optional || params->HasSampledTexture("gSkyReflectionTex"))
		params->GetSampledTextureParameter("gSkyReflectionTex", SkyReflectionsTexParam);

	// Reflections
	if(!optional || params->HasSampledTexture("gReflProbeCubemaps"))
	{
		params->GetSampledTextureParameter("gReflProbeCubemaps", ReflectionProbeCubemapsTexParam);

		if(probeArray)
			params->GetStorageBufferParameter("gReflectionProbes", ReflectionProbesParam);
	}

	if(!optional || params->HasSampledTexture("gPreintegratedEnvBRDF"))
		params->GetSampledTextureParameter("gPreintegratedEnvBRDF", PreintegratedEnvBrdfParam);

	// AO
	if(params->HasSampledTexture("gAmbientOcclusionTex"))
		params->GetSampledTextureParameter("gAmbientOcclusionTex", AmbientOcclusionTexParam);

	// SSR
	if(params->HasSampledTexture("gSSRTex"))
		params->GetSampledTextureParameter("gSSRTex", SsrTexParam);

	if(gridIndices)
	{
		if(!optional || params->HasStorageBuffer("gReflectionProbeIndices"))
			params->GetStorageBufferParameter("gReflectionProbeIndices", ReflectionProbeIndicesParam);
	}

	params->GetPipelineParameterInformation()->GetBinding("ReflProbeParams", ReflProbeParamBindings);

	params->GetPipelineParameterInformation()->GetBinding("ReflectionProbes", ReflProbesBinding);
}

ReflProbeParamBuffer::ReflProbeParamBuffer()
{
	Buffer = gReflProbeParamsParamDef.CreateBuffer();
}

void ReflProbeParamBuffer::Populate(const Skybox* sky, u32 numProbes, const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
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

	gReflProbeParamsParamDef.gSkyCubemapNumMips.Set(Buffer, numSkyMips);
	gReflProbeParamsParamDef.gSkyCubemapAvailable.Set(Buffer, skyReflectionsAvailable);
	gReflProbeParamsParamDef.gNumProbes.Set(Buffer, numProbes);

	u32 numReflProbeMips = 0;
	if(reflectionCubemaps != nullptr)
		numReflProbeMips = reflectionCubemaps->GetProperties().MipMapCount + 1;

	gReflProbeParamsParamDef.gReflCubemapNumMips.Set(Buffer, numReflProbeMips);
	gReflProbeParamsParamDef.gUseReflectionMaps.Set(Buffer, capturingReflections ? 0 : 1);
	gReflProbeParamsParamDef.gSkyBrightness.Set(Buffer, brightness);
}

ReflProbesParamDef gReflProbesParamDef;
}} // namespace b3d::render
