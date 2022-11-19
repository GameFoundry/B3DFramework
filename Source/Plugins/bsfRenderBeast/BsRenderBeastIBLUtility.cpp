//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastIBLUtility.h"
#include "Image/BsTexture.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsRenderBeast.h"

namespace bs { namespace ct {

ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

ReflectionCubeDownsampleMat::ReflectionCubeDownsampleMat()
{
	mParamBuffer = gReflectionCubeDownsampleParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void ReflectionCubeDownsampleMat::Execute(const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	gReflectionCubeDownsampleParamDef.gCubeFace.Set(mParamBuffer, face);

	const RenderAPICapabilities& caps = GetRenderBackendCapabilities();
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

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(target);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

const u32 ReflectionCubeImportanceSampleMat::kNumSamples = 1024;
ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

ReflectionCubeImportanceSampleMat::ReflectionCubeImportanceSampleMat()
{
	mParamBuffer = gReflectionCubeImportanceSampleParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void ReflectionCubeImportanceSampleMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_SAMPLES", kNumSamples);
}

void ReflectionCubeImportanceSampleMat::Execute(const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	mInputTexture.Set(source);
	gReflectionCubeImportanceSampleParamDef.gCubeFace.Set(mParamBuffer, face);
	gReflectionCubeImportanceSampleParamDef.gMipLevel.Set(mParamBuffer, mip);
	gReflectionCubeImportanceSampleParamDef.gNumMips.Set(mParamBuffer, source->GetProperties().GetNumMipmaps() + 1);

	float width = (float)source->GetProperties().GetWidth();
	float height = (float)source->GetProperties().GetHeight();

	// First part of the equation for determining mip level to sample from.
	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
	float mipFactor = 0.5f * std::log2(width * height / kNumSamples);
	gReflectionCubeImportanceSampleParamDef.gPrecomputedMipFactor.Set(mParamBuffer, mipFactor);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(target);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
const static u32 kTileWidth = 8;
const static u32 kTileHeight = 8;

// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
const static u32 kPixelsPerThread = 4;

IrradianceComputeSHMat::IrradianceComputeSHMat()
{
	mParamBuffer = gIrradianceComputeSHParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gInputTex", mInputTexture);
	mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBuffer);
}

void IrradianceComputeSHMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_WIDTH", kTileWidth);
	defines.Set("TILE_HEIGHT", kTileHeight);
	defines.Set("PIXELS_PER_THREAD", kPixelsPerThread);
}

void IrradianceComputeSHMat::Execute(const SPtr<Texture>& source, u32 face, const SPtr<GpuBuffer>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	auto& props = source->GetProperties();
	u32 faceSize = props.GetWidth();
	B3D_ASSERT(faceSize == props.GetHeight());

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	mInputTexture.Set(source);
	gIrradianceComputeSHParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceComputeSHParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().GetWidth());
	gIrradianceComputeSHParamDef.gDispatchSize.Set(mParamBuffer, dispatchSize);

	mOutputBuffer.Set(output);

	RenderAPI& rapi = RenderAPI::Instance();

	Bind();
	rapi.DispatchCompute(dispatchSize.X, dispatchSize.Y);
}

SPtr<GpuBuffer> IrradianceComputeSHMat::CreateOutputBuffer(const SPtr<Texture>& source, u32& numCoeffSets)
{
	auto& props = source->GetProperties();
	u32 faceSize = props.GetWidth();
	B3D_ASSERT(faceSize == props.GetHeight());

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	numCoeffSets = dispatchSize.X * dispatchSize.Y * 6;

	GPU_BUFFER_DESC bufferDesc;
	bufferDesc.Type = GBT_STRUCTURED;
	bufferDesc.ElementCount = numCoeffSets;
	bufferDesc.Format = BF_UNKNOWN;
	bufferDesc.Usage = GBU_LOADSTORE;

	if(mVariation.GetInt("SH_ORDER") == 3)
		bufferDesc.ElementSize = sizeof(SHCoeffsAndWeight3);
	else
		bufferDesc.ElementSize = sizeof(SHCoeffsAndWeight5);

	return GpuBuffer::Create(bufferDesc);
}

IrradianceComputeSHMat* IrradianceComputeSHMat::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

IrradianceComputeSHFragMat::IrradianceComputeSHFragMat()
{
	mParamBuffer = gIrradianceComputeSHFragParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void IrradianceComputeSHFragMat::Execute(const SPtr<Texture>& source, u32 face, u32 coefficientIdx, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	gIrradianceComputeSHFragParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceComputeSHFragParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().GetWidth());
	gIrradianceComputeSHFragParamDef.gCoeffEntryIdx.Set(mParamBuffer, coefficientIdx / 4);
	gIrradianceComputeSHFragParamDef.gCoeffComponentIdx.Set(mParamBuffer, coefficientIdx % 4);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC IrradianceComputeSHFragMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();
	return POOLED_RENDER_TEXTURE_DESC::CreateCube(PF_RGBA16F, props.GetWidth(), props.GetHeight(), TU_RENDERTARGET);
}

IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

IrradianceAccumulateSHMat::IrradianceAccumulateSHMat()
{
	mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void IrradianceAccumulateSHMat::Execute(const SPtr<Texture>& source, u32 face, u32 sourceMip, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.GetWidth(), 0.5f / props.GetHeight());

	gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
	gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateSHMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();

	// Assuming it's a cubemap
	u32 size = std::max(1U, (u32)(props.GetWidth() * 0.5f));

	return POOLED_RENDER_TEXTURE_DESC::CreateCube(PF_RGBA32F, size, size, TU_RENDERTARGET);
}

IrradianceAccumulateCubeSHMat::IrradianceAccumulateCubeSHMat()
{
	mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void IrradianceAccumulateCubeSHMat::Execute(const SPtr<Texture>& source, u32 sourceMip, const Vector2I& outputOffset, u32 coefficientIdx, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.GetWidth(), 0.5f / props.GetHeight());

	gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, 0);
	gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
	gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

