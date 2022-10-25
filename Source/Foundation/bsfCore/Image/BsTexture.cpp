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

using namespace bs;

TEXTURE_COPY_DESC TEXTURE_COPY_DESC::DEFAULT = TEXTURE_COPY_DESC();

TextureProperties::TextureProperties(const TEXTURE_DESC& desc)
	: mDesc(desc)
{
}

bool TextureProperties::HasAlpha() const
{
	return PixelUtil::HasAlpha(mDesc.Format);
}

u32 TextureProperties::GetNumFaces() const
{
	u32 facesPerSlice = GetTextureType() == TEX_TYPE_CUBE_MAP ? 6 : 1;

	return facesPerSlice * mDesc.NumArraySlices;
}

void TextureProperties::MapFromSubresourceIdx(u32 subresourceIdx, u32& face, u32& mip) const
{
	u32 numMipmaps = GetNumMipmaps() + 1;

	face = Math::FloorToInt((subresourceIdx) / (float)numMipmaps);
	mip = subresourceIdx % numMipmaps;
}

u32 TextureProperties::MapToSubresourceIdx(u32 face, u32 mip) const
{
	return face * (GetNumMipmaps() + 1) + mip;
}

SPtr<PixelData> TextureProperties::AllocBuffer(u32 face, u32 mipLevel) const
{
	u32 width = GetWidth();
	u32 height = GetHeight();
	u32 depth = GetDepth();

	for(u32 j = 0; j < mipLevel; j++)
	{
		if(width != 1) width /= 2;
		if(height != 1) height /= 2;
		if(depth != 1) depth /= 2;
	}

	SPtr<PixelData> dst = bs_shared_ptr_new<PixelData>(width, height, depth, GetFormat());
	dst->AllocateInternalBuffer();

	return dst;
}

Texture::Texture(const TEXTURE_DESC& desc)
	: mProperties(desc)
{
}

Texture::Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData)
	: mProperties(desc), mInitData(pixelData)
{
	if(mInitData != nullptr)
		mInitData->LockInternal();
}

void Texture::Initialize()
{
	mSize = CalculateSize();

	// Allocate CPU buffers if needed
	if((mProperties.GetUsage() & TU_CPUCACHED) != 0)
	{
		CreateCpuBuffers();

		if(mInitData != nullptr)
			UpdateCpuBuffers(0, *mInitData);
	}

	Resource::Initialize();
}

SPtr<ct::CoreObject> Texture::CreateCore() const
{
	const TextureProperties& props = GetProperties();

	SPtr<ct::CoreObject> coreObj = ct::TextureManager::Instance().CreateTextureInternal(props.mDesc, mInitData);

	if((mProperties.GetUsage() & TU_CPUCACHED) == 0)
		mInitData = nullptr;

	return coreObj;
}

AsyncOp Texture::WriteData(const SPtr<PixelData>& data, u32 face, u32 mipLevel, bool discardEntireBuffer)
{
	u32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
	UpdateCpuBuffers(subresourceIdx, *data);

	data->LockInternal();

	std::function<void(const SPtr<ct::Texture>&, u32, u32, const SPtr<PixelData>&, bool, AsyncOp&)> func =
		[&](const SPtr<ct::Texture>& texture, u32 _face, u32 _mipLevel, const SPtr<PixelData>& _pixData,
			bool _discardEntireBuffer, AsyncOp& asyncOp)
	{
		texture->WriteData(*_pixData, _mipLevel, _face, _discardEntireBuffer);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperationInternal();
	};

	return gCoreThread().QueueReturnCommand(std::bind(func, GetCore(), face, mipLevel, data, discardEntireBuffer, std::placeholders::_1));
}

AsyncOp Texture::ReadData(const SPtr<PixelData>& data, u32 face, u32 mipLevel)
{
	data->LockInternal();

	std::function<void(const SPtr<ct::Texture>&, u32, u32, const SPtr<PixelData>&, AsyncOp&)> func =
		[&](const SPtr<ct::Texture>& texture, u32 _face, u32 _mipLevel, const SPtr<PixelData>& _pixData,
			AsyncOp& asyncOp)
	{
		// Make sure any queued command start executing before reading
		ct::RenderAPI::Instance().SubmitCommandBuffer(nullptr);

		texture->ReadData(*_pixData, _mipLevel, _face);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperationInternal();
	};

	return gCoreThread().QueueReturnCommand(std::bind(func, GetCore(), face, mipLevel, data, std::placeholders::_1));
}

