//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalUtility.h"
#include "Debug/B3DLog.h"
#include "Math/B3DMath.h"

namespace b3d
{
	namespace render
	{
		MetalBuffer::MetalBuffer(MetalResourceManager* owner, const MetalBufferCreateInformation& createInformation, MetalBufferNativeHandle buffer, const GpuResourceLocation& allocation, void* mappedMemory)
			: TMetalResource<IGpuBufferResource>(owner, createInformation.DebugName), mType(createInformation.Type), mFlags(createInformation.Flags), mBuffer(buffer), mAllocation(allocation), mMappedMemory(mappedMemory)
		{ }

		MetalBuffer::~MetalBuffer()
		{
			{
				Lock lock(mViewCacheMutex);

#if !__has_feature(objc_arc)
				for (auto& viewEntry : mTextureBufferViews)
					[viewEntry.View release];
#endif

				mTextureBufferViews.clear();
			}

#if !__has_feature(objc_arc)
			[mBuffer release];
#endif
			mBuffer = nullptr;
			mMappedMemory = nullptr;

			if (mAllocation.IsValid())
				mAllocation.Allocator->Free(mAllocation);
		}

		void MetalBuffer::SetName(const StringView& name)
		{
			if (mBuffer == nullptr)
				return;

			@autoreleasepool
			{
				const String nameCopy(name.data(), name.size());
				[mBuffer setLabel:[NSString stringWithUTF8String:nameCopy.c_str()]];
			}
		}

		MetalGpuBuffer::MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation)
			: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device)), mGpuDevice(device), mMemoryType(MetalHeapAllocator::PickBufferMemoryType(createInformation)), mDirectlyMappable(createInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess) || createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite)
		{ }

		MetalGpuBuffer::MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation,
			IGpuAllocator& allocator)
			: MetalGpuBuffer(device, createInformation)
		{
			mAllocator = &allocator;
		}

		MetalGpuBuffer::~MetalGpuBuffer()
		{
			if (mBuffer != nullptr)
				mBuffer->Destroy();
		}

		void MetalGpuBuffer::Initialize()
		{
			RecreateInternalBuffer();
		}

		MetalBuffer* MetalGpuBuffer::CreateBuffer()
		{
			// Metal disallows zero-length buffers; clamp to a small minimum
			u64 size = mTotalSize;
			if (size == 0)
				size = 64;

			GpuResourceLocation location;
			MetalBufferNativeHandle handle = mAllocator != nullptr
				? mGpuDevice.GetHeapAllocator().AllocateBuffer(size, mMemoryType, *mAllocator, location)
				: mGpuDevice.GetHeapAllocator().AllocateBuffer(size, mMemoryType, location);
			if (handle == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create {0} MTLBuffer of {1} bytes.", mAllocator != nullptr ? "transient" : "persistent", size);
				return nullptr;
			}

			MetalBufferCreateInformation createInformation;
			createInformation.Type = mInformation.Type;
			createInformation.Flags = mInformation.Flags;
			createInformation.DebugName = mName;

			void* mappedMemory = mDirectlyMappable ? [handle contents] : nullptr;
			MetalBuffer* buffer = mGpuDevice.GetResourceManager().Create<MetalBuffer>(createInformation, handle, location, mappedMemory);

#if B3D_BUILD_TYPE_DEVELOPMENT
			if (mInformation.SuballocationCount > 1)
				buffer->InitializeSuballocationTracking(mInformation.SuballocationCount, mSuballocationSize);
#endif

			if (!mName.empty())
				buffer->SetName(mName);

			return buffer;
		}

		void MetalGpuBuffer::RecreateInternalBuffer()
		{
			MetalBuffer* newBuffer = CreateBuffer();

			if (mBuffer != nullptr)
				mBuffer->Destroy();

			mBuffer = newBuffer;
			mMappedMemory = mBuffer != nullptr ? mBuffer->GetMappedMemory() : nullptr;
		}

		void MetalGpuBuffer::SetName(const StringView& name)
		{
			GpuBuffer::SetName(name);

			if (mBuffer != nullptr)
				mBuffer->SetName(name);
		}

		GpuQueueMask MetalGpuBuffer::GetUseMask(GpuAccessFlags accessFlags)
		{
			if (mBuffer == nullptr)
				return GpuQueueMask::kNone;

			return mBuffer->GetUseInfo(accessFlags);
		}

		u32 MetalGpuBuffer::GetBoundCount() const
		{
			return mBuffer != nullptr ? mBuffer->GetBoundCount() : 0;
		}

		u32 MetalGpuBuffer::GetUseCount() const
		{
			return mBuffer != nullptr ? mBuffer->GetUseCount() : 0;
		}

