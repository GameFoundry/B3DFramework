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
		output.direction = internal->getRotation().zAxis();
		output.luminance = internal->getLuminance();
		output.spotAngles.x = spotAngle.valueRadians();
		output.spotAngles.y = Math::cos(output.spotAngles.x);
		output.spotAngles.z = 1.0f / std::max(Math::cos(spotFalloffAngle) - output.spotAngles.y, 0.001f);
		output.attRadiusSqrdInv = 1.0f / (output.attRadius * output.attRadius);
		output.color = Vector3(color.r, color.g, color.b);

		// If directional lights, convert angular radius in degrees to radians
		if (internal->getType() == LightType::Directional)
			output.srcRadius *= Math::DEG2RAD;

		// Create position for fake attenuation for area spot lights (with disc center)
		if (internal->getType() == LightType::Spot)
			output.shiftedLightPosition = output.position - output.direction * (output.srcRadius / Math::tan(spotAngle * 0.5f));
		else
			output.shiftedLightPosition = output.position;
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

		float coneRadius = Math::sin(lightData.spotAngles.x) * internal->getAttenuationRadius();
		lightGeometry.w = coneRadius;

		gPerLightParamDef.gLightGeometry.set(buffer, lightGeometry);

		Quaternion lightRotation(BsIdentity);
		lightRotation.lookRotation(internal->getRotation().zAxis());

		Matrix4 transform = Matrix4::TRS(internal->getPosition(), lightRotation, Vector3::ONE);
		gPerLightParamDef.gMatConeTransform.set(buffer, transform);
	}

	GBufferParams::GBufferParams(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet)
		: mMaterial(material), mParamsSet(paramsSet)
	{
		mGBufferA = material->getParamTexture("gGBufferATex");
		mGBufferB = material->getParamTexture("gGBufferBTex");
		mGBufferC = material->getParamTexture("gGBufferCTex");
		mGBufferDepth = material->getParamTexture("gDepthBufferTex");
	}

	void GBufferParams::bind(const RenderTargets& renderTargets)
	{
		mGBufferA.set(renderTargets.getGBufferA());
		mGBufferB.set(renderTargets.getGBufferB());
		mGBufferC.set(renderTargets.getGBufferC());
		mGBufferDepth.set(renderTargets.getSceneDepth());

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

	void TiledDeferredLighting::execute(const SPtr<RenderTargets>& renderTargets, 
		const SPtr<GpuParamBlockBuffer>& perCamera, bool noLighting, bool noShadows)
	{
		Vector2I framebufferSize;
		framebufferSize[0] = renderTargets->getWidth();
		framebufferSize[1] = renderTargets->getHeight();
		gTiledLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		if (noLighting)
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
			if(noShadows)
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, mLightCounts);
			else
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, mUnshadowedLightCounts);

			gTiledLightingParamDef.gLightStrides.set(mParamBuffer, mLightStrides);
		}

		mParamBuffer->flushToGPU();

		mGBufferParams.bind(*renderTargets);
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);

		if (mSampleCount > 1)
		{
			SPtr<GpuBuffer> lightAccumulation = renderTargets->getLightAccumulationBuffer();
			mOutputBufferParam.set(lightAccumulation);
		}
		else
		{
			SPtr<Texture> lightAccumulation = renderTargets->getLightAccumulation();
			mOutputTextureParam.set(lightAccumulation);
		}

		UINT32 width = renderTargets->getWidth();
		UINT32 height = renderTargets->getHeight();

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	void TiledDeferredLighting::setLights(const VisibleLightData& lightData)
	{
		mLightBufferParam.set(lightData.getLightBuffer());

		mUnshadowedLightCounts[0] = lightData.getNumUnshadowedLights(LightType::Directional);
		mUnshadowedLightCounts[1] = lightData.getNumUnshadowedLights(LightType::Radial);
		mUnshadowedLightCounts[2] = lightData.getNumUnshadowedLights(LightType::Spot);
		mUnshadowedLightCounts[3] = mUnshadowedLightCounts[0] + mUnshadowedLightCounts[1] + mUnshadowedLightCounts[2];

		mLightCounts[0] = lightData.getNumLights(LightType::Directional);
		mLightCounts[1] = lightData.getNumLights(LightType::Radial);
		mLightCounts[2] = lightData.getNumLights(LightType::Spot);
		mLightCounts[3] = mLightCounts[0] + mLightCounts[1] + mLightCounts[2];

		mLightStrides[0] = mLightCounts[0];
		mLightStrides[1] = mLightStrides[0] + mLightCounts[1];
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
	void TTiledDeferredLightingMat<MSAA_COUNT>::execute(const SPtr<RenderTargets>& gbuffer,
		const SPtr<GpuParamBlockBuffer>& perCamera, bool noLighting, bool noShadows)
	{
		mInternal.execute(gbuffer, perCamera, noLighting, noShadows);
	}

	template<int MSAA_COUNT>
	void TTiledDeferredLightingMat<MSAA_COUNT>::setLights(const VisibleLightData& lightData)
	{
		mInternal.setLights(lightData);
	}

	TiledDeferredLightingMaterials::TiledDeferredLightingMaterials()
	{
		mInstances[0] = bs_new<TTiledDeferredLightingMat<1>>();
		mInstances[1] = bs_new<TTiledDeferredLightingMat<2>>();
		mInstances[2] = bs_new<TTiledDeferredLightingMat<4>>();
		mInstances[3] = bs_new<TTiledDeferredLightingMat<8>>();
	}

	TiledDeferredLightingMaterials::~TiledDeferredLightingMaterials()
	{
		for (UINT32 i = 0; i < 4; i++)
			bs_delete(mInstances[i]);
	}

	ITiledDeferredLightingMat* TiledDeferredLightingMaterials::get(UINT32 msaa)
	{
		if (msaa == 1)
			return mInstances[0];
		else if (msaa == 2)
			return mInstances[1];
		else if (msaa == 4)
			return mInstances[2];
		else
			return mInstances[3];
	}

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