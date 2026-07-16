//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuParameterSetPool.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalShaderABI.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Rounds @p value up to the next multiple of @p alignment. */
			u64 AlignUp(u64 value, u32 alignment)
			{
				if (alignment <= 1)
					return value;
				const u64 mask = (u64)(alignment - 1);
				return (value + mask) & ~mask;
			}
		} // namespace

		MetalGpuParameterSetPool::MetalGpuParameterSetPool(MetalGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
			: GpuParameterSetPool(createInformation)
			, mDevice(device)
		{
		}

		MetalGpuParameterSetPool::~MetalGpuParameterSetPool()
		{
			// Under MRC CreateHeap-equivalent path returns +1 retained buffers; drop our refs so the
			// driver reclaims them. Under ARC this is a no-op (the Vector drops its strong refs). No
			// device-level teardown needed: MTLBuffer is CPU/GPU-auto-released once refcount hits zero.
#if !__has_feature(objc_arc)
			for (Block& block : mBlocks)
			{
				if (block.Buffer != nil)
					[block.Buffer release];
				block.Buffer = nil;
			}
			for (id<MTLBuffer> buffer : mDirectBuffers)
			{
				if (buffer != nil)
					[buffer release];
			}
#endif
			mBlocks.clear();
			mDirectBuffers.clear();
		}

		TShared<GpuParameterSet> MetalGpuParameterSetPool::Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize)
		{
			if (setIndex > kMetalMaximumParameterSetIndex)
			{
				B3D_LOG(Error, LogRenderBackend,
					"Metal parameter set index {0} exceeds the supported maximum of {1}.",
					setIndex, kMetalMaximumParameterSetIndex);
				return nullptr;
			}

			// A'6: worker fibers may call Create concurrently. The quota check and the counter
			// increment must be a single atomic step under mPoolMutex, otherwise two fibers can both
			// pass the gate at mAllocatedSetCount == MaxSets - 1 and overshoot the cap. We reserve
			// the slot here, then release the lock before the heavy work below — crucial because
			// MetalGpuParameters::Initialize() reenters this pool via AcquireArgumentBufferSlice,
			// which re-locks mPoolMutex; Banshee's Mutex is non-recursive, so holding it across
			// Initialize() would self-deadlock. The engine's Metal Initialize() path handles
			// sub-allocation failure by logging + returning a nil argument buffer rather than
			// throwing, so a no-rollback increment is safe: a set that fails to acquire its slice
			// will still consume one quota entry but never produces a phantom successful allocation.
			if (mInformation.Mode == GpuParameterSetPoolMode::Transient)
			{
				Lock lock(mPoolMutex);
				if (mAllocatedSetCount >= mInformation.MaxSets)
					return nullptr;

				mAllocatedSetCount++;
			}

			// B9: hand ourselves to the set so Initialize sub-allocates the argument buffer out of
			// our ring instead of minting a fresh MTLBuffer. The pool outlives every set it creates —
			// a set released after Reset must observe its argument buffer as potentially reused, but
			// the engine's contract is to drop all sets *before* Reset, so holding a raw pool pointer
			// here is safe.
			auto paramSet = B3DMakeShared<MetalGpuParameters>(mDevice, layout, setIndex,
				mInformation.Mode == GpuParameterSetPoolMode::Transient ? this : nullptr);
			paramSet->SetShared(paramSet);

			if (!deferredInitialize)
			{
				paramSet->Initialize();
				if (!paramSet->IsMetalBindingReady())
					return nullptr;
			}

			return paramSet;
		}

		void MetalGpuParameterSetPool::Reset()
		{
			if (mInformation.Mode == GpuParameterSetPoolMode::Persistent)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot perform Reset on a Persistent mode parameter set pool.");
				return;
			}

			@autoreleasepool
			{
			Lock lock(mPoolMutex);
			mAllocatedSetCount = 0;

			// B9: rewind every block's cursor. Crucially, do NOT release the MTLBuffers — subsequent
			// frames reuse them. Callers have guaranteed no in-flight command buffer references any
			// set that owned a slice in these blocks.
			for (Block& block : mBlocks)
				block.Cursor = 0;

			// Direct-allocated buffers from the persistent / large-request paths are released here
			// because they are not part of the ring. Caller contract says Reset invalidates every set
			// the pool has handed out, so the buffers backing them can drop too.
#if !__has_feature(objc_arc)
			for (id<MTLBuffer> buffer : mDirectBuffers)
			{
				if (buffer != nil)
					[buffer release];
			}
#endif
			mDirectBuffers.clear();
			} // @autoreleasepool
		}

		id<MTLBuffer> MetalGpuParameterSetPool::AcquireArgumentBufferSlice(u64 size, u32 alignment, u64& outOffset)
		{
			outOffset = 0;
			if (size == 0)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			// Persistent pools and oversized requests go through a dedicated MTLBuffer so their
			// lifetime is independent of the ring's Reset cycle. This mirrors MetalHeapAllocator's
			// large-resource fallback reasoning — we do not want a single multi-hundred-KB argument
			// buffer to pin an entire block, nor do we want a persistent set to keep a block alive
			// past its ring reset point.
			const bool useDirectPath = size > kLargeSliceThreshold;
			if (useDirectPath)
			{
				id<MTLBuffer> direct = [device newBufferWithLength:(NSUInteger)size
															options:MTLResourceStorageModeShared];
				if (direct == nil)
				{
					B3D_LOG(Error, LogRenderBackend,
						"MetalGpuParameterSetPool: direct-path argument buffer allocation failed for {0} bytes.",
						size);
					return nil;
				}

				{
					Lock lock(mPoolMutex);
					mDirectBuffers.push_back(direct);
				}
				outOffset = 0;
				return direct;
			}

			Lock lock(mPoolMutex);

			// Fast path: try to satisfy the request out of an existing block. Blocks are indexed in
			// insertion order; we scan from oldest to newest so the hottest allocations cluster in the
			// first block and later blocks only light up under pressure.
			for (Block& block : mBlocks)
			{
				const u64 alignedCursor = AlignUp(block.Cursor, alignment);
				if (alignedCursor + size <= block.Size)
				{
					block.Cursor = alignedCursor + size;
					outOffset = alignedCursor;
					return block.Buffer;
				}
			}

			// No existing block has room — grow. Block size is the max of kDefaultBlockSize and the
			// actual request so even a pathologically large (but-still-under-threshold) request always
			// fits.
			const u64 growSize = std::max<u64>(kDefaultBlockSize, size);
			Block* grown = GrowByBlock(growSize);
			if (grown == nullptr)
				return nil;

			const u64 alignedCursor = AlignUp(grown->Cursor, alignment);
			if (alignedCursor + size > grown->Size)
			{
				// Shouldn't happen — growSize was chosen to fit. Defensive guard so we never return a
				// slice that overlaps past the block boundary.
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuParameterSetPool: grown block of {0} bytes could not fit a {1}-byte request.",
					grown->Size, size);
				return nil;
			}
			grown->Cursor = alignedCursor + size;
			outOffset = alignedCursor;
			return grown->Buffer;
		}

		MetalGpuParameterSetPool::Block* MetalGpuParameterSetPool::GrowByBlock(u64 minimumSize)
		{
			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nullptr;

			id<MTLBuffer> buffer = [device newBufferWithLength:(NSUInteger)minimumSize
														options:MTLResourceStorageModeShared];
			if (buffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalGpuParameterSetPool: failed to grow ring by a {0}-byte block.",
					minimumSize);
				return nullptr;
			}

			Block block;
			block.Buffer = buffer;
			block.Size = minimumSize;
			block.Cursor = 0;
			mBlocks.push_back(block);
			return &mBlocks.back();
		}
	} // namespace render
} // namespace b3d
