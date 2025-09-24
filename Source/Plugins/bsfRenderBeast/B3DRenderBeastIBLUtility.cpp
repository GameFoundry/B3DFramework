//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastIBLUtility.h"
#include "Image/BsTexture.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRendererUtility.h"
#include "BsRenderBeast.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsRenderTexture.h"

namespace b3d { namespace render {

ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

void ReflectionCubeDownsampleMat::Initialize()
{
	mParamBuffer = gReflectionCubeDownsampleParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void ReflectionCubeDownsampleMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	gReflectionCubeDownsampleParamDef.gCubeFace.Set(mParamBuffer, face);

	const GpuDeviceCapabilities& gpuDeviceCapabilities = mGpuDevice->GetCapabilities();
	if(gpuDeviceCapabilities.HasCapability(RSC_TEXTURE_VIEWS))
	{
		mInputTexture.Set(source, TextureSurface(mip, 1, 0, 6));
		gReflectionCubeDownsampleParamDef.gMipLevel.Set(mParamBuffer, 0);
	}
	else
	{
		mInputTexture.Set(source);
		gReflectionCubeDownsampleParamDef.gMipLevel.Set(mParamBuffer, mip);
	}

	commandBuffer.SetRenderTarget(target);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

const u32 ReflectionCubeImportanceSampleMat::kNumSamples = 1024;
ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

void ReflectionCubeImportanceSampleMat::Initialize()
{
	mParamBuffer = gReflectionCubeImportanceSampleParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void ReflectionCubeImportanceSampleMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_SAMPLES", kNumSamples);
}

void ReflectionCubeImportanceSampleMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	mInputTexture.Set(source);
	gReflectionCubeImportanceSampleParamDef.gCubeFace.Set(mParamBuffer, face);
	gReflectionCubeImportanceSampleParamDef.gMipLevel.Set(mParamBuffer, mip);
	gReflectionCubeImportanceSampleParamDef.gNumMips.Set(mParamBuffer, source->GetProperties().MipMapCount + 1);

	float width = (float)source->GetProperties().Width;
	float height = (float)source->GetProperties().Height;

	// First part of the equation for determining mip level to sample from.
	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
	float mipFactor = 0.5f * std::log2(width * height / kNumSamples);
	gReflectionCubeImportanceSampleParamDef.gPrecomputedMipFactor.Set(mParamBuffer, mipFactor);

	commandBuffer.SetRenderTarget(target);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
const static u32 kTileWidth = 8;
const static u32 kTileHeight = 8;

// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
const static u32 kPixelsPerThread = 4;

void IrradianceComputeSHMat::Initialize()
{
	mParamBuffer = gIrradianceComputeSHParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
	mGPUParameters->GetStorageBufferParameter("gOutput", mOutputBuffer);
}

void IrradianceComputeSHMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_WIDTH", kTileWidth);
	defines.Set("TILE_HEIGHT", kTileHeight);
	defines.Set("PIXELS_PER_THREAD", kPixelsPerThread);
}

void IrradianceComputeSHMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, const SPtr<GpuBuffer>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	auto& props = source->GetProperties();
	u32 faceSize = props.Width;
	B3D_ASSERT(faceSize == props.Height);

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	mInputTexture.Set(source);
	gIrradianceComputeSHParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceComputeSHParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().Width);
	gIrradianceComputeSHParamDef.gDispatchSize.Set(mParamBuffer, dispatchSize);

	mOutputBuffer.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(dispatchSize.X, dispatchSize.Y);
}

SPtr<GpuBuffer> IrradianceComputeSHMat::CreateOutputBuffer(const SPtr<Texture>& source, u32& numCoeffSets)
{
	auto& props = source->GetProperties();
	u32 faceSize = props.Width;
	B3D_ASSERT(faceSize == props.Height);

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	numCoeffSets = dispatchSize.X * dispatchSize.Y * 6;

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	bufferCreateInformation.StructuredStorage.Count = numCoeffSets;

	if(mVariationParameters.GetI32("SH_ORDER") == 3)
		bufferCreateInformation.StructuredStorage.ElementSize = sizeof(SHCoeffsAndWeight3);
	else
		bufferCreateInformation.StructuredStorage.ElementSize = sizeof(SHCoeffsAndWeight5);

	return mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
}

