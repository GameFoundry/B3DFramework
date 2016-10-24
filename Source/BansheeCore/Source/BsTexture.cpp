//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTexture.h"
#include "BsTextureRTTI.h"
#include "BsDataStream.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsCoreThread.h"
#include "BsAsyncOp.h"
#include "BsResources.h"
#include "BsPixelUtil.h"

namespace BansheeEngine 
{
	TextureProperties::TextureProperties()
	{ }

	TextureProperties::TextureProperties(const TEXTURE_DESC& desc)
		:mDesc(desc)
	{

	}

	bool TextureProperties::hasAlpha() const
	{
		return PixelUtil::hasAlpha(mDesc.format);
	}

	UINT32 TextureProperties::getNumFaces() const
	{
		UINT32 facesPerSlice = getTextureType() == TEX_TYPE_CUBE_MAP ? 6 : 1;

		return facesPerSlice * mDesc.numArraySlices;
	}

	void TextureProperties::mapFromSubresourceIdx(UINT32 subresourceIdx, UINT32& face, UINT32& mip) const
	{
		UINT32 numMipmaps = getNumMipmaps() + 1;

		face = Math::floorToInt((subresourceIdx) / (float)numMipmaps);
		mip = subresourceIdx % numMipmaps;
	}

	UINT32 TextureProperties::mapToSubresourceIdx(UINT32 face, UINT32 mip) const
	{
		return face * (getNumMipmaps() + 1) + mip;
	}

	SPtr<PixelData> TextureProperties::allocateSubresourceBuffer(UINT32 subresourceIdx) const
	{
		UINT32 face = 0;
		UINT32 mip = 0;
		mapFromSubresourceIdx(subresourceIdx, face, mip);

		UINT32 width = getWidth();
		UINT32 height = getHeight();
		UINT32 depth = getDepth();

		UINT32 totalSize = PixelUtil::getMemorySize(width, height, depth, getFormat());

		for (UINT32 j = 0; j < mip; j++)
		{
			totalSize = PixelUtil::getMemorySize(width, height, depth, getFormat());

			if (width != 1) width /= 2;
			if (height != 1) height /= 2;
			if (depth != 1) depth /= 2;
		}

		SPtr<PixelData> dst = bs_shared_ptr_new<PixelData>(width, height, depth, getFormat());

		dst->allocateInternalBuffer();

		return dst;
	}

	SPtr<TextureCore> TextureCore::WHITE;
	SPtr<TextureCore> TextureCore::BLACK;
	SPtr<TextureCore> TextureCore::NORMAL;

