//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastIBLUtility.h"
#include "Image/B3DTexture.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"

namespace b3d { namespace render {

ReflectionCubeDownsampleUniformDefinition gReflectionCubeDownsampleUniformDefinition;

void ReflectionCubeDownsampleMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void ReflectionCubeDownsampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 mip, const TShared<RenderTarget>& target)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gReflectionCubeDownsampleUniformDefinition.AllocateTransient().Map();

	gReflectionCubeDownsampleUniformDefinition.gCubeFace.Set(uniforms, face);

	const GpuDeviceCapabilities& gpuDeviceCapabilities = mGpuDevice->GetCapabilities();
	if(gpuDeviceCapabilities.HasCapability(RSC_TEXTURE_VIEWS))
	{
		mInputTextureParameter.Set(source, TextureSurface(mip, 1, 0, 6));
		gReflectionCubeDownsampleUniformDefinition.gMipLevel.Set(uniforms, 0);
	}
	else
	{
		mInputTextureParameter.Set(source);
		gReflectionCubeDownsampleUniformDefinition.gMipLevel.Set(uniforms, mip);
	}

	mUniformBufferParameter.Set(uniforms);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(target, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

const u32 ReflectionCubeImportanceSampleMaterial::kNumSamples = 1024;
ReflectionCubeImportanceSampleUniformDefinition gReflectionCubeImportanceSampleUniformDefinition;

void ReflectionCubeImportanceSampleMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void ReflectionCubeImportanceSampleMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("NUM_SAMPLES", kNumSamples);
}

void ReflectionCubeImportanceSampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 mip, const TShared<RenderTarget>& target)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gReflectionCubeImportanceSampleUniformDefinition.AllocateTransient().Map();

	gReflectionCubeImportanceSampleUniformDefinition.gCubeFace.Set(uniforms, face);
	gReflectionCubeImportanceSampleUniformDefinition.gMipLevel.Set(uniforms, mip);
	gReflectionCubeImportanceSampleUniformDefinition.gNumMips.Set(uniforms, source->GetProperties().MipMapCount + 1);

	float width = (float)source->GetProperties().Width;
	float height = (float)source->GetProperties().Height;

	// First part of the equation for determining mip level to sample from.
	// See http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
	float mipFactor = 0.5f * std::log2(width * height / kNumSamples);
	gReflectionCubeImportanceSampleUniformDefinition.gPrecomputedMipFactor.Set(uniforms, mipFactor);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(source);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(target, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

IrradianceComputeSHUniformDefinition gIrradianceComputeSHUniformDefinition;

// TILE_WIDTH * TILE_HEIGHT must be pow2 because of parallel reduction algorithm
const static u32 kTileWidth = 8;
const static u32 kTileHeight = 8;

// For very small textures this should be reduced so number of launched threads can properly utilize GPU cores
const static u32 kPixelsPerThread = 4;

void IrradianceComputeSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetStorageBufferParameter("gOutput", mOutputBufferParameter);
}

void IrradianceComputeSHMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_WIDTH", kTileWidth);
	defines.Set("TILE_HEIGHT", kTileHeight);
	defines.Set("PIXELS_PER_THREAD", kPixelsPerThread);
}

void IrradianceComputeSHMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, const TShared<GpuBuffer>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	auto& props = source->GetProperties();
	u32 faceSize = props.Width;
	B3D_ASSERT(faceSize == props.Height);

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	GpuBufferMappedScope uniforms = gIrradianceComputeSHUniformDefinition.AllocateTransient().Map();

	gIrradianceComputeSHUniformDefinition.gCubeFace.Set(uniforms, face);
	gIrradianceComputeSHUniformDefinition.gFaceSize.Set(uniforms, source->GetProperties().Width);
	gIrradianceComputeSHUniformDefinition.gDispatchSize.Set(uniforms, dispatchSize);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(source);
	mOutputBufferParameter.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(dispatchSize.X, dispatchSize.Y);
}

TShared<GpuBuffer> IrradianceComputeSHMaterial::CreateOutputBuffer(const TShared<Texture>& source, u32& numCoeffSets)
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

IrradianceComputeSHMaterial* IrradianceComputeSHMaterial::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceComputeSHFragUniformDefinition gIrradianceComputeSHFragUniformDefinition;

void IrradianceComputeSHFragMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void IrradianceComputeSHFragMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 coefficientIdx, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gIrradianceComputeSHFragUniformDefinition.AllocateTransient().Map();

	gIrradianceComputeSHFragUniformDefinition.gCubeFace.Set(uniforms, face);
	gIrradianceComputeSHFragUniformDefinition.gFaceSize.Set(uniforms, source->GetProperties().Width);
	gIrradianceComputeSHFragUniformDefinition.gCoeffEntryIdx.Set(uniforms, coefficientIdx / 4);
	gIrradianceComputeSHFragUniformDefinition.gCoeffComponentIdx.Set(uniforms, coefficientIdx % 4);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(source);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation IrradianceComputeSHFragMaterial::GetOutputDesc(const TShared<Texture>& input)
{
	auto& props = input->GetProperties();
	return PooledRenderTextureCreateInformation::CreateCube(PF_RGBA16F, props.Width, props.Height, TextureUsageFlag::RenderTarget);
}

IrradianceAccumulateSHUniformDefinition gIrradianceAccumulateSHUniformDefinition;

void IrradianceAccumulateSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void IrradianceAccumulateSHMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 sourceMip, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gIrradianceAccumulateSHUniformDefinition.AllocateTransient().Map();

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.Width, 0.5f / props.Height);

	gIrradianceAccumulateSHUniformDefinition.gCubeFace.Set(uniforms, face);
	gIrradianceAccumulateSHUniformDefinition.gCubeMip.Set(uniforms, sourceMip);
	gIrradianceAccumulateSHUniformDefinition.gHalfPixel.Set(uniforms, halfPixel);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(source);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation IrradianceAccumulateSHMaterial::GetOutputDesc(const TShared<Texture>& input)
{
	auto& props = input->GetProperties();

	// Assuming it's a cubemap
	u32 size = std::max(1U, (u32)(props.Width * 0.5f));

	return PooledRenderTextureCreateInformation::CreateCube(PF_RGBA32F, size, size, TextureUsageFlag::RenderTarget);
}

void IrradianceAccumulateCubeSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void IrradianceAccumulateCubeSHMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 sourceMip, const Vector2I& outputOffset, u32 coefficientIdx, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gIrradianceAccumulateSHUniformDefinition.AllocateTransient().Map();

	auto& props = source->GetProperties();
	Vector2 halfPixel(0.5f / props.Width, 0.5f / props.Height);

	gIrradianceAccumulateSHUniformDefinition.gCubeFace.Set(uniforms, 0);
	gIrradianceAccumulateSHUniformDefinition.gCubeMip.Set(uniforms, sourceMip);
	gIrradianceAccumulateSHUniformDefinition.gHalfPixel.Set(uniforms, halfPixel);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(source);

	auto& rtProps = output->GetProperties();

	// Render to just one pixel corresponding to the coefficient
	Area2 viewRect;
	viewRect.X = (outputOffset.X + coefficientIdx) / (float)rtProps.Width;
	viewRect.Y = outputOffset.Y / (float)rtProps.Height;

	viewRect.Width = 1.0f / rtProps.Width;
	viewRect.Height = 1.0f / rtProps.Height;

	// Render
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGpuParameterSet));
	commandBuffer.SetViewport(viewRect);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();

	commandBuffer.SetViewport(Area2(0, 0, 1, 1));
}

PooledRenderTextureCreateInformation IrradianceAccumulateCubeSHMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA32F, 9, 1, TextureUsageFlag::RenderTarget);
}

IrradianceReduceSHUniformDefinition gIrradianceReduceSHUniformDefinition;

void IrradianceReduceSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetStorageBufferParameter("gInput", mInputBufferParameter);
	mGpuParameterSet->GetStorageTextureParameter("gOutput", mOutputTextureParameter);
}

void IrradianceReduceSHMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<GpuBuffer>& source, u32 numCoeffSets, const TShared<Texture>& output, u32 outputIdx)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gIrradianceReduceSHUniformDefinition.AllocateTransient().Map();

	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");

	Vector2I outputCoords = IBLUtility::GetShCoeffXyFromIdx(outputIdx, shOrder);
	gIrradianceReduceSHUniformDefinition.gOutputIdx.Set(uniforms, outputCoords);
	gIrradianceReduceSHUniformDefinition.gNumEntries.Set(uniforms, numCoeffSets);

	mUniformBufferParameter.Set(uniforms);
	mInputBufferParameter.Set(source);
	mOutputTextureParameter.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(1);
}

