//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		mParamBuffer = gReflectionCubeDownsampleParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Input", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubeDownsampleMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 mip,
		const SPtr<RenderTarget>& target)
	{
		BS_RENMAT_PROFILE_BLOCK

		gReflectionCubeDownsampleParamDef.gCubeFace.Set(mParamBuffer, face);

		const RenderAPICapabilities& caps = gCaps();
		if(caps.HasCapability(RSC_TEXTURE_VIEWS))
		{
			mInputTexture.Set(source, TextureSurface(mip, 1, 0, 6));
			gReflectionCubeDownsampleParamDef.gMipLevel.Set(mParamBuffer, 0);
		}
		else
		{
			mInputTexture.Set(source);
			gReflectionCubeDownsampleParamDef.gMipLevel.Set(mParamBuffer, mip);
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(target);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	const UINT32 ReflectionCubeImportanceSampleMat::NUM_SAMPLES = 1024;
	ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

	ReflectionCubeImportanceSampleMat::ReflectionCubeImportanceSampleMat()
	{
		mParamBuffer = gReflectionCubeImportanceSampleParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Input", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubeImportanceSampleMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("NUM_SAMPLES", NUM_SAMPLES);
	}

	void ReflectionCubeImportanceSampleMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 mip,
		const SPtr<RenderTarget>& target)
	{
		BS_RENMAT_PROFILE_BLOCK

		mInputTexture.Set(source);
		gReflectionCubeImportanceSampleParamDef.gCubeFace.Set(mParamBuffer, face);
		gReflectionCubeImportanceSampleParamDef.gMipLevel.Set(mParamBuffer, mip);
		gReflectionCubeImportanceSampleParamDef.gNumMips.Set(mParamBuffer, source->GetProperties().getNumMipmaps() + 1);

		float width = (float)source->GetProperties().GetWidth();
		float height = (float)source->GetProperties().GetHeight();

		// First part of the equation for determining mip level to sample from.
		// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
		float mipFactor = 0.5f * std::log2(width * height / NUM_SAMPLES);
		gReflectionCubeImportanceSampleParamDef.gPrecomputedMipFactor.Set(mParamBuffer, mipFactor);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(target);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

	// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
	const static UINT32 TILE_WIDTH = 8;
	const static UINT32 TILE_HEIGHT = 8;

	// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
	const static UINT32 PIXELS_PER_THREAD = 4;

	IrradianceComputeSHMat::IrradianceComputeSHMat()
	{
		mParamBuffer = gIrradianceComputeSHParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gInputTex", mInputTexture);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBuffer);
	}

	void IrradianceComputeSHMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILE_WIDTH", TILE_WIDTH);
		defines.Set("TILE_HEIGHT", TILE_HEIGHT);
		defines.Set("PIXELS_PER_THREAD", PIXELS_PER_THREAD);
	}

	void IrradianceComputeSHMat::Execute(const SPtr<Texture>& source, UINT32 face, const SPtr<GpuBuffer>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		auto& props = source->GetProperties();
		UINT32 faceSize = props.GetWidth();
		assert(faceSize == props.GetHeight());

		Vector2I dispatchSize;
		dispatchSize.x = Math::DivideAndRoundUp(faceSize, TILE_WIDTH * PIXELS_PER_THREAD);
		dispatchSize.y = Math::DivideAndRoundUp(faceSize, TILE_HEIGHT * PIXELS_PER_THREAD);

		mInputTexture.Set(source);
		gIrradianceComputeSHParamDef.gCubeFace.Set(mParamBuffer, face);
		gIrradianceComputeSHParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().getWidth());
		gIrradianceComputeSHParamDef.gDispatchSize.Set(mParamBuffer, dispatchSize);

		mOutputBuffer.Set(output);

		RenderAPI& rapi = RenderAPI::instance();

		bind();
		rapi.DispatchCompute(dispatchSize.x, dispatchSize.y);
	}

	SPtr<GpuBuffer> IrradianceComputeSHMat::CreateOutputBuffer(const SPtr<Texture>& source, UINT32& numCoeffSets)
	{
		auto& props = source->GetProperties();
		UINT32 faceSize = props.GetWidth();
		assert(faceSize == props.GetHeight());

		Vector2I dispatchSize;
		dispatchSize.x = Math::DivideAndRoundUp(faceSize, TILE_WIDTH * PIXELS_PER_THREAD);
		dispatchSize.y = Math::DivideAndRoundUp(faceSize, TILE_HEIGHT * PIXELS_PER_THREAD);

		numCoeffSets = dispatchSize.x * dispatchSize.y * 6;

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STRUCTURED;
		bufferDesc.elementCount = numCoeffSets;
		bufferDesc.format = BF_UNKNOWN;
		bufferDesc.usage = GBU_LOADSTORE;

		if(mVariation.GetInt("SH_ORDER") == 3)
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight3);
		else
			bufferDesc.elementSize = sizeof(SHCoeffsAndWeight5);

		return GpuBuffer::Create(bufferDesc);
	}

	IrradianceComputeSHMat* IrradianceComputeSHMat::getVariation(int order)
	{
		if (order == 3)
			return Get(getVariation<3>());

		return Get(getVariation<5>());
	}

	IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

	IrradianceComputeSHFragMat::IrradianceComputeSHFragMat()
	{
		mParamBuffer = gIrradianceComputeSHFragParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceComputeSHFragMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 coefficientIdx,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mInputTexture.Set(source);

		gIrradianceComputeSHFragParamDef.gCubeFace.Set(mParamBuffer, face);
		gIrradianceComputeSHFragParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().getWidth());
		gIrradianceComputeSHFragParamDef.gCoeffEntryIdx.Set(mParamBuffer, coefficientIdx / 4);
		gIrradianceComputeSHFragParamDef.gCoeffComponentIdx.Set(mParamBuffer, coefficientIdx % 4);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(output);

		bind();
		gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceComputeSHFragMat::GetOutputDesc(const SPtr<Texture>& input)
	{
		auto& props = input->GetProperties();
		return POOLED_RENDER_TEXTURE_DESC::createCube(PF_RGBA16F, props.GetWidth(), props.getHeight(), TU_RENDERTARGET);
	}

	IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

	IrradianceAccumulateSHMat::IrradianceAccumulateSHMat()
	{
		mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceAccumulateSHMat::execute(const SPtr<Texture>& source, UINT32 face, UINT32 sourceMip,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mInputTexture.Set(source);

		auto& props = source->GetProperties();
		Vector2 HalfPixel(0.5f / props.GetWidth(), 0.5f / props.getHeight());

		gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, face);
		gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
		gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(output);

		bind();
		gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateSHMat::GetOutputDesc(const SPtr<Texture>& input)
	{
		auto& props = input->GetProperties();

		// Assuming it's a cubemap
		UINT32 size = std::max(1U, (UINT32)(props.GetWidth() * 0.5f));

		return POOLED_RENDER_TEXTURE_DESC::createCube(PF_RGBA32F, size, size, TU_RENDERTARGET);
	}

	IrradianceAccumulateCubeSHMat::IrradianceAccumulateCubeSHMat()
	{
		mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void IrradianceAccumulateCubeSHMat::execute(const SPtr<Texture>& source, UINT32 sourceMip, const Vector2I& outputOffset,
		UINT32 coefficientIdx, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mInputTexture.Set(source);

		auto& props = source->GetProperties();
		Vector2 HalfPixel(0.5f / props.GetWidth(), 0.5f / props.getHeight());

		gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, 0);
		gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
		gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

		auto& rtProps = output->GetProperties();

		// Render to just one pixel corresponding to the coefficient
		Rect2 viewRect;
		viewRect.x = (outputOffset.x + coefficientIdx) / (float)rtProps.width;
		viewRect.y = outputOffset.y / (float)rtProps.height;

		viewRect.width = 1.0f / rtProps.width;
		viewRect.height = 1.0f / rtProps.height;

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(output);
		rapi.SetViewport(viewRect);

		bind();
		gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
		rapi.SetViewport(Rect2(0, 0, 1, 1));
	}

	POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateCubeSHMat::GetOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA32F, 9, 1, TU_RENDERTARGET);
	}

	IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

	IrradianceReduceSHMat::IrradianceReduceSHMat()
	{
		mParamBuffer = gIrradianceReduceSHParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gInput", mInputBuffer);
		mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTexture);
	}

	void IrradianceReduceSHMat::execute(const SPtr<GpuBuffer>& source, UINT32 numCoeffSets,
		const SPtr<Texture>& output, UINT32 outputIdx)
	{
		BS_RENMAT_PROFILE_BLOCK

		UINT32 shOrder = (UINT32)mVariation.GetInt("SH_ORDER");

		Vector2I outputCoords = IBLUtility::getSHCoeffXYFromIdx(outputIdx, shOrder);
		gIrradianceReduceSHParamDef.gOutputIdx.Set(mParamBuffer, outputCoords);
		gIrradianceReduceSHParamDef.gNumEntries.Set(mParamBuffer, numCoeffSets);

		mInputBuffer.Set(source);
		mOutputTexture.Set(output);

		bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.DispatchCompute(1);
	}

	SPtr<Texture> IrradianceReduceSHMat::CreateOutputTexture(UINT32 numCoeffSets)
	{
		UINT32 shOrder = (UINT32)mVariation.GetInt("SH_ORDER");
		Vector2I size = IBLUtility::getSHCoeffTextureSize(numCoeffSets, shOrder);

		TEXTURE_DESC textureDesc;
		textureDesc.width = (UINT32)size.x;
		textureDesc.height = (UINT32)size.y;
		textureDesc.format = PF_RGBA32F;
		textureDesc.usage = TU_STATIC | TU_LOADSTORE;

		return Texture::Create(textureDesc);
	}

	IrradianceReduceSHMat* IrradianceReduceSHMat::getVariation(int order)
	{
		if (order == 3)
			return Get(getVariation<3>());

		return Get(getVariation<5>());
	}

	IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

	IrradianceProjectSHMat::IrradianceProjectSHMat()
	{
		mParamBuffer = gIrradianceProjectSHParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mInputTexture);
	}

	void IrradianceProjectSHMat::Execute(const SPtr<Texture>& shCoeffs, UINT32 face, const SPtr<RenderTarget>& target)
	{
		BS_RENMAT_PROFILE_BLOCK

		gIrradianceProjectSHParamDef.gCubeFace.Set(mParamBuffer, face);

		mInputTexture.Set(shCoeffs);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(target);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	void RenderBeastIBLUtility::FilterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const
	{
		auto& props = cubemap->GetProperties();

		SPtr<Texture> scratchCubemap = scratch;
		if (scratchCubemap == nullptr)
		{
			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = props.GetFormat();
			cubemapDesc.width = props.GetWidth();
			cubemapDesc.height = props.GetHeight();
			cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
			cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			scratchCubemap = Texture::create(cubemapDesc);
		}

		// We sample the cubemaps using importance sampling to generate roughness
		UINT32 numMips = props.GetNumMipmaps() + 1;

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

			cubemap->Copy(scratchCubemap, copyDesc);
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
				material->Execute(scratchCubemap, face, mip, target);
			}
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(nullptr);
	}

	bool SupportsComputeSH()
	{
		return GRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	}

	void RenderBeastIBLUtility::FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const
	{
		SPtr<Texture> coeffTexture;
		if(supportsComputeSH())
		{
			IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::getVariation(5);
			IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::getVariation(5);

			UINT32 numCoeffSets;
			SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
			for (UINT32 face = 0; face < 6; face++)
				shCompute->Execute(cubemap, face, coeffSetBuffer);

			coeffTexture = shReduce->CreateOutputTexture(1);
			shReduce->Execute(coeffSetBuffer, numCoeffSets, coeffTexture, 0);
		}
		else
		{
			SPtr<PooledRenderTexture> finalCoeffs = gGpuResourcePool().Get(IrradianceAccumulateCubeSHMat::getOutputDesc());

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
			shProject->Execute(coeffTexture, face, target);
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
			SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
			for (UINT32 face = 0; face < 6; face++)
				shCompute->Execute(cubemap, face, coeffSetBuffer);

			shReduce->Execute(coeffSetBuffer, numCoeffSets, output, outputIdx);
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
		auto& srcProps = src->GetProperties();
		auto& dstProps = dst->GetProperties();

		SPtr<Texture> scratchTex = src;
		int sizeSrcLog2 = (int)log2((float)srcProps.GetWidth());
		int sizeDstLog2 = (int)log2((float)dstProps.GetWidth());

		int sizeLog2Diff = sizeSrcLog2 - sizeDstLog2;

		// If size difference is greater than one mip-level and we're downscaling, we need to generate intermediate mip
		// levels
		if(sizeLog2Diff > 1)
		{
			UINT32 mipSize = (UINT32)exp2((float)(sizeSrcLog2 - 1));
			UINT32 numDownsamples = sizeLog2Diff - 1;

			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = srcProps.GetFormat();
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

				src->Copy(dst, copyDesc);
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
			material->Execute(src, face, srcMip, target);
		}
	}

	void RenderBeastIBLUtility::filterCubemapForIrradianceNonCompute(const SPtr<Texture>& cubemap, UINT32 outputIdx,
		const SPtr<RenderTexture>& output)
	{
		static const UINT32 NUM_COEFFS = 9;

		GpuResourcePool& resPool = gGpuResourcePool();
		IrradianceComputeSHFragMat* shCompute = IrradianceComputeSHFragMat::get();
		IrradianceAccumulateSHMat* shAccum = IrradianceAccumulateSHMat::get();
		IrradianceAccumulateCubeSHMat* shAccumCube = IrradianceAccumulateCubeSHMat::get();

		for(UINT32 coeff = 0; coeff < NUM_COEFFS; ++coeff)
		{
			SPtr<PooledRenderTexture> coeffsTex = resPool.Get(shCompute->GetOutputDesc(cubemap));
			
			// Generate SH coefficients and weights per-texel
			for(UINT32 face = 0; face < 6; face++)
			{
				RENDER_TEXTURE_DESC cubeFaceRTDesc;
				cubeFaceRTDesc.colorSurfaces[0].texture = coeffsTex->texture;
				cubeFaceRTDesc.colorSurfaces[0].face = face;
				cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
				cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

				SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
				shCompute->Execute(cubemap, face, coeff, target);
			}

			// Downsample, summing up coefficients and weights all the way down to 1x1
			auto& sourceProps = cubemap->GetProperties();
			UINT32 numMips = PixelUtil::getMaxMipmaps(sourceProps.GetWidth(), sourceProps.getHeight(), 1,
				sourceProps.GetFormat());

			SPtr<PooledRenderTexture> downsampleInput = coeffsTex;
			coeffsTex = nullptr;

			for(UINT32 mip = 0; mip < numMips; mip++)
			{
				SPtr<PooledRenderTexture> accumCoeffsTex = resPool.Get(shAccum->GetOutputDesc(downsampleInput->texture));

				for(UINT32 face = 0; face < 6; face++)
				{
					RENDER_TEXTURE_DESC cubeFaceRTDesc;
					cubeFaceRTDesc.colorSurfaces[0].texture = accumCoeffsTex->texture;
					cubeFaceRTDesc.colorSurfaces[0].face = face;
					cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
					cubeFaceRTDesc.colorSurfaces[0].mipLevel = 0;

					SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);
					shAccum->Execute(downsampleInput->texture, face, 0, target);
				}

				downsampleInput = accumCoeffsTex;
			}

			// Sum up all the faces and write the coefficient to the final texture
			Vector2I outputOffset = getSHCoeffXYFromIdx(outputIdx, 3);
			shAccumCube->Execute(downsampleInput->texture, 0, outputOffset, coeff, output);
		}
	}
}}
