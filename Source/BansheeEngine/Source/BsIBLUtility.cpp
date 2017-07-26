//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsIBLUtility.h"
#include "BsTexture.h"
#include "BsGpuParamsSet.h"
#include "BsRendererUtility.h"
#include "BsGpuBuffer.h"

namespace bs { namespace ct
{
	struct IBLUtility::Members
	{
		ReflectionCubeDownsampleMat downsampleMat;
		ReflectionCubeImportanceSampleMat importanceSampleMat;

		IrradianceComputeSHMat<3> shCompute3;
		IrradianceComputeSHMat<5> shCompute5;
		IrradianceReduceSHMat<3> shReduce3;
		IrradianceReduceSHMat<5> shReduce5;
		IrradianceProjectSHMat shProject5;

		SPtr<Texture> preIntegratedGF;
	};

	IBLUtility::Members* IBLUtility::m = nullptr;

	void IBLUtility::startUp()
	{
		m = bs_new<Members>();

		m->preIntegratedGF = generatePreintegratedEnvBRDF();
	}

	void IBLUtility::shutDown()
	{
		bs_delete(m);
	}

	ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

	ReflectionCubeDownsampleMat::ReflectionCubeDownsampleMat()
	{
		mParamBuffer = gReflectionCubeDownsampleParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubeDownsampleMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void ReflectionCubeDownsampleMat::execute(const SPtr<Texture>& source, UINT32 face, const TextureSurface& surface, 
											 const SPtr<RenderTarget>& target)
	{
		mInputTexture.set(source, surface);
		gReflectionCubeDownsampleParamDef.gCubeFace.set(mParamBuffer, face);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	const UINT32 ReflectionCubeImportanceSampleMat::NUM_SAMPLES = 1024;
	ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

	ReflectionCubeImportanceSampleMat::ReflectionCubeImportanceSampleMat()
	{
		mParamBuffer = gReflectionCubeImportanceSampleParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubeImportanceSampleMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("NUM_SAMPLES", NUM_SAMPLES); 
	}

	void ReflectionCubeImportanceSampleMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 mip, 
													const SPtr<RenderTarget>& target)
	{
		mInputTexture.set(source);
		gReflectionCubeImportanceSampleParamDef.gCubeFace.set(mParamBuffer, face);
		gReflectionCubeImportanceSampleParamDef.gMipLevel.set(mParamBuffer, mip);
		gReflectionCubeImportanceSampleParamDef.gNumMips.set(mParamBuffer, source->getProperties().getNumMipmaps() + 1);

		float width = (float)source->getProperties().getWidth();
		float height = (float)source->getProperties().getHeight();

		// First part of the equation for determining mip level to sample from.
		// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
		float mipFactor = 0.5f * std::log2(width * height / NUM_SAMPLES);
		gReflectionCubeImportanceSampleParamDef.gPrecomputedMipFactor.set(mParamBuffer, mipFactor);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

	const static UINT32 TILE_WIDTH = 8;
	const static UINT32 TILE_HEIGHT = 8;
	const static UINT32 PIXELS_PER_THREAD = 4;

	template<int ORDER>
	IrradianceComputeSHMat<ORDER>::IrradianceComputeSHMat()
	{
		mParamBuffer = gIrradianceComputeSHParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Params", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gInputTex", mInputTexture);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBuffer);
	}

	template<int ORDER>
	void IrradianceComputeSHMat<ORDER>::_initDefines(ShaderDefines& defines)
	{
		// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
		defines.set("TILE_WIDTH", TILE_WIDTH);
		defines.set("TILE_HEIGHT", TILE_HEIGHT);

		// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
		defines.set("PIXELS_PER_THREAD", PIXELS_PER_THREAD);

		defines.set("ORDER", ORDER);
	}

	template<int ORDER>
	void IrradianceComputeSHMat<ORDER>::execute(const SPtr<Texture>& source, UINT32 face, const SPtr<GpuBuffer>& output)
	{
		auto& props = source->getProperties();
		UINT32 faceSize = props.getWidth();
		assert(faceSize == props.getHeight());

		Vector2I dispatchSize;
		dispatchSize.x = Math::divideAndRoundUp(faceSize, TILE_WIDTH * PIXELS_PER_THREAD);
		dispatchSize.y = Math::divideAndRoundUp(faceSize, TILE_HEIGHT * PIXELS_PER_THREAD);

		mInputTexture.set(source);
		gIrradianceComputeSHParamDef.gCubeFace.set(mParamBuffer, face);
		gIrradianceComputeSHParamDef.gFaceSize.set(mParamBuffer, source->getProperties().getWidth());
		gIrradianceComputeSHParamDef.gDispatchSize.set(mParamBuffer, dispatchSize);

		mOutputBuffer.set(output);

		RenderAPI& rapi = RenderAPI::instance();

		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(dispatchSize.x, dispatchSize.y);
	}

