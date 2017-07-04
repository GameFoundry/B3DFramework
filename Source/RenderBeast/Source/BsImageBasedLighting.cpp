//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsImageBasedLighting.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsReflectionProbe.h"
#include "BsLightProbeCache.h"
#include "BsGpuParamsSet.h"
#include "BsRenderTargets.h"
#include "BsRenderBeast.h"
#include "BsRendererUtility.h"

namespace bs { namespace ct
{
	static const UINT32 BUFFER_INCREMENT = 16 * sizeof(ReflProbeData);

	ReflProbeParamsParamDef gReflProbeParamsParamDef;
	TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

	VisibleReflProbeData::VisibleReflProbeData()
		:mNumProbes(0)
	{ }

	void VisibleReflProbeData::update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup)
	{
		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

		// Generate refl. probe data for the visible ones
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();
		for(UINT32 i = 0; i < numProbes; i++)
		{
			if (!visibility.reflProbes[i])
				continue;

			mReflProbeDataTemp.push_back(ReflProbeData());
			sceneInfo.reflProbes[i].getParameters(mReflProbeDataTemp.back());
		}

		// Sort probes so bigger ones get accessed first, this way we overlay smaller ones on top of biggers ones when
		// rendering
		auto sorter = [](const ReflProbeData& lhs, const ReflProbeData& rhs)
		{
			return rhs.radius < lhs.radius;
		};

		std::sort(mReflProbeDataTemp.begin(), mReflProbeDataTemp.end(), sorter);

		mNumProbes = (UINT32)mReflProbeDataTemp.size();

		// Move refl. probe data into a GPU buffer
		UINT32 size = mNumProbes * sizeof(ReflProbeData);
		UINT32 curBufferSize;

		if (mProbeBuffer != nullptr)
			curBufferSize = mProbeBuffer->getSize();
		else
			curBufferSize = 0;

		if (size > curBufferSize || curBufferSize == 0)
		{
			// Allocate at least one block even if no probes, to avoid issues with null buffers
			UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float)BUFFER_INCREMENT)) * BUFFER_INCREMENT;

			GPU_BUFFER_DESC bufferDesc;
			bufferDesc.type = GBT_STRUCTURED;
			bufferDesc.elementCount = bufferSize / sizeof(ReflProbeData);
			bufferDesc.elementSize = sizeof(ReflProbeData);
			bufferDesc.format = BF_UNKNOWN;

			mProbeBuffer = GpuBuffer::create(bufferDesc);
		}

		if (size > 0)
			mProbeBuffer->writeData(0, size, mReflProbeDataTemp.data(), BWT_DISCARD);

		mReflProbeDataTemp.clear();
	}

	RendererReflectionProbe::RendererReflectionProbe(ReflectionProbe* probe)
		:probe(probe)
	{
		arrayIdx = -1;
		texture = nullptr;
		customTexture = probe->getCustomTexture() != nullptr;
		textureDirty = LightProbeCache::instance().isRadianceDirty(probe->getUUID());
		arrayDirty = true;
		errorFlagged = false;
	}

	void RendererReflectionProbe::getParameters(ReflProbeData& output) const
	{
		output.type = probe->getType() == ReflectionProbeType::Sphere ? 0 
			: probe->getType() == ReflectionProbeType::Box ? 1 : 2;
		
		output.position = probe->getPosition();
		output.boxExtents = probe->getExtents();

		if (probe->getType() == ReflectionProbeType::Sphere)
			output.radius = probe->getRadius();
		else
			output.radius = output.boxExtents.length();

		output.transitionDistance = probe->getTransitionDistance();
		output.cubemapIdx = arrayIdx;
		output.invBoxTransform.setInverseTRS(output.position, probe->getRotation(), output.boxExtents);
	}

	void ImageBasedLightingParams::populate(const SPtr<GpuParamsSet>& paramsSet, GpuProgramType programType, bool optional, 
		bool gridIndices)
	{
		SPtr<GpuParams> params = paramsSet->getGpuParams();

		// Sky
		if (!optional || params->hasTexture(programType, "gSkyReflectionTex"))
		{
			params->getTextureParam(programType, "gSkyReflectionTex", skyReflectionsTexParam);

			if(params->hasSamplerState(programType, "gSkyReflectionSamp"))
				params->getSamplerStateParam(programType, "gSkyReflectionSamp", skyReflectionsSampParam);
			else
				params->getSamplerStateParam(programType, "gSkyReflectionTex", skyReflectionsSampParam);

			params->getTextureParam(programType, "gSkyIrradianceTex", skyIrradianceTexParam);
		}

		// Reflections
		if (!optional || params->hasTexture(programType, "gReflProbeCubemaps"))
		{
			params->getTextureParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsTexParam);

			if(params->hasSamplerState(programType, "gReflProbeSamp"))
				params->getSamplerStateParam(programType, "gReflProbeSamp", reflectionProbeCubemapsSampParam);
			else
				params->getSamplerStateParam(programType, "gReflProbeCubemaps", reflectionProbeCubemapsSampParam);

			params->getBufferParam(programType, "gReflectionProbes", reflectionProbesParam);

			params->getTextureParam(programType, "gPreintegratedEnvBRDF", preintegratedEnvBRDFParam);
		}

		if(gridIndices)
		{
			if (!optional || params->hasBuffer(programType, "gReflectionProbeIndices"))
				params->getBufferParam(programType, "gReflectionProbeIndices", reflectionProbeIndicesParam);
		}

		reflProbeParamsBindingIdx = paramsSet->getParamBlockBufferIndex("ReflProbeParams");
	}

	// Note: Using larger tiles than in tiled deferred lighting since we use AABB for intersections, which is more
	// expensive to compute than frustums. This way we amortize the cost even though other parts of the shader might suffer
	// due to increased thread group load.
	const UINT32 TiledDeferredImageBasedLighting::TILE_SIZE = 32;

	TiledDeferredImageBasedLighting::TiledDeferredImageBasedLighting(const SPtr<Material>& material, 
		const SPtr<GpuParamsSet>& paramsSet, UINT32 sampleCount)
		:mSampleCount(sampleCount), mMaterial(material), mParamsSet(paramsSet)
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		if (mSampleCount > 1)
			params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);
		else
			params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);

		mImageBasedParams.populate(mParamsSet, GPT_COMPUTE_PROGRAM, false, false);

		mReflectionsParamBuffer = gReflProbeParamsParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("ReflProbeParams", mReflectionsParamBuffer);

		SAMPLER_STATE_DESC reflSamplerDesc;
		reflSamplerDesc.magFilter = FO_LINEAR;
		reflSamplerDesc.minFilter = FO_LINEAR;
		reflSamplerDesc.mipFilter = FO_LINEAR;

		mReflectionSamplerState = SamplerState::create(reflSamplerDesc);

		mImageBasedParams.skyReflectionsSampParam.set(mReflectionSamplerState);
		mImageBasedParams.reflectionProbeCubemapsSampParam.set(mReflectionSamplerState);
	}

	void TiledDeferredImageBasedLighting::execute(const SPtr<RenderTargets>& renderTargets,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& preintegratedGF)
	{
		Vector2I framebufferSize;
		framebufferSize[0] = renderTargets->getWidth();
		framebufferSize[1] = renderTargets->getHeight();
		gTiledImageBasedLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		mParamBuffer->flushToGPU();
		mReflectionsParamBuffer->flushToGPU();

		mGBufferA.set(renderTargets->get(RTT_GBuffer, RT_COLOR0));
		mGBufferB.set(renderTargets->get(RTT_GBuffer, RT_COLOR1));
		mGBufferC.set(renderTargets->get(RTT_GBuffer, RT_COLOR2));
		mGBufferDepth.set(renderTargets->get(RTT_GBuffer, RT_DEPTH));

		mImageBasedParams.preintegratedEnvBRDFParam.set(preintegratedGF);

		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);

		mInColorTextureParam.set(renderTargets->get(RTT_LightAccumulation));
		if (mSampleCount > 1)
			mOutputBufferParam.set(renderTargets->getSceneColorBuffer());
		else
			mOutputTextureParam.set(renderTargets->get(RTT_SceneColor));

		UINT32 width = renderTargets->getWidth();
		UINT32 height = renderTargets->getHeight();

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	void TiledDeferredImageBasedLighting::setReflectionProbes(const VisibleReflProbeData& probeData,
		const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
	{
		mImageBasedParams.reflectionProbesParam.set(probeData.getProbeBuffer());
		mImageBasedParams.reflectionProbeCubemapsTexParam.set(reflectionCubemaps);

		gReflProbeParamsParamDef.gNumProbes.set(mReflectionsParamBuffer, probeData.getNumProbes());

		UINT32 numMips = 0;
		if (reflectionCubemaps != nullptr)
			numMips = reflectionCubemaps->getProperties().getNumMipmaps() + 1;

		gReflProbeParamsParamDef.gReflCubemapNumMips.set(mReflectionsParamBuffer, numMips);
		gReflProbeParamsParamDef.gUseReflectionMaps.set(mReflectionsParamBuffer, capturingReflections ? 0 : 1);
	}

	void TiledDeferredImageBasedLighting::setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance,
		float brightness)
	{
		mImageBasedParams.skyReflectionsTexParam.set(skyReflections);
		mImageBasedParams.skyIrradianceTexParam.set(skyIrradiance);

		UINT32 skyReflectionsAvailable = 0;
		UINT32 numMips = 0;
		if (skyReflections != nullptr)
		{
			numMips = skyReflections->getProperties().getNumMipmaps() + 1;
			skyReflectionsAvailable = 1;
		}

		gReflProbeParamsParamDef.gSkyCubemapNumMips.set(mReflectionsParamBuffer, numMips);
		gReflProbeParamsParamDef.gSkyCubemapAvailable.set(mReflectionsParamBuffer, skyReflectionsAvailable);
		gReflProbeParamsParamDef.gSkyBrightness.set(mReflectionsParamBuffer, brightness);
	}

	// Reverse bits functions used for Hammersley sequence
	float reverseBits(UINT32 bits)
	{
		bits = (bits << 16u) | (bits >> 16u);
		bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

		return (float)(double(bits) / (double)0x100000000LL);
	}

	void hammersleySequence(UINT32 i, UINT32 count, float& e0, float& e1)
	{
		e0 = i / (float)count;
		e1 = reverseBits(i);
	}

	Vector3 sphericalToCartesian(float cosTheta, float sinTheta, float phi)
	{
		Vector3 output;
		output.x = sinTheta * cos(phi);
		output.y = sinTheta * sin(phi);
		output.z = cosTheta;

		return output;
	}

	// Generates an angle in spherical coordinates, importance sampled for the specified roughness based on some uniformly
	// distributed random variables in range [0, 1].
	void importanceSampleGGX(float e0, float e1, float roughness4, float& cosTheta, float& phi)
	{
		// See GGXImportanceSample.nb for derivation (essentially, take base GGX, normalize it, generate PDF, split PDF into
		// marginal probability for theta and conditional probability for phi. Plug those into the CDF, invert it.)				
		cosTheta = sqrt((1.0f - e0) / (1.0f + (roughness4 - 1.0f) * e0));
		phi = 2.0f * Math::PI * e1;
	}

	float calcMicrofacetShadowingSmithGGX(float roughness4, float NoV, float NoL)
	{
		// Note: See lighting shader for derivation. Includes microfacet BRDF divisor.
		float g1V = NoV + sqrt(NoV * (NoV - NoV * roughness4) + roughness4);
		float g1L = NoL + sqrt(NoL * (NoL - NoL * roughness4) + roughness4);
		return 1.0f / (g1V * g1L);
	}

	SPtr<Texture> TiledDeferredImageBasedLighting::generatePreintegratedEnvBRDF()
	{
		TEXTURE_DESC desc;
		desc.type = TEX_TYPE_2D;
		desc.format = PF_FLOAT16_RG;
		desc.width = 128;
		desc.height = 32;

		SPtr<Texture> texture = Texture::create(desc);
		PixelData pixelData = texture->lock(GBL_WRITE_ONLY_DISCARD);

		for (UINT32 y = 0; y < desc.height; y++)
		{
			float roughness = (float)(y + 0.5f) / desc.height;
			float m = roughness * roughness;
			float m2 = m*m;

			for (UINT32 x = 0; x < desc.width; x++)
			{
				float NoV = (float)(x + 0.5f) / desc.width;

				Vector3 V;
				V.x = sqrt(1.0f - NoV * NoV); // sine
				V.y = 0.0f;
				V.z = NoV;

				// These are the two integrals resulting from the second part of the split-sum approximation. Described in
				// Epic's 2013 paper:
				//    http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
				float scale = 0.0f;
				float offset = 0.0f;

				// We use the same importance sampling function we use for reflection cube importance sampling, only we
				// sample G and F, instead of D factors of the microfactet BRDF. See GGXImportanceSample.nb for derivation.
				constexpr UINT32 NumSamples = 128;
				for (UINT32 i = 0; i < NumSamples; i++)
				{
					float e0, e1;
					hammersleySequence(i, NumSamples, e0, e1);

					float cosTheta, phi;
					importanceSampleGGX(e0, e1, m2, cosTheta, phi);

					float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
					Vector3 H = sphericalToCartesian(cosTheta, sinTheta, phi);
					Vector3 L = 2.0f * Vector3::dot(V, H) * H - V;

					float VoH = std::max(Vector3::dot(V, H), 0.0f);
					float NoL = std::max(L.z, 0.0f); // N assumed (0, 0, 1)
					float NoH = std::max(H.z, 0.0f); // N assumed (0, 0, 1)

					// Set second part of the split sum integral is split into two parts:
					//   F0*I[G * (1 - (1 - v.h)^5) * cos(theta)] + I[G * (1 - v.h)^5 * cos(theta)] (F0 * scale + bias)

					// We calculate the fresnel scale (1 - (1 - v.h)^5) and bias ((1 - v.h)^5) parts
					float fc = pow(1.0f - VoH, 5.0f);
					float fresnelScale = 1.0f - fc;
					float fresnelOffset = fc;

					// We calculate the G part
					float G = calcMicrofacetShadowingSmithGGX(m2, NoV, NoL);

					// When we factor out G and F, then divide D by PDF, this is what's left
					// Note: This is based on PDF: D * NoH / (4 * VoH). (4 * VoH) factor comes from the Jacobian of the
					// transformation from half vector to light vector
					float pdfFactor = 4.0f * VoH / NoH;

					if (NoL > 0.0f)
					{
						scale += NoL * pdfFactor * G * fresnelScale;
						offset += NoL * pdfFactor * G * fresnelOffset;
					}
				}

				scale /= NumSamples;
				offset /= NumSamples;

				Color color;
				color.r = Math::clamp01(scale);
				color.g = Math::clamp01(offset);

				pixelData.setColorAt(color, x, y);
			}
		}

		texture->unlock();

		return texture;
	}

	template<int MSAA_COUNT>
	TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::TTiledDeferredImageBasedLightingMat()
		:mInternal(mMaterial, mParamsSet, MSAA_COUNT)
	{

	}

	template<int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TiledDeferredImageBasedLighting::TILE_SIZE);
		defines.set("MSAA_COUNT", MSAA_COUNT);
	}

	template<int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::execute(const SPtr<RenderTargets>& gbuffer,
		const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& preintegratedGF)
	{
		mInternal.execute(gbuffer, perCamera, preintegratedGF);
	}

	template<int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::setReflectionProbes(const VisibleReflProbeData& probeData, 
		const SPtr<Texture>& reflectionCubemaps, bool capturingReflections)
	{
		mInternal.setReflectionProbes(probeData, reflectionCubemaps, capturingReflections);
	}

	template<int MSAA_COUNT>
	void TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::setSky(const SPtr<Texture>& skyReflections,
		const SPtr<Texture>& skyIrradiance, float brightness)
	{
		mInternal.setSky(skyReflections, skyIrradiance, brightness);
	}

	template<int MSAA_COUNT>
	SPtr<GpuParamBlockBuffer> TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::getReflectionsParamBuffer() const
	{
		return mInternal.getReflectionsParamBuffer();
	}

	template<int MSAA_COUNT>
	SPtr<SamplerState> TTiledDeferredImageBasedLightingMat<MSAA_COUNT>::getReflectionsSamplerState() const
	{
		return mInternal.getReflectionsSamplerState();
	}

	TiledDeferredImageBasedLightingMaterials::TiledDeferredImageBasedLightingMaterials()
	{
		mInstances[0] = bs_new<TTiledDeferredImageBasedLightingMat<1>>();
		mInstances[1] = bs_new<TTiledDeferredImageBasedLightingMat<2>>();
		mInstances[2] = bs_new<TTiledDeferredImageBasedLightingMat<4>>();
		mInstances[3] = bs_new<TTiledDeferredImageBasedLightingMat<8>>();
	}

	TiledDeferredImageBasedLightingMaterials::~TiledDeferredImageBasedLightingMaterials()
	{
		for (UINT32 i = 0; i < 4; i++)
			bs_delete(mInstances[i]);
	}

	ITiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMaterials::get(UINT32 msaa)
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
}}