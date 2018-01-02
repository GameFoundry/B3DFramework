//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeastIBLUtility.h"
#include "Image/BsTexture.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsRenderBeast.h"

namespace bs { namespace ct
{
	ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

	ReflectionCubeDownsampleMat::ReflectionCubeDownsampleMat()
	{
		mParamBuffer = gReflectionCubeDownsampleParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubeDownsampleMat::execute(
		const SPtr<Texture>& source,
		UINT32 face,
		UINT32 mip,
		const SPtr<RenderTarget>& target)
	{
		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		gReflectionCubeDownsampleParamDef.gCubeFace.set(mParamBuffer, face);

		if(rapiInfo.isFlagSet(RenderAPIFeatureFlag::TextureViews))
		{
			mInputTexture.set(source, TextureSurface(mip, 1, 0, 6));
			gReflectionCubeDownsampleParamDef.gMipLevel.set(mParamBuffer, 0);
		}
		else
		{
			mInputTexture.set(source);
			gReflectionCubeDownsampleParamDef.gMipLevel.set(mParamBuffer, mip);
		}

		rapi.setRenderTarget(target);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	const UINT32 ReflectionCubeImportanceSampleMat::NUM_SAMPLES = 1024;
	ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

	ReflectionCubeImportanceSampleMat::ReflectionCubeImportanceSampleMat()
	{
		mParamBuffer = gReflectionCubeImportanceSampleParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
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

		bind();
		gRendererUtility().drawScreenQuad();
	}

	IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

	// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
	const static UINT32 TILE_WIDTH = 8;
	const static UINT32 TILE_HEIGHT = 8;

	// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
	const static UINT32 PIXELS_PER_THREAD = 4;

	IrradianceComputeSHMat::IrradianceComputeSHMat()
	{
		mParamBuffer = gIrradianceComputeSHParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gInputTex", mInputTexture);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBuffer);
	}

	void IrradianceComputeSHMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_WIDTH", TILE_WIDTH);
		defines.set("TILE_HEIGHT", TILE_HEIGHT);
		defines.set("PIXELS_PER_THREAD", PIXELS_PER_THREAD);
	}

	void IrradianceComputeSHMat::execute(const SPtr<Texture>& source, UINT32 face, const SPtr<GpuBuffer>& output)
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

		bind();
		rapi.dispatchCompute(dispatchSize.x, dispatchSize.y);
	}

	SPtr<GpuBuffer> IrradianceComputeSHMat::createOutputBuffer(const SPtr<Texture>& source, UINT32& numCoeffSets)
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

