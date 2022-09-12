//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererLight.h"
#include "Material/BsMaterial.h"
#include "Material/BsGpuParamsSet.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuParams.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRenderBeast.h"
#include "Shading/BsStandardDeferred.h"

namespace bs { namespace ct
{
	static const UINT32 LIGHT_DATA_BUFFER_INCREMENT = 16 * sizeof(LightData);

	RendererLight::RendererLight(Light* light)
		:internal(light)
	{ }

	void RendererLight::GetParameters(LightData& output) const
	{
		Radian spotAngle = Math::clamp(internal->GetSpotAngle() * 0.5f, Degree(0), Degree(89));
		Radian spotFalloffAngle = Math::clamp(internal->GetSpotFalloffAngle() * 0.5f, Degree(0), (Degree)spotAngle);
		Color color = internal->GetColor();

		const Transform& tfrm = internal->GetTransform();
		output.position = tfrm.GetPosition();
		output.boundsRadius = internal->GetBounds().GetRadius();
		output.srcRadius = internal->GetSourceRadius();
		output.direction = -tfrm.GetRotation().zAxis();
		output.luminance = internal->GetLuminance();
		output.spotAngles.x = spotAngle.ValueRadians();
		output.spotAngles.y = Math::cos(output.spotAngles.x);
		output.spotAngles.z = 1.0f / std::max(Math::cos(spotFalloffAngle) - output.spotAngles.y, 0.001f);
		output.attRadiusSqrdInv = 1.0f / (internal->GetAttenuationRadius() * internal->getAttenuationRadius());
		output.color = Vector3(color.r, color.g, color.b);

		// If directional lights, convert angular radius in degrees to radians
		if (internal->GetType() == LightType::Directional)
			output.srcRadius *= Math::DEG2RAD;

		output.shiftedLightPosition = getShiftedLightPosition();
	}

	void RendererLight::GetParameters(SPtr<GpuParamBlockBuffer>& buffer) const
	{
		LightData lightData;
		getParameters(lightData);

		float type = 0.0f;
		switch (internal->GetType())
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

		gPerLightParamDef.gLightPositionAndSrcRadius.Set(buffer, Vector4(lightData.position, lightData.srcRadius));
		gPerLightParamDef.gLightColorAndLuminance.Set(buffer, Vector4(lightData.color, lightData.luminance));
		gPerLightParamDef.gLightSpotAnglesAndSqrdInvAttRadius.Set(buffer, Vector4(lightData.spotAngles, lightData.attRadiusSqrdInv));
		gPerLightParamDef.gLightDirectionAndBoundRadius.Set(buffer, Vector4(lightData.direction, lightData.boundsRadius));
		gPerLightParamDef.gShiftedLightPositionAndType.Set(buffer, Vector4(lightData.shiftedLightPosition, type));

		Vector4 lightGeometry;
		lightGeometry.x = internal->GetType() == LightType::Spot ? (float)Light::LIGHT_CONE_NUM_SIDES : 0;
		lightGeometry.y = (float)Light::LIGHT_CONE_NUM_SLICES;
		lightGeometry.z = internal->GetBounds().GetRadius();

		float extraRadius = lightData.srcRadius / Math::tan(lightData.spotAngles.x * 0.5f);
		float coneRadius = Math::sin(lightData.spotAngles.x) * (internal->GetAttenuationRadius() + extraRadius);
		lightGeometry.w = coneRadius;

		gPerLightParamDef.gLightGeometry.Set(buffer, lightGeometry);

		const Transform& tfrm = internal->GetTransform();

		Quaternion LightRotation(BsIdentity);
		lightRotation.LookRotation(-tfrm.getRotation().zAxis());

		Matrix4 transform = Matrix4::TRS(lightData.shiftedLightPosition, lightRotation, Vector3::ONE);
		gPerLightParamDef.gMatConeTransform.Set(buffer, transform);
	}

	Vector3 RendererLight::GetShiftedLightPosition() const
	{
		const Transform& tfrm = internal->GetTransform();
		Vector3 direction = -tfrm.GetRotation().zAxis();

		// Create position for fake attenuation for area spot lights (with disc center)
		if (internal->GetType() == LightType::Spot)
			return tfrm.GetPosition() - direction * (internal->GetSourceRadius() / Math::tan(internal->getSpotAngle() * 0.5f));
		else
			return tfrm.GetPosition();
	}

