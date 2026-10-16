//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastIBLUtility.h"
#include "Image/B3DTexture.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DRendererUtility.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "FrameGraph/B3DFrameGraph.h"

namespace b3d { namespace render {

ReflectionCubeDownsampleUniformDefinition gReflectionCubeDownsampleUniformDefinition;

void ReflectionCubeDownsampleMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

TShared<GpuParameterSet> ReflectionCubeDownsampleMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, u32 mip)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gReflectionCubeDownsampleUniformDefinition.AllocateTransient().Map();

	gReflectionCubeDownsampleUniformDefinition.gCubeFace.Set(uniforms, face);

	mInputTextureParameter.Set(set, source, TextureSurface(mip, 1, 0, 6));

	mUniformBufferParameter.Set(set, uniforms);

	return set;
}

void ReflectionCubeDownsampleMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> ReflectionCubeImportanceSampleMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, u32 mip)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

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

	mUniformBufferParameter.Set(set, uniforms);
	mInputTextureParameter.Set(set, source);

	return set;
}

void ReflectionCubeImportanceSampleMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> IrradianceComputeSHMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, const TShared<GpuBuffer>& output)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gIrradianceComputeSHUniformDefinition.AllocateTransient().Map();

	gIrradianceComputeSHUniformDefinition.gCubeFace.Set(uniforms, face);
	gIrradianceComputeSHUniformDefinition.gFaceSize.Set(uniforms, source->GetProperties().Width);
	gIrradianceComputeSHUniformDefinition.gDispatchSize.Set(uniforms, GetDispatchSize(source));

	mUniformBufferParameter.Set(set, uniforms);
	mInputTextureParameter.Set(set, source);
	mOutputBufferParameter.Set(set, output);

	return set;
}

void IrradianceComputeSHMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& source) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const Vector2I dispatchSize = GetDispatchSize(source);

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(dispatchSize.X, dispatchSize.Y);
}

GpuBufferCreateInformation IrradianceComputeSHMaterial::GetOutputBufferInformation(const TShared<Texture>& source, u32& numCoeffSets)
{
	const Vector2I dispatchSize = GetDispatchSize(source);
	numCoeffSets = dispatchSize.X * dispatchSize.Y * 6;

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	bufferCreateInformation.StructuredStorage.Count = numCoeffSets;

	if(mVariationParameters.GetI32("SH_ORDER") == 3)
		bufferCreateInformation.StructuredStorage.ElementSize = sizeof(SHCoeffsAndWeight3);
	else
		bufferCreateInformation.StructuredStorage.ElementSize = sizeof(SHCoeffsAndWeight5);

	return bufferCreateInformation;
}

Vector2I IrradianceComputeSHMaterial::GetDispatchSize(const TShared<Texture>& source)
{
	auto& props = source->GetProperties();
	u32 faceSize = props.Width;
	B3D_ASSERT(faceSize == props.Height);

	Vector2I dispatchSize;
	dispatchSize.X = Math::DivideAndRoundUp(faceSize, kTileWidth * kPixelsPerThread);
	dispatchSize.Y = Math::DivideAndRoundUp(faceSize, kTileHeight * kPixelsPerThread);

	return dispatchSize;
}

IrradianceComputeSHMaterial* IrradianceComputeSHMaterial::GetVariation(int order)
{
	if(order == 3)
		return Get(GetVariation<3>());

	return Get(GetVariation<5>());
}

IrradianceReduceSHUniformDefinition gIrradianceReduceSHUniformDefinition;

void IrradianceReduceSHMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetStorageBufferParameter("gInput", mInputBufferParameter);
	mGpuParameterSet->GetStorageTextureParameter("gOutput", mOutputTextureParameter);
}

TShared<GpuParameterSet> IrradianceReduceSHMaterial::Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& source, u32 numCoeffSets, const TShared<Texture>& output, u32 outputIdx)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gIrradianceReduceSHUniformDefinition.AllocateTransient().Map();

	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");

	Vector2I outputCoords = IBLUtility::GetShCoeffXyFromIdx(outputIdx, shOrder);
	gIrradianceReduceSHUniformDefinition.gOutputIdx.Set(uniforms, outputCoords);
	gIrradianceReduceSHUniformDefinition.gNumEntries.Set(uniforms, numCoeffSets);

	mUniformBufferParameter.Set(set, uniforms);
	mInputBufferParameter.Set(set, source);
	mOutputTextureParameter.Set(set, output);

	return set;
}

void IrradianceReduceSHMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(1);
}

