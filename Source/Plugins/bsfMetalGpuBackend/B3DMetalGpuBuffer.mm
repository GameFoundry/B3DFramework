//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalHeapAllocator.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuBuffer::Impl
		{
			id<MTLBuffer> Buffer = nil;
		};

		MetalGpuBuffer::MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation)
			: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device))
			, mGpuDevice(device)
			, mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuBuffer::~MetalGpuBuffer()
		{
			// A'10: mirror @c RecreateInternalBuffer's deferred-release pattern for dtor-initiated
			// releases. The MTLBuffer may still be referenced by in-flight command buffers when the
			// last TShared<MetalGpuBuffer> is dropped; queue the backing handle against the graphics
			// queue's last-committed watermark so it survives until those retire.
			//
			// Shortcuts:
			//   * No @c mImpl or no @c Buffer → nothing to defer.
			//   * Device is shutting down → synchronous release (see the matching comment in
			//     @c ~MetalTexture: @c ~MetalGpuDevice has already drained @c DeferredReleases and
			//     will reset the heap allocator below this point, so queuing now would strand heap-
			//     backed entries past @c mHeapAllocator.reset()).
			//   * No graphics queue resolvable, or @c lastCommitted == 0 → the buffer couldn't have
			//     been scheduled; release immediately.
			if (!mImpl || mImpl->Buffer == nil)
			{
				ReleaseInternalBuffer();
				return;
			}

			if (mGpuDevice.IsShuttingDown())
			{
				ReleaseInternalBuffer();
				return;
			}

			TShared<GpuQueue> gfxQueue = mGpuDevice.GetQueue(GQT_GRAPHICS, 0);
			MetalGpuQueue* metalQueue = gfxQueue ? static_cast<MetalGpuQueue*>(gfxQueue.get()) : nullptr;
			const u64 lastCommitted = metalQueue != nullptr ? metalQueue->GetLastCommittedEventValue() : 0;

			if (metalQueue != nullptr && lastCommitted > 0)
			{
				// Transfer the strong ref into the deferred-release list, then nil the impl field so
				// the dtor completes with a fully-cleared state.
				id<MTLBuffer> prior = mImpl->Buffer;
				mImpl->Buffer = nil;
				mMappedMemory = nullptr;
				mGpuDevice.QueueMetalResourceForDeferredRelease(prior, metalQueue, lastCommitted);
				return;
			}

			ReleaseInternalBuffer();
		}

		id<MTLBuffer> MetalGpuBuffer::GetMetalBuffer() const
		{
			return mImpl->Buffer;
		}

		void MetalGpuBuffer::SetName(const StringView& name)
		{
			// Delegate to the base so @c GpuBuffer::mName (read by @c GetName) is the single source
			// of truth — an earlier shadowing @c String mName here meant @c GetName returned an
			// empty string even after @c SetName ran.
			GpuBuffer::SetName(name);
			if (mImpl->Buffer)
			{
				NSString* nsName = [NSString stringWithUTF8String:GetName().c_str()];
				[mImpl->Buffer setLabel:nsName];
			}
		}

		void MetalGpuBuffer::Initialize()
		{
			// A'11: intentionally does NOT add an "allocation failed → skip base Initialize" guard
			// analogous to @c MetalTexture::Initialize. Reason: there is no base @c GpuBuffer::Initialize
			// asset-upload path to chain into on the render side — buffer content uploads are driven by
			// @c GpuBufferUtility::Write through the command buffer, which already nil-checks the target.
			// @c CreateInternalBuffer logs its own failure reason and sets @c mMappedMemory to nullptr;
			// no extra guard needed here. This comment is left to prevent future pattern-matching on the
			// Vulkan / Metal texture init sequence from re-introducing a phantom base-call check.
			CreateInternalBuffer();
		}

		void MetalGpuBuffer::RecreateInternalBuffer()
		{
			// The prior MTLBuffer may still be referenced by in-flight command buffers. Hand the
			// strong ref to the device's deferred-release list tagged with the graphics queue's
			// last-committed event value; the next @c BeginFrame drops it once that value has been
			// signaled. If the queue has never scheduled any work (ReservedValue == 0) the buffer
			// cannot have been bound, so we short-circuit and let the local strong ref fall out of
			// scope, releasing immediately.
			//
			// TODO: this watermark covers only the graphics queue. If the buffer was referenced solely
			// on the compute or transfer queue, the release may over-retain by up to one frame (the
			// graphics queue's frontier is usually ahead by that much). Tighten by recording the
			// resource's actual last-submit queue + value once per-resource submit tracking lands.
			id<MTLBuffer> prior = mImpl->Buffer;
			mImpl->Buffer = nil;
			mMappedMemory = nullptr;

			if (prior != nil)
			{
				TShared<GpuQueue> gfxQueue = mGpuDevice.GetQueue(GQT_GRAPHICS, 0);
				MetalGpuQueue* metalQueue = gfxQueue ? static_cast<MetalGpuQueue*>(gfxQueue.get()) : nullptr;
				const u64 lastCommitted = metalQueue != nullptr ? metalQueue->GetLastCommittedEventValue() : 0;

				if (metalQueue != nullptr && lastCommitted > 0)
					mGpuDevice.QueueMetalResourceForDeferredRelease(prior, metalQueue, lastCommitted);
				// else: the buffer could not have been scheduled yet — strong ref falls out of scope here.
			}

			CreateInternalBuffer();
		}

		void MetalGpuBuffer::CreateInternalBuffer()
		{
			// Wrapping the Obj-C allocations below in an autorelease pool — Metal's buffer/label
			// allocations briefly hold autoreleased references; under the engine's fiber scheduler
			// we cannot rely on a runloop draining them between invocations, so we drain locally.
			@autoreleasepool
			{
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil || mTotalSize == 0)
				return;

			// Pick a storage mode based on the engine flags:
			//   * StoreOnCPUWithGPUAccess, or Staging types — Shared: CPU writes are directly visible
			//     to the GPU, so Map returns [buffer contents] and the engine's GpuBufferUtility can
			//     memcpy in place.
			//   * StoreOnGPU (default) — Private: no CPU-visible backing. Map returns null, and
			//     GpuBufferUtility falls back to its staging-buffer + CopyBufferToBuffer path.
			// Managed storage (for discrete Mac GPUs) would need explicit didModifyRange on every
			// write; that's a phase-3 follow-up. Shared works on all Apple platforms and is coherent
			// on Apple Silicon; the CPU-GPU bandwidth hit on discrete Macs is tolerable for now.
			const bool cpuVisible = mInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess)
				|| mInformation.Type == GpuBufferType::StagingRead
				|| mInformation.Type == GpuBufferType::StagingWrite;
			const MTLResourceOptions options = cpuVisible
				? MTLResourceStorageModeShared
				: MTLResourceStorageModePrivate;

			// TODO: phase-2 review S37 — DEFERRED. Leave @c MTLResourceHazardTrackingModeDefault
			// (→ Tracked on macOS) in @c options so the driver auto-tracks read-after-write hazards
			// for buffers reached through argument-buffer bindings + @c useResource:. Flipping to
			// @c MTLResourceHazardTrackingModeUntracked is a perf optimization gated on S14's
			// engine-side tracker; see the TODO in @c B3DMetalGpuCommandBuffer.mm::IssueBarriers.
			// Route through the device's heap allocator so non-volatile buffers sub-allocate from a
			// pooled MTLHeap rather than paying the per-resource driver-side allocation cost.
			// Storage mode is already encoded in @c options, so the allocator picks the correct
			// bucket (private vs shared) internally. Buffers above the large-resource threshold
			// (e.g. streaming terrain, oversized uniform ring allocations) fall through to a direct
			// device allocation inside the allocator.
			mImpl->Buffer = mGpuDevice.GetHeapAllocator().AllocateBuffer(mTotalSize, options);
			if (mImpl->Buffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create MTLBuffer of {0} bytes.", mTotalSize);
				mMappedMemory = nullptr;
				return;
			}

			// Only expose the CPU-visible pointer when the buffer actually has one. GPU-private
			// buffers return nullptr here; the engine interprets that as "use a staging path".
			mMappedMemory = cpuVisible ? [mImpl->Buffer contents] : nullptr;

			if (!GetName().empty())
			{
				NSString* nsName = [NSString stringWithUTF8String:GetName().c_str()];
				[mImpl->Buffer setLabel:nsName];
			}
			} // @autoreleasepool
		}

		void MetalGpuBuffer::ReleaseInternalBuffer()
		{
			if (mImpl)
				mImpl->Buffer = nil;
			mMappedMemory = nullptr;
		}
	} // namespace render
} // namespace b3d