	GBufferParams::GBufferParams(GpuProgramType type, const SPtr<GpuParams>& gpuParams)
		: MParams(gpuParams)
	{
		if(mParams->HasTexture(type, "gGBufferATex"))
			mParams->GetTextureParam(type, "gGBufferATex", mGBufferA);

		if(mParams->HasTexture(type, "gGBufferBTex"))
			mParams->GetTextureParam(type, "gGBufferBTex", mGBufferB);

		if(mParams->HasTexture(type, "gGBufferCTex"))
			mParams->GetTextureParam(type, "gGBufferCTex", mGBufferC);

		if(mParams->HasTexture(type, "gDepthBufferTex"))
			mParams->GetTextureParam(type, "gDepthBufferTex", mGBufferDepth);

		if(mParams->HasSamplerState(type, "gDepthBufferSamp"))
		{
			GpuParamSampState samplerStateParam;
			mParams->GetSamplerStateParam(type, "gDepthBufferSamp", samplerStateParam);

			SAMPLER_STATE_DESC desc;
			desc.minFilter = FO_POINT;
			desc.magFilter = FO_POINT;
			desc.mipFilter = FO_POINT;

			SPtr<SamplerState> ss = SamplerState::create(desc);
			samplerStateParam.Set(ss);
		}
	}

	void GBufferParams::Bind(const GBufferTextures& gbuffer)
	{
		mGBufferA.Set(gbuffer.albedo);
		mGBufferB.Set(gbuffer.normals);
		mGBufferC.Set(gbuffer.roughMetal);
		mGBufferDepth.Set(gbuffer.depth);
	}

	void ForwardLightingParams::Populate(const SPtr<GpuParams>& params, bool clustered)
	{
		if (clustered)
		{
			params->GetParamInfo()->getBindings(
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"GridParams",
				gridParamsBindings
			);

			if (params->HasBuffer(GPT_FRAGMENT_PROGRAM, "gLights"))
				params->GetBufferParam(GPT_FRAGMENT_PROGRAM, "gLights", lightsBufferParam);

			if (params->HasBuffer(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize"))
				params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize",
					gridLightOffsetsAndSizeParam);

			if (params->HasBuffer(GPT_FRAGMENT_PROGRAM, "gLightIndices"))
				params->GetBufferParam(GPT_FRAGMENT_PROGRAM, "gLightIndices", gridLightIndicesParam);

			if (params->HasBuffer(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize"))
				params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize",
					gridProbeOffsetsAndSizeParam);
		}
		else
		{
			params->GetParamInfo()->getBinding(
				GPT_FRAGMENT_PROGRAM,
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"Lights",
				lightsParamBlockBinding
			);

			params->GetParamInfo()->getBinding(
				GPT_FRAGMENT_PROGRAM,
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"LightAndReflProbeParams",
				lightAndReflProbeParamsParamBlockBinding
			);
		}
	}

	VisibleLightData::VisibleLightData()
		:mNumLights{}, mNumShadowedLights{}
	{ }

	void VisibleLightData::Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
	{
		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mVisibleLights[i].Clear();

		// Generate a list of lights and their GPU buffers
		UINT32 numDirLights = (UINT32)sceneInfo.directionalLights.Size();
		for (UINT32 i = 0; i < numDirLights; i++)
			mVisibleLights[(UINT32)LightType::Directional].push_back(&sceneInfo.directionalLights[i]);

		UINT32 numRadialLights = (UINT32)sceneInfo.radialLights.Size();
		for(UINT32 i = 0; i < numRadialLights; i++)
		{
			if (!visibility.radialLights[i])
				continue;

			mVisibleLights[(UINT32)LightType::Radial].push_back(&sceneInfo.radialLights[i]);
		}

		UINT32 numSpotLights = (UINT32)sceneInfo.spotLights.Size();
		for (UINT32 i = 0; i < numSpotLights; i++)
		{
			if (!visibility.spotLights[i])
				continue;

			mVisibleLights[(UINT32)LightType::Spot].push_back(&sceneInfo.spotLights[i]);
		}

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mNumLights[i] = (UINT32)mVisibleLights[i].Size();

		// Partition all visible lights so that unshadowed ones come first
		auto partition = [](Vector<const RendererLight*>& entries)
		{
			UINT32 numUnshadowed = 0;
			int first = -1;
			for (UINT32 i = 0; i < (UINT32)entries.Size(); ++i)
			{
				if(entries[i]->internal->GetCastsShadow())
				{
					first = i;
					break;
				}
				else
					++numUnshadowed;
			}

			if(first != -1)
			{
				for(UINT32 i = first + 1; i < (UINT32)entries.Size(); ++i)
				{
					if(!entries[i]->internal->GetCastsShadow())
					{
						std::swap(entries[i], entries[first]);
						++numUnshadowed;
					}
				}
			}

			return numUnshadowed;
		};

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mNumShadowedLights[i] = mNumLights[i] - partition(mVisibleLights[i]);

		// Generate light data to initialize the GPU buffer with
		mVisibleLightData.Clear();
		for(auto& lightsPerType : mVisibleLights)
		{
			for(auto& entry : lightsPerType)
			{
				mVisibleLightData.push_back(LightData());
				entry->GetParameters(mVisibleLightData.Back());
			}
		}

		bool supportsStructuredBuffers = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsStructuredBuffers)
		{
			UINT32 size = (UINT32) mVisibleLightData.Size() * sizeof(LightData);
			UINT32 curBufferSize;

			if (mLightBuffer != nullptr)
				curBufferSize = mLightBuffer->GetSize();
			else
				curBufferSize = 0;

			if (size > curBufferSize || curBufferSize == 0)
			{
				// Allocate at least one block even if no lights, to avoid issues with null buffers
				UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float) LIGHT_DATA_BUFFER_INCREMENT)) * LIGHT_DATA_BUFFER_INCREMENT;

				GPU_BUFFER_DESC bufferDesc;
				bufferDesc.type = GBT_STRUCTURED;
				bufferDesc.elementCount = bufferSize / sizeof(LightData);
				bufferDesc.elementSize = sizeof(LightData);
				bufferDesc.format = BF_UNKNOWN;

				mLightBuffer = GpuBuffer::create(bufferDesc);
			}

