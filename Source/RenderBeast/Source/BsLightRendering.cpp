//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightRendering.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsRenderBeast.h"
#include "BsRenderTargets.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsGpuBuffer.h"
#include "BsLight.h"
#include "BsRendererUtility.h"
#include "BsStandardDeferredLighting.h"

namespace bs { namespace ct
{
	static const UINT32 BUFFER_INCREMENT = 16 * sizeof(LightData);

	TiledLightingParamDef gTiledLightingParamDef;

	RendererLight::RendererLight(Light* light)
		:internal(light)
	{ }

	void RendererLight::getParameters(LightData& output) const
	{
		Radian spotAngle = Math::clamp(internal->getSpotAngle() * 0.5f, Degree(0), Degree(89));
		Radian spotFalloffAngle = Math::clamp(internal->getSpotFalloffAngle() * 0.5f, Degree(0), (Degree)spotAngle);
		Color color = internal->getColor();

		output.position = internal->getPosition();
		output.attRadius = internal->getBounds().getRadius();
		output.srcRadius = internal->getSourceRadius();
		output.direction = -internal->getRotation().zAxis();
		output.luminance = internal->getLuminance();
		output.spotAngles.x = spotAngle.valueRadians();
		output.spotAngles.y = Math::cos(output.spotAngles.x);
		output.spotAngles.z = 1.0f / std::max(Math::cos(spotFalloffAngle) - output.spotAngles.y, 0.001f);
		output.attRadiusSqrdInv = 1.0f / (output.attRadius * output.attRadius);
		output.color = Vector3(color.r, color.g, color.b);

		// If directional lights, convert angular radius in degrees to radians
		if (internal->getType() == LightType::Directional)
			output.srcRadius *= Math::DEG2RAD;

		output.shiftedLightPosition = getShiftedLightPosition();
	}

	void RendererLight::getParameters(SPtr<GpuParamBlockBuffer>& buffer) const
	{
		LightData lightData;
		getParameters(lightData);

		float type = 0.0f;
		switch (internal->getType())
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
		}

		gPerLightParamDef.gLightPositionAndSrcRadius.set(buffer, Vector4(lightData.position, lightData.srcRadius));
		gPerLightParamDef.gLightColorAndLuminance.set(buffer, Vector4(lightData.color, lightData.luminance));
		gPerLightParamDef.gLightSpotAnglesAndSqrdInvAttRadius.set(buffer, Vector4(lightData.spotAngles, lightData.attRadiusSqrdInv));
		gPerLightParamDef.gLightDirectionAndAttRadius.set(buffer, Vector4(lightData.direction, lightData.attRadius));
		gPerLightParamDef.gShiftedLightPositionAndType.set(buffer, Vector4(lightData.shiftedLightPosition, type));

		Vector4 lightGeometry;
		lightGeometry.x = internal->getType() == LightType::Spot ? (float)Light::LIGHT_CONE_NUM_SIDES : 0;
		lightGeometry.y = (float)Light::LIGHT_CONE_NUM_SLICES;
		lightGeometry.z = internal->getBounds().getRadius();

		float extraRadius = lightData.srcRadius / Math::tan(lightData.spotAngles.x * 0.5f);
		float coneRadius = Math::sin(lightData.spotAngles.x) * (internal->getAttenuationRadius() + extraRadius);
		lightGeometry.w = coneRadius;

		gPerLightParamDef.gLightGeometry.set(buffer, lightGeometry);

		Quaternion lightRotation(BsIdentity);
		lightRotation.lookRotation(-internal->getRotation().zAxis());

		Matrix4 transform = Matrix4::TRS(lightData.shiftedLightPosition, lightRotation, Vector3::ONE);
		gPerLightParamDef.gMatConeTransform.set(buffer, transform);
	}

	Vector3 RendererLight::getShiftedLightPosition() const
	{
		Vector3 direction = -internal->getRotation().zAxis();

		// Create position for fake attenuation for area spot lights (with disc center)
		if (internal->getType() == LightType::Spot)
			return internal->getPosition() - direction * (internal->getSourceRadius() / Math::tan(internal->getSpotAngle() * 0.5f));
		else
			return internal->getPosition();
	}

	GBufferParams::GBufferParams(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet)
		: mMaterial(material), mParamsSet(paramsSet)
	{
		mGBufferA = material->getParamTexture("gGBufferATex");
		mGBufferB = material->getParamTexture("gGBufferBTex");
		mGBufferC = material->getParamTexture("gGBufferCTex");
		mGBufferDepth = material->getParamTexture("gDepthBufferTex");

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;

		SPtr<SamplerState> ss = SamplerState::create(desc);
		material->setSamplerState("gDepthBufferSamp", ss);
	}

