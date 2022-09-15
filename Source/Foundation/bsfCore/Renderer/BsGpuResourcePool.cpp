//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuResourcePool.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsGpuBuffer.h"

namespace bs { namespace ct
{
	SPtr<PooledRenderTexture> GpuResourcePool::Get(const POOLED_RENDER_TEXTURE_DESC& desc)
	{
		for (auto& entry : mTextures)
		{
			bool isFree = entry.use_count() == 1;
			if (!isFree)
				continue;

			if (entry->texture == nullptr)
				continue;

			if (Matches(entry->texture, desc))
			{
				entry->mLastUsedFrame = mCurrentFrame;
				return entry;
			}
		}

		SPtr<PooledRenderTexture> newTexture = bs_shared_ptr_new<PooledRenderTexture>(mCurrentFrame);
		mTextures.Add(newTexture);

		TEXTURE_DESC texDesc;
		texDesc.type = desc.type;
		texDesc.width = desc.width;
		texDesc.height = desc.height;
		texDesc.depth = desc.depth;
		texDesc.format = desc.format;
		texDesc.usage = desc.flag;
		texDesc.hwGamma = desc.hwGamma;
		texDesc.numSamples = desc.numSamples;
		texDesc.numMips = desc.numMipLevels;

		if (desc.type != TEX_TYPE_3D)
			texDesc.numArraySlices = desc.arraySize;

		newTexture->texture = Texture::Create(texDesc);
		
		if ((desc.flag & (TU_RENDERTARGET | TU_DEPTHSTENCIL)) != 0)
		{
			RENDER_TEXTURE_DESC rtDesc;

			if ((desc.flag & TU_RENDERTARGET) != 0)
			{
				rtDesc.colorSurfaces[0].texture = newTexture->texture;
				rtDesc.colorSurfaces[0].face = 0;
				rtDesc.colorSurfaces[0].numFaces = newTexture->texture->GetProperties().GetNumFaces();
				rtDesc.colorSurfaces[0].mipLevel = 0;
			}

			if ((desc.flag & TU_DEPTHSTENCIL) != 0)
			{
				rtDesc.depthStencilSurface.texture = newTexture->texture;
				rtDesc.depthStencilSurface.face = 0;
				rtDesc.depthStencilSurface.numFaces = newTexture->texture->GetProperties().GetNumFaces();
				rtDesc.depthStencilSurface.mipLevel = 0;
			}

			newTexture->renderTexture = RenderTexture::Create(rtDesc);
		}

		return newTexture;
	}

	void GpuResourcePool::Get(SPtr<PooledRenderTexture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc)
	{
		if(texture && Matches(texture->texture, desc))
			return;

		texture = Get(desc);
	}

	SPtr<PooledStorageBuffer> GpuResourcePool::Get(const POOLED_STORAGE_BUFFER_DESC& desc)
	{
		for (auto& entry : mBuffers)
		{
			bool isFree = entry.use_count() == 1;
			if (!isFree)
				continue;

			if (entry->buffer == nullptr)
				continue;

			if (Matches(entry->buffer, desc))
			{
				entry->mLastUsedFrame = mCurrentFrame;
				return entry;
			}
		}

		SPtr<PooledStorageBuffer> newBuffer = bs_shared_ptr_new<PooledStorageBuffer>(mCurrentFrame);
		mBuffers.Add(newBuffer);

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = desc.type;
		bufferDesc.elementSize = desc.elementSize;
		bufferDesc.elementCount = desc.numElements;
		bufferDesc.format = desc.format;
		bufferDesc.usage = desc.usage;

		newBuffer->buffer = GpuBuffer::Create(bufferDesc);

		return newBuffer;
	}

	void GpuResourcePool::Get(SPtr<PooledStorageBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc)
	{
		if(buffer && Matches(buffer->buffer, desc))
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

	void GpuResourcePool::Prune(UINT32 age)
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

			UINT32 entryAge = mCurrentFrame - entry->mLastUsedFrame;
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

			UINT32 entryAge = mCurrentFrame - entry->mLastUsedFrame;
			if(entryAge >= age)
				mBuffers.SwapAndErase(iter);
			else
				++iter;
		}
	}

