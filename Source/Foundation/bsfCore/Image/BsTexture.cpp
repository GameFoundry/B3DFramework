//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsTexture.h"

#include "BsCoreApplication.h"
#include "Private/RTTI/BsTextureRTTI.h"
#include "FileSystem/BsDataStream.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "CoreThread/BsCoreThread.h"
#include "Threading/BsAsyncOp.h"
#include "Resources/BsResources.h"
#include "Image/BsPixelUtil.h"
#include "RenderAPI/BsGpuCommandBuffer.h"

using namespace bs;

TextureCreateInformation::TextureCreateInformation(const SPtr<PixelData>& initialData)
	:InitialData(initialData)
{
	if(initialData != nullptr)
	{
		Type = initialData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
		Width = initialData->GetWidth();
		Height = initialData->GetHeight();
		Depth = initialData->GetDepth();
		Format = initialData->GetFormat();
	}
}

const TextureCopyInformation TextureCopyInformation::kDefault = TextureCopyInformation();
const TextureBlitInformation TextureBlitInformation::kDefault = TextureBlitInformation();

TextureProperties::TextureProperties(const TextureCreateInformation& createInformation)
	:TextureInformation(createInformation)
{
}

bool TextureProperties::HasAlpha() const
{
	return PixelUtil::HasAlpha(Format);
}

u32 TextureProperties::GetFaceCount() const
{
	u32 facesPerSlice = Type == TEX_TYPE_CUBE_MAP ? 6 : 1;

	return facesPerSlice * ArraySliceCount;
}

void TextureProperties::MapFromSubresourceIdx(u32 subresourceIdx, u32& face, u32& mip) const
{
	u32 numMipmaps = MipMapCount + 1;

	face = Math::FloorToInt((subresourceIdx) / (float)numMipmaps);
	mip = subresourceIdx % numMipmaps;
}

u32 TextureProperties::MapToSubresourceIdx(u32 face, u32 mip) const
{
	return face * (MipMapCount + 1) + mip;
}

SPtr<PixelData> TextureProperties::AllocBuffer(u32 face, u32 mipLevel) const
{
	u32 width = Width;
	u32 height = Height;
	u32 depth = Depth;

	for(u32 j = 0; j < mipLevel; j++)
	{
		if(width != 1) width /= 2;
		if(height != 1) height /= 2;
		if(depth != 1) depth /= 2;
	}

	SPtr<PixelData> dst = B3DMakeShared<PixelData>(width, height, depth, Format);
	dst->AllocateInternalBuffer();

	return dst;
}

Texture::Texture(const TextureCreateInformation& createInformation, const SPtr<PixelData>& pixelData)
	: Resource(true, createInformation.Name), mProperties(createInformation), mInitData(pixelData)
{
	if(mInitData != nullptr)
		mInitData->LockInternal();
}

Texture::Texture(const TextureCreateInformation& createInformation)
	: Texture(createInformation, nullptr)
{
}

void Texture::Initialize()
{
	mSize = CalculateSize();

	// Allocate CPU buffers if needed
	if((mProperties.Usage & TU_CPUCACHED) != 0)
	{
		CreateCpuBuffers();

		if(mInitData != nullptr)
			UpdateCpuBuffers(0, *mInitData);
	}

	Resource::Initialize();
}

SPtr<ct::CoreObject> Texture::CreateCore() const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	TextureCreateInformation createInformation = mProperties;
	if((mProperties.Usage & TU_CPUCACHED) != 0)
		createInformation.InitialData = mInitData;

	return gpuDevice->CreateTexture(createInformation, true);
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
		asyncOp.CompleteOperation();
	};

	return GetCoreThread().QueueReturnCommand(std::bind(func, GetCore(), face, mipLevel, data, discardEntireBuffer, std::placeholders::_1));
}

AsyncOp Texture::ReadData(const SPtr<PixelData>& data, u32 face, u32 mipLevel)
{
	data->LockInternal();

	std::function<void(const SPtr<ct::Texture>&, u32, u32, const SPtr<PixelData>&, AsyncOp&)> func =
		[&](const SPtr<ct::Texture>& texture, u32 _face, u32 _mipLevel, const SPtr<PixelData>& _pixData,
			AsyncOp& asyncOp)
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		texture->ReadData(*_pixData, _mipLevel, _face);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	return GetCoreThread().QueueReturnCommand(std::bind(func, GetCore(), face, mipLevel, data, std::placeholders::_1));
}

