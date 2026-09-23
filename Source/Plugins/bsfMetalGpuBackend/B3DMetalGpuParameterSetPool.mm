//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuParameterSetPool.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "B3DMetalShaderABI.h"
#include "Debug/B3DLog.h"
#include "Utility/B3DBitwise.h"

namespace b3d
{
	namespace render
	{
		MetalGpuParameterSetPool::MetalGpuParameterSetPool(MetalGpuDevice& device, const GpuParameterSetPoolCreateInformation& createInformation)
			: GpuParameterSetPool(createInformation), mDevice(device)
		{
		}

		MetalGpuParameterSetPool::~MetalGpuParameterSetPool()
		{
			// The owning work context drains its GPU work before tearing down the pool, so nothing can still read these.
#if !__has_feature(objc_arc)
			for (Block& block : mBlocks)
				[block.Buffer release];

			for (id<MTLBuffer> buffer : mDirectBuffers)
				[buffer release];
#endif

			mBlocks.clear();
			mDirectBuffers.clear();
		}

		TShared<GpuParameterSet> MetalGpuParameterSetPool::Create(const TShared<GpuPipelineParameterSetLayout>& layout, u32 setIndex, bool deferredInitialize)
		{
			if (setIndex > kMetalMaximumParameterSetIndex)
			{
				B3D_LOG(Error, LogRenderBackend, "Metal parameter set index {0} exceeds the supported maximum of {1}.", setIndex, kMetalMaximumParameterSetIndex);
				return nullptr;
			}

			const bool isTransient = mInformation.Mode == GpuParameterSetPoolMode::Transient;
			if (isTransient)
			{
				if (mAllocatedSetCount >= mInformation.MaxSets)
					return nullptr;

				mAllocatedSetCount++;
			}

			auto parameterSet = B3DMakeShared<MetalGpuParameters>(mDevice, layout, setIndex, isTransient ? this : nullptr);
			parameterSet->SetShared(parameterSet);

			if (!deferredInitialize)
			{
				parameterSet->Initialize();
				if (!parameterSet->IsArgumentBufferAllocated())
					return nullptr;
			}

			return parameterSet;
		}

		void MetalGpuParameterSetPool::Reset()
		{
			if (mInformation.Mode == GpuParameterSetPoolMode::Persistent)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot perform Reset on a Persistent mode parameter set pool.");
				return;
			}

			mAllocatedSetCount = 0;

			// Rewind the ring but keep its blocks, so subsequent frames reuse them. The caller guarantees no in-flight
			// command buffer references any set that owned a slice.
			for (Block& block : mBlocks)
				block.Cursor = 0;

			// Dedicated buffers are not part of the ring. Reset invalidates every set the pool handed out, so they can go.
#if !__has_feature(objc_arc)
			for (id<MTLBuffer> buffer : mDirectBuffers)
				[buffer release];
#endif
			mDirectBuffers.clear();
		}

		id<MTLBuffer> MetalGpuParameterSetPool::AcquireArgumentBufferSlice(u64 size, u32 alignment, u64& outOffset)
		{
			outOffset = 0;
			if (size == 0)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			// TODO - Allocate ring blocks (GrowByBlock) and the dedicated buffers below through MetalHeapAllocator::AllocateBuffer.
			// Oversized requests get a dedicated buffer rather than pinning most of a block.
			if (size > kLargeSliceThreshold)
			{
				id<MTLBuffer> direct = [device newBufferWithLength:(NSUInteger)size options:MTLResourceStorageModeShared];
				if (direct == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "MetalGpuParameterSetPool: direct-path argument buffer allocation failed for {0} bytes.", size);
					return nil;
				}

				mDirectBuffers.push_back(direct);
				return direct;
			}

			// Scan from oldest to newest so allocations cluster in the first block and later blocks only light up
			// under pressure.
			for (Block& block : mBlocks)
			{
				const u64 alignedCursor = Bitwise::AlignUp<u64>(block.Cursor, alignment);
				if (alignedCursor + size <= block.Size)
				{
					block.Cursor = alignedCursor + size;
					outOffset = alignedCursor;
					return block.Buffer;
				}
			}

			// No existing block has room. A fresh block starts at offset zero, which satisfies any alignment, and is at
			// least as large as the request.
			Block* grown = GrowByBlock(std::max<u64>(kDefaultBlockSize, size));
			if (grown == nullptr)
				return nil;

			grown->Cursor = size;
			return grown->Buffer;
		}

		MetalGpuParameterSetPool::Block* MetalGpuParameterSetPool::GrowByBlock(u64 minimumSize)
		{
			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nullptr;

			id<MTLBuffer> buffer = [device newBufferWithLength:(NSUInteger)minimumSize options:MTLResourceStorageModeShared];
			if (buffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalGpuParameterSetPool: failed to grow ring by a {0}-byte block.", minimumSize);
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