	bool GpuResourcePool::Matches(const SPtr<Texture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc)
	{
		const TextureProperties& texProps = texture->GetProperties();

		bool match = texProps.GetTextureType() == desc.type
			&& texProps.GetFormat() == desc.format
			&& texProps.GetWidth() == desc.width
			&& texProps.GetHeight() == desc.height
			&& (texProps.GetUsage() & desc.flag) == desc.flag
			&& (
				(desc.type == TEX_TYPE_2D
					&& texProps.IsHardwareGammaEnabled() == desc.hwGamma
					&& texProps.GetNumSamples() == desc.numSamples)
				|| (desc.type == TEX_TYPE_3D
					&& texProps.GetDepth() == desc.depth)
				|| (desc.type == TEX_TYPE_CUBE_MAP)
				)
			&& texProps.GetNumArraySlices() == desc.arraySize
			&& texProps.GetNumMipmaps() == desc.numMipLevels
			;

		return match;
	}

	bool GpuResourcePool::Matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc)
	{
		const GpuBufferProperties& props = buffer->GetProperties();

		bool match = props.GetType() == desc.type && props.GetElementCount() == desc.numElements;
		if(match)
		{
			if (desc.type == GBT_STANDARD)
				match = props.GetFormat() == desc.format;
			else // Structured
				match = props.GetElementSize() == desc.elementSize;

			if(match)
				match = props.GetUsage() == desc.usage;
		}

		return match;
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::Create2D(PixelFormat format, UINT32 width, UINT32 height,
		INT32 usage, UINT32 samples, bool hwGamma, UINT32 arraySize, UINT32 mipCount)
	{
		POOLED_RENDER_TEXTURE_DESC desc;
		desc.width = width;
		desc.height = height;
		desc.depth = 1;
		desc.format = format;
		desc.numSamples = samples;
		desc.flag = (TextureUsage)usage;
		desc.hwGamma = hwGamma;
		desc.type = TEX_TYPE_2D;
		desc.arraySize = arraySize;
		desc.numMipLevels = mipCount;

		return desc;
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::Create3D(PixelFormat format, UINT32 width, UINT32 height,
		UINT32 depth, INT32 usage)
	{
		POOLED_RENDER_TEXTURE_DESC desc;
		desc.width = width;
		desc.height = height;
		desc.depth = depth;
		desc.format = format;
		desc.numSamples = 1;
		desc.flag = (TextureUsage)usage;
		desc.hwGamma = false;
		desc.type = TEX_TYPE_3D;
		desc.arraySize = 1;
		desc.numMipLevels = 0;

		return desc;
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::CreateCube(PixelFormat format, UINT32 width, UINT32 height,
		INT32 usage, UINT32 arraySize)
	{
		POOLED_RENDER_TEXTURE_DESC desc;
		desc.width = width;
		desc.height = height;
		desc.depth = 1;
		desc.format = format;
		desc.numSamples = 1;
		desc.flag = (TextureUsage)usage;
		desc.hwGamma = false;
		desc.type = TEX_TYPE_CUBE_MAP;
		desc.arraySize = arraySize;
		desc.numMipLevels = 0;

		return desc;
	}

	POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::CreateStandard(GpuBufferFormat format, UINT32 numElements,
		GpuBufferUsage usage)
	{
		POOLED_STORAGE_BUFFER_DESC desc;
		desc.type = GBT_STANDARD;
		desc.format = format;
		desc.numElements = numElements;
		desc.elementSize = 0;
		desc.usage = usage;

		return desc;
	}

	POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::CreateStructured(UINT32 elementSize, UINT32 numElements,
		GpuBufferUsage usage)
	{
		POOLED_STORAGE_BUFFER_DESC desc;
		desc.type = GBT_STRUCTURED;
		desc.format = BF_UNKNOWN;
		desc.numElements = numElements;
		desc.elementSize = elementSize;
		desc.usage = usage;

		return desc;
	}

	GpuResourcePool& gGpuResourcePool()
	{
		return GpuResourcePool::Instance();
	}

}}