TextureCreateInformation IrradianceReduceSHMaterial::GetOutputTextureInformation(u32 numCoeffSets)
{
	u32 shOrder = (u32)mVariationParameters.GetI32("SH_ORDER");
	Vector2I size = IBLUtility::GetShCoeffTextureSize(numCoeffSets, shOrder);

	TextureCreateInformation textureDesc;
	textureDesc.Width = (u32)size.X;
	textureDesc.Height = (u32)size.Y;
	textureDesc.Format = PF_RGBA32F;
	textureDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

	return textureDesc;
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

TShared<GpuParameterSet> IrradianceProjectSHMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& shCoeffs, u32 face)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gIrradianceProjectSHUniformDefinition.AllocateTransient().Map();

	gIrradianceProjectSHUniformDefinition.gCubeFace.Set(uniforms, face);

	mUniformBufferParameter.Set(set, uniforms);
	mInputTextureParameter.Set(set, shCoeffs);

	return set;
}

void IrradianceProjectSHMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

void RenderBeastIBLUtility::FilterCubemapForSpecular(FrameGraph& graph, const TShared<Texture>& cubemap) const
{
	auto& props = cubemap->GetProperties();

	TextureCreateInformation cubemapDesc;
	cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
	cubemapDesc.Format = props.Format;
	cubemapDesc.Width = props.Width;
	cubemapDesc.Height = props.Height;
	cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
	cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

	const TShared<Texture> scratchCubemap = graph.CreateTexture("SpecularCubemapFilterScratch", cubemapDesc);

	graph.ImportTexture(cubemap);

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
	FrameGraphCopyPass copyPass;
	copyPass.AddSource(cubemap, TextureSurface(0, 1, 0, 6));
	copyPass.AddDestination(scratchCubemap, TextureSurface(0, 1, 0, 6));

	graph.AddCopyPass("CubemapCopyBaseMip", std::move(copyPass), [cubemap, scratchCubemap](FrameGraphPassContext& context)
	{
		for(u32 face = 0; face < 6; face++)
		{
			TextureCopyInformation copyDesc;
			copyDesc.SourceFace = face;
			copyDesc.DestinationFace = face;

			context.GetCommandBuffer().CopyTexture(cubemap, scratchCubemap, copyDesc);
		}
	});

	// Fill out remaining scratch mip levels by downsampling
	for(u32 mip = 1; mip < numMips; mip++)
	{
		u32 sourceMip = mip - 1;
		DownsampleCubemap(graph, scratchCubemap, sourceMip, scratchCubemap, mip);
	}

	// Importance sample
	ReflectionCubeImportanceSampleMaterial* material = ReflectionCubeImportanceSampleMaterial::Get();
	for(u32 mip = 1; mip < numMips; mip++)
	{
		for(u32 face = 0; face < 6; face++)
		{
			FrameGraphRasterPass importanceSamplePass;
			importanceSamplePass.SetColorAttachment(0, cubemap, FrameGraphAttachmentLoad::DontCare, mip, face, 1);

			const TShared<GpuParameterSet> set = material->Prepare(importanceSamplePass, scratchCubemap, face, mip);
			graph.AddRasterPass("ReflectionCubeImportanceSample", std::move(importanceSamplePass), [material, set](FrameGraphPassContext& context)
			{
				context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
				material->Execute(context, set);
			});
		}
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(FrameGraph& graph, const TShared<Texture>& cubemap, const TShared<Texture>& output) const
{
	graph.ImportTexture(cubemap);
	graph.ImportTexture(output);

	u32 numCoeffSets;
	const TShared<GpuBuffer> coeffSetBuffer = ComputeCoefficientSets(graph, cubemap, 5, numCoeffSets);

	IrradianceReduceSHMaterial* shReduce = IrradianceReduceSHMaterial::GetVariation(5);
	const TShared<Texture> coeffTexture = graph.CreateTexture("IrradianceCoefficients", shReduce->GetOutputTextureInformation(1));

	FrameGraphComputePass reducePass;
	const TShared<GpuParameterSet> reduceSet = shReduce->Prepare(reducePass, coeffSetBuffer, numCoeffSets, coeffTexture, 0);
	graph.AddComputePass("IrradianceReduceSH", std::move(reducePass), [shReduce, reduceSet](FrameGraphPassContext& context)
	{
		shReduce->Execute(context, reduceSet);
	});

	IrradianceProjectSHMaterial* shProject = IrradianceProjectSHMaterial::Get();
	for(u32 face = 0; face < 6; face++)
	{
		FrameGraphRasterPass projectPass;
		projectPass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::DontCare, 0, face, 1);

		const TShared<GpuParameterSet> projectSet = shProject->Prepare(projectPass, coeffTexture, face);
		graph.AddRasterPass("IrradianceProjectSH", std::move(projectPass), [shProject, projectSet](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			shProject->Execute(context, projectSet);
		});
	}
}

void RenderBeastIBLUtility::FilterCubemapForIrradiance(FrameGraph& graph, const TShared<Texture>& cubemap, const TShared<Texture>& output, u32 outputIdx) const
{
	graph.ImportTexture(cubemap);
	graph.ImportTexture(output);

	u32 numCoeffSets;
	const TShared<GpuBuffer> coeffSetBuffer = ComputeCoefficientSets(graph, cubemap, 3, numCoeffSets);

	IrradianceReduceSHMaterial* shReduce = IrradianceReduceSHMaterial::GetVariation(3);

	FrameGraphComputePass reducePass;
	const TShared<GpuParameterSet> reduceSet = shReduce->Prepare(reducePass, coeffSetBuffer, numCoeffSets, output, outputIdx);
	graph.AddComputePass("IrradianceReduceSH", std::move(reducePass), [shReduce, reduceSet](FrameGraphPassContext& context)
	{
		shReduce->Execute(context, reduceSet);
	});
}

void RenderBeastIBLUtility::ScaleCubemap(FrameGraph& graph, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip) const
{
	graph.ImportTexture(src);
	graph.ImportTexture(dst);

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
		cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.Format = PF_RGBA16F;
		cubemapDesc.Width = mipSize;
		cubemapDesc.Height = mipSize;
		cubemapDesc.MipMapCount = numDownsamples - 1;
		cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

		scratchTex = graph.CreateTexture("ScaleCubemapScratch", cubemapDesc);

		DownsampleCubemap(graph, src, srcMip, scratchTex, 0);
		for(u32 i = 0; i < cubemapDesc.MipMapCount; i++)
			DownsampleCubemap(graph, scratchTex, i, scratchTex, i + 1);

		srcMip = cubemapDesc.MipMapCount;
	}

	// Same size and format -> just copy
	if(sizeSrcLog2 == sizeDstLog2)
	{
		if(srcProps.Format == dstProps.Format)
		{
			FrameGraphCopyPass copyPass;
			copyPass.AddSource(src, TextureSurface(srcMip, 1, 0, 6));
			copyPass.AddDestination(dst, TextureSurface(dstMip, 1, 0, 6));

			graph.AddCopyPass("CubemapCopy", std::move(copyPass), [src, srcMip, dst, dstMip](FrameGraphPassContext& context)
			{
				for(u32 face = 0; face < 6; face++)
				{
					TextureCopyInformation copyDesc;
					copyDesc.SourceFace = face;
					copyDesc.SourceMip = srcMip;
					copyDesc.DestinationFace = face;
					copyDesc.DestinationMip = dstMip;

					context.GetCommandBuffer().CopyTexture(src, dst, copyDesc);
				}
			});
		}
		else
			DownsampleCubemap(graph, src, srcMip, dst, dstMip);
	}
	else
		DownsampleCubemap(graph, scratchTex, srcMip, dst, dstMip);
}