#if B3D_BUILD_TYPE_DEVELOPMENT
		bool MetalGpuBuffer::IsRangeBound(u32 offset, u32 size) const
		{
			return mBuffer != nullptr && mBuffer->IsRangeBound(offset, size);
		}

		bool MetalGpuBuffer::IsRangeInUse(u32 offset, u32 size) const
		{
			return mBuffer != nullptr && mBuffer->IsRangeInUse(offset, size);
		}
#endif

		id<MTLBuffer> MetalGpuBuffer::GetMetalBuffer() const
		{
			return mBuffer != nullptr ? mBuffer->GetMetalHandle() : nil;
		}

		id<MTLTexture> MetalBuffer::GetTextureBufferView(GpuBufferFormat format, u32 offset, u32 range, bool writable)
		{
			if (mBuffer == nil)
				return nil;

			// One fiber may be fetching a view while another adds to the same cache — serialize
			// both the find and the insert so the pair is atomic (see mViewCacheMutex docs).
			Lock lock(mViewCacheMutex);
			for (const TextureBufferView& entry : mTextureBufferViews)
			{
				if (entry.Format == format && entry.Offset == offset && entry.Range == range && entry.Writable == writable)
					return entry.View;
			}

			const MTLPixelFormat pixelFormat = MetalUtility::GetBufferFormat(format);
			if (pixelFormat == MTLPixelFormatInvalid)
			{
				B3D_LOG(Error, LogRenderBackend, "Typed-buffer element format {0} has no Metal pixel-format mapping.", (u32)format);
				return nil;
			}

			const u32 elementSize = b3d::GpuBuffer::GetFormatSize(format);
			const u64 bufferLength = (u64)[mBuffer length];
			if (elementSize == 0 || offset >= bufferLength)
			{
				B3D_LOG(Error, LogRenderBackend, "Typed-buffer view range is outside the buffer. Offset: {0}, buffer length: {1}.", offset, bufferLength);
				return nil;
			}

			const u64 availableBytes = bufferLength - offset;
			const u64 rangeBytes = range == 0 ? availableBytes : Math::Min((u64)range, availableBytes);
			const u64 elementCount = rangeBytes / elementSize;
			if (elementCount == 0)
				return nil;

			id<MTLDevice> device = [mBuffer device];
			const NSUInteger alignment = [device minimumLinearTextureAlignmentForPixelFormat:pixelFormat];
			if (alignment != 0 && (offset % alignment) != 0)
			{
				B3D_LOG(Error, LogRenderBackend, "Typed-buffer view offset {0} violates the device's linear-texture alignment of {1}.", offset, (u64)alignment);
				return nil;
			}

			id<MTLTexture> view = nil;
			@autoreleasepool
			{
				MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
					textureBufferDescriptorWithPixelFormat:pixelFormat
													 width:(NSUInteger)elementCount
										   resourceOptions:MetalUtility::GetResourceOptions([mBuffer storageMode])
													 usage:writable ? (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite) : MTLTextureUsageShaderRead];
				view = [mBuffer newTextureWithDescriptor:descriptor
												  offset:offset
											 bytesPerRow:(NSUInteger)(elementCount * elementSize)];
			}
			if (view == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create a Metal texture-buffer view. Format: {0}, elements: {1}.", (u32)format, elementCount);
				return nil;
			}

			TextureBufferView entry;
			entry.Format = format;
			entry.Offset = offset;
			entry.Range = range;
			entry.Writable = writable;
			entry.View = view;
			mTextureBufferViews.push_back(entry);

			return view;
		}

		id<MTLTexture> MetalGpuBuffer::GetTextureBufferView(GpuBufferFormat format, u32 offset, u32 range, bool writable)
		{
			if (mBuffer == nullptr)
				return nil;

			// An unspecified view format means "interpret the buffer with its own element format", matching
			// GpuBufferViewInformation::Format. Shader reflection cannot supply this: texture_buffer<float> 
			// reports only the component type, so a float4 buffer would otherwise be viewed as single-component 
			// and read back garbage.
			if (format == BF_UNKNOWN)
				format = mInformation.SimpleStorage.Format;

			return mBuffer->GetTextureBufferView(format, offset, range, writable);
		}
	} // namespace render
} // namespace b3d
