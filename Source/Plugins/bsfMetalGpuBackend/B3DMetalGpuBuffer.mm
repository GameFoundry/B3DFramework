//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		MetalBuffer::MetalBuffer(MetalResourceManager* owner, const MetalBufferCreateInformation& createInformation, MetalBufferNativeHandle buffer, const GpuResourceLocation& allocation, void* mappedMemory)
			: TMetalResource<IGpuBufferResource>(owner, createInformation.DebugName)
			, mType(createInformation.Type)
			, mFlags(createInformation.Flags)
			, mBuffer(buffer)
			, mAllocation(allocation)
			, mMappedMemory(mappedMemory)
		{ }

		MetalBuffer::~MetalBuffer()
		{
			// The manager's deferred-destroy path guarantees this destructor only runs once the
			// resource is no longer bound to any command buffer nor in flight on the GPU (the
			// resource tracker drives NotifyBound/Used/Done), so the native handle and its memory
			// span can be released synchronously — mirroring VulkanBuffer's destructor.
#if !__has_feature(objc_arc)
			[mBuffer release];
#endif
			mBuffer = nullptr;
			mMappedMemory = nullptr;

			// Allocator-backed spans return through their stamped allocator; direct device allocations
			// carry an invalid location and were fully released by the handle release above. Persistent
			// TLSF spans reclaim immediately, while transient linear spans retire with their page.
			if (mAllocation.IsValid())
				mAllocation.Allocator->Free(mAllocation);
		}

		void MetalBuffer::SetName(const StringView& name)
		{
			if (mBuffer == nullptr)
				return;

			// NSString below is autoreleased; drain locally — there may be no runloop under the
			// engine's fiber scheduler. StringView is not guaranteed null-terminated, so copy first.
			@autoreleasepool
			{
				const String nameCopy(name.data(), name.size());
				[mBuffer setLabel:[NSString stringWithUTF8String:nameCopy.c_str()]];
			}
		}

		MetalGpuBuffer::MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation)
			: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device))
			, mGpuDevice(device)
			, mMemoryType(MetalHeapAllocator::PickBufferMemoryType(createInformation))
			, mDirectlyMappable(createInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess)
				|| createInformation.Type == GpuBufferType::StagingRead
				|| createInformation.Type == GpuBufferType::StagingWrite)
			, mSupportsGPUWrites(createInformation.Flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		{ }

		MetalGpuBuffer::MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation,
			IGpuAllocator& allocator)
			: MetalGpuBuffer(device, createInformation)
		{
			mAllocator = &allocator;
		}

		MetalGpuBuffer::~MetalGpuBuffer()
		{
			// Queue the wrapper for destruction; the manager defers the actual release until every
			// command buffer referencing it has retired.
			if (mBuffer != nullptr)
				mBuffer->Destroy();
		}

		void MetalGpuBuffer::Initialize()
		{
			// Intentionally no "allocation failed -> skip base Initialize" guard analogous to
			// MetalTexture::Initialize: there is no base GpuBuffer::Initialize asset-upload path to
			// chain into on the render side — buffer content uploads are driven by
			// GpuBufferUtility::Write through the command buffer, which already nil-checks the
			// target. CreateBuffer logs its own failure reason.
			RecreateInternalBuffer();
		}

		MetalBuffer* MetalGpuBuffer::CreateBuffer()
		{
			// Metal disallows zero-length buffers; clamp to a small minimum (mirrors the Vulkan
			// backend's CreateBuffer).
			u64 size = mTotalSize;
			if (size == 0)
				size = 64;

			GpuResourceLocation location;
			MetalBufferNativeHandle handle = mAllocator != nullptr
				? mGpuDevice.GetHeapAllocator().AllocateBuffer(size, mMemoryType, *mAllocator, location)
				: mGpuDevice.GetHeapAllocator().AllocateBuffer(size, mMemoryType, location);
			if (handle == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create {0} MTLBuffer of {1} bytes.",
					mAllocator != nullptr ? "transient" : "persistent", size);
				return nullptr;
			}

			MetalBufferCreateInformation createInformation;
			createInformation.Type = mInformation.Type;
			createInformation.Flags = mInformation.Flags;
			createInformation.DebugName = mName;

			// Only expose the CPU-visible pointer when the buffer actually has one. GPU-private
			// buffers return nullptr here; the engine interprets that as "use a staging path".
			void* mappedMemory = mDirectlyMappable ? [handle contents] : nullptr;

			MetalBuffer* buffer = mGpuDevice.GetResourceManager().Create<MetalBuffer>(createInformation, handle, location, mappedMemory);

#if B3D_BUILD_TYPE_DEVELOPMENT
			// Range-level bound/in-use validation for sub-allocated buffers (dynamic-offset binds).
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

			// Queue the previous wrapper for destruction. The manager defers the release until
			// every command buffer referencing it has retired, so in-flight GPU work keeps reading
			// the old MTLBuffer safely while new writes target the fresh one.
			if (mBuffer != nullptr)
				mBuffer->Destroy();

			mBuffer = newBuffer;
			mMappedMemory = mBuffer != nullptr ? mBuffer->GetMappedMemory() : nullptr;
		}

		void MetalGpuBuffer::SetName(const StringView& name)
		{
			// Delegate to the base so GpuBuffer::mName (read by GetName) is the single source of truth.
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
	} // namespace render
} // namespace b3d