TAsyncOp<SPtr<PixelData>> Texture::ReadData(u32 face, u32 mipLevel)
{
	TAsyncOp<SPtr<PixelData>> op;

	auto func = [texture = GetCore(), face, mipLevel, op]() mutable
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		SPtr<PixelData> output = texture->GetProperties().AllocBuffer(face, mipLevel);
		texture->ReadData(*output, mipLevel, face);

		op.CompleteOperation(output);
	};

	GetCoreThread().QueueCommand(func);
	return op;
}

u32 Texture::CalculateSize() const
{
	return mProperties.GetFaceCount() * PixelUtil::GetMemorySize(mProperties.Width, mProperties.Height, mProperties.Depth, mProperties.Format);
}

void Texture::UpdateCpuBuffers(u32 subresourceIdx, const PixelData& pixelData)
{
	if((mProperties.Usage & TU_CPUCACHED) == 0)
		return;

	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		B3D_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	u32 mipLevel;
	u32 face;
	mProperties.MapFromSubresourceIdx(subresourceIdx, face, mipLevel);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(pixelData.GetWidth() != mipWidth || pixelData.GetHeight() != mipHeight ||
	   pixelData.GetDepth() != mipDepth || pixelData.GetFormat() != mProperties.Format)
	{
		B3D_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to update this texture.");
		return;
	}

	if(mCPUSubresourceData[subresourceIdx]->GetSize() != pixelData.GetSize())
		B3D_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

	u8* dest = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* src = pixelData.GetData();

	memcpy(dest, src, pixelData.GetSize());
}