void RenderBeastIBLUtility::DownsampleCubemap(FrameGraph& graph, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip)
{
	ReflectionCubeDownsampleMaterial* material = ReflectionCubeDownsampleMaterial::Get();
	for(u32 face = 0; face < 6; face++)
	{
		FrameGraphRasterPass downsamplePass;
		downsamplePass.SetColorAttachment(0, dst, FrameGraphAttachmentLoad::DontCare, dstMip, face, 1);

		const TShared<GpuParameterSet> set = material->Prepare(downsamplePass, src, face, srcMip);
		graph.AddRasterPass("ReflectionCubeDownsample", std::move(downsamplePass), [material, set](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			material->Execute(context, set);
		});
	}
}

TShared<GpuBuffer> RenderBeastIBLUtility::ComputeCoefficientSets(FrameGraph& graph, const TShared<Texture>& cubemap, int shOrder, u32& numCoeffSets)
{
	IrradianceComputeSHMaterial* shCompute = IrradianceComputeSHMaterial::GetVariation(shOrder);
	const TShared<GpuBuffer> coeffSetBuffer = graph.CreateBuffer("IrradianceCoefficientSets", shCompute->GetOutputBufferInformation(cubemap, numCoeffSets));

	// A pass per face, as each dispatch binds the whole buffer although it writes only the sets of its face
	for(u32 face = 0; face < 6; face++)
	{
		FrameGraphComputePass computePass;
		const TShared<GpuParameterSet> set = shCompute->Prepare(computePass, cubemap, face, coeffSetBuffer);
		graph.AddComputePass("IrradianceComputeSH", std::move(computePass), [shCompute, set, cubemap](FrameGraphPassContext& context)
		{
			shCompute->Execute(context, set, cubemap);
		});
	}

	return coeffSetBuffer;
}
}} // namespace b3d::render