		if(mVariation.getInt("SH_ORDER") == 3)
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight3);
		else
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight5);

		return GpuBuffer::create(bufferDesc);
	}

	IrradianceComputeSHMat* IrradianceComputeSHMat::getVariation(int order)
	{
		if (order == 3)
			return get(getVariation<3>());

		return get(getVariation<5>());
	}

	IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

	IrradianceComputeSHFragMat::IrradianceComputeSHFragMat()
	{
		mParamBuffer = gIrradianceComputeSHFragParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceComputeSHFragMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 coefficientIdx, 
		const SPtr<RenderTarget>& output)
	{
		// Set parameters
		mInputTexture.set(source);

		gIrradianceComputeSHFragParamDef.gCubeFace.set(mParamBuffer, face);
		gIrradianceComputeSHFragParamDef.gFaceSize.set(mParamBuffer, source->getProperties().getWidth());
		gIrradianceComputeSHFragParamDef.gCoeffIdx.set(mParamBuffer, coefficientIdx);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceComputeSHFragMat::getOutputDesc(const SPtr<Texture>& input)
	{
		auto& props = input->getProperties();
		return POOLED_RENDER_TEXTURE_DESC::createCube(PF_RGBA16F, props.getWidth(), props.getHeight(), TU_RENDERTARGET);
	}

	IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

	IrradianceAccumulateSHMat::IrradianceAccumulateSHMat()
	{
		mParamBuffer = gIrradianceAccumulateSHParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceAccumulateSHMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 sourceMip, 
		const SPtr<RenderTarget>& output)
	{
		// Set parameters
		mInputTexture.set(source);

		auto& props = source->getProperties();
		Vector2 halfPixel(0.5f / props.getWidth(), 0.5f / props.getHeight());

		gIrradianceAccumulateSHParamDef.gCubeFace.set(mParamBuffer, face);
		gIrradianceAccumulateSHParamDef.gCubeMip.set(mParamBuffer, sourceMip);
		gIrradianceAccumulateSHParamDef.gHalfPixel.set(mParamBuffer, halfPixel);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateSHMat::getOutputDesc(const SPtr<Texture>& input)
	{
		auto& props = input->getProperties();

		// Assuming it's a cubemap
		UINT32 size = std::max(1U, (UINT32)(props.getWidth() * 0.5f));

		return POOLED_RENDER_TEXTURE_DESC::createCube(PF_RGBA16F, size, size, TU_RENDERTARGET);
	}

	IrradianceAccumulateCubeSHMat::IrradianceAccumulateCubeSHMat()
	{
		mParamBuffer = gIrradianceAccumulateSHParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceAccumulateCubeSHMat::execute(const SPtr<Texture>& source, UINT32 sourceMip, const Vector2I& outputOffset,
		UINT32 coefficientIdx, const SPtr<RenderTarget>& output)
	{
		// Set parameters
		mInputTexture.set(source);

		auto& props = source->getProperties();
		Vector2 halfPixel(0.5f / props.getWidth(), 0.5f / props.getHeight());

		gIrradianceAccumulateSHParamDef.gCubeFace.set(mParamBuffer, 0);
		gIrradianceAccumulateSHParamDef.gCubeMip.set(mParamBuffer, sourceMip);
		gIrradianceAccumulateSHParamDef.gHalfPixel.set(mParamBuffer, halfPixel);

		auto& rtProps = output->getProperties();

		// Render to just one pixel corresponding to the coefficient
		Rect2 viewRect;
		viewRect.x = (outputOffset.x + coefficientIdx) / (float)rtProps.width;
		viewRect.y = outputOffset.y / (float)rtProps.height;

		viewRect.width = 1.0f / rtProps.width;
		viewRect.height = 1.0f / rtProps.height;

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);
		rapi.setViewport(viewRect);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
		rapi.setViewport(Rect2(0, 0, 1, 1));
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateCubeSHMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA32F, 9, 1, TU_RENDERTARGET);
	}

	IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

	IrradianceReduceSHMat::IrradianceReduceSHMat()
	{
		mParamBuffer = gIrradianceReduceSHParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInput", mInputBuffer);
		mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTexture);
	}

	void IrradianceReduceSHMat::execute(const SPtr<GpuBuffer>& source, UINT32 numCoeffSets, 
		const SPtr<Texture>& output, UINT32 outputIdx)
	{
		UINT32 shOrder = (UINT32)mVariation.getInt("SH_ORDER");

		Vector2I outputCoords = IBLUtility::getSHCoeffXYFromIdx(outputIdx, shOrder);
		gIrradianceReduceSHParamDef.gOutputIdx.set(mParamBuffer, outputCoords);
		gIrradianceReduceSHParamDef.gNumEntries.set(mParamBuffer, numCoeffSets);

		mInputBuffer.set(source);
		mOutputTexture.set(output);

		bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.dispatchCompute(1);
	}

	SPtr<Texture> IrradianceReduceSHMat::createOutputTexture(UINT32 numCoeffSets)
	{
		UINT32 shOrder = (UINT32)mVariation.getInt("SH_ORDER");
		Vector2I size = IBLUtility::getSHCoeffTextureSize(numCoeffSets, shOrder);

		TEXTURE_DESC textureDesc;
		textureDesc.width = (UINT32)size.x;
		textureDesc.height = (UINT32)size.y;
		textureDesc.format = PF_RGBA32F;
		textureDesc.usage = TU_STATIC | TU_LOADSTORE;

		return Texture::create(textureDesc);
	}

	IrradianceReduceSHMat* IrradianceReduceSHMat::getVariation(int order)
	{
		if (order == 3)
			return get(getVariation<3>());

		return get(getVariation<5>());
	}

	IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

	IrradianceProjectSHMat::IrradianceProjectSHMat()
	{
		mParamBuffer = gIrradianceProjectSHParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mInputTexture);
	}

	void IrradianceProjectSHMat::execute(const SPtr<Texture>& shCoeffs, UINT32 face, const SPtr<RenderTarget>& target)
	{
		gIrradianceProjectSHParamDef.gCubeFace.set(mParamBuffer, face);

		mInputTexture.set(shCoeffs);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	void RenderBeastIBLUtility::filterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const
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
		{
			TEXTURE_COPY_DESC copyDesc;
			copyDesc.srcFace = face;
			copyDesc.dstFace = face;

			cubemap->copy(scratchCubemap, copyDesc);
		}

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

				ReflectionCubeImportanceSampleMat* material = ReflectionCubeImportanceSampleMat::get();
				material->execute(scratchCubemap, face, mip, target);
			}
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(nullptr);
	}

	bool supportsComputeSH()
	{
		return gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;
	}

	void RenderBeastIBLUtility::filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const
	{
		SPtr<Texture> coeffTexture;
		if(supportsComputeSH())
		{
			IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::getVariation(5);
			IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::getVariation(5);

			UINT32 numCoeffSets;
			SPtr<GpuBuffer> coeffSetBuffer = shCompute->createOutputBuffer(cubemap, numCoeffSets);
			for (UINT32 face = 0; face < 6; face++)
				shCompute->execute(cubemap, face, coeffSetBuffer);

			coeffTexture = shReduce->createOutputTexture(1);
			shReduce->execute(coeffSetBuffer, numCoeffSets, coeffTexture, 0);
		}
		else
		{
			GpuResourcePool& resPool = GpuResourcePool::instance();
			SPtr<PooledRenderTexture> finalCoeffs = resPool.get(IrradianceAccumulateCubeSHMat::getOutputDesc());

			filterCubemapForIrradianceNonCompute(cubemap, 0, finalCoeffs->renderTexture);
			coeffTexture = finalCoeffs->texture;
		}

		IrradianceProjectSHMat* shProject = IrradianceProjectSHMat::get();
		for (UINT32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = output;
			cubeFaceRTDesc.colorSurfaces[0].face = face;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

			SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
			shProject->execute(coeffTexture, face, target);
		}
	}
	
	void RenderBeastIBLUtility::filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output, 
		UINT32 outputIdx) const
	{
		if(supportsComputeSH())
		{
			IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::getVariation(3);
			IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::getVariation(3);

			UINT32 numCoeffSets;
			SPtr<GpuBuffer> coeffSetBuffer = shCompute->createOutputBuffer(cubemap, numCoeffSets);
			for (UINT32 face = 0; face < 6; face++)
				shCompute->execute(cubemap, face, coeffSetBuffer);

			shReduce->execute(coeffSetBuffer, numCoeffSets, output, outputIdx);
		}
		else
		{
			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = output;

			SPtr<RenderTexture> target = RenderTexture::create(rtDesc);
			filterCubemapForIrradianceNonCompute(cubemap, outputIdx, target);
		}
	}

	void RenderBeastIBLUtility::scaleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, 
		UINT32 dstMip) const
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
			{
				TEXTURE_COPY_DESC copyDesc;
				copyDesc.srcFace = face;
				copyDesc.srcMip = srcMip;
				copyDesc.dstFace = face;
				copyDesc.dstMip = dstMip;

				src->copy(dst, copyDesc);
			}
		}
		else
			downsampleCubemap(scratchTex, srcMip, dst, dstMip);
	}

	void RenderBeastIBLUtility::downsampleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, 
		UINT32 dstMip)
	{
		for (UINT32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = dst;
			cubeFaceRTDesc.colorSurfaces[0].face = face;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			cubeFaceRTDesc.colorSurfaces[0].mipLevel = dstMip;

			SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);

			ReflectionCubeDownsampleMat* material = ReflectionCubeDownsampleMat::get();
			material->execute(src, face, srcMip, target);
		}
	}

	void RenderBeastIBLUtility::filterCubemapForIrradianceNonCompute(const SPtr<Texture>& cubemap, UINT32 outputIdx,
		const SPtr<RenderTexture>& output)
	{
		static const UINT32 NUM_COEFFS = 9;

		GpuResourcePool& resPool = GpuResourcePool::instance();
		IrradianceComputeSHFragMat* shCompute = IrradianceComputeSHFragMat::get();
		IrradianceAccumulateSHMat* shAccum = IrradianceAccumulateSHMat::get();
		IrradianceAccumulateCubeSHMat* shAccumCube = IrradianceAccumulateCubeSHMat::get();

		for(UINT32 coeff = 0; coeff < NUM_COEFFS; ++coeff)
		{
			SPtr<PooledRenderTexture> coeffsTex = resPool.get(shCompute->getOutputDesc(cubemap));
			
			// Generate SH coefficients and weights per-texel
			for(UINT32 face = 0; face < 6; face++)
			{
				RENDER_TEXTURE_DESC cubeFaceRTDesc;
				cubeFaceRTDesc.colorSurfaces[0].texture = coeffsTex->texture;
				cubeFaceRTDesc.colorSurfaces[0].face = face;
				cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
				cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

				SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
				shCompute->execute(cubemap, face, coeff, target);
			}

			// Downsample, summing up coefficients and weights all the way down to 1x1
			auto& sourceProps = cubemap->getProperties();
			UINT32 numMips = PixelUtil::getMaxMipmaps(sourceProps.getWidth(), sourceProps.getHeight(), 1, 
				sourceProps.getFormat());

			SPtr<PooledRenderTexture> downsampleInput = coeffsTex;
			coeffsTex = nullptr;

			for(UINT32 mip = 0; mip < numMips; mip++)
			{
				SPtr<PooledRenderTexture> accumCoeffsTex = resPool.get(shAccum->getOutputDesc(downsampleInput->texture));

				for(UINT32 face = 0; face < 6; face++)
				{
					RENDER_TEXTURE_DESC cubeFaceRTDesc;
					cubeFaceRTDesc.colorSurfaces[0].texture = accumCoeffsTex->texture;
					cubeFaceRTDesc.colorSurfaces[0].face = face;
					cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
					cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

					SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
					shAccum->execute(downsampleInput->texture, face, 0, target);
				}

				downsampleInput = accumCoeffsTex;
			}

			// Sum up all the faces and write the coefficient to the final texture
			Vector2I outputOffset = getSHCoeffXYFromIdx(outputIdx, 3);
			shAccumCube->execute(downsampleInput->texture, 0, outputOffset, coeff, output);
		}
	}
}}