void Texture::ReadCachedData(PixelData& dest, u32 face, u32 mipLevel)
{
	if((mProperties.Usage & TU_CPUCACHED) == 0)
	{
		B3D_LOG(Error, Texture, "Attempting to read CPU data from a texture that is created without CPU caching.");
		return;
	}

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(dest.GetWidth() != mipWidth || dest.GetHeight() != mipHeight ||
	   dest.GetDepth() != mipDepth || dest.GetFormat() != mProperties.Format)
	{
		B3D_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	u32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		B3D_LOG(Error, Texture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	if(mCPUSubresourceData[subresourceIdx]->GetSize() != dest.GetSize())
		B3D_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

	u8* srcPtr = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* destPtr = dest.GetData();

	memcpy(destPtr, srcPtr, dest.GetSize());
}

void Texture::CreateCpuBuffers()
{
	u32 numFaces = mProperties.GetFaceCount();
	u32 numMips = mProperties.MipMapCount + 1;

	u32 numSubresources = numFaces * numMips;
	mCPUSubresourceData.resize(numSubresources);

	for(u32 i = 0; i < numFaces; i++)
	{
		u32 curWidth = mProperties.Width;
		u32 curHeight = mProperties.Height;
		u32 curDepth = mProperties.Depth;

		for(u32 j = 0; j < numMips; j++)
		{
			u32 subresourceIdx = mProperties.MapToSubresourceIdx(i, j);

			mCPUSubresourceData[subresourceIdx] = B3DMakeShared<PixelData>(curWidth, curHeight, curDepth, mProperties.Format);
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
HTexture Texture::Create(const TextureCreateInformation& createInformation)
{
	SPtr<Texture> texture = CreateShared(createInformation);

	return B3DStaticResourceCast<Texture>(GetResources().CreateResourceHandleInternal(texture));
}

SPtr<Texture> Texture::CreateShared(const TextureCreateInformation& createInformation)
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture(createInformation);
	SPtr<Texture> shared = B3DMakeCoreFromExisting<Texture>(texture);

	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

SPtr<Texture> Texture::CreateEmpty()
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture();
	SPtr<Texture> shared = B3DMakeCoreFromExisting<Texture>(texture);

	shared->SetShared(shared);
	return shared;
}

namespace bs { namespace ct
{
SPtr<Texture> Texture::kWhite;
SPtr<Texture> Texture::kBlack;
SPtr<Texture> Texture::kNormal;

Texture::Texture(const TextureCreateInformation& createInformation)
	: mProperties(createInformation), mInitData(createInformation.InitialData), mName(createInformation.Name)
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

void Texture::WriteData(const PixelData& source, u32 mipLevel, u32 face, bool discardEntireBuffer, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD;

	if(discardEntireBuffer)
	{
		if((mProperties.Usage & TU_DYNAMIC) == 0)
		{
			// Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.
			discardEntireBuffer = false;
		}
	}

	WriteDataInternal(source, mipLevel, face, discardEntireBuffer, commandBuffer);
}

void Texture::ReadData(PixelData& destination, u32 mipLevel, u32 face, const SPtr<GpuQueue>& gpuQueue)
{
	THROW_IF_NOT_CORE_THREAD;

	PixelData& pixelData = static_cast<PixelData&>(destination);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtil::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(pixelData.GetWidth() != mipWidth || pixelData.GetHeight() != mipHeight ||
	   pixelData.GetDepth() != mipDepth || pixelData.GetFormat() != mProperties.Format)
	{
		B3D_LOG(Error, Texture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	ReadDataInternal(pixelData, mipLevel, face, gpuQueue);
}

TAsyncOp<SPtr<PixelData>> Texture::ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel, u32 face)
{
	SPtr<PixelData> pixelData = GetProperties().AllocBuffer(face, mipLevel);

	// We fall-back to sync read if the backend doesn't implement an async method
	ReadData(*pixelData, mipLevel, face);

	TAsyncOp<SPtr<PixelData>> output;
	output.CompleteOperation(pixelData);

	return output;
}

PixelData Texture::Lock(GpuLockOptions options, u32 mipLevel, u32 face)
{
	THROW_IF_NOT_CORE_THREAD;

	if(mipLevel > mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Invalid mip level: {0}. Min is 0, max is {1}", mipLevel, mProperties.MipMapCount);
		return PixelData(0, 0, 0, PF_UNKNOWN);
	}

	if(face >= mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid face index: {0}. Min is 0, max is {1}", face, mProperties.GetFaceCount());
		return PixelData(0, 0, 0, PF_UNKNOWN);
	}

	return LockInternal(options, mipLevel, face);
}

void Texture::Unlock()
{
	THROW_IF_NOT_CORE_THREAD;

	UnlockInternal();
}

void Texture::Copy(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureCopyInformation& copyInformation)
{
	THROW_IF_NOT_CORE_THREAD;

	if (copyInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, Texture, "Copy operation failed. Face count is zero.");
		return;
	}

	if(target->mProperties.Type != mProperties.Type)
	{
		B3D_LOG(Error, Texture, "Source and destination textures must be of same type.");
		return;
	}

	if(mProperties.Format != target->mProperties.Format) // Note: It might be okay to use different formats of the same size
	{
		B3D_LOG(Error, Texture, "Source and destination texture formats must match.");
		return;
	}

	if(target->mProperties.SampleCount > 1 && mProperties.SampleCount != target->mProperties.SampleCount)
	{
		B3D_LOG(Error, Texture, "When copying to a multisampled texture, source texture must have the same number of samples.");
		return;
	}

	if((copyInformation.SourceFace + copyInformation.FaceCount) > mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid source face index.");
		return;
	}

	if((copyInformation.DestinationFace + copyInformation.FaceCount) > target->mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid destination face index.");
		return;
	}

	if(copyInformation.SourceMip > mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Source mip level out of range. Valid range is [0, {0}].", mProperties.MipMapCount);
		return;
	}

	if(copyInformation.DestinationMip > target->mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Destination mip level out of range. Valid range is [0, {0}].", target->mProperties.MipMapCount);
		return;
	}

	u32 srcWidth, srcHeight, srcDepth;
	PixelUtil::GetSizeForMipLevel(
		mProperties.Width,
		mProperties.Height,
		mProperties.Depth,
		copyInformation.SourceMip,
		srcWidth,
		srcHeight,
		srcDepth);

	u32 dstWidth, dstHeight, dstDepth;
	PixelUtil::GetSizeForMipLevel(
		target->mProperties.Width,
		target->mProperties.Height,
		target->mProperties.Depth,
		copyInformation.DestinationMip,
		dstWidth,
		dstHeight,
		dstDepth);

	if(copyInformation.DestinationPosition.X < 0 || copyInformation.DestinationPosition.X >= (i32)dstWidth ||
	   copyInformation.DestinationPosition.Y < 0 || copyInformation.DestinationPosition.Y >= (i32)dstHeight ||
	   copyInformation.DestinationPosition.Z < 0 || copyInformation.DestinationPosition.Z >= (i32)dstDepth)
	{
		B3D_LOG(Error, Texture, "Destination position falls outside the destination texture.");
		return;
	}

	bool entireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	u32 dstRight = (u32)copyInformation.DestinationPosition.X;
	u32 dstBottom = (u32)copyInformation.DestinationPosition.Y;
	u32 dstBack = (u32)copyInformation.DestinationPosition.Z;
	if(!entireSurface)
	{
		if(copyInformation.SourceVolume.Left >= srcWidth || copyInformation.SourceVolume.Right > srcWidth ||
		   copyInformation.SourceVolume.Top >= srcHeight || copyInformation.SourceVolume.Bottom > srcHeight ||
		   copyInformation.SourceVolume.Front >= srcDepth || copyInformation.SourceVolume.Back > srcDepth)
		{
			B3D_LOG(Error, Texture, "Source volume falls outside the source texture.");
			return;
		}

		dstRight += copyInformation.SourceVolume.GetWidth();
		dstBottom += copyInformation.SourceVolume.GetHeight();
		dstBack += copyInformation.SourceVolume.GetDepth();
	}
	else
	{
		dstRight += srcWidth;
		dstBottom += srcHeight;
		dstBack += srcDepth;
	}

	if(dstRight > dstWidth || dstBottom > dstHeight || dstBack > dstDepth)
	{
		B3D_LOG(Error, Texture, "Destination volume falls outside the destination texture.");
		return;
	}

	CopyInternal(commandBuffer, target, copyInformation);
}

void Texture::Blit(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureBlitInformation& blitInformation)
{
	THROW_IF_NOT_CORE_THREAD;

	if (blitInformation.FaceCount == 0)
	{
		B3D_LOG(Warning, Texture, "Blit operation failed. Face count is zero.");
		return;
	}

	if((blitInformation.SourceFace + blitInformation.FaceCount) > mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Source face out of valid range.");
		return;
	}

	if((blitInformation.DestinationFace + blitInformation.FaceCount) > target->mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Destination face out of valid range.");
		return;
	}

	if(blitInformation.SourceMip > mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Source mip level out of valid range. Valid range is [0, {0}].", mProperties.MipMapCount);
		return;
	}

	if(blitInformation.DestinationMip > target->mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Blit operation failed. Destination mip level out of range. Valid range is [0, {0}].", target->mProperties.MipMapCount);
		return;
	}

	BlitInternal(commandBuffer, target, blitInformation);
}

void Texture::Clear(const Color& value, u32 mipLevel, u32 face, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	THROW_IF_NOT_CORE_THREAD;

	if(face >= mProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid face index.");
		return;
	}

	if(mipLevel > mProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Mip level out of range. Valid range is [0, {0}].", mProperties.MipMapCount);
		return;
	}

	ClearInternal(value, mipLevel, face, commandBuffer);
}

void Texture::ClearInternal(const Color& value, u32 mipLevel, u32 face, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	SPtr<PixelData> data = mProperties.AllocBuffer(face, mipLevel);
	data->SetColors(value);

	WriteData(*data, mipLevel, face, true, commandBuffer);
}

/************************************************************************/
/* 								TEXTURE VIEW                      		*/
/************************************************************************/

SPtr<TextureView> Texture::CreateView(const TextureViewInformation& desc)
{
	return B3DMakeSharedFromExisting<TextureView>(new(B3DAllocate<TextureView>()) TextureView(desc));
}

void Texture::ClearBufferViews()
{
	mTextureViews.clear();
}

SPtr<TextureView> Texture::RequestView(const TextureSurface& surface, GpuViewUsage usage)
{
	THROW_IF_NOT_CORE_THREAD;

	TextureViewInformation key;
	key.Surface = surface;
	key.Usage = usage;

	auto iterFind = mTextureViews.find(key);
	if(iterFind == mTextureViews.end())
	{
		mTextureViews[key] = CreateView(key);

		iterFind = mTextureViews.find(key);
	}

	return iterFind->second;
}
}}
