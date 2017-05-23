//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuResourcePool.h"
#include "BsRenderTexture.h"
#include "BsTexture.h"
#include "BsGpuBuffer.h"
#include "BsTextureManager.h"

namespace bs { namespace ct
{
	PooledRenderTexture::PooledRenderTexture(GpuResourcePool* pool)
		:mPool(pool), mIsFree(false)
	{ }

	PooledRenderTexture::~PooledRenderTexture()
	{
		if (mPool != nullptr)
			mPool->_unregisterTexture(this);
	}

	PooledStorageBuffer::PooledStorageBuffer(GpuResourcePool* pool)
		:mPool(pool), mIsFree(false)
	{ }

	PooledStorageBuffer::~PooledStorageBuffer()
	{
		if (mPool != nullptr)
			mPool->_unregisterBuffer(this);
	}

	GpuResourcePool::~GpuResourcePool()
	{
		for (auto& texture : mTextures)
			texture.second.lock()->mPool = nullptr;

		for (auto& buffer : mBuffers)
			buffer.second.lock()->mPool = nullptr;
	}

	SPtr<PooledRenderTexture> GpuResourcePool::get(const POOLED_RENDER_TEXTURE_DESC& desc)
	{
		for (auto& texturePair : mTextures)
		{
			SPtr<PooledRenderTexture> textureData = texturePair.second.lock();

			if (!textureData->mIsFree)
				continue;

			if (textureData->texture == nullptr)
				continue;

			if (matches(textureData->texture, desc))
			{
				textureData->mIsFree = false;
				return textureData;
			}
		}

		SPtr<PooledRenderTexture> newTextureData = bs_shared_ptr_new<PooledRenderTexture>(this);
		_registerTexture(newTextureData);

		TEXTURE_DESC texDesc;
		texDesc.type = desc.type;
		texDesc.width = desc.width;
		texDesc.height = desc.height;
		texDesc.depth = desc.depth;
		texDesc.format = desc.format;
		texDesc.usage = desc.flag;
		texDesc.hwGamma = desc.hwGamma;
		texDesc.numSamples = desc.numSamples;

		if (desc.type != TEX_TYPE_3D)
			texDesc.numArraySlices = desc.arraySize;

		newTextureData->texture = TextureManager::instance().createTexture(texDesc);
		
		if ((desc.flag & (TU_RENDERTARGET | TU_DEPTHSTENCIL)) != 0)
		{
			RENDER_TEXTURE_DESC rtDesc;

			if ((desc.flag & TU_RENDERTARGET) != 0)
			{
				rtDesc.colorSurfaces[0].texture = newTextureData->texture;
				rtDesc.colorSurfaces[0].face = 0;
				rtDesc.colorSurfaces[0].numFaces = newTextureData->texture->getProperties().getNumFaces();
				rtDesc.colorSurfaces[0].mipLevel = 0;
			}

			if ((desc.flag & TU_DEPTHSTENCIL) != 0)
			{
				rtDesc.depthStencilSurface.texture = newTextureData->texture;
				rtDesc.depthStencilSurface.face = 0;
				rtDesc.depthStencilSurface.numFaces = newTextureData->texture->getProperties().getNumFaces();
				rtDesc.depthStencilSurface.mipLevel = 0;
			}

			newTextureData->renderTexture = TextureManager::instance().createRenderTexture(rtDesc);
		}

		return newTextureData;
	}

	SPtr<PooledStorageBuffer> GpuResourcePool::get(const POOLED_STORAGE_BUFFER_DESC& desc)
	{
		for (auto& bufferPair : mBuffers)
		{
			SPtr<PooledStorageBuffer> bufferData = bufferPair.second.lock();

			if (!bufferData->mIsFree)
				continue;

			if (bufferData->buffer == nullptr)
				continue;

			if (matches(bufferData->buffer, desc))
			{
				bufferData->mIsFree = false;
				return bufferData;
			}
		}

		SPtr<PooledStorageBuffer> newBufferData = bs_shared_ptr_new<PooledStorageBuffer>(this);
		_registerBuffer(newBufferData);

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = desc.type;
		bufferDesc.elementSize = desc.elementSize;
		bufferDesc.elementCount = desc.numElements;
		bufferDesc.format = desc.format;
		bufferDesc.randomGpuWrite = true;

		newBufferData->buffer = GpuBuffer::create(bufferDesc);

		return newBufferData;
	}