TAsyncOp<SPtr<PixelData>> Texture::ReadData(u32 face, u32 mipLevel)
{
	TAsyncOp<SPtr<PixelData>> op;

	auto func = [texture = GetCore(), face, mipLevel, op]() mutable
	{
		// Make sure any queued command start executing before reading
		ct::RenderAPI::Instance().SubmitCommandBuffer(nullptr);

		SPtr<PixelData> output = texture->GetProperties().AllocBuffer(face, mipLevel);
		texture->ReadData(*output, mipLevel, face);

		op.CompleteOperationInternal(output);
	};

	gCoreThread().QueueCommand(func);
	return op;
}

u32 Texture::CalculateSize() const
{
	return mProperties.GetNumFaces() * PixelUtil::GetMemorySize(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), mProperties.GetFormat());
}

void Texture::UpdateCpuBuffers(u32 subresourceIdx, const PixelData& pixelData)
{
	if((mProperties.GetUsage() & TU_CPUCACHED) == 0)
		return;

	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		BS_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	u32 mipLevel;
	u32 face;
	mProperties.MapFromSubresourceIdx(subresourceIdx, face, mipLevel);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), mipLevel, mipWidth, mipHeight, mipDepth);

	if(pixelData.GetWidth() != mipWidth || pixelData.GetHeight() != mipHeight ||
	   pixelData.GetDepth() != mipDepth || pixelData.GetFormat() != mProperties.GetFormat())
	{
		BS_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to update this texture.");
		return;
	}

	if(mCPUSubresourceData[subresourceIdx]->GetSize() != pixelData.GetSize())
		BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

	u8* dest = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* src = pixelData.GetData();

	memcpy(dest, src, pixelData.GetSize());
}

void Texture::ReadCachedData(PixelData& dest, u32 face, u32 mipLevel)
{
	if((mProperties.GetUsage() & TU_CPUCACHED) == 0)
	{
		BS_LOG(Error, Texture, "Attempting to read CPU data from a texture that is created without CPU caching.");
		return;
	}

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), mipLevel, mipWidth, mipHeight, mipDepth);

	if(dest.GetWidth() != mipWidth || dest.GetHeight() != mipHeight ||
	   dest.GetDepth() != mipDepth || dest.GetFormat() != mProperties.GetFormat())
	{
		BS_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	u32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		BS_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	if(mCPUSubresourceData[subresourceIdx]->GetSize() != dest.GetSize())
		BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

	u8* srcPtr = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* destPtr = dest.GetData();

	memcpy(destPtr, srcPtr, dest.GetSize());
}