	auto& rtProps = output->GetProperties();

	// Render to just one pixel corresponding to the coefficient
	Rect2 viewRect;
	viewRect.X = (outputOffset.X + coefficientIdx) / (float)rtProps.Width;
	viewRect.Y = outputOffset.Y / (float)rtProps.Height;

	viewRect.Width = 1.0f / rtProps.Width;
	viewRect.Height = 1.0f / rtProps.Height;

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);
	rapi.SetViewport(viewRect);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
	rapi.SetViewport(Rect2(0, 0, 1, 1));
}

POOLED_RENDER_TEXTURE_DESC IrradianceAccumulateCubeSHMat::GetOutputDesc()
{
	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA32F, 9, 1, TU_RENDERTARGET);
}

IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

IrradianceReduceSHMat::IrradianceReduceSHMat()
{
	mParamBuffer = gIrradianceReduceSHParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gInput", mInputBuffer);
	mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTexture);
}

void IrradianceReduceSHMat::Execute(const SPtr<GpuBuffer>& source, u32 numCoeffSets, const SPtr<Texture>& output, u32 outputIdx)
{
	BS_RENMAT_PROFILE_BLOCK

	u32 shOrder = (u32)mVariation.GetInt("SH_ORDER");

	Vector2I outputCoords = IBLUtility::GetShCoeffXyFromIdx(outputIdx, shOrder);
	gIrradianceReduceSHParamDef.gOutputIdx.Set(mParamBuffer, outputCoords);
	gIrradianceReduceSHParamDef.gNumEntries.Set(mParamBuffer, numCoeffSets);

	mInputBuffer.Set(source);
	mOutputTexture.Set(output);

	Bind();

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.DispatchCompute(1);
}

SPtr<Texture> IrradianceReduceSHMat::CreateOutputTexture(u32 numCoeffSets)
{
	u32 shOrder = (u32)mVariation.GetInt("SH_ORDER");
	Vector2I size = IBLUtility::GetShCoeffTextureSize(numCoeffSets, shOrder);

	TEXTURE_DESC textureDesc;
	textureDesc.Width = (u32)size.X;
	textureDesc.Height = (u32)size.Y;
	textureDesc.Format = PF_RGBA32F;
	textureDesc.Usage = TU_STATIC | TU_LOADSTORE;

	return Texture::Create(textureDesc);
}

IrradianceReduceSHMat* IrradianceReduceSHMat::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

IrradianceProjectSHMat::IrradianceProjectSHMat()
{
	mParamBuffer = gIrradianceProjectSHParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mInputTexture);
}