	TextureCore::TextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initData, GpuDeviceFlags deviceMask)
		:mProperties(desc), mInitData(initData)
	{ }

	void TextureCore::initialize()
	{
		if (mInitData != nullptr)
		{
			writeSubresource(0, *mInitData, true);
			mInitData->_unlock();
			mInitData = nullptr;
		}
	}

	void TextureCore::writeSubresource(UINT32 subresourceIdx, const PixelData& pixelData, bool discardEntireBuffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(discardEntireBuffer)
		{
			if(mProperties.getUsage() != TU_DYNAMIC)
			{
				// Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.
				discardEntireBuffer = false;
			}
		}
		else
		{
			if (mProperties.getUsage() == TU_DYNAMIC)
			{
				LOGWRN("Buffer discard is not enabled but buffer was not created as dynamic. Enabling discard.");
				discardEntireBuffer = true;
			}
		}

		UINT32 face = 0;
		UINT32 mip = 0;
		mProperties.mapFromSubresourceIdx(subresourceIdx, face, mip);

		writeData(pixelData, mip, face, discardEntireBuffer);
	}

	void TextureCore::readSubresource(UINT32 subresourceIdx, PixelData& data)
	{
		THROW_IF_NOT_CORE_THREAD;

		UINT32 face = 0;
		UINT32 mip = 0;
		mProperties.mapFromSubresourceIdx(subresourceIdx, face, mip);

		PixelData& pixelData = static_cast<PixelData&>(data);

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.getWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mip, mipWidth, mipHeight, mipDepth);

		if (pixelData.getWidth() != mipWidth || pixelData.getHeight() != mipHeight ||
			pixelData.getDepth() != mipDepth || pixelData.getFormat() != mProperties.getFormat())
		{
			BS_EXCEPT(RenderingAPIException, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		}

		readData(pixelData, mip, face);
	}

	PixelData TextureCore::lock(GpuLockOptions options, UINT32 mipLevel, UINT32 face)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mipLevel > mProperties.getNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Invalid mip level: " + toString(mipLevel) + ". Min is 0, max is " + toString(mProperties.getNumMipmaps()));

		if (face >= mProperties.getNumFaces())
			BS_EXCEPT(InvalidParametersException, "Invalid face index: " + toString(face) + ". Min is 0, max is " + toString(mProperties.getNumFaces()));

		return lockImpl(options, mipLevel, face);
	}

	void TextureCore::unlock()
	{
		THROW_IF_NOT_CORE_THREAD;

		unlockImpl();
	}

	void TextureCore::copy(UINT32 srcSubresourceIdx, UINT32 destSubresourceIdx, const SPtr<TextureCore>& target)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (target->mProperties.getTextureType() != mProperties.getTextureType())
			BS_EXCEPT(InvalidParametersException, "Source and destination textures must be of same type and must have the same usage and type.");

		if (mProperties.getFormat() != target->mProperties.getFormat()) // Note: It might be okay to use different formats of the same size
			BS_EXCEPT(InvalidParametersException, "Source and destination texture formats must match.");

		if (target->mProperties.getNumSamples() > 0 && mProperties.getNumSamples() != target->mProperties.getNumSamples())
			BS_EXCEPT(InvalidParametersException, "When copying to a multisampled texture, source texture must have the same number of samples.");

		UINT32 srcFace = 0;
		UINT32 srcMipLevel = 0;

		UINT32 destFace = 0;
		UINT32 destMipLevel = 0;

		mProperties.mapFromSubresourceIdx(srcSubresourceIdx, srcFace, srcMipLevel);
		target->mProperties.mapFromSubresourceIdx(destSubresourceIdx, destFace, destMipLevel);

		if (destFace >= mProperties.getNumFaces())
			BS_EXCEPT(InvalidParametersException, "Invalid destination face index");

		if (srcFace >= target->mProperties.getNumFaces())
			BS_EXCEPT(InvalidParametersException, "Invalid destination face index");

		if (srcMipLevel > mProperties.getNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Source mip level out of range. Valid range is [0, " + toString(mProperties.getNumMipmaps()) + "]");

		if (destMipLevel > target->mProperties.getNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Destination mip level out of range. Valid range is [0, " + toString(target->mProperties.getNumMipmaps()) + "]");

		UINT32 srcMipWidth = mProperties.getWidth() >> srcMipLevel;
		UINT32 srcMipHeight = mProperties.getHeight() >> srcMipLevel;
		UINT32 srcMipDepth = mProperties.getDepth() >> srcMipLevel;

		UINT32 dstMipWidth = target->mProperties.getWidth() >> destMipLevel;
		UINT32 dstMipHeight = target->mProperties.getHeight() >> destMipLevel;
		UINT32 dstMipDepth = target->mProperties.getDepth() >> destMipLevel;

		if (srcMipWidth != dstMipWidth || srcMipHeight != dstMipHeight || srcMipDepth != dstMipDepth)
			BS_EXCEPT(InvalidParametersException, "Source and destination sizes must match");

		copyImpl(srcFace, srcMipLevel, destFace, destMipLevel, target);
	}

	/************************************************************************/
	/* 								TEXTURE VIEW                      		*/
	/************************************************************************/

	SPtr<TextureView> TextureCore::createView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc)
	{
		return bs_shared_ptr<TextureView>(new (bs_alloc<TextureView>()) TextureView(texture, desc));
	}

	void TextureCore::clearBufferViews()
	{
		mTextureViews.clear();
	}

	SPtr<TextureView> TextureCore::requestView(const SPtr<TextureCore>& texture, UINT32 mostDetailMip, UINT32 numMips, UINT32 firstArraySlice, UINT32 numArraySlices, GpuViewUsage usage)
	{
		THROW_IF_NOT_CORE_THREAD;

		assert(texture != nullptr);

		TEXTURE_VIEW_DESC key;
		key.mostDetailMip = mostDetailMip;
		key.numMips = numMips;
		key.firstArraySlice = firstArraySlice;
		key.usage = usage;

		const TextureProperties& texProps = texture->getProperties();
		if(numArraySlices == 0) // Automatically determine slice count
		{
			if(texProps.getTextureType() == TEX_TYPE_3D)
				key.numArraySlices = texProps.getDepth();
			else
				key.numArraySlices = texProps.getNumFaces();
		}
		else
		{
			key.numArraySlices = numArraySlices;
		}

		auto iterFind = texture->mTextureViews.find(key);
		if (iterFind == texture->mTextureViews.end())
		{
			SPtr<TextureView> newView = texture->createView(texture, key);
			texture->mTextureViews[key] = new (bs_alloc<TextureViewReference>()) TextureViewReference(newView);

			iterFind = texture->mTextureViews.find(key);
		}

		iterFind->second->refCount++;
		return iterFind->second->view;
	}

	void TextureCore::releaseView(const SPtr<TextureView>& view)
	{
		THROW_IF_NOT_CORE_THREAD;

		assert(view != nullptr);

		SPtr<TextureCore> texture = view->getTexture();

		auto iterFind = texture->mTextureViews.find(view->getDesc());
		if (iterFind == texture->mTextureViews.end())
		{
			BS_EXCEPT(InternalErrorException, "Trying to release a texture view that doesn't exist!");
		}

		iterFind->second->refCount--;

		if (iterFind->second->refCount == 0)
		{
			TextureViewReference* toRemove = iterFind->second;

			texture->mTextureViews.erase(iterFind);

			bs_delete(toRemove);
		}
	}

	/************************************************************************/
	/* 								STATICS	                      			*/
	/************************************************************************/
	SPtr<TextureCore> TextureCore::create(const TEXTURE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return TextureCoreManager::instance().createTexture(desc, deviceMask);
	}

	SPtr<TextureCore> TextureCore::create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection, 
		GpuDeviceFlags deviceMask)
	{
		TEXTURE_DESC desc;
		desc.type = pixelData->getDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
		desc.width = pixelData->getWidth();
		desc.height = pixelData->getHeight();
		desc.depth = pixelData->getDepth();
		desc.format = pixelData->getFormat();
		desc.usage = usage;
		desc.hwGamma = hwGammaCorrection;

		return TextureCoreManager::instance().createTextureInternal(desc, pixelData, deviceMask);
	}

	Texture::Texture()
	{

	}

	Texture::Texture(const TEXTURE_DESC& desc)
		:mProperties(desc)
    {
        
    }

	Texture::Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData)
		: mProperties(desc), mInitData(pixelData)
	{
		if (mInitData != nullptr)
			mInitData->_lock();
	}

	void Texture::initialize()
	{
		mSize = calculateSize();

		// Allocate CPU buffers if needed
		if ((mProperties.getUsage() & TU_CPUCACHED) != 0)
		{
			createCPUBuffers();

			if (mInitData != nullptr)
				updateCPUBuffers(0, *mInitData);
		}

		Resource::initialize();
	}

	SPtr<CoreObjectCore> Texture::createCore() const
	{
		const TextureProperties& props = getProperties();

		SPtr<CoreObjectCore> coreObj = TextureCoreManager::instance().createTextureInternal(props.mDesc, mInitData);

		if ((mProperties.getUsage() & TU_CPUCACHED) == 0)
			mInitData = nullptr;

		return coreObj;
	}

	AsyncOp Texture::writeSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<PixelData>& data, bool discardEntireBuffer)
	{
		updateCPUBuffers(subresourceIdx, *data);

		data->_lock();

		std::function<void(const SPtr<TextureCore>&, UINT32, const SPtr<PixelData>&, bool, AsyncOp&)> func =
			[&](const SPtr<TextureCore>& texture, UINT32 _subresourceIdx, const SPtr<PixelData>& _pixData, bool _discardEntireBuffer, AsyncOp& asyncOp)
		{
			texture->writeSubresource(_subresourceIdx, *_pixData, _discardEntireBuffer);
			_pixData->_unlock();
			asyncOp._completeOperation();

		};

		return accessor.queueReturnCommand(std::bind(func, getCore(), subresourceIdx,
			data, discardEntireBuffer, std::placeholders::_1));
	}

	AsyncOp Texture::readSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<PixelData>& data)
	{
		data->_lock();

		std::function<void(const SPtr<TextureCore>&, UINT32, const SPtr<PixelData>&, AsyncOp&)> func =
			[&](const SPtr<TextureCore>& texture, UINT32 _subresourceIdx, const SPtr<PixelData>& _pixData, AsyncOp& asyncOp)
		{
			texture->readSubresource(_subresourceIdx, *_pixData);
			_pixData->_unlock();
			asyncOp._completeOperation();

		};

		return accessor.queueReturnCommand(std::bind(func, getCore(), subresourceIdx,
			data, std::placeholders::_1));
	}

	UINT32 Texture::calculateSize() const
	{
		return mProperties.getNumFaces() * PixelUtil::getMemorySize(mProperties.getWidth(),
			mProperties.getHeight(), mProperties.getDepth(), mProperties.getFormat());
	}

	void Texture::updateCPUBuffers(UINT32 subresourceIdx, const PixelData& pixelData)
	{
		if ((mProperties.getUsage() & TU_CPUCACHED) == 0)
			return;

		if (subresourceIdx >= (UINT32)mCPUSubresourceData.size())
		{
			LOGERR("Invalid subresource index: " + toString(subresourceIdx) + ". Supported range: 0 .. " + toString(mCPUSubresourceData.size()));
			return;
		}

		UINT32 mipLevel;
		UINT32 face;
		mProperties.mapFromSubresourceIdx(subresourceIdx, face, mipLevel);

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.getWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mipLevel, mipWidth, mipHeight, mipDepth);

		if (pixelData.getWidth() != mipWidth || pixelData.getHeight() != mipHeight ||
			pixelData.getDepth() != mipDepth || pixelData.getFormat() != mProperties.getFormat())
		{
			LOGERR("Provided buffer is not of valid dimensions or format in order to update this texture.");
			return;
		}

		if (mCPUSubresourceData[subresourceIdx]->getSize() != pixelData.getSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* dest = mCPUSubresourceData[subresourceIdx]->getData();
		UINT8* src = pixelData.getData();

		memcpy(dest, src, pixelData.getSize());
	}

	void Texture::readData(PixelData& dest, UINT32 mipLevel, UINT32 face)
	{
		if ((mProperties.getUsage() & TU_CPUCACHED) == 0)
		{
			LOGERR("Attempting to read CPU data from a texture that is created without CPU caching.");
			return;
		}

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.getWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mipLevel, mipWidth, mipHeight, mipDepth);

		if (dest.getWidth() != mipWidth || dest.getHeight() != mipHeight ||
			dest.getDepth() != mipDepth || dest.getFormat() != mProperties.getFormat())
		{
			LOGERR("Provided buffer is not of valid dimensions or format in order to read from this texture.");
			return;
		}

		UINT32 subresourceIdx = mProperties.mapToSubresourceIdx(face, mipLevel);
		if (subresourceIdx >= (UINT32)mCPUSubresourceData.size())
		{
			LOGERR("Invalid subresource index: " + toString(subresourceIdx) + ". Supported range: 0 .. " + toString(mCPUSubresourceData.size()));
			return;
		}

		if (mCPUSubresourceData[subresourceIdx]->getSize() != dest.getSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* srcPtr = mCPUSubresourceData[subresourceIdx]->getData();
		UINT8* destPtr = dest.getData();

		memcpy(destPtr, srcPtr, dest.getSize());
	}

	void Texture::createCPUBuffers()
	{
		UINT32 numFaces = mProperties.getNumFaces();
		UINT32 numMips = mProperties.getNumMipmaps() + 1;

		UINT32 numSubresources = numFaces * numMips;
		mCPUSubresourceData.resize(numSubresources);

		for (UINT32 i = 0; i < numFaces; i++)
		{
			UINT32 curWidth = mProperties.getWidth();
			UINT32 curHeight = mProperties.getHeight();
			UINT32 curDepth = mProperties.getDepth();

			for (UINT32 j = 0; j < numMips; j++)
			{
				UINT32 subresourceIdx = mProperties.mapToSubresourceIdx(i, j);

				mCPUSubresourceData[subresourceIdx] = bs_shared_ptr_new<PixelData>(curWidth, curHeight, curDepth, mProperties.getFormat());
				mCPUSubresourceData[subresourceIdx]->allocateInternalBuffer();

				if (curWidth > 1)
					curWidth = curWidth / 2;

				if (curHeight > 1)
					curHeight = curHeight / 2;

				if (curDepth > 1)
					curDepth = curDepth / 2;
			}
		}
	}

	SPtr<TextureCore> Texture::getCore() const
	{
		return std::static_pointer_cast<TextureCore>(mCoreSpecific);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* Texture::getRTTIStatic()
	{
		return TextureRTTI::instance();
	}

	RTTITypeBase* Texture::getRTTI() const
	{
		return Texture::getRTTIStatic();
	}

	/************************************************************************/
	/* 								STATICS	                      			*/
	/************************************************************************/
	HTexture Texture::create(const TEXTURE_DESC& desc)
	{
		SPtr<Texture> texturePtr = _createPtr(desc);

		return static_resource_cast<Texture>(gResources()._createResourceHandle(texturePtr));
	}
	
	HTexture Texture::create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
	{
		SPtr<Texture> texturePtr = _createPtr(pixelData, usage, hwGammaCorrection);

		return static_resource_cast<Texture>(gResources()._createResourceHandle(texturePtr));
	}

	SPtr<Texture> Texture::_createPtr(const TEXTURE_DESC& desc)
	{
		return TextureManager::instance().createTexture(desc);
	}

	SPtr<Texture> Texture::_createPtr(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
	{
		TEXTURE_DESC desc;
		desc.type = pixelData->getDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
		desc.width = pixelData->getWidth();
		desc.height = pixelData->getHeight();
		desc.depth = pixelData->getDepth();
		desc.format = pixelData->getFormat();
		desc.usage = usage;
		desc.hwGamma = hwGammaCorrection;

		return TextureManager::instance().createTexture(desc, pixelData);
	}
}