void Texture::CreateCpuBuffers()
{
	u32 numFaces = mProperties.GetNumFaces();
	u32 numMips = mProperties.GetNumMipmaps() + 1;

	u32 numSubresources = numFaces * numMips;
	mCPUSubresourceData.resize(numSubresources);

	for(u32 i = 0; i < numFaces; i++)
	{
		u32 curWidth = mProperties.GetWidth();
		u32 curHeight = mProperties.GetHeight();
		u32 curDepth = mProperties.GetDepth();

		for(u32 j = 0; j < numMips; j++)
		{
			u32 subresourceIdx = mProperties.MapToSubresourceIdx(i, j);

			mCPUSubresourceData[subresourceIdx] = bs_shared_ptr_new<PixelData>(curWidth, curHeight, curDepth, mProperties.GetFormat());
			mCPUSubresourceData[subresourceIdx]->AllocateInternalBuffer();

			if(curWidth > 1)
				curWidth = curWidth / 2;

			if(curHeight > 1)
				curHeight = curHeight / 2;

			if(curDepth > 1)
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

RTTITypeBase* Texture::GetRttiStatic()
{
	return TextureRTTI::Instance();
}

RTTITypeBase* Texture::GetRtti() const
{
	return Texture::GetRttiStatic();
}

/************************************************************************/
/* 								STATICS	                      			*/
/************************************************************************/
HTexture Texture::Create(const TEXTURE_DESC& desc)
{
	SPtr<Texture> texturePtr = CreatePtrInternal(desc);

	return static_resource_cast<Texture>(gResources().CreateResourceHandleInternal(texturePtr));
}

HTexture Texture::Create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
{
	SPtr<Texture> texturePtr = CreatePtrInternal(pixelData, usage, hwGammaCorrection);

	return static_resource_cast<Texture>(gResources().CreateResourceHandleInternal(texturePtr));
}

SPtr<Texture> Texture::CreatePtrInternal(const TEXTURE_DESC& desc)
{
	return TextureManager::Instance().CreateTexture(desc);
}

SPtr<Texture> Texture::CreatePtrInternal(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection)
{
	TEXTURE_DESC desc;
	desc.Type = pixelData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
	desc.Width = pixelData->GetWidth();
	desc.Height = pixelData->GetHeight();
	desc.Depth = pixelData->GetDepth();
	desc.Format = pixelData->GetFormat();
	desc.Usage = usage;
	desc.HwGamma = hwGammaCorrection;

	return TextureManager::Instance().CreateTexture(desc, pixelData);
}

namespace bs { namespace ct
{
SPtr<Texture> Texture::WHITE;
SPtr<Texture> Texture::BLACK;
SPtr<Texture> Texture::NORMAL;

Texture::Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initData, GpuDeviceFlags deviceMask)
	: mProperties(desc), mInitData(initData)
{}

void Texture::Initialize()
{
	if(mInitData != nullptr)
	{
		WriteData(*mInitData, 0, 0, true);
		mInitData->UnlockInternal();
		mInitData = nullptr;
	}

	CoreObject::Initialize();
}

void Texture::WriteData(const PixelData& src, u32 mipLevel, u32 face, bool discardEntireBuffer, u32 queueIdx)
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

	WriteDataImpl(src, mipLevel, face, discardEntireBuffer, queueIdx);
}

void Texture::ReadData(PixelData& dest, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	PixelData& pixelData = static_cast<PixelData&>(dest);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), mipLevel, mipWidth, mipHeight, mipDepth);

	if(pixelData.GetWidth() != mipWidth || pixelData.GetHeight() != mipHeight ||
	   pixelData.GetDepth() != mipDepth || pixelData.GetFormat() != mProperties.GetFormat())
	{
		BS_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	ReadDataImpl(pixelData, mipLevel, face, deviceIdx, queueIdx);
}

PixelData Texture::Lock(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mipLevel > mProperties.GetNumMipmaps())
	{
		BS_LOG(Error, Texture, "Invalid mip level: {0}. Min is 0, max is {1}", mipLevel, mProperties.GetNumMipmaps());
		return PixelData(0, 0, 0, PF_UNKNOWN);
	}

	if(face >= mProperties.GetNumFaces())
	{
		BS_LOG(Error, Texture, "Invalid face index: {0}. Min is 0, max is {1}", face, mProperties.GetNumFaces());
		return PixelData(0, 0, 0, PF_UNKNOWN);
	}

	return LockImpl(options, mipLevel, face, deviceIdx, queueIdx);
}

void Texture::Unlock()
{
	THROW_IF_NOT_CORE_THREAD;

	UnlockImpl();
}

void Texture::Copy(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, const SPtr<CommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD;

	if(target->mProperties.GetTextureType() != mProperties.GetTextureType())
	{
		BS_LOG(Error, Texture, "Source and destination textures must be of same type.");
		return;
	}

	if(mProperties.GetFormat() != target->mProperties.GetFormat()) // Note: It might be okay to use different formats of the same size
	{
		BS_LOG(Error, Texture, "Source and destination texture formats must match.");
		return;
	}

	if(target->mProperties.GetNumSamples() > 1 && mProperties.GetNumSamples() != target->mProperties.GetNumSamples())
	{
		BS_LOG(Error, Texture, "When copying to a multisampled texture, source texture must have the same number of samples.");
		return;
	}

	if(desc.SrcFace >= mProperties.GetNumFaces())
	{
		BS_LOG(Error, Texture, "Invalid source face index.");
		return;
	}

	if(desc.DstFace >= target->mProperties.GetNumFaces())
	{
		BS_LOG(Error, Texture, "Invalid destination face index.");
		return;
	}

	if(desc.SrcMip > mProperties.GetNumMipmaps())
	{
		BS_LOG(Error, Texture, "Source mip level out of range. Valid range is [0, {0}].", mProperties.GetNumMipmaps());
		return;
	}

	if(desc.DstMip > target->mProperties.GetNumMipmaps())
	{
		BS_LOG(Error, Texture, "Destination mip level out of range. Valid range is [0, {0}].", target->mProperties.GetNumMipmaps());
		return;
	}

	u32 srcWidth, srcHeight, srcDepth;
	PixelUtil::GetSizeForMipLevel(
		mProperties.GetWidth(),
		mProperties.GetHeight(),
		mProperties.GetDepth(),
		desc.SrcMip,
		srcWidth,
		srcHeight,
		srcDepth);

	u32 dstWidth, dstHeight, dstDepth;
	PixelUtil::GetSizeForMipLevel(
		target->mProperties.GetWidth(),
		target->mProperties.GetHeight(),
		target->mProperties.GetDepth(),
		desc.DstMip,
		dstWidth,
		dstHeight,
		dstDepth);

	if(desc.DstPosition.X < 0 || desc.DstPosition.X >= (i32)dstWidth ||
	   desc.DstPosition.Y < 0 || desc.DstPosition.Y >= (i32)dstHeight ||
	   desc.DstPosition.Z < 0 || desc.DstPosition.Z >= (i32)dstDepth)
	{
		BS_LOG(Error, Texture, "Destination position falls outside the destination texture.");
		return;
	}

	bool entireSurface = desc.SrcVolume.GetWidth() == 0 ||
		desc.SrcVolume.GetHeight() == 0 ||
		desc.SrcVolume.GetDepth() == 0;

	u32 dstRight = (u32)desc.DstPosition.X;
	u32 dstBottom = (u32)desc.DstPosition.Y;
	u32 dstBack = (u32)desc.DstPosition.Z;
	if(!entireSurface)
	{
		if(desc.SrcVolume.Left >= srcWidth || desc.SrcVolume.Right > srcWidth ||
		   desc.SrcVolume.Top >= srcHeight || desc.SrcVolume.Bottom > srcHeight ||
		   desc.SrcVolume.Front >= srcDepth || desc.SrcVolume.Back > srcDepth)
		{
			BS_LOG(Error, Texture, "Source volume falls outside the source texture.");
			return;
		}

		dstRight += desc.SrcVolume.GetWidth();
		dstBottom += desc.SrcVolume.GetHeight();
		dstBack += desc.SrcVolume.GetDepth();
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

	CopyImpl(target, desc, commandBuffer);
}

void Texture::Clear(const Color& value, u32 mipLevel, u32 face, u32 queueIdx)
{
	THROW_IF_NOT_CORE_THREAD;

	if(face >= mProperties.GetNumFaces())
	{
		BS_LOG(Error, Texture, "Invalid face index.");
		return;
	}

	if(mipLevel > mProperties.GetNumMipmaps())
	{
		BS_LOG(Error, Texture, "Mip level out of range. Valid range is [0, {0}].", mProperties.GetNumMipmaps());
		return;
	}

	ClearImpl(value, mipLevel, face, queueIdx);
}

void Texture::ClearImpl(const Color& value, u32 mipLevel, u32 face, u32 queueIdx)
{
	SPtr<PixelData> data = mProperties.AllocBuffer(face, mipLevel);
	data->SetColors(value);

	WriteData(*data, mipLevel, face, true, queueIdx);
}

/************************************************************************/
/* 								TEXTURE VIEW                      		*/
/************************************************************************/

SPtr<TextureView> Texture::CreateView(const TEXTURE_VIEW_DESC& desc)
{
	return bs_shared_ptr<TextureView>(new(bs_alloc<TextureView>()) TextureView(desc));
}

void Texture::ClearBufferViews()
{
	mTextureViews.clear();
}

SPtr<TextureView> Texture::RequestView(u32 mostDetailMip, u32 numMips, u32 firstArraySlice, u32 numArraySlices, GpuViewUsage usage)
{
	THROW_IF_NOT_CORE_THREAD;

	const TextureProperties& texProps = GetProperties();

	TEXTURE_VIEW_DESC key;
	key.MostDetailMip = mostDetailMip;
	key.NumMips = numMips == 0 ? (texProps.GetNumMipmaps() + 1) : numMips;
	key.FirstArraySlice = firstArraySlice;
	key.NumArraySlices = numArraySlices == 0 ? texProps.GetNumFaces() : numArraySlices;
	key.Usage = usage;

	auto iterFind = mTextureViews.find(key);
	if(iterFind == mTextureViews.end())
	{
		mTextureViews[key] = CreateView(key);

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

SPtr<Texture> Texture::Create(const SPtr<PixelData>& pixelData, int usage, bool hwGammaCorrection, GpuDeviceFlags deviceMask)
{
	TEXTURE_DESC desc;
	desc.Type = pixelData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
	desc.Width = pixelData->GetWidth();
	desc.Height = pixelData->GetHeight();
	desc.Depth = pixelData->GetDepth();
	desc.Format = pixelData->GetFormat();
	desc.Usage = usage;
	desc.HwGamma = hwGammaCorrection;

	SPtr<Texture> newTex = TextureManager::Instance().CreateTextureInternal(desc, pixelData, deviceMask);
	newTex->Initialize();

	return newTex;
}
}}