void IrradianceProjectSHMat::Execute(const SPtr<Texture>& shCoeffs, u32 face, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	gIrradianceProjectSHParamDef.gCubeFace.Set(mParamBuffer, face);

	mInputTexture.Set(shCoeffs);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(target);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

void RenderBeastIBLUtility::FilterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const
{
	auto& props = cubemap->GetProperties();

	SPtr<Texture> scratchCubemap = scratch;
	if(scratchCubemap == nullptr)
	{
		TEXTURE_DESC cubemapDesc;
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = props.GetFormat();
		cubemapDesc.Width = props.GetWidth();
		cubemapDesc.Height = props.GetHeight();
		cubemapDesc.NumMips = PixelUtil::GetMaxMipmaps(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		scratchCubemap = Texture::Create(cubemapDesc);
	}

	// We sample the cubemaps using importance sampling to generate roughness
	u32 numMips = props.GetNumMipmaps() + 1;

	// Before importance sampling the cubemaps we first create box filtered versions for each mip level. This helps fix
	// the aliasing artifacts that would otherwise be noticeable on importance sampled cubemaps. The aliasing happens
	// because:
	//  1. We use the same random samples for all pixels, which appears to duplicate reflections instead of creating
	//     noise, which is usually more acceptable
	//  2. Even if we were to use fully random samples we would need a lot to avoid noticeable noise, which isn't
	//     practical

	// Copy base mip level to scratch cubemap
	for(u32 face = 0; face < 6; face++)
	{
		TextureCopyInformation copyDesc;
		copyDesc.SourceFace = face;
		copyDesc.DestinationFace = face;

		cubemap->Copy(scratchCubemap, copyDesc);
	}

	// Fill out remaining scratch mip levels by downsampling
	for(u32 mip = 1; mip < numMips; mip++)
	{
		u32 sourceMip = mip - 1;
		DownsampleCubemap(scratchCubemap, sourceMip, scratchCubemap, mip);
	}

	// Importance sample
	for(u32 mip = 1; mip < numMips; mip++)
	{
		for(u32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.ColorSurfaces[0].Texture = cubemap;
			cubeFaceRTDesc.ColorSurfaces[0].Face = face;
			cubeFaceRTDesc.ColorSurfaces[0].NumFaces = 1;
			cubeFaceRTDesc.ColorSurfaces[0].MipLevel = mip;

			SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

			ReflectionCubeImportanceSampleMat* material = ReflectionCubeImportanceSampleMat::Get();
			material->Execute(scratchCubemap, face, mip, target);
		}
	}

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(nullptr);
}

bool SupportsComputeSh()
{
	return GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const
{
	SPtr<Texture> coeffTexture;
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::GetVariation(5);
		IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::GetVariation(5);

		u32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(cubemap, face, coeffSetBuffer);

		coeffTexture = shReduce->CreateOutputTexture(1);
		shReduce->Execute(coeffSetBuffer, numCoeffSets, coeffTexture, 0);
	}
	else
	{
		SPtr<PooledRenderTexture> finalCoeffs = GetGpuResourcePool().Get(IrradianceAccumulateCubeSHMat::GetOutputDesc());

		FilterCubemapForIrradianceNonCompute(cubemap, 0, finalCoeffs->RenderTexture);
		coeffTexture = finalCoeffs->Texture;
	}

	IrradianceProjectSHMat* shProject = IrradianceProjectSHMat::Get();
	for(u32 face = 0; face < 6; face++)
	{
		RENDER_TEXTURE_DESC cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = output;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].NumFaces = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

		SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
		shProject->Execute(coeffTexture, face, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const
{
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::GetVariation(3);
		IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::GetVariation(3);

		u32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(cubemap, face, coeffSetBuffer);

		shReduce->Execute(coeffSetBuffer, numCoeffSets, output, outputIdx);
	}
	else
	{
		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.ColorSurfaces[0].Texture = output;

		SPtr<RenderTexture> target = RenderTexture::Create(rtDesc);
		FilterCubemapForIrradianceNonCompute(cubemap, outputIdx, target);
	}
}

void RenderBeastIBLUtility::ScaleCubemap(const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const
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
		u32 mipSize = (u32)exp2((float)(sizeSrcLog2 - 1));
		u32 numDownsamples = sizeLog2Diff - 1;

		TEXTURE_DESC cubemapDesc;
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = srcProps.GetFormat();
		cubemapDesc.Width = mipSize;
		cubemapDesc.Height = mipSize;
		cubemapDesc.NumMips = numDownsamples - 1;
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		scratchTex = Texture::Create(cubemapDesc);

		DownsampleCubemap(src, srcMip, scratchTex, 0);
		for(u32 i = 0; i < cubemapDesc.NumMips; i++)
			DownsampleCubemap(scratchTex, i, scratchTex, i + 1);

		srcMip = cubemapDesc.NumMips;
	}

	// Same size so just copy
	if(sizeSrcLog2 == sizeDstLog2)
	{
		for(u32 face = 0; face < 6; face++)
		{
			TextureCopyInformation copyDesc;
			copyDesc.SourceFace = face;
			copyDesc.SourceMip = srcMip;
			copyDesc.DestinationFace = face;
			copyDesc.DestinationMip = dstMip;

			src->Copy(dst, copyDesc);
		}
	}
	else
		DownsampleCubemap(scratchTex, srcMip, dst, dstMip);
}

void RenderBeastIBLUtility::DownsampleCubemap(const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip)
{
	for(u32 face = 0; face < 6; face++)
	{
		RENDER_TEXTURE_DESC cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = dst;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].NumFaces = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = dstMip;

		SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

		ReflectionCubeDownsampleMat* material = ReflectionCubeDownsampleMat::Get();
		material->Execute(src, face, srcMip, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradianceNonCompute(const SPtr<Texture>& cubemap, u32 outputIdx, const SPtr<RenderTexture>& output)
{
	static const u32 kNumCoeffs = 9;

	GpuResourcePool& resPool = GetGpuResourcePool();
	IrradianceComputeSHFragMat* shCompute = IrradianceComputeSHFragMat::Get();
	IrradianceAccumulateSHMat* shAccum = IrradianceAccumulateSHMat::Get();
	IrradianceAccumulateCubeSHMat* shAccumCube = IrradianceAccumulateCubeSHMat::Get();

	for(u32 coeff = 0; coeff < kNumCoeffs; ++coeff)
	{
		SPtr<PooledRenderTexture> coeffsTex = resPool.Get(shCompute->GetOutputDesc(cubemap));

		// Generate SH coefficients and weights per-texel
		for(u32 face = 0; face < 6; face++)
		{
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.ColorSurfaces[0].Texture = coeffsTex->Texture;
			cubeFaceRTDesc.ColorSurfaces[0].Face = face;
			cubeFaceRTDesc.ColorSurfaces[0].NumFaces = 1;
			cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

			SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
			shCompute->Execute(cubemap, face, coeff, target);
		}

		// Downsample, summing up coefficients and weights all the way down to 1x1
		auto& sourceProps = cubemap->GetProperties();
		u32 numMips = PixelUtil::GetMaxMipmaps(sourceProps.GetWidth(), sourceProps.GetHeight(), 1, sourceProps.GetFormat());

		SPtr<PooledRenderTexture> downsampleInput = coeffsTex;
		coeffsTex = nullptr;

		for(u32 mip = 0; mip < numMips; mip++)
		{
			SPtr<PooledRenderTexture> accumCoeffsTex = resPool.Get(shAccum->GetOutputDesc(downsampleInput->Texture));

			for(u32 face = 0; face < 6; face++)
			{
				RENDER_TEXTURE_DESC cubeFaceRTDesc;
				cubeFaceRTDesc.ColorSurfaces[0].Texture = accumCoeffsTex->Texture;
				cubeFaceRTDesc.ColorSurfaces[0].Face = face;
				cubeFaceRTDesc.ColorSurfaces[0].NumFaces = 1;
				cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

				SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
				shAccum->Execute(downsampleInput->Texture, face, 0, target);
			}

			downsampleInput = accumCoeffsTex;
		}

		// Sum up all the faces and write the coefficient to the final texture
		Vector2I outputOffset = GetShCoeffXyFromIdx(outputIdx, 3);
		shAccumCube->Execute(downsampleInput->Texture, 0, outputOffset, coeff, output);
	}
}
}} // namespace bs::ct