			if (size > 0)
				mLightBuffer->WriteData(0, size, mVisibleLightData.Data(), BWT_DISCARD);
		}
	}

	void VisibleLightData::gatherInfluencingLights(const Bounds& bounds,
		const LightData* (&output)[STANDARD_FORWARD_MAX_NUM_LIGHTS], Vector3I& counts) const
	{
		UINT32 outputIndices[STANDARD_FORWARD_MAX_NUM_LIGHTS];
		UINT32 numInfluencingLights = 0;

		UINT32 numDirLights = getNumDirLights();
		for(UINT32 i = 0; i < numDirLights; i++)
		{
			if (numInfluencingLights >= STANDARD_FORWARD_MAX_NUM_LIGHTS)
				return;

			outputIndices[numInfluencingLights] = i;
			numInfluencingLights++;
		}

		UINT32 pointLightOffset = numInfluencingLights;
		
		float distances[STANDARD_FORWARD_MAX_NUM_LIGHTS];
		for(UINT32 i = 0; i < STANDARD_FORWARD_MAX_NUM_LIGHTS; i++)
			distances[i] = std::numeric_limits<float>::max();

		// Note: This is an ad-hoc way of evaluating light influence, a better way might be wanted
		UINT32 numLights = (UINT32)mVisibleLightData.Size();
		UINT32 furthestLightIdx = (UINT32)-1;
		float furthestDistance = 0.0f;
		for (UINT32 j = numDirLights; j < numLights; j++)
		{
			const LightData* lightData = &mVisibleLightData[j];

			Sphere LightSphere(lightData->position, lightData->boundsRadius);
			if (bounds.GetSphere().intersects(lightSphere))
			{
				float distance = bounds.GetSphere().getCenter().squaredDistance(lightData->position);

				// See where in the array can we fit the light
				if (numInfluencingLights < STANDARD_FORWARD_MAX_NUM_LIGHTS)
				{
					outputIndices[numInfluencingLights] = j;
					distances[numInfluencingLights] = distance;

					if (distance > furthestDistance)
					{
						furthestLightIdx = numInfluencingLights;
						furthestDistance = distance;
					}

					numInfluencingLights++;
				}
				else if (distance < furthestDistance)
				{
					outputIndices[furthestLightIdx] = j;
					distances[furthestLightIdx] = distance;

					furthestDistance = distance;
					for (UINT32 k = 0; k < STANDARD_FORWARD_MAX_NUM_LIGHTS; k++)
					{
						if (distances[k] > furthestDistance)
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

		for(UINT32 i = 0; i < pointLightOffset; i++)
		{
			output[i] = &mVisibleLightData[outputIndices[i]];
			counts.x += 1;
		}

		UINT32 outputIdx = pointLightOffset;
		UINT32 spotLightIdx = getNumDirLights() + getNumRadialLights();
		for(UINT32 i = pointLightOffset; i < numInfluencingLights; i++)
		{
			bool isSpot = outputIndices[i] >= spotLightIdx;
			if(isSpot)
				continue;

			output[outputIdx++] = &mVisibleLightData[outputIndices[i]];
			counts.y += 1;
		}

		for(UINT32 i = pointLightOffset; i < numInfluencingLights; i++)
		{
			bool isSpot = outputIndices[i] >= spotLightIdx;
			if(!isSpot)
				continue;

			output[outputIdx++] = &mVisibleLightData[outputIndices[i]];
			counts.z += 1;
		}
	}

	LightsParamDef gLightsParamDef;
	LightAndReflProbeParamsParamDef gLightAndReflProbeParamsParamDef;
}}
