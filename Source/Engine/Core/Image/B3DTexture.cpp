//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DTexture.h"

#include "B3DApplication.h"
#include "RTTI/B3DTextureRTTI.h"
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"
#include "Resources/B3DResources.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Renderer/B3DRenderer.h"

using namespace b3d;

TextureCreateInformation::TextureCreateInformation(const TShared<PixelData>& initialData)
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

TextureCreateInformation TextureCreateInformation::CreateFromPixelData(const TShared<PixelData>& pixelData)
{
	TextureCreateInformation createInformation;
	createInformation.Type = pixelData->GetDepth() > 1 ? TEX_TYPE_3D : TEX_TYPE_2D;
	createInformation.Width = pixelData->GetWidth();
	createInformation.Height = pixelData->GetHeight();
	createInformation.Depth = pixelData->GetDepth();
	createInformation.Format = pixelData->GetFormat();
	createInformation.InitialData = pixelData;

	return createInformation;
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

void TextureProperties::MapFromSubresourceIndex(u32 subresourceIndex, u32& outFace, u32& outMip) const
{
	u32 mipmapCount = MipMapCount + 1;

	outFace = Math::FloorToInt((subresourceIndex) / (float)mipmapCount);
	outMip = subresourceIndex % mipmapCount;
}

u32 TextureProperties::MapToSubresourceIndex(u32 face, u32 mip) const
{
	return face * (MipMapCount + 1) + mip;
}

TShared<PixelData> TextureProperties::AllocBuffer(u32 face, u32 mipLevel) const
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

	TShared<PixelData> dst = B3DMakeShared<PixelData>(width, height, depth, Format);
	dst->AllocateInternalBuffer();

	return dst;
}

Texture::Texture(const TextureCreateInformation& createInformation, const TShared<PixelData>& pixelData)
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
	if(mProperties.Usage.IsSetAny(TextureUsageFlag::CPUCached))
	{
		CreateCpuBuffers();

		if(mInitData != nullptr)
			UpdateCpuBuffers(0, *mInitData);
	}

	Resource::Initialize();
}

TShared<render::RenderProxy> Texture::CreateRenderProxy() const
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	TextureCreateInformation createInformation = mProperties;
	createInformation.InitialData = mInitData;
	mInitData = nullptr;

	return gpuDevice->CreateTexture(createInformation, GpuObjectCreateFlag::DeferredInitialize | GpuObjectCreateFlag::RenderThreadDestroy);
}

TAsyncOp<void> Texture::WriteData(const TShared<PixelData>& data, u32 face, u32 mipLevel, bool discardEntireBuffer)
{
	u32 subresourceIdx = mProperties.MapToSubresourceIndex(face, mipLevel);
	UpdateCpuBuffers(subresourceIdx, *data);

	data->LockInternal();

	auto fnWriteData = [](const TShared<render::Texture>& texture, u32 _face, u32 _mipLevel, const TShared<PixelData>& _pixData,
		bool _discardEntireBuffer, TAsyncOp<void>& asyncOp)
	{
		render::TextureWriteFlags flags = _discardEntireBuffer ? render::TextureWriteFlag::Discard : render::TextureWriteFlag::Normal;
		GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
		render::TextureUtility::Write(gpuContext, texture, *_pixData, _mipLevel, _face, flags);
		_pixData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnWriteData = std::move(fnWriteData), renderProxy = B3DGetRenderProxy(this), face, mipLevel, data, discardEntireBuffer, asyncOp]() mutable { fnWriteData(renderProxy, face, mipLevel, data, discardEntireBuffer, asyncOp); }, "Texture::WriteData", false, GetName());

	return asyncOp;
}

TAsyncOp<void> Texture::ReadData(const TShared<PixelData>& data, u32 face, u32 mipLevel)
{
	data->LockInternal();

	auto fnReadData = [](const TShared<render::Texture>& texture, u32 face, u32 mipLevel, const TShared<PixelData>& pixelData, TAsyncOp<void>& asyncOp)
	{
		GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
		gpuContext.SubmitTransferCommandBuffers();

		render::TextureUtility::Read(gpuContext, texture, *pixelData, mipLevel, face);
		pixelData->UnlockInternal();
		asyncOp.CompleteOperation();
	};

	TAsyncOp<void> asyncOp;
	GetRenderThread().PostCommand([fnReadData = std::move(fnReadData), renderProxy = B3DGetRenderProxy(this), face, mipLevel, data, asyncOp]() mutable { fnReadData(renderProxy, face, mipLevel, data, asyncOp); }, "Texture::ReadData", false, GetName());

	return asyncOp;
}

