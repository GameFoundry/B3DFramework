//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsTexture.h"
#include "Private/RTTI/BsTextureRTTI.h"
#include "FileSystem/BsDataStream.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsAsyncOp.h"
#include "Resources/BsResources.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	TEXTURE_COPY_DESC TEXTURE_COPY_DESC::DEFAULT = TEXTURE_COPY_DESC();

	TextureProperties::TextureProperties(const TEXTURE_DESC& desc)
		:mDesc(desc)
	{

	}

	bool TextureProperties::HasAlpha() const
	{
		return PixelUtil::HasAlpha(mDesc.format);
	}

	UINT32 TextureProperties::GetNumFaces() const
	{
		UINT32 facesPerSlice = getTextureType() == TEX_TYPE_CUBE_MAP ? 6 : 1;

		return facesPerSlice * mDesc.numArraySlices;
	}

	void TextureProperties::MapFromSubresourceIdx(UINT32 subresourceIdx, UINT32& face, UINT32& mip) const
	{
		UINT32 numMipmaps = getNumMipmaps() + 1;

		face = Math::floorToInt((subresourceIdx) / (float)numMipmaps);
		mip = subresourceIdx % numMipmaps;
	}

	UINT32 TextureProperties::MapToSubresourceIdx(UINT32 face, UINT32 mip) const
	{
		return face * (getNumMipmaps() + 1) + mip;
	}

	SPtr<PixelData> TextureProperties::AllocBuffer(UINT32 face, UINT32 mipLevel) const
	{
		UINT32 width = getWidth();
		UINT32 height = getHeight();
		UINT32 depth = getDepth();

		for (UINT32 j = 0; j < mipLevel; j++)
		{
			if (width != 1) width /= 2;
			if (height != 1) height /= 2;
			if (depth != 1) depth /= 2;
		}

		SPtr<PixelData> dst = bs_shared_ptr_new<PixelData>(width, height, depth, getFormat());
		dst->AllocateInternalBuffer();

		return dst;
	}

	Texture::Texture(const TEXTURE_DESC& desc)
		:mProperties(desc)
	{
		
	}

	Texture::Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData)
		: MProperties(desc), mInitData(pixelData)
	{
		if (mInitData != nullptr)
			mInitData->_lock();
	}

	void Texture::Initialize()
	{
		mSize = calculateSize();

		// Allocate CPU buffers if needed
		if ((mProperties.GetUsage() & TU_CPUCACHED) != 0)
		{
			createCPUBuffers();

			if (mInitData != nullptr)
				updateCPUBuffers(0, *mInitData);
		}

		Resource::initialize();
	}

	SPtr<ct::CoreObject> Texture::CreateCore() const
	{
		const TextureProperties& props = getProperties();

		SPtr<ct::CoreObject> coreObj = ct::TextureManager::instance().CreateTextureInternal(props.mDesc, mInitData);

		if ((mProperties.GetUsage() & TU_CPUCACHED) == 0)
			mInitData = nullptr;

		return coreObj;
	}

	AsyncOp Texture::WriteData(const SPtr<PixelData>& data, UINT32 face, UINT32 mipLevel, bool discardEntireBuffer)
	{
		UINT32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
		updateCPUBuffers(subresourceIdx, *data);

		data->_lock();

		std::function<void(const SPtr<ct::Texture>&, UINT32, UINT32, const SPtr<PixelData>&, bool, AsyncOp&)> func =
			[&](const SPtr<ct::Texture>& texture, UINT32 _face, UINT32 _mipLevel, const SPtr<PixelData>& _pixData,
				bool _discardEntireBuffer, AsyncOp& asyncOp)
		{
			texture->WriteData(*_pixData, _mipLevel, _face, _discardEntireBuffer);
			_pixData->_unlock();
			asyncOp._completeOperation();

		};

		return GCoreThread().QueueReturnCommand(std::bind(func, getCore(), face, mipLevel,
			data, discardEntireBuffer, std::placeholders::_1));
	}

	AsyncOp Texture::ReadData(const SPtr<PixelData>& data, UINT32 face, UINT32 mipLevel)
	{
		data->_lock();

		std::function<void(const SPtr<ct::Texture>&, UINT32, UINT32, const SPtr<PixelData>&, AsyncOp&)> func =
			[&](const SPtr<ct::Texture>& texture, UINT32 _face, UINT32 _mipLevel, const SPtr<PixelData>& _pixData,
				AsyncOp& asyncOp)
		{
			// Make sure any queued command start executing before reading
			ct::RenderAPI::instance().SubmitCommandBuffer(nullptr);

			texture->ReadData(*_pixData, _mipLevel, _face);
			_pixData->_unlock();
			asyncOp._completeOperation();

		};

		return GCoreThread().QueueReturnCommand(std::bind(func, getCore(), face, mipLevel,
			data, std::placeholders::_1));
	}

	TAsyncOp<SPtr<PixelData>> Texture::ReadData(UINT32 face, UINT32 mipLevel)
	{
		TAsyncOp<SPtr<PixelData>> op;

		auto func = [texture = getCore(), face, mipLevel, op]() mutable
		{
			// Make sure any queued command start executing before reading
			ct::RenderAPI::instance().SubmitCommandBuffer(nullptr);

			SPtr<PixelData> output = texture->GetProperties().AllocBuffer(face, mipLevel);
			texture->ReadData(*output, mipLevel, face);

			op._completeOperation(output);

		};

		gCoreThread().QueueCommand(func);
		return op;
	}

	UINT32 Texture::CalculateSize() const
	{
		return mProperties.GetNumFaces() * PixelUtil::getMemorySize(mProperties.getWidth(),
			mProperties.GetHeight(), mProperties.getDepth(), mProperties.getFormat());
	}

	void Texture::UpdateCPUBuffers(UINT32 subresourceIdx, const PixelData& pixelData)
	{
		if ((mProperties.GetUsage() & TU_CPUCACHED) == 0)
			return;

		if (subresourceIdx >= (UINT32)mCPUSubresourceData.size())
		{
			BS_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}",
				subresourceIdx, (UINT32)mCPUSubresourceData.size());
			return;
		}

		UINT32 mipLevel;
		UINT32 face;
		mProperties.MapFromSubresourceIdx(subresourceIdx, face, mipLevel);

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.GetWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mipLevel, mipWidth, mipHeight, mipDepth);

		if (pixelData.GetWidth() != mipWidth || pixelData.getHeight() != mipHeight ||
			pixelData.GetDepth() != mipDepth || pixelData.getFormat() != mProperties.getFormat())
		{
			BS_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to update this texture.");
			return;
		}

		if (mCPUSubresourceData[subresourceIdx]->GetSize() != pixelData.GetSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* dest = mCPUSubresourceData[subresourceIdx]->GetData();
		UINT8* src = pixelData.GetData();

		memcpy(dest, src, pixelData.GetSize());
	}

	void Texture::ReadCachedData(PixelData& dest, UINT32 face, UINT32 mipLevel)
	{
		if ((mProperties.GetUsage() & TU_CPUCACHED) == 0)
		{
			BS_LOG(Error, Texture, "Attempting to read CPU data from a texture that is created without CPU caching.");
			return;
		}

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.GetWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mipLevel, mipWidth, mipHeight, mipDepth);

		if (dest.GetWidth() != mipWidth || dest.getHeight() != mipHeight ||
			dest.GetDepth() != mipDepth || dest.getFormat() != mProperties.getFormat())
		{
			BS_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
			return;
		}

		UINT32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
		if (subresourceIdx >= (UINT32)mCPUSubresourceData.size())
		{
			BS_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}",
				subresourceIdx, (UINT32)mCPUSubresourceData.size());
			return;
		}

		if (mCPUSubresourceData[subresourceIdx]->GetSize() != dest.GetSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* srcPtr = mCPUSubresourceData[subresourceIdx]->GetData();
		UINT8* destPtr = dest.GetData();

		memcpy(destPtr, srcPtr, dest.GetSize());
	}

	void Texture::CreateCPUBuffers()
	{
		UINT32 numFaces = mProperties.GetNumFaces();
		UINT32 numMips = mProperties.GetNumMipmaps() + 1;

		UINT32 numSubresources = numFaces * numMips;
		mCPUSubresourceData.Resize(numSubresources);

		for (UINT32 i = 0; i < numFaces; i++)
		{
			UINT32 curWidth = mProperties.GetWidth();
			UINT32 curHeight = mProperties.GetHeight();
			UINT32 curDepth = mProperties.GetDepth();

			for (UINT32 j = 0; j < numMips; j++)
			{
				UINT32 subresourceIdx = mProperties.MapToSubresourceIdx(i, j);

				mCPUSubresourceData[subresourceIdx] = bs_shared_ptr_new<PixelData>(curWidth, curHeight, curDepth, mProperties.GetFormat());
				mCPUSubresourceData[subresourceIdx]->AllocateInternalBuffer();

				if (curWidth > 1)
					curWidth = curWidth / 2;

				if (curHeight > 1)
					curHeight = curHeight / 2;

				if (curDepth > 1)
					curDepth = curDepth / 2;
			}
		}
	}

	SPtr<ct::Texture> Texture::GetCore() const
	{
		return std::static_pointer_cast<ct::Texture>(mCoreSpecific);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* Texture::getRTTIStatic()
	{
		return TextureRTTI::Instance();
	}

	RTTITypeBase* Texture::getRTTI() const
	{
		return Texture::GetRTTIStatic();
	}

	/************************************************************************/
	/* 								STATICS	                      			*/
	/************************************************************************/
	HTexture Texture::Create(const TEXTURE_DESC& desc)
	{
		SPtr<Texture> texturePtr = _createPtr(desc);

		return static_resource_cast<Texture>(gResources()._createResourceHandle(texturePtr));
	}
	
	HTexture Texture::Create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
	{
		SPtr<Texture> texturePtr = _createPtr(pixelData, usage, hwGammaCorrection);

		return static_resource_cast<Texture>(gResources()._createResourceHandle(texturePtr));
	}

	SPtr<Texture> Texture::_createPtr(const TEXTURE_DESC& desc)
	{
		return TextureManager::Instance().CreateTexture(desc);
	}

	SPtr<Texture> Texture::_createPtr(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
	{
		TEXTURE_DESC desc;
		desc.type = pixelData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
		desc.width = pixelData->GetWidth();
		desc.height = pixelData->GetHeight();
		desc.depth = pixelData->GetDepth();
		desc.format = pixelData->GetFormat();
		desc.usage = usage;
		desc.hwGamma = hwGammaCorrection;

		return TextureManager::Instance().CreateTexture(desc, pixelData);
	}

	namespace ct
	{
	SPtr<Texture> Texture::WHITE;
	SPtr<Texture> Texture::BLACK;
	SPtr<Texture> Texture::NORMAL;

	Texture::Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initData, GpuDeviceFlags deviceMask)
		:mProperties(desc), mInitData(initData)
	{ }

	void Texture::Initialize()
	{
		if (mInitData != nullptr)
		{
			writeData(*mInitData, 0, 0, true);
			mInitData->_unlock();
			mInitData = nullptr;
		}

		CoreObject::initialize();
	}

	void Texture::writeData(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardEntireBuffer,
		UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(discardEntireBuffer)
		{
			if((mProperties.GetUsage() & TU_DYNAMIC) == 0)
			{
				// Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.
				discardEntireBuffer = false;
			}
		}

		writeDataImpl(src, mipLevel, face, discardEntireBuffer, queueIdx);
	}

	void Texture::ReadData(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		PixelData& pixelData = static_cast<PixelData&>(dest);

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(mProperties.GetWidth(), mProperties.getHeight(), mProperties.getDepth(),
			mipLevel, mipWidth, mipHeight, mipDepth);

		if (pixelData.GetWidth() != mipWidth || pixelData.getHeight() != mipHeight ||
			pixelData.GetDepth() != mipDepth || pixelData.getFormat() != mProperties.getFormat())
		{
			BS_LOG(Error, Texture,
				"Provided buffer is not of valid dimensions or format in order to read from this texture.");
			return;
		}

		readDataImpl(pixelData, mipLevel, face, deviceIdx, queueIdx);
	}

	PixelData Texture::Lock(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mipLevel > mProperties.GetNumMipmaps())
		{
			BS_LOG(Error, Texture, "Invalid mip level: {0}. Min is 0, max is {1}", mipLevel, mProperties.GetNumMipmaps());
			return PixelData(0, 0, 0, PF_UNKNOWN);
		}

		if (face >= mProperties.GetNumFaces())
		{
			BS_LOG(Error, Texture, "Invalid face index: {0}. Min is 0, max is {1}", face, mProperties.GetNumFaces());
			return PixelData(0, 0, 0, PF_UNKNOWN);
		}

		return LockImpl(options, mipLevel, face, deviceIdx, queueIdx);
	}

	void Texture::Unlock()
	{
		THROW_IF_NOT_CORE_THREAD;

		unlockImpl();
	}

	void Texture::Copy(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, const SPtr<CommandBuffer>& commandBuffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (target->mProperties.GetTextureType() != mProperties.getTextureType())
		{
			BS_LOG(Error, Texture, "Source and destination textures must be of same type.");
			return;
		}

		if (mProperties.GetFormat() != target->mProperties.getFormat()) // Note: It might be okay to use different formats of the same size
		{
			BS_LOG(Error, Texture, "Source and destination texture formats must match.");
			return;
		}

		if (target->mProperties.GetNumSamples() > 1 && mProperties.getNumSamples() != target->mProperties.getNumSamples())
		{
			BS_LOG(Error, Texture,
				"When copying to a multisampled texture, source texture must have the same number of samples.");
			return;
		}

		if (desc.srcFace >= mProperties.GetNumFaces())
		{
			BS_LOG(Error, Texture, "Invalid source face index.");
			return;
		}

		if (desc.dstFace >= target->mProperties.GetNumFaces())
		{
			BS_LOG(Error, Texture, "Invalid destination face index.");
			return;
		}

		if (desc.srcMip > mProperties.GetNumMipmaps())
		{
			BS_LOG(Error, Texture, "Source mip level out of range. Valid range is [0, {0}].", mProperties.GetNumMipmaps());
			return;
		}

		if (desc.dstMip > target->mProperties.GetNumMipmaps())
		{
			BS_LOG(Error, Texture, "Destination mip level out of range. Valid range is [0, {0}].",
				target->mProperties.GetNumMipmaps());
			return;
		}

		UINT32 srcWidth, srcHeight, srcDepth;
		PixelUtil::getSizeForMipLevel(
			mProperties.GetWidth(),
			mProperties.GetHeight(),
			mProperties.GetDepth(),
			desc.srcMip,
			srcWidth,
			srcHeight,
			srcDepth);

		UINT32 dstWidth, dstHeight, dstDepth;
		PixelUtil::getSizeForMipLevel(
			target->mProperties.GetWidth(),
			target->mProperties.GetHeight(),
			target->mProperties.GetDepth(),
			desc.dstMip,
			dstWidth,
			dstHeight,
			dstDepth);

		if(desc.dstPosition.x < 0 || desc.dstPosition.x >= (INT32)dstWidth ||
			desc.dstPosition.y < 0 || desc.dstPosition.y >= (INT32)dstHeight ||
			desc.dstPosition.z < 0 || desc.dstPosition.z >= (INT32)dstDepth)
		{
			BS_LOG(Error, Texture, "Destination position falls outside the destination texture.");
			return;
		}

		bool entireSurface = desc.srcVolume.GetWidth() == 0 ||
			desc.srcVolume.GetHeight() == 0 ||
			desc.srcVolume.GetDepth() == 0;

		UINT32 dstRight = (UINT32)desc.dstPosition.x;
		UINT32 dstBottom = (UINT32)desc.dstPosition.y;
		UINT32 dstBack = (UINT32)desc.dstPosition.z;
		if(!entireSurface)
		{
			if(desc.srcVolume.left >= srcWidth || desc.srcVolume.right > srcWidth ||
				desc.srcVolume.top >= srcHeight || desc.srcVolume.bottom > srcHeight ||
				desc.srcVolume.front >= srcDepth || desc.srcVolume.back > srcDepth)
			{
				BS_LOG(Error, Texture, "Source volume falls outside the source texture.");
				return;
			}

			dstRight += desc.srcVolume.GetWidth();
			dstBottom += desc.srcVolume.GetHeight();
			dstBack += desc.srcVolume.GetDepth();
		}
		else
		{
			dstRight += srcWidth;
			dstBottom += srcHeight;
			dstBack += srcDepth;
		}

		if(dstRight > dstWidth || dstBottom > dstHeight || dstBack > dstDepth)
		{
			BS_LOG(Error, Texture, "Destination volume falls outside the destination texture.");
			return;
		}

		copyImpl(target, desc, commandBuffer);
	}

	void Texture::Clear(const Color& value, UINT32 mipLevel, UINT32 face, UINT32 queueIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (face >= mProperties.GetNumFaces())
		{
			BS_LOG(Error, Texture, "Invalid face index.");
			return;
		}

		if (mipLevel > mProperties.GetNumMipmaps())
		{
			BS_LOG(Error, Texture, "Mip level out of range. Valid range is [0, {0}].", mProperties.GetNumMipmaps());
			return;
		}

		clearImpl(value, mipLevel, face, queueIdx);
	}

	void Texture::ClearImpl(const Color& value, UINT32 mipLevel, UINT32 face, UINT32 queueIdx)
	{
		SPtr<PixelData> data = mProperties.AllocBuffer(face, mipLevel);
		data->SetColors(value);
		
		writeData(*data, mipLevel, face, true, queueIdx);
	}

	/************************************************************************/
	/* 								TEXTURE VIEW                      		*/
	/************************************************************************/

	SPtr<TextureView> Texture::CreateView(const TEXTURE_VIEW_DESC& desc)
	{
		return bs_shared_ptr<TextureView>(new (bs_alloc<TextureView>()) TextureView(desc));
	}

	void Texture::ClearBufferViews()
	{
		mTextureViews.Clear();
	}

	SPtr<TextureView> Texture::requestView(UINT32 mostDetailMip, UINT32 numMips, UINT32 firstArraySlice,
										   UINT32 numArraySlices, GpuViewUsage usage)
	{
		THROW_IF_NOT_CORE_THREAD;

		const TextureProperties& texProps = getProperties();

		TEXTURE_VIEW_DESC key;
		key.mostDetailMip = mostDetailMip;
		key.numMips = numMips == 0 ? (texProps.GetNumMipmaps() + 1) : numMips;
		key.firstArraySlice = firstArraySlice;
		key.numArraySlices = numArraySlices == 0 ? texProps.GetNumFaces() : numArraySlices;
		key.usage = usage;

		auto iterFind = mTextureViews.find(key);
		if (iterFind == mTextureViews.end())
		{
			mTextureViews[key] = createView(key);

			iterFind = mTextureViews.find(key);
		}

		return iterFind->second;
	}

	/************************************************************************/
	/* 								STATICS	                      			*/
	/************************************************************************/
	SPtr<Texture> Texture::Create(const TEXTURE_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return TextureManager::Instance().CreateTexture(desc, deviceMask);
	}

	SPtr<Texture> Texture::create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection,
		GpuDeviceFlags deviceMask)
	{
		TEXTURE_DESC desc;
		desc.type = pixelData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
		desc.width = pixelData->GetWidth();
		desc.height = pixelData->GetHeight();
		desc.depth = pixelData->GetDepth();
		desc.format = pixelData->GetFormat();
		desc.usage = usage;
		desc.hwGamma = hwGammaCorrection;

		SPtr<Texture> newTex = TextureManager::instance().CreateTextureInternal(desc, pixelData, deviceMask);
		newTex->Initialize();

		return newTex;
	}
	}
}
