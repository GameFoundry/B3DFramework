//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuResourcePool.h"

#include "B3DApplication.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace b3d;

namespace b3d { namespace render
{
GpuResourcePool::GpuResourcePool()
{
	mDevice = GetApplication().GetPrimaryGpuDevice();
}

SPtr<PooledRenderTexture> GpuResourcePool::Get(const PooledRenderTextureCreateInformation& desc)
{
	for(auto& entry : mTextures)
	{
		bool isFree = entry.use_count() == 1;
		if(!isFree)
			continue;

		if(entry->Texture == nullptr)
			continue;

		if(Matches(entry->Texture, desc))
		{
			entry->mLastUsedFrame = mCurrentFrame;
			return entry;
		}
	}

	SPtr<PooledRenderTexture> newTexture = B3DMakeShared<PooledRenderTexture>(mCurrentFrame);
	mTextures.Add(newTexture);

	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Type = desc.type;
	textureCreateInformation.Width = desc.width;
	textureCreateInformation.Height = desc.height;
	textureCreateInformation.Depth = desc.depth;
	textureCreateInformation.Format = desc.format;
	textureCreateInformation.Usage = desc.flag;
	textureCreateInformation.UseHardwareSRGB = desc.hwGamma;
	textureCreateInformation.SampleCount = desc.numSamples;
	textureCreateInformation.MipMapCount = desc.numMipLevels;

	if(desc.type != TEX_TYPE_3D)
		textureCreateInformation.ArraySliceCount = desc.arraySize;

	newTexture->Texture = mDevice->CreateTexture(textureCreateInformation);

	if(desc.flag.IsSetAny(TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil))
	{
		RenderTextureCreateInformation rtDesc;

		if(desc.flag.IsSet(TextureUsageFlag::RenderTarget))
		{
			rtDesc.ColorSurfaces[0].Texture = newTexture->Texture;
			rtDesc.ColorSurfaces[0].Face = 0;
			rtDesc.ColorSurfaces[0].FaceCount = newTexture->Texture->GetProperties().GetFaceCount();
			rtDesc.ColorSurfaces[0].MipLevel = 0;
		}

		if(desc.flag.IsSet(TextureUsageFlag::DepthStencil))
		{
			rtDesc.DepthStencilSurface.Texture = newTexture->Texture;
			rtDesc.DepthStencilSurface.Face = 0;
			rtDesc.DepthStencilSurface.FaceCount = newTexture->Texture->GetProperties().GetFaceCount();
			rtDesc.DepthStencilSurface.MipLevel = 0;
		}

		newTexture->RenderTexture = RenderTexture::Create(rtDesc);
	}

	return newTexture;
}

void GpuResourcePool::Get(SPtr<PooledRenderTexture>& texture, const PooledRenderTextureCreateInformation& desc)
{
	if(texture && Matches(texture->Texture, desc))
		return;

	texture = Get(desc);
}

SPtr<PooledStorageBuffer> GpuResourcePool::Get(const POOLED_STORAGE_BUFFER_DESC& desc)
{
	for(auto& entry : mBuffers)
	{
		bool isFree = entry.use_count() == 1;
		if(!isFree)
			continue;

		if(entry->Buffer == nullptr)
			continue;

		if(Matches(entry->Buffer, desc))
		{
			entry->mLastUsedFrame = mCurrentFrame;
			return entry;
		}
	}

	SPtr<PooledStorageBuffer> newBuffer = B3DMakeShared<PooledStorageBuffer>(mCurrentFrame);
	mBuffers.Add(newBuffer);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = desc.type;
	bufferCreateInformation.Flags = desc.flags;

	if(bufferCreateInformation.Type == GpuBufferType::SimpleStorage)
	{
		bufferCreateInformation.SimpleStorage.Count = desc.numElements;
		bufferCreateInformation.SimpleStorage.Format = desc.format;
	}
	else
	{
		B3D_ENSURE(bufferCreateInformation.Type == GpuBufferType::StructuredStorage);
		bufferCreateInformation.StructuredStorage.ElementSize = desc.elementSize;
		bufferCreateInformation.StructuredStorage.Count = desc.numElements;
	}

	newBuffer->Buffer = mDevice->CreateGpuBuffer(bufferCreateInformation);

	return newBuffer;
}

void GpuResourcePool::Get(SPtr<PooledStorageBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc)
{
	if(buffer && Matches(buffer->Buffer, desc))
		return;

	buffer = Get(desc);
}

void GpuResourcePool::Update()
{
	mCurrentFrame++;

	// Note: Should also force pruning when over some memory limit (in which case I can probably increase the
	// age pruning limit higher)
	Prune(3);
}

void GpuResourcePool::Prune(u32 age)
{
	for(auto iter = mTextures.begin(); iter != mTextures.end();)
	{
		auto& entry = *iter;

		bool isFree = entry.use_count() == 1;
		if(!isFree)
		{
			++iter;
			continue;
		}

		u32 entryAge = mCurrentFrame - entry->mLastUsedFrame;
		if(entryAge >= age)
			mTextures.SwapAndErase(iter);
		else
			++iter;
	}

	for(auto iter = mBuffers.begin(); iter != mBuffers.end();)
	{
		auto& entry = *iter;

		bool isFree = entry.use_count() == 1;
		if(!isFree)
		{
			++iter;
			continue;
		}

		u32 entryAge = mCurrentFrame - entry->mLastUsedFrame;
		if(entryAge >= age)
			mBuffers.SwapAndErase(iter);
		else
			++iter;
	}
}

bool GpuResourcePool::Matches(const SPtr<Texture>& texture, const PooledRenderTextureCreateInformation& desc)
{
	const TextureProperties& texProps = texture->GetProperties();

	bool match = texProps.Type == desc.type && texProps.Format == desc.format && texProps.Width == desc.width && texProps.Height == desc.height && (texProps.Usage & desc.flag) == desc.flag && ((desc.type == TEX_TYPE_2D && texProps.UseHardwareSRGB == desc.hwGamma && texProps.SampleCount == desc.numSamples) || (desc.type == TEX_TYPE_3D && texProps.Depth == desc.depth) || (desc.type == TEX_TYPE_CUBE_MAP)) && texProps.ArraySliceCount == desc.arraySize && texProps.MipMapCount == desc.numMipLevels;

	return match;
}

bool GpuResourcePool::Matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc)
{
	const GpuBufferInformation& gpuBufferInformation = buffer->GetInformation();

	bool match = gpuBufferInformation.Type == desc.type;
	if(match)
	{
		if(desc.type == GpuBufferType::SimpleStorage)
			match = gpuBufferInformation.SimpleStorage.Format == desc.format && gpuBufferInformation.SimpleStorage.Count == desc.numElements;
		else // Structured
			match = gpuBufferInformation.StructuredStorage.ElementSize == desc.elementSize && gpuBufferInformation.StructuredStorage.Count == desc.numElements;

		if(match)
			match = gpuBufferInformation.Flags == desc.flags;
	}

	return match;
}

PooledRenderTextureCreateInformation PooledRenderTextureCreateInformation::Create2D(PixelFormat format, u32 width, u32 height, TextureUsageFlags usage, u32 samples, bool hwGamma, u32 arraySize, u32 mipCount)
{
	PooledRenderTextureCreateInformation desc;
	desc.width = width;
	desc.height = height;
	desc.depth = 1;
	desc.format = format;
	desc.numSamples = samples;
	desc.flag = usage;
	desc.hwGamma = hwGamma;
	desc.type = TEX_TYPE_2D;
	desc.arraySize = arraySize;
	desc.numMipLevels = mipCount;

	return desc;
}

PooledRenderTextureCreateInformation PooledRenderTextureCreateInformation::Create3D(PixelFormat format, u32 width, u32 height, u32 depth, TextureUsageFlags usage)
{
	PooledRenderTextureCreateInformation desc;
	desc.width = width;
	desc.height = height;
	desc.depth = depth;
	desc.format = format;
	desc.numSamples = 1;
	desc.flag = usage;
	desc.hwGamma = false;
	desc.type = TEX_TYPE_3D;
	desc.arraySize = 1;
	desc.numMipLevels = 0;

	return desc;
}

PooledRenderTextureCreateInformation PooledRenderTextureCreateInformation::CreateCube(PixelFormat format, u32 width, u32 height, TextureUsageFlags usage, u32 arraySize)
{
	PooledRenderTextureCreateInformation desc;
	desc.width = width;
	desc.height = height;
	desc.depth = 1;
	desc.format = format;
	desc.numSamples = 1;
	desc.flag = usage;
	desc.hwGamma = false;
	desc.type = TEX_TYPE_CUBE_MAP;
	desc.arraySize = arraySize;
	desc.numMipLevels = 0;

	return desc;
}

POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::CreateStandard(GpuBufferFormat format, u32 numElements, GpuBufferFlags flags)
{
	POOLED_STORAGE_BUFFER_DESC desc;
	desc.type = GpuBufferType::SimpleStorage;
	desc.format = format;
	desc.numElements = numElements;
	desc.elementSize = 0;
	desc.flags = flags;

	return desc;
}

POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::CreateStructured(u32 elementSize, u32 numElements, GpuBufferFlags flags)
{
	POOLED_STORAGE_BUFFER_DESC desc;
	desc.type = GpuBufferType::StructuredStorage;
	desc.format = BF_UNKNOWN;
	desc.numElements = numElements;
	desc.elementSize = elementSize;
	desc.flags = flags;

	return desc;
}

GpuResourcePool& GetGpuResourcePool()
{
	return GpuResourcePool::Instance();
}

}}
