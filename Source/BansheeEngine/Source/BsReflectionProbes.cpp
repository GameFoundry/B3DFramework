//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsReflectionProbes.h"
#include "BsTexture.h"
#include "BsGpuParamsSet.h"
#include "BsRendererUtility.h"

namespace bs { namespace ct
{
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

		float width = (float)source->getProperties().getWidth();
		float height = (float)source->getProperties().getHeight();

		// First part of the equation for determining mip level for sample from.
		// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
		float mipFactor = 0.5f * std::log2(width * height / NUM_SAMPLES);
		gReflectionCubeImportanceSampleParamDef.gPrecomputedMipFactor.set(mParamBuffer, mipFactor);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	const UINT32 ReflectionProbes::REFLECTION_CUBEMAP_SIZE = 256;

	void ReflectionProbes::filterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch)
	{
		static ReflectionCubeDownsampleMat downsampleMat;
		static ReflectionCubeImportanceSampleMat importanceSampleMat;

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

				importanceSampleMat.execute(scratchCubemap, face, mip, target);
			}
		}
	}

	void ReflectionProbes::scaleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip)
	{
		static ReflectionCubeDownsampleMat downsampleMat;

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
			downsampleCubemap(src, srcMip, dst, dstMip);
	}

	void ReflectionProbes::downsampleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip)
	{
		static ReflectionCubeDownsampleMat downsampleMat;

		for (UINT32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = dst;
			cubeFaceRTDesc.colorSurfaces[0].face = face;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			cubeFaceRTDesc.colorSurfaces[0].mipLevel = dstMip;

			SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);

			TextureSurface sourceSurface(srcMip, 1, 0, 6);
			downsampleMat.execute(src, face, sourceSurface, target);
		}
	}
}}