IrradianceComputeSHMat* IrradianceComputeSHMat::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

void IrradianceComputeSHFragMat::Initialize()
{
	mParamBuffer = gIrradianceComputeSHFragParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void IrradianceComputeSHFragMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 coefficientIdx, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	gIrradianceComputeSHFragParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceComputeSHFragParamDef.gFaceSize.Set(mParamBuffer, source->GetProperties().Width);
	gIrradianceComputeSHFragParamDef.gCoeffEntryIdx.Set(mParamBuffer, coefficientIdx / 4);
	gIrradianceComputeSHFragParamDef.gCoeffComponentIdx.Set(mParamBuffer, coefficientIdx % 4);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

POOLED_RenderTextureCreateInformation IrradianceComputeSHFragMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();
	return POOLED_RenderTextureCreateInformation::CreateCube(PF_RGBA16F, props.Width, props.Height, TU_RENDERTARGET);
}

IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

void IrradianceAccumulateSHMat::Initialize()
{
	mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void IrradianceAccumulateSHMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 sourceMip, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.Width, 0.5f / props.Height);

	gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, face);
	gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
	gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

POOLED_RenderTextureCreateInformation IrradianceAccumulateSHMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();

	// Assuming it's a cubemap
	u32 size = std::max(1U, (u32)(props.Width * 0.5f));

	return POOLED_RenderTextureCreateInformation::CreateCube(PF_RGBA32F, size, size, TU_RENDERTARGET);
}

void IrradianceAccumulateCubeSHMat::Initialize()
{
	mParamBuffer = gIrradianceAccumulateSHParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void IrradianceAccumulateCubeSHMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 sourceMip, const Vector2I& outputOffset, u32 coefficientIdx, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(source);

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.Width, 0.5f / props.Height);

	gIrradianceAccumulateSHParamDef.gCubeFace.Set(mParamBuffer, 0);
	gIrradianceAccumulateSHParamDef.gCubeMip.Set(mParamBuffer, sourceMip);
	gIrradianceAccumulateSHParamDef.gHalfPixel.Set(mParamBuffer, halfPixel);

	auto& rtProps = output->GetProperties();

	// Render to just one pixel corresponding to the coefficient
	Area2 viewRect;
	viewRect.X = (outputOffset.X + coefficientIdx) / (float)rtProps.Width;
	viewRect.Y = outputOffset.Y / (float)rtProps.Height;

	viewRect.Width = 1.0f / rtProps.Width;
	viewRect.Height = 1.0f / rtProps.Height;

	// Render
	commandBuffer.SetRenderTarget(output);
	commandBuffer.SetViewport(viewRect);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
	commandBuffer.SetViewport(Area2(0, 0, 1, 1));
}

POOLED_RenderTextureCreateInformation IrradianceAccumulateCubeSHMat::GetOutputDesc()
{
	return POOLED_RenderTextureCreateInformation::Create2D(PF_RGBA32F, 9, 1, TU_RENDERTARGET);
}

IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

void IrradianceReduceSHMat::Initialize()
{
	mParamBuffer = gIrradianceReduceSHParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetStorageBufferParameter("gInput", mInputBuffer);
	mGPUParameters->GetStorageTextureParameter("gOutput", mOutputTexture);
}

void IrradianceReduceSHMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& source, u32 numCoeffSets, const SPtr<Texture>& output, u32 outputIdx)
{
	BS_RENMAT_PROFILE_BLOCK

	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");

	Vector2I outputCoords = IBLUtility::GetShCoeffXyFromIdx(outputIdx, shOrder);
	gIrradianceReduceSHParamDef.gOutputIdx.Set(mParamBuffer, outputCoords);
	gIrradianceReduceSHParamDef.gNumEntries.Set(mParamBuffer, numCoeffSets);

	mInputBuffer.Set(source);
	mOutputTexture.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(1);
}