	void GpuResourcePool::release(const SPtr<PooledRenderTexture>& texture)
	{
		auto iterFind = mTextures.find(texture.get());
		iterFind->second.lock()->mIsFree = true;
	}

	void GpuResourcePool::release(const SPtr<PooledStorageBuffer>& buffer)
	{
		auto iterFind = mBuffers.find(buffer.get());
		iterFind->second.lock()->mIsFree = true;
	}

	bool GpuResourcePool::matches(const SPtr<Texture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc)
	{
		const TextureProperties& texProps = texture->getProperties();

		bool match = texProps.getTextureType() == desc.type 
			&& texProps.getFormat() == desc.format 
			&& texProps.getWidth() == desc.width 
			&& texProps.getHeight() == desc.height
			&& (texProps.getUsage() & desc.flag) == desc.flag
			&& (
				(desc.type == TEX_TYPE_2D 
					&& texProps.isHardwareGammaEnabled() == desc.hwGamma 
					&& texProps.getNumSamples() == desc.numSamples)
				|| (desc.type == TEX_TYPE_3D 
					&& texProps.getDepth() == desc.depth)
				|| (desc.type == TEX_TYPE_CUBE_MAP)
				)
			&& texProps.getNumArraySlices() == desc.arraySize
			;

		return match;
	}

	bool GpuResourcePool::matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc)
	{
		const GpuBufferProperties& props = buffer->getProperties();

		bool match = props.getType() == desc.type && props.getElementCount() == desc.numElements;
		if(match)
		{
			if (desc.type == GBT_STANDARD)
				match = props.getFormat() == desc.format;
			else // Structured
				match = props.getElementSize() == desc.elementSize;
		}

		return match;
	}

	void GpuResourcePool::_registerTexture(const SPtr<PooledRenderTexture>& texture)
	{
		mTextures.insert(std::make_pair(texture.get(), texture));
	}

	void GpuResourcePool::_unregisterTexture(PooledRenderTexture* texture)
	{
		mTextures.erase(texture);
	}

	void GpuResourcePool::_registerBuffer(const SPtr<PooledStorageBuffer>& buffer)
	{
		mBuffers.insert(std::make_pair(buffer.get(), buffer));
	}

	void GpuResourcePool::_unregisterBuffer(PooledStorageBuffer* buffer)
	{
		mBuffers.erase(buffer);
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::create2D(PixelFormat format, UINT32 width, UINT32 height,
		INT32 usage, UINT32 samples, bool hwGamma, UINT32 arraySize)
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

		return desc;
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::create3D(PixelFormat format, UINT32 width, UINT32 height, 
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

		return desc;
	}

	POOLED_RENDER_TEXTURE_DESC POOLED_RENDER_TEXTURE_DESC::createCube(PixelFormat format, UINT32 width, UINT32 height,
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

		return desc;
	}

	POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::createStandard(GpuBufferFormat format, UINT32 numElements)
	{
		POOLED_STORAGE_BUFFER_DESC desc;
		desc.type = GBT_STANDARD;
		desc.format = format;
		desc.numElements = numElements;
		desc.elementSize = 0;

		return desc;
	}

	POOLED_STORAGE_BUFFER_DESC POOLED_STORAGE_BUFFER_DESC::createStructured(UINT32 elementSize, UINT32 numElements)
	{
		POOLED_STORAGE_BUFFER_DESC desc;
		desc.type = GBT_STRUCTURED;
		desc.format = BF_UNKNOWN;
		desc.numElements = numElements;
		desc.elementSize = elementSize;

		return desc;
	}
}}