TShared<Texture> IrradianceReduceSHMaterial::CreateOutputTexture(u32 numCoeffSets)
{
	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");
	Vector2I size = IBLUtility::GetShCoeffTextureSize(numCoeffSets, shOrder);

	TextureCreateInformation textureDesc;
	textureDesc.Name = "Irradiance Reduce Output";
	textureDesc.Width = (u32)size.X;
	textureDesc.Height = (u32)size.Y;
	textureDesc.Format = PF_RGBA32F;
	textureDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

	return mGpuDevice->CreateTexture(textureDesc);
}

IrradianceReduceSHMaterial* IrradianceReduceSHMaterial::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceProjectSHUniformDefinition gIrradianceProjectSHUniformDefinition;

void IrradianceProjectSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSHCoeffs", mInputTextureParameter);
}

void IrradianceProjectSHMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& shCoeffs, u32 face, const TShared<RenderTarget>& target)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gIrradianceProjectSHUniformDefinition.AllocateTransient().Map();

	gIrradianceProjectSHUniformDefinition.gCubeFace.Set(uniforms, face);

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(shCoeffs);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(target, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void RenderBeastIBLUtility::FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& scratch) const
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	auto& props = cubemap->GetProperties();

	TShared<Texture> scratchCubemap = scratch;
	if(scratchCubemap == nullptr)
	{
		TextureCreateInformation cubemapDesc;
		cubemapDesc.Name = "Specular Cubemap Filter Scratch";
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = props.Format;
		cubemapDesc.Width = props.Width;
		cubemapDesc.Height = props.Height;
		cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
		cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

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

		commandBuffer.CopyTexture(cubemap, scratchCubemap, copyDesc);
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

			TShared<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

			ReflectionCubeImportanceSampleMaterial* material = ReflectionCubeImportanceSampleMaterial::Get();
			material->Execute(commandBuffer, scratchCubemap, face, mip, target);
		}
	}
}

bool SupportsComputeSh()
{
	return GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output) const
{
	TShared<Texture> coeffTexture;
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMaterial* shCompute = IrradianceComputeSHMaterial::GetVariation(5);
		IrradianceReduceSHMaterial* shReduce = IrradianceReduceSHMaterial::GetVariation(5);

		u32 numCoeffSets;
		TShared<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(commandBuffer, cubemap, face, coeffSetBuffer);

		coeffTexture = shReduce->CreateOutputTexture(1);
		shReduce->Execute(commandBuffer, coeffSetBuffer, numCoeffSets, coeffTexture, 0);
	}
	else
	{
		TShared<PooledRenderTexture> finalCoeffs = GetGpuResourcePool().Get(IrradianceAccumulateCubeSHMaterial::GetOutputDesc());

		FilterCubemapForIrradianceNonCompute(commandBuffer, cubemap, 0, finalCoeffs->RenderTexture);
		coeffTexture = finalCoeffs->Texture;
	}

	IrradianceProjectSHMaterial* shProject = IrradianceProjectSHMaterial::Get();
	for(u32 face = 0; face < 6; face++)
	{
		RenderTextureCreateInformation cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = output;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

		TShared<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
		shProject->Execute(commandBuffer, coeffTexture, face, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output, u32 outputIdx) const
{
	if(SupportsComputeSh())
	{
		IrradianceComputeSHMaterial* shCompute = IrradianceComputeSHMaterial::GetVariation(3);
		IrradianceReduceSHMaterial* shReduce = IrradianceReduceSHMaterial::GetVariation(3);

		u32 numCoeffSets;
		TShared<GpuBuffer> coeffSetBuffer = shCompute->CreateOutputBuffer(cubemap, numCoeffSets);
		for(u32 face = 0; face < 6; face++)
			shCompute->Execute(commandBuffer, cubemap, face, coeffSetBuffer);

		shReduce->Execute(commandBuffer, coeffSetBuffer, numCoeffSets, output, outputIdx);
	}
	else
	{
		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = output;

		TShared<RenderTexture> target = RenderTexture::Create(rtDesc);
		FilterCubemapForIrradianceNonCompute(commandBuffer, cubemap, outputIdx, target);
	}
}

void RenderBeastIBLUtility::ScaleCubemap(GpuCommandBuffer& commandBuffer, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip) const
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	auto& srcProps = src->GetProperties();
	auto& dstProps = dst->GetProperties();

	TShared<Texture> scratchTex = src;
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
		cubemapDesc.Format = PF_RGBA16F;
		cubemapDesc.Width = mipSize;
		cubemapDesc.Height = mipSize;
		cubemapDesc.MipMapCount = numDownsamples - 1;
		cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

		scratchTex = gpuDevice->CreateTexture(cubemapDesc);

		DownsampleCubemap(commandBuffer, src, srcMip, scratchTex, 0);
		for(u32 i = 0; i < cubemapDesc.MipMapCount; i++)
			DownsampleCubemap(commandBuffer, scratchTex, i, scratchTex, i + 1);

		srcMip = cubemapDesc.MipMapCount;
	}

	// Same size and format -> just copy
	if(sizeSrcLog2 == sizeDstLog2)
	{
		if(srcProps.Format == dstProps.Format)
		{
			for(u32 face = 0; face < 6; face++)
			{
				TextureCopyInformation copyDesc;
				copyDesc.SourceFace = face;
				copyDesc.SourceMip = srcMip;
				copyDesc.DestinationFace = face;
				copyDesc.DestinationMip = dstMip;

				commandBuffer.CopyTexture(src, dst, copyDesc);
			}
		}
		else
			DownsampleCubemap(commandBuffer, src, srcMip, dst, dstMip);
	}
	else
		DownsampleCubemap(commandBuffer, scratchTex, srcMip, dst, dstMip);
}