	void GBufferParams::bind(const RenderTargets& renderTargets)
	{
		mGBufferA.set(renderTargets.get(RTT_GBuffer, RT_COLOR0));
		mGBufferB.set(renderTargets.get(RTT_GBuffer, RT_COLOR1));
		mGBufferC.set(renderTargets.get(RTT_GBuffer, RT_COLOR2));
		mGBufferDepth.set(renderTargets.get(RTT_GBuffer, RT_DEPTH));

		mMaterial->updateParamsSet(mParamsSet);
	}

	void GBufferParams::bind(const GBufferInput& gbuffer)
	{
		mGBufferA.set(gbuffer.albedo);
		mGBufferB.set(gbuffer.normals);
		mGBufferC.set(gbuffer.roughMetal);
		mGBufferDepth.set(gbuffer.depth);

		mMaterial->updateParamsSet(mParamsSet);
	}

	VisibleLightData::VisibleLightData()
		:mNumLights{}, mNumShadowedLights{}
	{ }

	void VisibleLightData::update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
	{
		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mVisibleLights[i].clear();

		// Generate a list of lights and their GPU buffers
		UINT32 numDirLights = (UINT32)sceneInfo.directionalLights.size();
		for (UINT32 i = 0; i < numDirLights; i++)
			mVisibleLights[(UINT32)LightType::Directional].push_back(&sceneInfo.directionalLights[i]);

		UINT32 numRadialLights = (UINT32)sceneInfo.radialLights.size();
		for(UINT32 i = 0; i < numRadialLights; i++)
		{
			if (!visibility.radialLights[i])
				continue;

			mVisibleLights[(UINT32)LightType::Radial].push_back(&sceneInfo.radialLights[i]);
		}

		UINT32 numSpotLights = (UINT32)sceneInfo.spotLights.size();
		for (UINT32 i = 0; i < numSpotLights; i++)
		{
			if (!visibility.spotLights[i])
				continue;

			mVisibleLights[(UINT32)LightType::Spot].push_back(&sceneInfo.spotLights[i]);
		}

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mNumLights[i] = (UINT32)mVisibleLights[i].size();

		// Partition all visible lights so that unshadowed ones come first
		auto partition = [](Vector<const RendererLight*>& entries)
		{
			int first = 0;
			for (int i = 0; i < entries.size(); ++i)
			{
				if(entries[i]->internal->getCastsShadow())
				{
					first = i;
					break;
				}
			}

			for(int i = first + 1; i < entries.size(); ++i)
			{
				if(!entries[i]->internal->getCastsShadow())
				{
					std::swap(entries[i], entries[first]);
					++first;
				}
			}

			return first;
		};

		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			mNumShadowedLights[i] = mNumLights[i] - partition(mVisibleLights[i]);

		// Generate light data to initialize the GPU buffer with
		for(auto& lightsPerType : mVisibleLights)
		{
			for(auto& entry : lightsPerType)
			{
				mLightDataTemp.push_back(LightData());
				entry->getParameters(mLightDataTemp.back());
			}
		}

		UINT32 size = (UINT32)mLightDataTemp.size() * sizeof(LightData);
		UINT32 curBufferSize;

		if (mLightBuffer != nullptr)
			curBufferSize = mLightBuffer->getSize();
		else
			curBufferSize = 0;

		if (size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no lights, to avoid issues with null buffers
			UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float)BUFFER_INCREMENT)) * BUFFER_INCREMENT;

			GPU_BUFFER_DESC bufferDesc;
			bufferDesc.type = GBT_STRUCTURED;
			bufferDesc.elementCount = bufferSize / sizeof(LightData);
			bufferDesc.elementSize = sizeof(LightData);
			bufferDesc.format = BF_UNKNOWN;

			mLightBuffer = GpuBuffer::create(bufferDesc);
		}

		if (size > 0)
			mLightBuffer->writeData(0, size, mLightDataTemp.data(), BWT_DISCARD);

		mLightDataTemp.clear();
	}

	const UINT32 TiledDeferredLighting::TILE_SIZE = 16;

	TiledDeferredLighting::TiledDeferredLighting(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet,
													UINT32 sampleCount)
		: mSampleCount(sampleCount), mMaterial(material), mParamsSet(paramsSet), mGBufferParams(material, paramsSet)
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);

		if (params->hasLoadStoreTexture(GPT_COMPUTE_PROGRAM, "gOutput"))
			params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		if (params->hasBuffer(GPT_COMPUTE_PROGRAM, "gOutput"))
			params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);

		mParamBuffer = gTiledLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);
	}

	void TiledDeferredLighting::execute(const RendererView& view, const VisibleLightData& lightData, 
		const GBufferInput& gbuffer, const SPtr<Texture>& lightAccumTex, const SPtr<GpuBuffer>& lightAccumBuffer)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		mLightBufferParam.set(lightData.getLightBuffer());

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		if (viewProps.noLighting)
		{
			Vector4I lightCounts;
			lightCounts[0] = 0;
			lightCounts[1] = 0;
			lightCounts[2] = 0;
			lightCounts[3] = 0;

			Vector2I lightStrides;
			lightStrides[0] = 0;
			lightStrides[1] = 0;

			gTiledLightingParamDef.gLightCounts.set(mParamBuffer, lightCounts);
			gTiledLightingParamDef.gLightStrides.set(mParamBuffer, lightStrides);
		}
		else
		{
			Vector4I unshadowedLightCounts;
			unshadowedLightCounts[0] = lightData.getNumUnshadowedLights(LightType::Directional);
			unshadowedLightCounts[1] = lightData.getNumUnshadowedLights(LightType::Radial);
			unshadowedLightCounts[2] = lightData.getNumUnshadowedLights(LightType::Spot);
			unshadowedLightCounts[3] = unshadowedLightCounts[0] + unshadowedLightCounts[1] + unshadowedLightCounts[2];

			Vector4I lightCounts;
			lightCounts[0] = lightData.getNumLights(LightType::Directional);
			lightCounts[1] = lightData.getNumLights(LightType::Radial);
			lightCounts[2] = lightData.getNumLights(LightType::Spot);
			lightCounts[3] = lightCounts[0] + lightCounts[1] + lightCounts[2];

			Vector2I lightStrides;
			lightStrides[0] = lightCounts[0];
			lightStrides[1] = lightStrides[0] + lightCounts[1];

			if(viewProps.noShadows)
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, lightCounts);
			else
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, unshadowedLightCounts);

			gTiledLightingParamDef.gLightStrides.set(mParamBuffer, lightStrides);
		}

		mParamBuffer->flushToGPU();

		mGBufferParams.bind(gbuffer);
		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

		if (mSampleCount > 1)
			mOutputBufferParam.set(lightAccumBuffer);
		else
			mOutputTextureParam.set(lightAccumTex);

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	template<int MSAA_COUNT>
	TTiledDeferredLightingMat<MSAA_COUNT>::TTiledDeferredLightingMat()
		:mInternal(mMaterial, mParamsSet, MSAA_COUNT)
	{

	}

	template<int MSAA_COUNT>
	void TTiledDeferredLightingMat<MSAA_COUNT>::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TiledDeferredLighting::TILE_SIZE);
		defines.set("MSAA_COUNT", MSAA_COUNT);
	}

	template<int MSAA_COUNT>
	void TTiledDeferredLightingMat<MSAA_COUNT>::execute(const RendererView& view, const VisibleLightData& lightData, 
		const GBufferInput& gbuffer, const SPtr<Texture>& lightAccumTex, const SPtr<GpuBuffer>& lightAccumBuffer)
	{
		mInternal.execute(view, lightData, gbuffer, lightAccumTex, lightAccumBuffer);
	}

	template class TTiledDeferredLightingMat<1>;
	template class TTiledDeferredLightingMat<2>;
	template class TTiledDeferredLightingMat<4>;
	template class TTiledDeferredLightingMat<8>;

	FlatFramebufferToTextureParamDef gFlatFramebufferToTextureParamDef;

	FlatFramebufferToTextureMat::FlatFramebufferToTextureMat()
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gInput", mInputParam);

		mParamBuffer = gTiledLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);
	}

	void FlatFramebufferToTextureMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void FlatFramebufferToTextureMat::execute(const SPtr<GpuBuffer>& flatFramebuffer, const SPtr<Texture>& target)
	{
		const TextureProperties& props = target->getProperties();

		Vector2I framebufferSize;
		framebufferSize[0] = props.getWidth();
		framebufferSize[1] = props.getHeight();
		gFlatFramebufferToTextureParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		gFlatFramebufferToTextureParamDef.gSampleCount.set(mParamBuffer, props.getNumSamples());

		mParamBuffer->flushToGPU();

		mInputParam.set(flatFramebuffer);

		gRendererUtility().setPass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		Rect2 area(0.0f, 0.0f, (float)props.getWidth(), (float)props.getHeight());
		gRendererUtility().drawScreenQuad(area);
	}
}}