SPtr<Texture> IrradianceReduceSHMat::CreateOutputTexture(u32 numCoeffSets)
{
	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");
	Vector2I size = IBLUtility::GetShCoeffTextureSize(numCoeffSets, shOrder);

	TextureCreateInformation textureDesc;
	textureDesc.Name = "Irradiance Reduce Output";
	textureDesc.Width = (u32)size.X;
	textureDesc.Height = (u32)size.Y;
	textureDesc.Format = PF_RGBA32F;
	textureDesc.Usage = TU_STATIC | TU_LOADSTORE;

	return mGpuDevice->CreateTexture(textureDesc);
}

IrradianceReduceSHMat* IrradianceReduceSHMat::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

void IrradianceProjectSHMat::Initialize()
{
	mParamBuffer = gIrradianceProjectSHParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gSHCoeffs", mInputTexture);
}

void IrradianceProjectSHMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& shCoeffs, u32 face, const SPtr<RenderTarget>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	gIrradianceProjectSHParamDef.gCubeFace.Set(mParamBuffer, face);

	mInputTexture.Set(shCoeffs);

	commandBuffer.SetRenderTarget(target);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

void RenderBeastIBLUtility::FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	auto& props = cubemap->GetProperties();

	SPtr<Texture> scratchCubemap = scratch;
	if(scratchCubemap == nullptr)
	{
		TextureCreateInformation cubemapDesc;
		cubemapDesc.Name = "Specular Cubemap Filter Scratch";
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = props.Format;
		cubemapDesc.Width = props.Width;
		cubemapDesc.Height = props.Height;
		cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		scratchCubemap = gpuDevice->CreateTexture(cubemapDesc);
	}

	// We sample the cubemaps using importance sampling to generate roughness
	u32 numMips = props.MipMapCount + 1;

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

		cubemap->Copy(commandBuffer, scratchCubemap, copyDesc);
	}

	// Fill out remaining scratch mip levels by downsampling
	for(u32 mip = 1; mip < numMips; mip++)
	{
		u32 sourceMip = mip - 1;
		DownsampleCubemap(commandBuffer, scratchCubemap, sourceMip, scratchCubemap, mip);
	}

	// Importance sample
	for(u32 mip = 1; mip < numMips; mip++)
	{
		for(u32 face = 0; face < 6; face++)
		{
			RenderTextureCreateInformation cubeFaceRTDesc;
			cubeFaceRTDesc.ColorSurfaces[0].Texture = cubemap;
			cubeFaceRTDesc.ColorSurfaces[0].Face = face;
			cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
			cubeFaceRTDesc.ColorSurfaces[0].MipLevel = mip;

			SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

			ReflectionCubeImportanceSampleMat* material = ReflectionCubeImportanceSampleMat::Get();
			material->Execute(commandBuffer, scratchCubemap, face, mip, target);
		}
	}

	commandBuffer.SetRenderTarget(nullptr);
}