TAsyncOp<TShared<PixelData>> Texture::ReadData(u32 face, u32 mipLevel)
{
	TAsyncOp<TShared<PixelData>> op;

	auto fnReadDataAsync = [texture = B3DGetRenderProxy(this), face, mipLevel, op]() mutable
	{
		GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
		gpuContext.SubmitTransferCommandBuffers();

		TShared<PixelData> output = texture->GetProperties().AllocBuffer(face, mipLevel);
		render::TextureUtility::Read(gpuContext, texture, *output, mipLevel, face);

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
	if(!mProperties.Usage.IsSetAny(TextureUsageFlag::CPUCached))
		return;

	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		B3D_LOG(Error, LogTexture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	u32 mipLevel;
	u32 face;
	mProperties.MapFromSubresourceIndex(subresourceIdx, face, mipLevel);

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(pixelData.GetWidth() != mipWidth || pixelData.GetHeight() != mipHeight ||
	   pixelData.GetDepth() != mipDepth || pixelData.GetFormat() != mProperties.Format)
	{
		B3D_LOG(Error, LogTexture, "Provided buffer is not of valid dimensions or format in order to update this texture.");
		return;
	}

	if(!B3D_ENSURE_LOG(mCPUSubresourceData[subresourceIdx]->GetSize() == pixelData.GetSize(), "Buffer sizes don't match."))
		return;

	u8* dest = mCPUSubresourceData[subresourceIdx]->GetData();
	u8* src = pixelData.GetData();

	memcpy(dest, src, pixelData.GetSize());
}

void Texture::ReadCachedData(PixelData& dest, u32 face, u32 mipLevel)
{
	if(!mProperties.Usage.IsSetAny(TextureUsageFlag::CPUCached))
	{
		B3D_LOG(Error, LogTexture, "Attempting to read CPU data from a texture that is created without CPU caching.");
		return;
	}

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(dest.GetWidth() != mipWidth || dest.GetHeight() != mipHeight ||
	   dest.GetDepth() != mipDepth || dest.GetFormat() != mProperties.Format)
	{
		B3D_LOG(Error, LogTexture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	u32 subresourceIdx = mProperties.MapToSubresourceIndex(face, mipLevel);
	if(subresourceIdx >= (u32)mCPUSubresourceData.size())
	{
		B3D_LOG(Error, LogTexture, "Invalid subresource index: {0}. Supported range: 0 .. {1}", subresourceIdx, (u32)mCPUSubresourceData.size());
		return;
	}

	if(!B3D_ENSURE_LOG(mCPUSubresourceData[subresourceIdx]->GetSize() == dest.GetSize(), "Buffer sizes don't match."))
		return;

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
			u32 subresourceIdx = mProperties.MapToSubresourceIndex(faceIndex, mipIndex);

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
	TShared<Texture> texture = CreateShared(createInformation);

	return B3DStaticResourceCast<Texture>(GetResources().CreateResourceHandle(texture));
}

TShared<Texture> Texture::CreateShared(const TextureCreateInformation& createInformation)
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture(createInformation);
	TShared<Texture> shared = B3DMakeSharedFromExisting<Texture>(texture);

	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

TShared<Texture> Texture::CreateEmpty()
{
	Texture* const texture = new(B3DAllocate<Texture>()) Texture();
	TShared<Texture> shared = B3DMakeSharedFromExisting<Texture>(texture);

	shared->SetShared(shared);
	return shared;
}

namespace b3d { namespace render
{
TShared<Texture> Texture::kWhite;
TShared<Texture> Texture::kBlack;
TShared<Texture> Texture::kPink;
TShared<Texture> Texture::kNormal;

Texture::Texture(const TextureCreateInformation& createInformation)
	: mProperties(createInformation), mInitData(createInformation.InitialData), mName(createInformation.Name)
{}

void Texture::Initialize()
{
	if(mInitData != nullptr)
	{
		GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
		TextureUtility::Write(gpuContext, std::static_pointer_cast<Texture>(GetShared()), *mInitData, 0, 0, TextureWriteFlag::Discard);
		mInitData->UnlockInternal();
		mInitData = nullptr;
	}

	RenderProxy::Initialize();
}

/************************************************************************/
/* 								TEXTURE VIEW                      		*/
/************************************************************************/

TShared<TextureView> Texture::CreateView(const TextureViewInformation& desc)
{
	return B3DMakeSharedFromExisting<TextureView>(new(B3DAllocate<TextureView>()) TextureView(desc));
}

void Texture::ClearBufferViews()
{
	mTextureViews.clear();
}

TShared<TextureView> Texture::RequestView(const TextureSurface& surface, GpuViewUsage usage)
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

ImageSubresourcePitch Texture::GetStagingBufferPitchForSubresource(u32 face, u32 mipLevel) const
{
	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	u32 rowPitch, depthPitch;
	PixelUtility::GetPitch(mipWidth, mipHeight, mipDepth, mProperties.Format, rowPitch, depthPitch);

	const u32 blockSize = PixelUtility::GetBlockSize(mProperties.Format);

	B3D_ASSERT(rowPitch % blockSize == 0);
	B3D_ASSERT(depthPitch % blockSize == 0);

	u32 rowPitchInPixels = rowPitch / blockSize;
	u32 depthPitchInPixels = depthPitch / blockSize;

	if(PixelUtility::IsCompressed(mProperties.Format))
	{
		// For compressed formats the pitch is expressed in blocks and we need to convert it to pixels
		const Vector2I blockDimension = PixelUtility::GetBlockDimensions(mProperties.Format);
		rowPitchInPixels *= blockDimension.X;
		depthPitchInPixels *= blockDimension.X * blockDimension.Y;
	}

	return ImageSubresourcePitch(rowPitchInPixels, rowPitchInPixels != 0 ? depthPitchInPixels / rowPitchInPixels : 0);
}

namespace
{
	/**
	 * Applies a subresource staging pitch (in blocks) to a PixelData object as byte pitches. Must be called before
	 * the pixel data buffer is allocated or assigned.
	 */
	void ApplyRowAndSlicePitch(const ImageSubresourcePitch& pitch, PixelData& inOutPixelData)
	{
		const PixelFormat format = inOutPixelData.GetFormat();
		const u32 blockSize = PixelUtility::GetBlockSize(format);

		u32 rowPitchBytes;
		u32 slicePitchBytes;
		if(PixelUtility::IsCompressed(format))
		{
			const Vector2I blockDimension = PixelUtility::GetBlockDimensions(format);
			rowPitchBytes = pitch.RowPitch / blockDimension.X * blockSize;
			slicePitchBytes = rowPitchBytes * (pitch.SliceHeight / blockDimension.Y);
		}
		else
		{
			rowPitchBytes = pitch.RowPitch * blockSize;
			slicePitchBytes = rowPitchBytes * pitch.SliceHeight;
		}

		inOutPixelData.SetRowPitch(rowPitchBytes);
		inOutPixelData.SetSlicePitch(slicePitchBytes);
	}
}

TShared<GpuBuffer> TextureUtility::CreateStagingBuffer(GpuWorkContext& gpuContext, const TShared<Texture>& texture, u32 mipLevel, bool readable)
{
	B3D_ASSERT(texture != nullptr);

	const TextureProperties& properties = texture->GetProperties();

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(properties.Width, properties.Height, properties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	PixelData pixelData(mipWidth, mipHeight, mipDepth, texture->GetProperties().Format);
	ApplyRowAndSlicePitch(texture->GetStagingBufferPitchForSubresource(0, mipLevel), pixelData);
	return CreateStagingBuffer(gpuContext, texture, pixelData, readable);
}

TShared<GpuBuffer> TextureUtility::CreateStagingBuffer(GpuWorkContext& gpuContext, const TShared<Texture>& texture, const PixelData& pixelData, bool readable)
{
	GpuBufferCreateInformation createInformation;
	createInformation.Type = readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite;
	createInformation.Staging.Size = pixelData.GetSize();

	return gpuContext.CreateTransientGpuBuffer(createInformation);
}

void TextureUtility::Write(GpuWorkContext& gpuContext, const TShared<Texture>& texture, const PixelData& source, u32 mipLevel, u32 arrayLayer, TextureWriteFlags flags, TShared<GpuCommandBuffer> commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD
	B3D_ASSERT(texture != nullptr);

	if(source.GetSize() == 0)
		return;

	const TextureProperties& textureProperties = texture->GetProperties();
	if(textureProperties.SampleCount > 1)
	{
		B3D_LOG(Error, LogRenderBackend, "Multisampled textures cannot be written to from the CPU.");
		return;
	}

	mipLevel = Math::Clamp(mipLevel, 0u, textureProperties.MipMapCount);
	arrayLayer = Math::Clamp(arrayLayer, 0u, textureProperties.GetFaceCount() - 1);

	if(arrayLayer > 0 && textureProperties.Type == TEX_TYPE_3D)
	{
		B3D_LOG(Error, LogRenderBackend, "3D texture arrays are not supported.");
		return;
	}

	const bool canDiscardContents = flags.IsSet(TextureWriteFlag::Discard);
	const bool noOverwrite = flags.IsSet(TextureWriteFlag::NoOverwrite);
	const bool supportsGPUWrites = textureProperties.Usage.IsSetAny(TextureUsageFlag::AllowUnorderedAccessOnTheGPU);

	GpuMapOptions mapOptions = GpuMapOption::Write;
	if(noOverwrite)
		mapOptions |= GpuMapOption::NoOverwrite;

	// Check is the GPU currently reading or writing from the image
	const GpuQueueMask subresourceUseMask = texture->GetUseMask(mipLevel, arrayLayer, GpuAccessFlag::Read | GpuAccessFlag::Write);
	const u32 subresourceIndex = textureProperties.MapToSubresourceIndex(arrayLayer, mipLevel);
	const u32 subresourceUseCount = texture->GetUseCount(subresourceIndex);
	const u32 subresourceBoundCount = texture->GetBoundCount(subresourceIndex);

	// Try direct mapping if texture supports it
	void* mappedMemory = texture->GetMappedMemory();
	if(mappedMemory != nullptr)
	{
		const bool isBound = subresourceBoundCount > 0;

		// Recreate the internal image if it is bound to a command buffer, to avoid overwriting the old data. But only if the user
		// allows discard via a flag. In case the user provided an explicit command buffer, perfer a staging buffer over discard
		// (it still costs us creation of a new buffer, and the original buffer bindings remain valid). Finally, if no-overwrite is
		// specified, we never recreate the buffer as the user guarantees he won't touch the previously bound region.
		const bool recreateImage = isBound && commandBuffer == nullptr && canDiscardContents && !noOverwrite;

		// Even if the texture is directly mappable we might wish to avoid mapping it directly in these situations:
		const bool shouldMapDirectly =
			recreateImage // A freshly recreated backing resource is unreferenced by the GPU, so mapping it is always safe
			|| (!isBound && !supportsGPUWrites) // Image not bound and not being used, nor are there any pending GPU writes
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

	// Create staging buffer
	const u32 mipWidth = Math::Max(1u, textureProperties.Width >> mipLevel);
	const u32 mipHeight = Math::Max(1u, textureProperties.Height >> mipLevel);
	const u32 mipDepth = Math::Max(1u, textureProperties.Depth >> mipLevel);

	PixelData lockedArea(mipWidth, mipHeight, mipDepth, textureProperties.Format);
	ApplyRowAndSlicePitch(texture->GetStagingBufferPitchForSubresource(arrayLayer, mipLevel), lockedArea);
	TShared<GpuBuffer> stagingBuffer = CreateStagingBuffer(gpuContext, texture, lockedArea, false);

	if(!B3D_ENSURE(stagingBuffer != nullptr))
		return;

	// Copy data to staging buffer
	{
		GpuBufferMappedScope bufferScope = stagingBuffer->Map(GpuMapOption::Write);
		lockedArea.SetExternalBuffer(static_cast<u8*>(bufferScope.GetMappedMemory()));
		PixelUtility::BulkPixelConversion(source, lockedArea);
	}

	// If the image is used in any way on the GPU, we need to wait for that use to finish before we issue our copy -
	// unless the user promised via no-overwrite not to touch any region the GPU is using.
	GpuQueueMask syncMask;
	if(!subresourceUseMask.IsEmpty() && !mapOptions.IsSet(GpuMapOption::NoOverwrite)) // Image is currently used on the GPU
		syncMask = subresourceUseMask;

	// Check if the image will still be bound somewhere after the command buffers using it finish. If it is, we have to recreate the internal image otherwise the copy
	// operation might just get overwritten by those command buffers when the execute. This is because the transfer command buffers are always submitted before regular
	// command buffers. If user provided an explicit command buffer, then it's up to him to ensure the correct ordering.
	const bool isBoundWithoutUse = subresourceBoundCount > subresourceUseCount;
	if(isBoundWithoutUse && commandBuffer == nullptr)
	{
		if(!canDiscardContents)
		{
			B3D_LOG(Warning, LogRenderBackend, "Writing to a image '{0}' that is currently bound on a command buffer, without providing an explicit command buffer. Such writes will be queued on the transfer buffer which is submitted before any user command buffers. This means multiple writes will overwrite it each other if not careful.", texture->GetName());
		}
		else
			texture->RecreateInternalTexture();
	}

	// Get or create command buffer
	if(commandBuffer == nullptr)
		commandBuffer = gpuContext.GetTransferCommandBuffer();

	// Issue copy command
	commandBuffer->CopyBufferToTexture(stagingBuffer, texture, 0, mipLevel, arrayLayer);
	commandBuffer->AddQueueSyncMask(syncMask);
}

void TextureUtility::Read(GpuWorkContext& gpuContext, const TShared<Texture>& texture, PixelData& destination, u32 mipLevel, u32 arrayLayer, const TShared<GpuQueue>& gpuQueue)
{
	B3D_ASSERT(texture != nullptr);

	const TextureProperties& textureProperties = texture->GetProperties();

	u32 mipWidth, mipHeight, mipDepth;
	PixelUtility::GetSizeForMipLevel(textureProperties.Width, textureProperties.Height, textureProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

	if(destination.GetWidth() != mipWidth || destination.GetHeight() != mipHeight ||
	   destination.GetDepth() != mipDepth || destination.GetFormat() != textureProperties.Format)
	{
		B3D_LOG(Error, LogTexture, "Provided buffer is not of valid dimensions or format in order to read from this texture.");
		return;
	}

	if(textureProperties.SampleCount > 1)
	{
		B3D_LOG(Error, LogRenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	const bool supportsGPUWrites = textureProperties.Usage.IsSetAny(TextureUsageFlag::AllowUnorderedAccessOnTheGPU);

	GpuQueue& transferGpuQueue = gpuQueue != nullptr ? *gpuQueue : *texture->GetDevice().GetQueue(GQT_GRAPHICS, 0);

	// Check is the GPU currently writing to the texture
	const GpuQueueMask subresourceWriteUseMask = texture->GetUseMask(mipLevel, arrayLayer, GpuAccessFlag::Write);

	// If memory is host visible try mapping it directly
	void* mappedMemory = texture->GetMappedMemory();
	if(mappedMemory != nullptr)
	{
		// Note: Even if GPU isn't currently using the buffer, but the buffer supports GPU writes, we consider it as
		// being used because the write could have completed yet still not visible, so we need to wait for any
		// GPU operations to complete.
		const bool isUsedOnGPU = !subresourceWriteUseMask.IsEmpty() || supportsGPUWrites;

		// If used on the GPU, we need to wait until all write operations complete before mapping it
		if(isUsedOnGPU)
		{
			TShared<GpuCommandBuffer> commandBuffer = gpuContext.GetTransferCommandBuffer();

			// Make any writes visible before mapping
			if(supportsGPUWrites)
			{
				// Issue a barrier so the device makes the written memory available for read (read-after-write hazard)
				commandBuffer->IssueBarriers({{ GpuTextureBarrier(texture, GpuResourceUseFlag::Host, GpuAccessFlag::Read)}});
			}

			// Submit the command buffer and wait until it finishes
			commandBuffer->AddQueueSyncMask(subresourceWriteUseMask);
			gpuContext.SubmitTransferCommandBuffers(true);
		}

		GpuTextureMappedScope mappedScope = texture->Map(mipLevel, arrayLayer, GpuMapOption::Read);
		PixelUtility::BulkPixelConversion(mappedScope.GetPixelData(), destination);

		return;
	}

	// Can't use direct mapping, so use a staging buffer

	// Allocate a staging buffer
	PixelData pixelData(mipWidth, mipHeight, mipDepth, texture->GetSupportedFormat());
	ApplyRowAndSlicePitch(texture->GetStagingBufferPitchForSubresource(arrayLayer, mipLevel), pixelData);
	TShared<GpuBuffer> stagingBuffer = CreateStagingBuffer(gpuContext, texture, pixelData, true);

	// Similar to above, if image supports GPU writes or is currently being written to, we need to wait on any
	// potential writes to complete
	GpuQueueMask syncMask;
	if(supportsGPUWrites || !subresourceWriteUseMask.IsEmpty())
	{
		// Ensure flush will wait for all queues currently writing to the image (if any) to finish
		syncMask = subresourceWriteUseMask;
	}

	TShared<GpuCommandBuffer> commandBuffer = gpuContext.GetTransferCommandBuffer();

	// Queue copy command
	commandBuffer->CopyTextureToBuffer(texture, stagingBuffer, mipLevel, arrayLayer, 0);

	// Submit the command buffer and wait until it finishes
	commandBuffer->AddQueueSyncMask(syncMask);
	gpuContext.SubmitTransferCommandBuffers(true);

	{
		GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);
		pixelData.SetExternalBuffer((u8*)mapping.GetMappedMemory());
		PixelUtility::BulkPixelConversion(pixelData, destination);
	}
}

TAsyncOp<TShared<PixelData>> TextureUtility::ReadAsync(GpuWorkContext& gpuContext, const TShared<Texture>& texture, GpuCommandBuffer& commandBuffer, u32 mipLevel, u32 arrayLayer)
{
	if(texture == nullptr)
		return {};

	const TextureProperties& textureProperties = texture->GetProperties();
	const u32 mipWidth = Math::Max(1u, textureProperties.Width >> mipLevel);
	const u32 mipHeight = Math::Max(1u, textureProperties.Height >> mipLevel);
	const u32 mipDepth = Math::Max(1u, textureProperties.Depth >> mipLevel);

	const TShared<PixelData> pixelData = B3DMakeShared<PixelData>(mipWidth, mipHeight, mipDepth, texture->GetSupportedFormat());
	ApplyRowAndSlicePitch(texture->GetStagingBufferPitchForSubresource(arrayLayer, mipLevel), *pixelData);

	// TODO - Staging buffer might not be necessary if he texture is directly mappable
	TShared<GpuBuffer> stagingBuffer = CreateStagingBuffer(gpuContext, texture, *pixelData, true);
	commandBuffer.CopyTextureToBuffer(texture, stagingBuffer, mipLevel, arrayLayer);

	TAsyncOp<TShared<PixelData>> op;
	auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData]() mutable
	{
		GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);

		pixelData->AllocateInternalBuffer();
		memcpy(pixelData->GetData(), mapping.GetMappedMemory(), pixelData->GetSize());

		op.CompleteOperation(pixelData);
	};

	auto fnOnCommandBufferDestroyed = [op](bool isSubmitted) mutable
	{
		// In this case the completion callback will trigger.
		if(isSubmitted)
			return;

		op.CompleteOperation(nullptr);
	};

	commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
	commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

	return op;
}

void TextureUtility::Clear(GpuWorkContext& gpuContext, const TShared<Texture>& texture, const Color& value, u32 mipLevel, u32 arrayLayer, const TShared<GpuCommandBuffer>& commandBuffer)
{
	ASSERT_IF_NOT_RENDER_THREAD

	const TextureProperties& textureProperties = texture->GetProperties();
	if(arrayLayer >= textureProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid array index.");
		return;
	}

	if(mipLevel > textureProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Mip level out of range. Valid range is [0, {0}].", textureProperties.MipMapCount);
		return;
	}

	TShared<PixelData> data = textureProperties.AllocBuffer(arrayLayer, mipLevel);
	data->SetColors(value);

	Write(gpuContext, texture, *data, mipLevel, arrayLayer, TextureWriteFlag::Discard, commandBuffer);
}
}}
