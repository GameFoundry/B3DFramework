//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLightRenderState.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Components/B3DLight.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRenderBeast.h"
#include "B3DRenderBeastScene.h"
#include "Shading/B3DStandardDeferred.h"

namespace b3d {
namespace render {

static const u32 kLightDataBufferIncrement = 16 * sizeof(LightData);

void GetLightParameters(const LightProxy& proxy, LightData& output)
{
	Radian spotAngle = Math::Clamp(proxy.GetSpotAngle() * 0.5f, Degree(0), Degree(89));
	Radian spotFalloffAngle = Math::Clamp(proxy.GetSpotFalloffAngle() * 0.5f, Degree(0), (Degree)spotAngle);
	Color color = proxy.GetColor();

	const Transform& tfrm = proxy.GetWorldTransform();
	output.Position = tfrm.GetPosition();
	output.BoundsRadius = proxy.GetBounds().Radius;
	output.SrcRadius = proxy.GetSourceRadius();
	output.Direction = -tfrm.GetRotation().ZAxis();
	output.Luminance = proxy.GetLuminance();
	output.SpotAngles.X = spotAngle.GetValueInRadians();
	output.SpotAngles.Y = Math::Cos(output.SpotAngles.X);
	output.SpotAngles.Z = 1.0f / std::max(Math::Cos(spotFalloffAngle) - output.SpotAngles.Y, 0.001f);
	output.AttRadiusSqrdInv = 1.0f / (proxy.GetAttenuationRadius() * proxy.GetAttenuationRadius());
	output.Color = Vector3(color.R, color.G, color.B);

	// If directional lights, convert angular radius in degrees to radians
	if(proxy.GetType() == LightType::Directional)
		output.SrcRadius *= Math::kDeG2Rad;

	output.ShiftedLightPosition = GetShiftedLightPosition(proxy);
}

void PopulateLightUniformBuffer(const LightProxy& proxy, SPtr<GpuBuffer>& buffer, u32 index)
{
	LightData lightData;
	GetLightParameters(proxy, lightData);

	float type = 0.0f;
	switch(proxy.GetType())
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

	GpuBufferMappedScope uniforms = buffer->Map(GpuMapOption::Write);

	gPerLightUniformDefinition.gLightPositionAndSrcRadius.Set(uniforms, Vector4(lightData.Position, lightData.SrcRadius), 0, index);
	gPerLightUniformDefinition.gLightColorAndLuminance.Set(uniforms, Vector4(lightData.Color, lightData.Luminance), 0, index);
	gPerLightUniformDefinition.gLightSpotAnglesAndSqrdInvAttRadius.Set(uniforms, Vector4(lightData.SpotAngles, lightData.AttRadiusSqrdInv), 0, index);
	gPerLightUniformDefinition.gLightDirectionAndBoundRadius.Set(uniforms, Vector4(lightData.Direction, lightData.BoundsRadius), 0, index);
	gPerLightUniformDefinition.gShiftedLightPositionAndType.Set(uniforms, Vector4(lightData.ShiftedLightPosition, type), 0, index);

	Vector4 lightGeometry;
	lightGeometry.X = proxy.GetType() == LightType::Spot ? (float)RendererUtility::kSpotLightStencilSideCount : 0;
	lightGeometry.Y = (float)RendererUtility::kSpotLightStencilSliceCount;
	lightGeometry.Z = proxy.GetBounds().Radius;

	float extraRadius = lightData.SrcRadius / Math::Tan(lightData.SpotAngles.X * 0.5f);
	float coneRadius = Math::Sin(lightData.SpotAngles.X) * (proxy.GetAttenuationRadius() + extraRadius);
	lightGeometry.W = coneRadius;

	gPerLightUniformDefinition.gLightGeometry.Set(uniforms, lightGeometry, 0, index);

	const Transform& tfrm = proxy.GetWorldTransform();

	Quaternion lightRotation(kIdentityTag);
	lightRotation.LookRotation(-tfrm.GetRotation().ZAxis());

	Matrix4 transform = Matrix4::TRS(lightData.ShiftedLightPosition, lightRotation, Vector3::kOne);
	gPerLightUniformDefinition.gMatConeTransform.Set(uniforms, transform, 0, index);
}

Vector3 GetShiftedLightPosition(const LightProxy& proxy)
{
	const Transform& tfrm = proxy.GetWorldTransform();
	Vector3 direction = -tfrm.GetRotation().ZAxis();

	// Create position for fake attenuation for area spot lights (with disc center)
	if(proxy.GetType() == LightType::Spot)
		return tfrm.GetPosition() - direction * (proxy.GetSourceRadius() / Math::Tan(proxy.GetSpotAngle() * 0.5f));
	else
		return tfrm.GetPosition();
}

void GBufferParameterBinding::Initialize(GpuDevice& gpuDevice, GpuProgramType type, const SPtr<GpuParameterSet>& gpuParams)
{
	mParams = gpuParams;

	mParams->TryGetSampledTextureParameter(kAlbedoTextureName, mGBufferA);
	mParams->TryGetSampledTextureParameter(kNormalsTextureName, mGBufferB);
	mParams->TryGetSampledTextureParameter(kRoughMetalTextureName, mGBufferC);
	mParams->TryGetSampledTextureParameter(kDepthTextureName, mGBufferDepth);

	if(mParams->HasSamplerState(kDepthSamplerName))
	{
		GpuParameterSampler samplerStateParam;
		mParams->GetSamplerStateParameter(kDepthSamplerName, samplerStateParam);

		SamplerStateInformation desc;
		desc.MinFilter = FO_POINT;
		desc.MagFilter = FO_POINT;
		desc.MipFilter = FO_POINT;

		SPtr<SamplerState> ss = gpuDevice.FindOrCreateSamplerState(desc);
		samplerStateParam.Set(ss);
	}
}

void GBufferParameterBinding::Bind(const GBufferTextures& gbuffer)
{
	mGBufferA.Set(gbuffer.Albedo);
	mGBufferB.Set(gbuffer.Normals);
	mGBufferC.Set(gbuffer.RoughMetal);
	mGBufferDepth.Set(gbuffer.Depth);
}

void GBufferParameterBinding::Set(GpuDevice& gpuDevice, const SPtr<GpuParameterSet>& gpuParameters, const GBufferTextures& textures)
{
	if(gpuParameters->HasSampledTexture(kAlbedoTextureName))
		gpuParameters->SetSampledTexture(kAlbedoTextureName, textures.Albedo);

	if(gpuParameters->HasSampledTexture(kNormalsTextureName))
		gpuParameters->SetSampledTexture(kNormalsTextureName, textures.Normals);

	if(gpuParameters->HasSampledTexture(kRoughMetalTextureName))
		gpuParameters->SetSampledTexture(kRoughMetalTextureName, textures.RoughMetal);

	if(gpuParameters->HasSampledTexture(kDepthTextureName))
		gpuParameters->SetSampledTexture(kDepthTextureName, textures.Depth);

	if(gpuParameters->HasSamplerState(kDepthSamplerName))
	{
		GpuParameterSampler samplerStateParam;
		gpuParameters->GetSamplerStateParameter(kDepthSamplerName, samplerStateParam);

		SamplerStateInformation samplerStateInformation;
		samplerStateInformation.MinFilter = FO_POINT;
		samplerStateInformation.MagFilter = FO_POINT;
		samplerStateInformation.MipFilter = FO_POINT;

		SPtr<SamplerState> sampleState = gpuDevice.FindOrCreateSamplerState(samplerStateInformation);
		samplerStateParam.Set(sampleState);
	}
}

void ForwardLightingParams::Populate(const SPtr<GpuParameterSet>& params, bool clustered)
{
	if(clustered)
	{
		params->TryGetUniformBufferParameter("GridParams", GridUniformBufferParameter);
		params->TryGetStorageBufferParameter("gLights", LightsBufferParameter);
		params->TryGetStorageBufferParameter("gGridLightOffsetsAndSize", GridLightOffsetsAndSizeParameter);
		params->TryGetStorageBufferParameter("gLightIndices", GridLightIndicesParameter);
		params->TryGetStorageBufferParameter("gGridProbeOffsetsAndSize", GridProbeOffsetsAndSizeParameter);
	}
	else
	{
		params->TryGetUniformBufferParameter("Lights", LightsUniformBufferParameter);
		params->TryGetUniformBufferParameter("LightAndReflProbeParams", LightAndReflectionProbeUniformBufferParameter);
	}
}

VisibleLightData::VisibleLightData()
	: mLightCounts{}, mShadowedLightCounts{}
{}

void VisibleLightData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
{
	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mVisibleLights[i].clear();

	// Generate a list of visible light packed IDs
	TArrayView<const PackedRendererId> directionalLights = sceneInfo.GetDirectionalLights();
	for(u32 i = 0; i < (u32)directionalLights.size(); i++)
		mVisibleLights[(u32)LightType::Directional].push_back(directionalLights[i]);

	TArrayView<const PackedRendererId> radialLights = sceneInfo.GetRadialLights();
	for(u32 i = 0; i < (u32)radialLights.size(); i++)
	{
		if(!visibility.RadialLights[i])
			continue;

		mVisibleLights[(u32)LightType::Radial].push_back(radialLights[i]);
	}

	TArrayView<const PackedRendererId> spotLights = sceneInfo.GetSpotLights();
	for(u32 i = 0; i < (u32)spotLights.size(); i++)
	{
		if(!visibility.SpotLights[i])
			continue;

		mVisibleLights[(u32)LightType::Spot].push_back(spotLights[i]);
	}

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mLightCounts[i] = (u32)mVisibleLights[i].size();

	// Partition all visible lights so that unshadowed ones come first
	auto partition = [&sceneInfo](Vector<PackedRendererId>& entries)
	{
		u32 numUnshadowed = 0;
		int first = -1;
		for(u32 i = 0; i < (u32)entries.size(); ++i)
		{
			if(sceneInfo.GetLightProxy(entries[i]).GetCastsShadow())
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
				if(!sceneInfo.GetLightProxy(entries[i]).GetCastsShadow())
				{
					std::swap(entries[i], entries[first]);
					++numUnshadowed;
				}
			}
		}

		return numUnshadowed;
	};