bool SupportsComputeSh()
{
	return GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const
{
	SPtr<Texture> coeffTexture;
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::GetVariation(5);
		IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::GetVariation(5);

		u32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(commandBuffer, cubemap, face, coeffSetBuffer);

		coeffTexture = shReduce->CreateOutputTexture(1);
		shReduce->Execute(commandBuffer, coeffSetBuffer, numCoeffSets, coeffTexture, 0);
	}
	else
	{
		SPtr<PooledRenderTexture> finalCoeffs = GetGpuResourcePool().Get(IrradianceAccumulateCubeSHMat::GetOutputDesc());

		FilterCubemapForIrradianceNonCompute(commandBuffer, cubemap, 0, finalCoeffs->RenderTexture);
		coeffTexture = finalCoeffs->Texture;
	}

	IrradianceProjectSHMat* shProject = IrradianceProjectSHMat::Get();
	for(u32 face = 0; face < 6; face++)
	{
		RenderTextureCreateInformation cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = output;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

		SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
		shProject->Execute(commandBuffer, coeffTexture, face, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const
{
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMat* shCompute = IrradianceComputeSHMat::GetVariation(3);
		IrradianceReduceSHMat* shReduce = IrradianceReduceSHMat::GetVariation(3);

		u32 numCoeffSets;
		SPtr<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(commandBuffer, cubemap, face, coeffSetBuffer);

		shReduce->Execute(commandBuffer, coeffSetBuffer, numCoeffSets, output, outputIdx);
	}
	else
	{
		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = output;

		SPtr<RenderTexture> target = RenderTexture::Create(rtDesc);
		FilterCubemapForIrradianceNonCompute(commandBuffer, cubemap, outputIdx, target);
	}
}

void RenderBeastIBLUtility::ScaleCubemap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	auto& srcProps = src->GetProperties();
	auto& dstProps = dst->GetProperties();

	SPtr<Texture> scratchTex = src;
	int sizeSrcLog2 = (int)log2((float)srcProps.Width);
	int sizeDstLog2 = (int)log2((float)dstProps.Width);

	int sizeLog2Diff = sizeSrcLog2 - sizeDstLog2;

	// If size difference is greater than one mip-level and we're downscaling, we need to generate intermediate mip
	// levels
	if(sizeLog2Diff > 1)
	{
		u32 mipSize = (u32)exp2((float)(sizeSrcLog2 - 1));
		u32 numDownsamples = sizeLog2Diff - 1;

		TextureCreateInformation cubemapDesc;
		cubemapDesc.Name = "Scale Cubemap Scratch";
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = srcProps.Format;
		cubemapDesc.Width = mipSize;
		cubemapDesc.Height = mipSize;
		cubemapDesc.MipMapCount = numDownsamples - 1;
		cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

		scratchTex = gpuDevice->CreateTexture(cubemapDesc);

		DownsampleCubemap(commandBuffer, src, srcMip, scratchTex, 0);
		for(u32 i = 0; i < cubemapDesc.MipMapCount; i++)
			DownsampleCubemap(commandBuffer, scratchTex, i, scratchTex, i + 1);

		srcMip = cubemapDesc.MipMapCount;
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

			src->Copy(commandBuffer, dst, copyDesc);
		}
	}
	else
		DownsampleCubemap(commandBuffer, scratchTex, srcMip, dst, dstMip);
}

void RenderBeastIBLUtility::DownsampleCubemap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip)
{
	for(u32 face = 0; face < 6; face++)
	{
		RenderTextureCreateInformation cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = dst;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = dstMip;

		SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

		ReflectionCubeDownsampleMat* material = ReflectionCubeDownsampleMat::Get();
		material->Execute(commandBuffer, src, face, srcMip, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradianceNonCompute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, u32 outputIdx, const SPtr<RenderTexture>& output)
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
			RenderTextureCreateInformation cubeFaceRTDesc;
			cubeFaceRTDesc.ColorSurfaces[0].Texture = coeffsTex->Texture;
			cubeFaceRTDesc.ColorSurfaces[0].Face = face;
			cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
			cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

			SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
			shCompute->Execute(commandBuffer, cubemap, face, coeff, target);
		}

		// Downsample, summing up coefficients and weights all the way down to 1x1
		auto& sourceProps = cubemap->GetProperties();
		u32 numMips = PixelUtility::GetMipmapCount(sourceProps.Width, sourceProps.Height, 1, sourceProps.Format);

		SPtr<PooledRenderTexture> downsampleInput = coeffsTex;
		coeffsTex = nullptr;

		for(u32 mip = 0; mip < numMips; mip++)
		{
			SPtr<PooledRenderTexture> accumCoeffsTex = resPool.Get(shAccum->GetOutputDesc(downsampleInput->Texture));

			for(u32 face = 0; face < 6; face++)
			{
				RenderTextureCreateInformation cubeFaceRTDesc;
				cubeFaceRTDesc.ColorSurfaces[0].Texture = accumCoeffsTex->Texture;
				cubeFaceRTDesc.ColorSurfaces[0].Face = face;
				cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
				cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

				SPtr<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
				shAccum->Execute(commandBuffer, downsampleInput->Texture, face, 0, target);
			}

			downsampleInput = accumCoeffsTex;
		}

		// Sum up all the faces and write the coefficient to the final texture
		Vector2I outputOffset = GetShCoeffXyFromIdx(outputIdx, 3);
		shAccumCube->Execute(commandBuffer, downsampleInput->Texture, 0, outputOffset, coeff, output);
	}
}
}} // namespace b3d::render
