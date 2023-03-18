//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererLight.h"
#include "Material/BsMaterial.h"
#include "Material/BsGpuParamsSet.h"
#include "RenderAPI/BsGenericGpuBuffer.h"
#include "RenderAPI/BsGpuParams.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRenderBeast.h"
#include "Shading/BsStandardDeferred.h"

namespace bs {
namespace ct {

static const u32 kLightDataBufferIncrement = 16 * sizeof(LightData);

RendererLight::RendererLight(Light* light)
	: Internal(light)
{}

void RendererLight::GetParameters(LightData& output) const
{
	Radian spotAngle = Math::Clamp(Internal->GetSpotAngle() * 0.5f, Degree(0), Degree(89));
	Radian spotFalloffAngle = Math::Clamp(Internal->GetSpotFalloffAngle() * 0.5f, Degree(0), (Degree)spotAngle);
	Color color = Internal->GetColor();

	const Transform& tfrm = Internal->GetTransform();
	output.Position = tfrm.GetPosition();
	output.BoundsRadius = Internal->GetBounds().GetRadius();
	output.SrcRadius = Internal->GetSourceRadius();
	output.Direction = -tfrm.GetRotation().ZAxis();
	output.Luminance = Internal->GetLuminance();
	output.SpotAngles.X = spotAngle.GetValueInRadians();
	output.SpotAngles.Y = Math::Cos(output.SpotAngles.X);
	output.SpotAngles.Z = 1.0f / std::max(Math::Cos(spotFalloffAngle) - output.SpotAngles.Y, 0.001f);
	output.AttRadiusSqrdInv = 1.0f / (Internal->GetAttenuationRadius() * Internal->GetAttenuationRadius());
	output.Color = Vector3(color.R, color.G, color.B);

	// If directional lights, convert angular radius in degrees to radians
	if(Internal->GetType() == LightType::Directional)
		output.SrcRadius *= Math::kDeG2Rad;

	output.ShiftedLightPosition = GetShiftedLightPosition();
}

void RendererLight::GetParameters(SPtr<GpuParamBlockBuffer>& buffer) const
{
	LightData lightData;
	GetParameters(lightData);

	float type = 0.0f;
	switch(Internal->GetType())
	{
	case LightType::Directional:
		type = 0;
		break;
	case LightType::Radial:
		type = 0.3f;
		break;
	case LightType::Spot:
		type = 0.8f;
		break;
	default:
		break;
	}

	gPerLightParamDef.gLightPositionAndSrcRadius.Set(buffer, Vector4(lightData.Position, lightData.SrcRadius));
	gPerLightParamDef.gLightColorAndLuminance.Set(buffer, Vector4(lightData.Color, lightData.Luminance));
	gPerLightParamDef.gLightSpotAnglesAndSqrdInvAttRadius.Set(buffer, Vector4(lightData.SpotAngles, lightData.AttRadiusSqrdInv));
	gPerLightParamDef.gLightDirectionAndBoundRadius.Set(buffer, Vector4(lightData.Direction, lightData.BoundsRadius));
	gPerLightParamDef.gShiftedLightPositionAndType.Set(buffer, Vector4(lightData.ShiftedLightPosition, type));

	Vector4 lightGeometry;
	lightGeometry.X = Internal->GetType() == LightType::Spot ? (float)Light::kLightConeNumSides : 0;
	lightGeometry.Y = (float)Light::kLightConeNumSlices;
	lightGeometry.Z = Internal->GetBounds().GetRadius();

	float extraRadius = lightData.SrcRadius / Math::Tan(lightData.SpotAngles.X * 0.5f);
	float coneRadius = Math::Sin(lightData.SpotAngles.X) * (Internal->GetAttenuationRadius() + extraRadius);
	lightGeometry.W = coneRadius;

	gPerLightParamDef.gLightGeometry.Set(buffer, lightGeometry);

	const Transform& tfrm = Internal->GetTransform();

	Quaternion lightRotation(BsIdentity);
	lightRotation.LookRotation(-tfrm.GetRotation().ZAxis());

	Matrix4 transform = Matrix4::TRS(lightData.ShiftedLightPosition, lightRotation, Vector3::kOne);
	gPerLightParamDef.gMatConeTransform.Set(buffer, transform);
}

Vector3 RendererLight::GetShiftedLightPosition() const
{
	const Transform& tfrm = Internal->GetTransform();
	Vector3 direction = -tfrm.GetRotation().ZAxis();

	// Create position for fake attenuation for area spot lights (with disc center)
	if(Internal->GetType() == LightType::Spot)
		return tfrm.GetPosition() - direction * (Internal->GetSourceRadius() / Math::Tan(Internal->GetSpotAngle() * 0.5f));
	else
		return tfrm.GetPosition();
}

void GBufferParams::Initialize(GpuProgramType type, const SPtr<GpuParams>& gpuParams)
{
	mParams = gpuParams;

	if(mParams->HasSampledTexture(type, "gGBufferATex"))
		mParams->GetSampledTextureParameter(type, "gGBufferATex", mGBufferA);

	if(mParams->HasSampledTexture(type, "gGBufferBTex"))
		mParams->GetSampledTextureParameter(type, "gGBufferBTex", mGBufferB);

	if(mParams->HasSampledTexture(type, "gGBufferCTex"))
		mParams->GetSampledTextureParameter(type, "gGBufferCTex", mGBufferC);

	if(mParams->HasSampledTexture(type, "gDepthBufferTex"))
		mParams->GetSampledTextureParameter(type, "gDepthBufferTex", mGBufferDepth);

	if(mParams->HasSamplerState(type, "gDepthBufferSamp"))
	{
		GpuParameterSampler samplerStateParam;
		mParams->GetSamplerStateParameter(type, "gDepthBufferSamp", samplerStateParam);

		SamplerStateInformation desc;
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_POINT;

		SPtr<SamplerState> ss = SamplerState::Create(desc);
		samplerStateParam.Set(ss);
	}
}

void GBufferParams::Bind(const GBufferTextures& gbuffer)
{
	mGBufferA.Set(gbuffer.Albedo);
	mGBufferB.Set(gbuffer.Normals);
	mGBufferC.Set(gbuffer.RoughMetal);
	mGBufferDepth.Set(gbuffer.Depth);
}

void ForwardLightingParams::Populate(const SPtr<GpuParams>& params, bool clustered)
{
	if(clustered)
	{
		params->GetPipelineParameterInformation()->GetBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"GridParams",
			GridParamsBindings);

		if(params->HasStorageBuffer(GPT_FRAGMENT_PROGRAM, "gLights"))
			params->GetStorageBufferParameter(GPT_FRAGMENT_PROGRAM, "gLights", LightsBufferParam);

		if(params->HasStorageBuffer(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize"))
			params->GetStorageBufferParameter(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize", GridLightOffsetsAndSizeParam);

		if(params->HasStorageBuffer(GPT_FRAGMENT_PROGRAM, "gLightIndices"))
			params->GetStorageBufferParameter(GPT_FRAGMENT_PROGRAM, "gLightIndices", GridLightIndicesParam);

		if(params->HasStorageBuffer(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize"))
			params->GetStorageBufferParameter(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize", GridProbeOffsetsAndSizeParam);
	}
	else
	{
		params->GetPipelineParameterInformation()->GetBinding(
			GPT_FRAGMENT_PROGRAM,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"Lights",
			LightsParamBlockBinding);

		params->GetPipelineParameterInformation()->GetBinding(
			GPT_FRAGMENT_PROGRAM,
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"LightAndReflProbeParams",
			LightAndReflProbeParamsParamBlockBinding);
	}
}

VisibleLightData::VisibleLightData()
	: mNumLights{}, mNumShadowedLights{}
{}

void VisibleLightData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
{
	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mVisibleLights[i].clear();

	// Generate a list of lights and their GPU buffers
	u32 numDirLights = (u32)sceneInfo.DirectionalLights.size();
	for(u32 i = 0; i < numDirLights; i++)
		mVisibleLights[(u32)LightType::Directional].push_back(&sceneInfo.DirectionalLights[i]);

	u32 numRadialLights = (u32)sceneInfo.RadialLights.size();
	for(u32 i = 0; i < numRadialLights; i++)
	{
		if(!visibility.RadialLights[i])
			continue;

		mVisibleLights[(u32)LightType::Radial].push_back(&sceneInfo.RadialLights[i]);
	}

	u32 numSpotLights = (u32)sceneInfo.SpotLights.size();
	for(u32 i = 0; i < numSpotLights; i++)
	{
		if(!visibility.SpotLights[i])
			continue;

		mVisibleLights[(u32)LightType::Spot].push_back(&sceneInfo.SpotLights[i]);
	}

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mNumLights[i] = (u32)mVisibleLights[i].size();

	// Partition all visible lights so that unshadowed ones come first
	auto partition = [](Vector<const RendererLight*>& entries)
	{
		u32 numUnshadowed = 0;
		int first = -1;
		for(u32 i = 0; i < (u32)entries.size(); ++i)
		{
			if(entries[i]->Internal->GetCastsShadow())
			{
				first = i;
				break;
			}
			else
				++numUnshadowed;
		}

		if(first != -1)
		{
			for(u32 i = first + 1; i < (u32)entries.size(); ++i)
			{
				if(!entries[i]->Internal->GetCastsShadow())
				{
					std::swap(entries[i], entries[first]);
					++numUnshadowed;
				}
			}
		}

		return numUnshadowed;
	};

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mNumShadowedLights[i] = mNumLights[i] - partition(mVisibleLights[i]);

	// Generate light data to initialize the GPU buffer with
	mVisibleLightData.clear();
	for(auto& lightsPerType : mVisibleLights)
	{
		for(auto& entry : lightsPerType)
		{
			mVisibleLightData.push_back(LightData());
			entry->GetParameters(mVisibleLightData.back());
		}
	}

	bool supportsStructuredBuffers = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsStructuredBuffers)
	{
		u32 size = (u32)mVisibleLightData.size() * sizeof(LightData);
		u32 curBufferSize;

		if(mLightBuffer != nullptr)
			curBufferSize = mLightBuffer->GetSize();
		else
			curBufferSize = 0;

		if(size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no lights, to avoid issues with null buffers
			u32 bufferSize = std::max(1, Math::CeilToInt(size / (float)kLightDataBufferIncrement)) * kLightDataBufferIncrement;

			GenericGpuBufferCreateInformation bufferDesc;
			bufferDesc.Type = GBT_STRUCTURED;
			bufferDesc.ElementCount = bufferSize / sizeof(LightData);
			bufferDesc.ElementSize = sizeof(LightData);
			bufferDesc.Format = BF_UNKNOWN;

			mLightBuffer = GenericGpuBuffer::Create(bufferDesc);
		}

		if(size > 0)
			mLightBuffer->WriteData(0, size, mVisibleLightData.data(), BWT_DISCARD);
	}
}

void VisibleLightData::GatherInfluencingLights(const Bounds& bounds, const LightData* (&output)[kStandardForwardMaxNumLights], Vector3I& counts) const
{
	u32 outputIndices[kStandardForwardMaxNumLights];
	u32 numInfluencingLights = 0;

	u32 numDirLights = GetNumDirLights();
	for(u32 i = 0; i < numDirLights; i++)
	{
		if(numInfluencingLights >= kStandardForwardMaxNumLights)
			return;

		outputIndices[numInfluencingLights] = i;
		numInfluencingLights++;
	}

	u32 pointLightOffset = numInfluencingLights;

	float distances[kStandardForwardMaxNumLights];
	for(u32 i = 0; i < kStandardForwardMaxNumLights; i++)
		distances[i] = std::numeric_limits<float>::max();

	// Note: This is an ad-hoc way of evaluating light influence, a better way might be wanted
	u32 numLights = (u32)mVisibleLightData.size();
	u32 furthestLightIdx = (u32)-1;
	float furthestDistance = 0.0f;
	for(u32 j = numDirLights; j < numLights; j++)
	{
		const LightData* lightData = &mVisibleLightData[j];

		Sphere lightSphere(lightData->Position, lightData->BoundsRadius);
		if(bounds.GetSphere().Intersects(lightSphere))
		{
			float distance = bounds.GetSphere().GetCenter().SquaredDistance(lightData->Position);

			// See where in the array can we fit the light
			if(numInfluencingLights < kStandardForwardMaxNumLights)
			{
				outputIndices[numInfluencingLights] = j;
				distances[numInfluencingLights] = distance;

				if(distance > furthestDistance)
				{
					furthestLightIdx = numInfluencingLights;
					furthestDistance = distance;
				}

				numInfluencingLights++;
			}
			else if(distance < furthestDistance)
			{
				outputIndices[furthestLightIdx] = j;
				distances[furthestLightIdx] = distance;

				furthestDistance = distance;
				for(u32 k = 0; k < kStandardForwardMaxNumLights; k++)
				{
					if(distances[k] > furthestDistance)
					{
						furthestDistance = distances[k];
						furthestLightIdx = k;
					}
				}
			}
		}
	}

	// Output actual light data, sorted by type
	counts = Vector3I(0, 0, 0);

	for(u32 i = 0; i < pointLightOffset; i++)
	{
		output[i] = &mVisibleLightData[outputIndices[i]];
		counts.X += 1;
	}

	u32 outputIdx = pointLightOffset;
	u32 spotLightIdx = GetNumDirLights() + GetNumRadialLights();
	for(u32 i = pointLightOffset; i < numInfluencingLights; i++)
	{
		bool isSpot = outputIndices[i] >= spotLightIdx;
		if(isSpot)
			continue;

		output[outputIdx++] = &mVisibleLightData[outputIndices[i]];
		counts.Y += 1;
	}

	for(u32 i = pointLightOffset; i < numInfluencingLights; i++)
	{
		bool isSpot = outputIndices[i] >= spotLightIdx;
		if(!isSpot)
			continue;

		output[outputIdx++] = &mVisibleLightData[outputIndices[i]];
		counts.Z += 1;
	}
}

LightsParamDef gLightsParamDef;
LightAndReflProbeParamsParamDef gLightAndReflProbeParamsParamDef;
}} // namespace bs::ct
