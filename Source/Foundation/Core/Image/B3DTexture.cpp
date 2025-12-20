//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DTexture.h"

#include "B3DApplication.h"
#include "RTTI/B3DTextureRTTI.h"
#include "FileSystem/B3DDataStream.h"
#include "Error/B3DException.h"
#include "Debug/B3DDebug.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"
#include "Resources/B3DResources.h"
#include "Image/B3DPixelUtility.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace b3d;

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
	return PixelUtility::HasAlpha(Format);
}

u32 TextureProperties::GetFaceCount() const
{
	u32 facesPerSlice = Type == TEX_TYPE_CUBE_MAP ? 6 : 1;

	return facesPerSlice * ArraySliceCount;
}

void TextureProperties::MapFromSubresourceIdx(u32 subresourceIdx, u32& face, u32& mip) const
{
	u32 mipmapCount = MipMapCount + 1;

	face = Math::FloorToInt((subresourceIdx) / (float)mipmapCount);
	mip = subresourceIdx % mipmapCount;
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

	for(u32 mipIndex = 0; mipIndex < mipLevel; mipIndex++)
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
	: Texture(createInformation, createInformation.InitialData)
{
}

void Texture::Initialize()
{
	// Allocate CPU buffers if needed
	if((mProperties.Usage & TU_CPUCACHED) != 0)
	{
		CreateCpuBuffers();

		if(mInitData != nullptr)
			UpdateCpuBuffers(0, *mInitData);
	}

	Resource::Initialize();
}

SPtr<render::RenderProxy> Texture::CreateRenderProxy() const
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	TextureCreateInformation createInformation = mProperties;
	createInformation.InitialData = mInitData;
	mInitData = nullptr;

	return gpuDevice->CreateTexture(createInformation, true);
}

TAsyncOp<void> Texture::WriteData(const SPtr<PixelData>& data, u32 face, u32 mipLevel, bool discardEntireBuffer)
{
	u32 subresourceIdx = mProperties.MapToSubresourceIdx(face, mipLevel);
	UpdateCpuBuffers(subresourceIdx, *data);

	data->LockInternal();

	auto fnWriteData = [](const SPtr<render::Texture>& texture, u32 _face, u32 _mipLevel, const SPtr<PixelData>& _pixData,
		bool _discardEntireBuffer, TAsyncOp<void>& asyncOp)
	{
		render::TextureWriteFlags flags = _discardEntireBuffer ? render::TextureWriteFlag::Discard : render::TextureWriteFlag::Normal;
		render::TextureUtility::Write(texture, *_pixData, _mipLevel, _face, flags);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnWriteData = std::move(fnWriteData), renderProxy = B3DGetRenderProxy(this), face, mipLevel, data, discardEntireBuffer, asyncOp]() mutable { fnWriteData(renderProxy, face, mipLevel, data, discardEntireBuffer, asyncOp); }, "Texture::WriteData", false, GetName());

	return asyncOp;
}

TAsyncOp<void> Texture::ReadData(const SPtr<PixelData>& data, u32 face, u32 mipLevel)
{
	data->LockInternal();

	std::function<void(const SPtr<render::Texture>&, u32, u32, const SPtr<PixelData>&, TAsyncOp<void>&)> fnReadData =
		[&](const SPtr<render::Texture>& texture, u32 _face, u32 _mipLevel, const SPtr<PixelData>& _pixData,
			TAsyncOp<void>& asyncOp)
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		texture->ReadData(*_pixData, _mipLevel, _face);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnReadData = std::move(fnReadData), renderProxy = B3DGetRenderProxy(this), face, mipLevel, data, asyncOp]() mutable { fnReadData(renderProxy, face, mipLevel, data, asyncOp); }, "Texture::ReadData", false, GetName());

	return asyncOp;
}

TAsyncOp<SPtr<PixelData>> Texture::ReadData(u32 face, u32 mipLevel)
{
	TAsyncOp<SPtr<PixelData>> op;

	auto fnReadDataAsync = [texture = B3DGetRenderProxy(this), face, mipLevel, op]() mutable
	{
		// TODO - Transfer buffers should be handled by the Renderer
		const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(gpuDevice != nullptr)
			gpuDevice->SubmitTransferCommandBuffers();

		SPtr<PixelData> output = texture->GetProperties().AllocBuffer(face, mipLevel);
		texture->ReadData(*output, mipLevel, face);

		op.CompleteOperation(output);
	};

	GetRenderThread().PostCommand(fnReadDataAsync, "Texture::ReadData", false, GetName());
	return op;
}

u32 Texture::CalculateSize() const
{
	return mProperties.GetFaceCount() * PixelUtility::GetMemorySize(mProperties.Width, mProperties.Height, mProperties.Depth, mProperties.Format);
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
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

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
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

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

	u8* sourcePointer = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* destinationPointer = dest.GetData();

	memcpy(destinationPointer, sourcePointer, dest.GetSize());
}

