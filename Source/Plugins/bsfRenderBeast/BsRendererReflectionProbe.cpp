//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererReflectionProbe.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Renderer/BsReflectionProbe.h"
#include "BsRenderBeast.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"

namespace bs { namespace ct {

static const u32 REFL_PROBE_BUFFER_INCREMENT = 16 * sizeof(ReflProbeData);

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
	bool supportsStructuredBuffers = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsStructuredBuffers)
	{
		u32 size = mNumProbes * sizeof(ReflProbeData);
		u32 curBufferSize;

		if(mProbeBuffer != nullptr)
			curBufferSize = mProbeBuffer->GetSize();
		else
			curBufferSize = 0;

		if(size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no probes, to avoid issues with null buffers
			u32 bufferSize = std::max(1, Math::CeilToInt(size / (float)REFL_PROBE_BUFFER_INCREMENT)) * REFL_PROBE_BUFFER_INCREMENT;

			GPU_BUFFER_DESC bufferDesc;
			bufferDesc.Type = GBT_STRUCTURED;
			bufferDesc.ElementCount = bufferSize / sizeof(ReflProbeData);
			bufferDesc.ElementSize = sizeof(ReflProbeData);
			bufferDesc.Format = BF_UNKNOWN;

			mProbeBuffer = GpuBuffer::Create(bufferDesc);
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

	const Transform& tfrm = Probe->GetTransform();
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

void ImageBasedLightingParams::Populate(const SPtr<GpuParams>& params, GpuProgramType programType, bool optional, bool gridIndices, bool probeArray)
{
	// Sky
	if(!optional || params->HasTexture(programType, "gSkyReflectionTex"))
		params->GetTextureParam(programType, "gSkyReflectionTex", SkyReflectionsTexParam);

	// Reflections
	if(!optional || params->HasTexture(programType, "gReflProbeCubemaps"))
	{
		params->GetTextureParam(programType, "gReflProbeCubemaps", ReflectionProbeCubemapsTexParam);

		if(probeArray)
			params->GetBufferParam(programType, "gReflectionProbes", ReflectionProbesParam);
	}

	if(!optional || params->HasTexture(programType, "gPreintegratedEnvBRDF"))
		params->GetTextureParam(programType, "gPreintegratedEnvBRDF", PreintegratedEnvBrdfParam);

	// AO
	if(params->HasTexture(programType, "gAmbientOcclusionTex"))
		params->GetTextureParam(programType, "gAmbientOcclusionTex", AmbientOcclusionTexParam);

	// SSR
	if(params->HasTexture(programType, "gSSRTex"))
		params->GetTextureParam(programType, "gSSRTex", SsrTexParam);

	if(gridIndices)
	{
		if(!optional || params->HasBuffer(programType, "gReflectionProbeIndices"))
			params->GetBufferParam(programType, "gReflectionProbeIndices", ReflectionProbeIndicesParam);
	}

	params->GetParamInfo()->GetBinding(
		programType,
		GpuPipelineParamInfoBase::ParamType::ParamBlock,
		"ReflProbeParams",
		ReflProbeParamBindings);

	params->GetParamInfo()->GetBinding(
		programType,
		GpuPipelineParamInfoBase::ParamType::ParamBlock,
		"ReflectionProbes",
		ReflProbesBinding);
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
			numSkyMips = filteredReflections->GetProperties().GetNumMipmaps() + 1;
			skyReflectionsAvailable = 1;
		}

		brightness = sky->GetBrightness();
	}

	gReflProbeParamsParamDef.gSkyCubemapNumMips.Set(Buffer, numSkyMips);
	gReflProbeParamsParamDef.gSkyCubemapAvailable.Set(Buffer, skyReflectionsAvailable);
	gReflProbeParamsParamDef.gNumProbes.Set(Buffer, numProbes);

	u32 numReflProbeMips = 0;
	if(reflectionCubemaps != nullptr)
		numReflProbeMips = reflectionCubemaps->GetProperties().GetNumMipmaps() + 1;

	gReflProbeParamsParamDef.gReflCubemapNumMips.Set(Buffer, numReflProbeMips);
	gReflProbeParamsParamDef.gUseReflectionMaps.Set(Buffer, capturingReflections ? 0 : 1);
	gReflProbeParamsParamDef.gSkyBrightness.Set(Buffer, brightness);
}

ReflProbesParamDef gReflProbesParamDef;
}} // namespace bs::ct