	template<int ORDER>
	SPtr<GpuBuffer> IrradianceComputeSHMat<ORDER>::createOutputBuffer(const SPtr<Texture>& source, UINT32& numCoeffSets)
	{
		auto& props = source->getProperties();
		UINT32 faceSize = props.getWidth();
		assert(faceSize == props.getHeight());

		Vector2I dispatchSize;
		dispatchSize.x = Math::divideAndRoundUp(faceSize, TILE_WIDTH * PIXELS_PER_THREAD);
		dispatchSize.y = Math::divideAndRoundUp(faceSize, TILE_HEIGHT * PIXELS_PER_THREAD);

		numCoeffSets = dispatchSize.x * dispatchSize.y * 6;

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STRUCTURED;
		bufferDesc.elementCount = numCoeffSets;
		bufferDesc.format = BF_UNKNOWN;
		bufferDesc.randomGpuWrite = true;

		if(ORDER == 3)
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight3);
		else
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight5);

		return GpuBuffer::create(bufferDesc);
	}

	template class IrradianceComputeSHMat<3>;
	template class IrradianceComputeSHMat<5>;

	IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

	template<int ORDER>
	IrradianceReduceSHMat<ORDER>::IrradianceReduceSHMat()
	{
		mParamBuffer = gIrradianceReduceSHParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Params", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gInput", mInputBuffer);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBuffer);
	}

	template<int ORDER>
	void IrradianceReduceSHMat<ORDER>::_initDefines(ShaderDefines& defines)
	{
		defines.set("ORDER", ORDER);
	}

	template<int ORDER>
	void IrradianceReduceSHMat<ORDER>::execute(const SPtr<GpuBuffer>& source, UINT32 numCoeffSets, 
		const SPtr<GpuBuffer>& output, UINT32 outputIdx)
	{
		gIrradianceReduceSHParamDef.gNumEntries.set(mParamBuffer, numCoeffSets);
		gIrradianceReduceSHParamDef.gOutputIdx.set(mParamBuffer, outputIdx);

		mInputBuffer.set(source);
		mOutputBuffer.set(output);

		RenderAPI& rapi = RenderAPI::instance();

		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(1);
	}

	template<int ORDER>
	SPtr<GpuBuffer> IrradianceReduceSHMat<ORDER>::createOutputBuffer(UINT32 numEntries)
	{
		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STRUCTURED;
		bufferDesc.elementCount = numEntries;
		bufferDesc.format = BF_UNKNOWN;
		bufferDesc.randomGpuWrite = true;

		if(ORDER == 3)
			bufferDesc.elementSize = sizeof(SHVector3RGB);
		else
			bufferDesc.elementSize = sizeof(SHVector5RGB);

		return GpuBuffer::create(bufferDesc);
	}

	template class IrradianceReduceSHMat<3>;
	template class IrradianceReduceSHMat<5>;

	IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

	IrradianceProjectSHMat::IrradianceProjectSHMat()
	{
		mParamBuffer = gIrradianceProjectSHParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Params", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mInputBuffer);
	}

	void IrradianceProjectSHMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void IrradianceProjectSHMat::execute(const SPtr<GpuBuffer>& shCoeffs, UINT32 face, const SPtr<RenderTarget>& target)
	{
		gIrradianceProjectSHParamDef.gCubeFace.set(mParamBuffer, face);

		mInputBuffer.set(shCoeffs);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	const UINT32 IBLUtility::REFLECTION_CUBEMAP_SIZE = 256;
	const UINT32 IBLUtility::IRRADIANCE_CUBEMAP_SIZE = 32;

	void IBLUtility::filterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch)
	{
		auto& props = cubemap->getProperties();

		SPtr<Texture> scratchCubemap = scratch;
		if (scratchCubemap == nullptr)
		{
			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = props.getFormat();
			cubemapDesc.width = props.getWidth();
			cubemapDesc.height = props.getHeight();
			cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
			cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			scratchCubemap = Texture::create(cubemapDesc);
		}

		// We sample the cubemaps using importance sampling to generate roughness
		UINT32 numMips = props.getNumMipmaps();

		// Before importance sampling the cubemaps we first create box filtered versions for each mip level. This helps fix
		// the aliasing artifacts that would otherwise be noticeable on importance sampled cubemaps. The aliasing happens
		// because: 
		//  1. We use the same random samples for all pixels, which appears to duplicate reflections instead of creating
		//     noise, which is usually more acceptable 
		//  2. Even if we were to use fully random samples we would need a lot to avoid noticeable noise, which isn't
		//     practical

		// Copy base mip level to scratch cubemap
		for (UINT32 face = 0; face < 6; face++)
			cubemap->copy(scratchCubemap, face, 0, face, 0);

		// Fill out remaining scratch mip levels by downsampling
		for (UINT32 mip = 1; mip < numMips; mip++)
		{
			UINT32 sourceMip = mip - 1;
			downsampleCubemap(scratchCubemap, sourceMip, scratchCubemap, mip);
		}

		// Importance sample
		for (UINT32 mip = 1; mip < numMips; mip++)
		{
			for (UINT32 face = 0; face < 6; face++)
			{
				RENDER_TEXTURE_DESC cubeFaceRTDesc;
				cubeFaceRTDesc.colorSurfaces[0].texture = cubemap;
				cubeFaceRTDesc.colorSurfaces[0].face = face;
				cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
				cubeFaceRTDesc.colorSurfaces[0].mipLevel = mip;

				SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);

				m->importanceSampleMat.execute(scratchCubemap, face, mip, target);
			}
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(nullptr);
	}

	void IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output)
	{
		UINT32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = IrradianceComputeSHMat<5>::createOutputBuffer(cubemap, numCoeffSets);
		for (UINT32 face = 0; face < 6; face++)
			m->shCompute5.execute(cubemap, face, coeffSetBuffer);

		SPtr<GpuBuffer> coeffBuffer = IrradianceReduceSHMat<5>::createOutputBuffer(1);
		m->shReduce5.execute(coeffSetBuffer, numCoeffSets, coeffBuffer, 0);

		for (UINT32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = output;
			cubeFaceRTDesc.colorSurfaces[0].face = face;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

			SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
			m->shProject5.execute(coeffBuffer, face, target);
		}
	}

	void IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<GpuBuffer>& output, 
		UINT32 outputIdx)
	{
		UINT32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = IrradianceComputeSHMat<3>::createOutputBuffer(cubemap, numCoeffSets);
		for (UINT32 face = 0; face < 6; face++)
			m->shCompute3.execute(cubemap, face, coeffSetBuffer);

		m->shReduce3.execute(coeffSetBuffer, numCoeffSets, output, outputIdx);
	}

	void IBLUtility::scaleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip)
	{
		auto& srcProps = src->getProperties();
		auto& dstProps = dst->getProperties();

		SPtr<Texture> scratchTex = src;
		int sizeSrcLog2 = (int)log2((float)srcProps.getWidth());
		int sizeDstLog2 = (int)log2((float)dstProps.getWidth());

		int sizeLog2Diff = sizeSrcLog2 - sizeDstLog2;

		// If size difference is greater than one mip-level and we're downscaling, we need to generate intermediate mip
		// levels
		if(sizeLog2Diff > 1)
		{
			UINT32 mipSize = (UINT32)exp2((float)(sizeSrcLog2 - 1));
			UINT32 numDownsamples = sizeLog2Diff - 1;

			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = srcProps.getFormat();
			cubemapDesc.width = mipSize;
			cubemapDesc.height = mipSize;
			cubemapDesc.numMips = numDownsamples - 1;
			cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			scratchTex = Texture::create(cubemapDesc);

			downsampleCubemap(src, srcMip, scratchTex, 0);
			for(UINT32 i = 0; i < cubemapDesc.numMips; i++)
				downsampleCubemap(scratchTex, i, scratchTex, i + 1);

			srcMip = cubemapDesc.numMips;
		}

		// Same size so just copy
		if(sizeSrcLog2 == sizeDstLog2)
		{
			for (UINT32 face = 0; face < 6; face++)
				src->copy(dst, face, srcMip, face, dstMip);
		}
		else
			downsampleCubemap(scratchTex, srcMip, dst, dstMip);
	}

	SPtr<Texture> IBLUtility::getPreintegratedEnvBRDF()
	{
		return m->preIntegratedGF;
	}

	void IBLUtility::downsampleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip)
	{
		for (UINT32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = dst;
			cubeFaceRTDesc.colorSurfaces[0].face = face;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			cubeFaceRTDesc.colorSurfaces[0].mipLevel = dstMip;

			SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);

			TextureSurface sourceSurface(srcMip, 1, 0, 6);
			m->downsampleMat.execute(src, face, sourceSurface, target);
		}
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

	SPtr<Texture> IBLUtility::generatePreintegratedEnvBRDF()
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

}}