void Texture::CreateCpuBuffers()
{
	u32 numFaces = mProperties.GetFaceCount();
	u32 numMips = mProperties.MipMapCount + 1;

	u32 numSubresources = numFaces * numMips;
	mCPUSubresourceData.resize(numSubresources);

	for(u32 faceIndex = 0; faceIndex < numFaces; faceIndex++)
	{
		u32 curWidth = mProperties.Width;
		u32 curHeight = mProperties.Height;
		u32 curDepth = mProperties.Depth;

		for(u32 mipIndex = 0; mipIndex < numMips; mipIndex++)
		{
			u32 subresourceIdx = mProperties.MapToSubresourceIdx(faceIndex, mipIndex);

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

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* Texture::GetRttiStatic()
{
	return TextureRTTI::Instance();
}

RTTIType* Texture::GetRtti() const
{
	return Texture::GetRttiStatic();
}

/************************************************************************/
/* 								STATICS	                      			*/
/************************************************************************/
HTexture Texture::Create(const TextureCreateInformation& createInformation)
{
	SPtr<Texture> texture = CreateShared(createInformation);

	return B3DStaticResourceCast<Texture>(GetResources().CreateResourceHandle(texture));
}

SPtr<Texture> Texture::CreateShared(const TextureCreateInformation& createInformation)
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture(createInformation);
	SPtr<Texture> shared = B3DMakeSharedFromExisting<Texture>(texture);

	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

SPtr<Texture> Texture::CreateEmpty()
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture();
	SPtr<Texture> shared = B3DMakeSharedFromExisting<Texture>(texture);

	shared->SetShared(shared);
	return shared;
}

namespace b3d { namespace render
{
SPtr<Texture> Texture::kWhite;
SPtr<Texture> Texture::kBlack;
SPtr<Texture> Texture::kPink;
SPtr<Texture> Texture::kNormal;

Texture::Texture(const TextureCreateInformation& createInformation)
	: mProperties(createInformation), mInitData(createInformation.InitialData), mName(createInformation.Name)
{}

void Texture::Initialize()
{
	if(mInitData != nullptr)
	{
		TextureUtility::Write(std::static_pointer_cast<Texture>(GetShared()), *mInitData, 0, 0, TextureWriteFlag::Discard);
		mInitData->UnlockInternal();
		mInitData = nullptr;
	}

	RenderProxy::Initialize();
}

void Texture::ReadData(PixelData& destination, u32 mipLevel, u32 face, const SPtr<GpuQueue>& gpuQueue)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	PixelData& pixelData = static_cast<PixelData&>(destination);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

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
	ASSERT_IF_NOT_RENDER_THREAD;

	TextureViewInformation key;
	key.Surface = surface;
	key.Usage = usage;

	auto found = mTextureViews.find(key);
	if(found == mTextureViews.end())
	{
		mTextureViews[key] = CreateView(key);

		found = mTextureViews.find(key);
	}

	return found->second;
}

SPtr<GpuBuffer> TextureUtility::CreateStagingBuffer(const SPtr<Texture>& texture, u32 mipLevel, bool readable)
{
	B3D_ASSERT(texture != nullptr);

	const TextureProperties& properties = texture->GetProperties();

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(properties.Width, properties.Height, properties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	PixelData pixelData(mipWidth, mipHeight, mipDepth, texture->GetProperties().Format);
	return CreateStagingBuffer(texture, pixelData, readable);
}

SPtr<GpuBuffer> TextureUtility::CreateStagingBuffer(const SPtr<Texture>& texture, const PixelData& pixelData, bool readable)
{
	GpuBufferCreateInformation createInformation;
	createInformation.Type = readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite;
	createInformation.Staging.Size = pixelData.GetSize();

	return texture->GetGpuDevice().CreateGpuBuffer(createInformation);
}

void TextureUtility::Write(const SPtr<Texture>& texture, const PixelData& source, u32 mipLevel, u32 arrayLayer, TextureWriteFlags flags, SPtr<GpuCommandBuffer> commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD
	B3D_ASSERT(texture != nullptr);

	if(source.GetSize() == 0)
		return;

	const TextureProperties& textureProperties = texture->GetProperties();
	if(textureProperties.SampleCount > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be written to from the CPU.");
		return;
	}

	mipLevel = Math::Clamp(mipLevel, 0u, textureProperties.MipMapCount);
	arrayLayer = Math::Clamp(arrayLayer, 0u, textureProperties.GetFaceCount() - 1);

	if(arrayLayer > 0 && textureProperties.Type == TEX_TYPE_3D)
	{
		B3D_LOG(Error, RenderBackend, "3D texture arrays are not supported.");
		return;
	}

	const bool canDiscardContents = flags.IsSet(TextureWriteFlag::Discard);
	const bool noOverwrite = flags.IsSet(TextureWriteFlag::NoOverwrite);

	GpuMapOptions mapOptions = GpuMapOption::Write;
	if(noOverwrite)
		mapOptions |= GpuMapOption::NoOverwrite;

	// Check is the GPU currently reading or writing from the image
	const GpuQueueMask subresourceUseMask = texture->GetUseMask(mipLevel, arrayLayer, GpuAccessFlag::Read | GpuAccessFlag::Write);
	const u32 subresourceUseCount = texture->GetUseCount(mipLevel, arrayLayer);
	const u32 subresourceBoundCount = texture->GetBoundCount(mipLevel, arrayLayer);

	// Try direct mapping if texture supports it
	void* mappedMemory = texture->GetMappedMemory();
	if(mappedMemory != nullptr)
	{
		const bool isUsedOnGPU = !subresourceUseMask.IsEmpty();
		const bool isBound = subresourceBoundCount > 0;

		// Recreate the internal image if it is bound to a command buffer, to avoid overwriting the old data. But only if the user
		// allows discard via a flag. In case the user provided an explicit command buffer, perfer a staging buffer over discard
		// (it still costs us creation of a new buffer, and the original buffer bindings remain valid). Finally, if no-overwrite is
		// specified, we never recreate the buffer as the user guarantees he won't touch the previously bound region.
		const bool recreateImage = isBound && commandBuffer == nullptr && canDiscardContents && !noOverwrite;

		// Even if the texture is directly mappable we might wish to avoid mapping it directly in these situations:
		const bool shouldMapDirectly =
			(!isUsedOnGPU // GPU is currently using the texture
			&& (!isBound || recreateImage)) // Image is bound to a command buffer already, and we're not creating a new one. Cannot map without affecting the previous binding
			|| noOverwrite; // If no-overwrite flag is set, user guarantees he won't touch the memory the GPU is using

		if(shouldMapDirectly)
		{
			if(recreateImage)
				texture->RecreateInternalTexture();
			
			GpuTextureMappedScope scope = texture->Map(mipLevel, arrayLayer, mapOptions);
			PixelUtility::BulkPixelConversion(source, scope.GetPixelData());

			return;
		}
	}

	// Fall back to staging buffer approach
	GpuDevice& device = texture->GetGpuDevice();

	// Create staging buffer
	const u32 mipWidth = Math::Max(1u, textureProperties.Width >> mipLevel);
	const u32 mipHeight = Math::Max(1u, textureProperties.Height >> mipLevel);
	const u32 mipDepth = Math::Max(1u, textureProperties.Depth >> mipLevel);

	PixelData lockedArea(mipWidth, mipHeight, mipDepth, textureProperties.Format);
	SPtr<GpuBuffer> stagingBuffer = CreateStagingBuffer(texture, lockedArea, false);

	if(!B3D_ENSURE(stagingBuffer != nullptr))
		return;

	// Copy data to staging buffer
	{
		GpuBufferMappedScope bufferScope = stagingBuffer->Map(GpuMapOption::Write);
		lockedArea.SetExternalBuffer(static_cast<u8*>(bufferScope.GetMappedMemory()));
		PixelUtility::BulkPixelConversion(source, lockedArea);
	}

	// If the image is used in any way on the GPU, we need to wait for that use to finish before we issue our copy
	GpuQueueMask syncMask;
	if(!subresourceUseMask.IsEmpty() && mapOptions.IsSet(GpuMapOption::NoOverwrite)) // Buffer is currently used on the GPU
		syncMask = subresourceUseMask;

	// Check if the image will still be bound somewhere after the command buffers using it finish. If it is, we have to recreate the internal image otherwise the copy
	// operation might just get overwritten by those command buffers when the execute. This is because the transfer command buffers are always submitted before regular
	// command buffers. If user provided an explicit command buffer, then it's up to him to ensure the correct ordering.
	const bool isBoundWithoutUse = subresourceBoundCount > subresourceUseCount;
	if(isBoundWithoutUse && commandBuffer == nullptr)
	{
		if(!canDiscardContents)
		{
			B3D_LOG(Warning, RenderBackend, "Writing to a image '{0}' that is currently bound on a command buffer, without providing an explicit command buffer. Such writes will be queued on the transfer buffer which is submitted before any user command buffers. This means multiple writes will overwrite it each other if not careful.", texture->GetName());
		}
		else
			texture->RecreateInternalTexture();
	}

	// Get or create command buffer
	if(commandBuffer == nullptr)
	{
		const SPtr<GpuQueue>& transferQueue = device.GetQueue(GQT_GRAPHICS, 0);
		commandBuffer = transferQueue->GetOrCreateTransferCommandBuffer();
	}

	// Issue copy command
	commandBuffer->CopyBufferToTexture(stagingBuffer, texture, 0, mipLevel, arrayLayer);
	commandBuffer->AddQueueSyncMask(syncMask);
}

void TextureUtility::Clear(const SPtr<Texture>& texture, const Color& value, u32 mipLevel, u32 arrayLayer, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD

	const TextureProperties& textureProperties = texture->GetProperties();
	if(arrayLayer >= textureProperties.GetFaceCount())
	{
		B3D_LOG(Error, Texture, "Invalid array index.");
		return;
	}

	if(mipLevel > textureProperties.MipMapCount)
	{
		B3D_LOG(Error, Texture, "Mip level out of range. Valid range is [0, {0}].", textureProperties.MipMapCount);
		return;
	}

	SPtr<PixelData> data = textureProperties.AllocBuffer(arrayLayer, mipLevel);
	data->SetColors(value);

	Write(texture, *data, mipLevel, arrayLayer, TextureWriteFlag::Discard, commandBuffer);
}
}}