void RenderBeastIBLUtility::DownsampleCubemap(GpuCommandBuffer& commandBuffer, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip)
{
	for(u32 face = 0; face < 6; face++)
	{
		RenderTextureCreateInformation cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = dst;
		cubeFaceRTDesc.ColorSurfaces[0].Face = face;
		cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
		cubeFaceRTDesc.ColorSurfaces[0].MipLevel = dstMip;

		TShared<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);

		ReflectionCubeDownsampleMaterial* material = ReflectionCubeDownsampleMaterial::Get();
		material->Execute(commandBuffer, src, face, srcMip, target);
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradianceNonCompute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, u32 outputIdx, const TShared<RenderTexture>& output)
{
	static const u32 kNumCoeffs = 9;

	GpuResourcePool& resPool = GetGpuResourcePool();
	IrradianceComputeSHFragMaterial* shCompute = IrradianceComputeSHFragMaterial::Get();
	IrradianceAccumulateSHMaterial* shAccum = IrradianceAccumulateSHMaterial::Get();
	IrradianceAccumulateCubeSHMaterial* shAccumCube = IrradianceAccumulateCubeSHMaterial::Get();

	for(u32 coeff = 0; coeff < kNumCoeffs; ++coeff)
	{
		TShared<PooledRenderTexture> coeffsTex = resPool.Get(shCompute->GetOutputDesc(cubemap));

		// Generate SH coefficients and weights per-texel
		for(u32 face = 0; face < 6; face++)
		{
			RenderTextureCreateInformation cubeFaceRTDesc;
			cubeFaceRTDesc.ColorSurfaces[0].Texture = coeffsTex->Texture;
			cubeFaceRTDesc.ColorSurfaces[0].Face = face;
			cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
			cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

			TShared<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
			shCompute->Execute(commandBuffer, cubemap, face, coeff, target);
		}

		// Downsample, summing up coefficients and weights all the way down to 1x1
		auto& sourceProps = cubemap->GetProperties();
		u32 numMips = PixelUtility::GetMipmapCount(sourceProps.Width, sourceProps.Height, 1, sourceProps.Format);

		TShared<PooledRenderTexture> downsampleInput = coeffsTex;
		coeffsTex = nullptr;

		for(u32 mip = 0; mip < numMips; mip++)
		{
			TShared<PooledRenderTexture> accumCoeffsTex = resPool.Get(shAccum->GetOutputDesc(downsampleInput->Texture));

			for(u32 face = 0; face < 6; face++)
			{
				RenderTextureCreateInformation cubeFaceRTDesc;
				cubeFaceRTDesc.ColorSurfaces[0].Texture = accumCoeffsTex->Texture;
				cubeFaceRTDesc.ColorSurfaces[0].Face = face;
				cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;
				cubeFaceRTDesc.ColorSurfaces[0].MipLevel = 0;

				TShared<RenderTarget> target = RenderTexture::Create(cubeFaceRTDesc);
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