	for(u32 i = 0; i < (u32)LightType::Count; i++)
		mShadowedLightCounts[i] = mLightCounts[i] - partition(mVisibleLights[i]);

	// Generate light data to initialize the GPU buffer with
	mVisibleLightData.clear();
	for(auto& lightsPerType : mVisibleLights)
	{
		for(PackedRendererId lightId : lightsPerType)
		{
			mVisibleLightData.push_back(LightData());
			GetLightParameters(sceneInfo.GetLightProxy(lightId), mVisibleLightData.back());
		}
	}

	bool supportsStructuredBuffers = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsStructuredBuffers)
	{
		u32 size = (u32)mVisibleLightData.size() * sizeof(LightData);
		u32 curBufferSize;

		if(mLightBuffer != nullptr)
			curBufferSize = mLightBuffer->GetTotalSize();
		else
			curBufferSize = 0;

		if(size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no lights, to avoid issues with null buffers
			u32 bufferSize = std::max(1, Math::CeilToInt(size / (float)kLightDataBufferIncrement)) * kLightDataBufferIncrement;

			GpuBufferCreateInformation bufferCreateInformation;
			bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
			bufferCreateInformation.StructuredStorage.Count = bufferSize / sizeof(LightData);
			bufferCreateInformation.StructuredStorage.ElementSize = sizeof(LightData);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			mLightBuffer = gpuDevice->CreateGpuBuffer(bufferCreateInformation);
		}

		if(size > 0)
			GpuBufferUtility::Write(mLightBuffer, 0, size, mVisibleLightData.data(), GpuBufferWriteFlag::Discard);
	}
}

void VisibleLightData::GatherInfluencingLights(const Bounds& bounds, const LightData* (&output)[kStandardForwardMaxNumLights], Vector3I& counts) const
{
	u32 outputIndices[kStandardForwardMaxNumLights];
	u32 numInfluencingLights = 0;

	u32 numDirLights = GetDirectionalLightCount();
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
			float distance = bounds.GetSphere().Center.SquaredDistance(lightData->Position);

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
	u32 spotLightIdx = GetDirectionalLightCount() + GetRadialLightCount();
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

LightsUniformDefinition gLightsUniformDefinition;
LightAndReflProbeParamsUniformDefinition gLightAndReflProbeParamsUniformDefinition;
}} // namespace b3d::render
