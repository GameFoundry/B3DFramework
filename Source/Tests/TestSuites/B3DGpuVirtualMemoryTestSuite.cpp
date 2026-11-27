//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuVirtualMemoryTestSuite.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuPageMappingQueue.h"
#include "GpuBackend/B3DGpuPageTable.h"
#include "GpuBackend/B3DGpuVirtualMemory.h"
#include "GpuBackend/Allocators/B3DGpuPagePool.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Image/B3DTexture.h"
#include "Renderer/B3DRenderer.h"

using namespace b3d;

GpuVirtualMemoryTestSuite::GpuVirtualMemoryTestSuite()
	: TestSuite("GpuVirtualMemoryTestSuite")
{
	B3D_ADD_TEST(GpuVirtualMemoryTestSuite::TestPagePool)
	B3D_ADD_TEST(GpuVirtualMemoryTestSuite::TestTextureLayout)
	B3D_ADD_TEST(GpuVirtualMemoryTestSuite::TestPageTable)
	B3D_ADD_TEST(GpuVirtualMemoryTestSuite::TestVirtualTexture)
}

namespace
{
	/** Pages of a page pool heap in the tests. */
	constexpr u32 kHeapPageCount = 16;

	/** Pages of the virtual buffer in the tests. */
	constexpr u32 kPageCount = 10;

	/** Number of 4 byte elements of the virtual buffer in the tests. */
	constexpr u32 kElementCount = kPageCount * kGpuPageSize / sizeof(u32);

	/** Heap of TestHeapBackend. */
	struct TestHeap : IGpuHeap
	{ };

	/** Heap backend without memory that counts its heaps, and fails to create more than a maximum number of heaps. */
	class TestHeapBackend
	{
	public:
		using HeapHandle = IGpuHeap*;

		struct HeapCreateInformation
		{ };

		TestHeapBackend(u32 maximumHeapCount)
			: mMaximumHeapCount(maximumHeapCount)
		{ }

		IGpuHeap* CreateHeap(u64 sizeInBytes, const HeapCreateInformation&)
		{
			if(mLiveHeapCount == mMaximumHeapCount)
				return nullptr;

			B3D_ASSERT(sizeInBytes == (u64)kHeapPageCount * kGpuPageSize);

			mCreatedHeapCount++;
			mLiveHeapCount++;
			return B3DNew<TestHeap>();
		}

		void DestroyHeap(IGpuHeap* heap)
		{
			B3DDelete(static_cast<TestHeap*>(heap));
			mDestroyedHeapCount++;
			mLiveHeapCount--;
		}

		u32 GetCreatedHeapCount() const { return mCreatedHeapCount; }
		u32 GetDestroyedHeapCount() const { return mDestroyedHeapCount; }
		u32 GetLiveHeapCount() const { return mLiveHeapCount; }

	private:
		u32 mMaximumHeapCount;
		u32 mCreatedHeapCount = 0;
		u32 mDestroyedHeapCount = 0;
		u32 mLiveHeapCount = 0;
	};

	/** Completion tracker whose frames advance and complete when the test says so. */
	class TestCompletionTracker : public IGpuCompletionTracker
	{
	public:
		u64 GetCurrentMarker() const override { return mCurrentFrame; }
		bool IsMarkerComplete(u64 marker) const override { return mHasCompletedFrames && marker <= mCompletedFrame; }

		/** Starts the next frame. */
		void AdvanceFrame() { mCurrentFrame++; }

		/** Completes all frames up to and including @p frame. */
		void CompleteFrame(u64 frame)
		{
			mCompletedFrame = frame;
			mHasCompletedFrames = true;
		}

	private:
		u64 mCurrentFrame = 0;
		u64 mCompletedFrame = 0;
		bool mHasCompletedFrames = false;
	};

	using PagePool = TGpuPagePool<TestHeapBackend>;

	/** Sets the page pool configuration variables, and restores their earlier values when destroyed. */
	class PagePoolConfigurationScope
	{
	public:
		PagePoolConfigurationScope()
			: mHeapSize(gGpuVirtualMemoryPageHeapSize), mIdleFrames(gGpuVirtualMemoryPageHeapIdleFrames)
		{
			gGpuVirtualMemoryPageHeapSize.Set(kHeapPageCount * kGpuPageSize / (1024 * 1024));
			gGpuVirtualMemoryPageHeapIdleFrames.Set(2);
		}

		~PagePoolConfigurationScope()
		{
			gGpuVirtualMemoryPageHeapSize.Set(mHeapSize);
			gGpuVirtualMemoryPageHeapIdleFrames.Set(mIdleFrames);
		}

	private:
		u32 mHeapSize;
		u32 mIdleFrames;
	};

	/** Returns true if @p allocation has @p pageCount pages of @p heap, starting at @p firstPageIndex. */
	bool IsAllocation(const GpuPageAllocation& allocation, IGpuHeap* heap, u32 firstPageIndex, u32 pageCount)
	{
		return allocation.Heap == heap && allocation.Pages.FirstPageIndex == firstPageIndex && allocation.Pages.PageCount == pageCount;
	}

	/** Returns the pages allocated from the page pool of the device's first memory type. */
	u32 GetAllocatedPageCount(GpuDevice& device)
	{
		return device.GetPagePool(0)->GetStatistics().AllocatedPageCount;
	}

	/** Ends enough frames on the render thread for the GPU to complete the frames before. Render thread only. */
	void CompleteFrames(GpuDevice& device)
	{
		const TShared<render::Renderer> renderer = render::GetRenderer();
		for(u32 frameIndex = 0; frameIndex <= RenderThread::kMaximumFramesInFlight; frameIndex++)
		{
			renderer->BeginFrame();
			renderer->EndFrame();
		}

		device.WaitUntilIdle();
	}
}

void GpuVirtualMemoryTestSuite::TestPagePool()
{
	const PagePoolConfigurationScope configurationScope;

	{
		TestHeapBackend backend(~0u);
		TestCompletionTracker tracker;

		PagePool::Configuration configuration;
		configuration.CompletionTracker = &tracker;
		PagePool pool(&backend, configuration);

		TArray<GpuPageAllocation> allocations;
		B3D_TEST_ASSERT(pool.TryAllocate(0, allocations) && allocations.Empty())
		B3D_TEST_ASSERT(backend.GetCreatedHeapCount() == 0)

		// The first request creates a heap and takes the start of it
		B3D_TEST_ASSERT(pool.TryAllocate(4, allocations) && allocations.Size() == 1)
		IGpuHeap* const firstHeap = allocations[0].Heap;
		B3D_TEST_ASSERT(IsAllocation(allocations[0], firstHeap, 0, 4))

		GpuPagePoolStatistics statistics = pool.GetStatistics();
		B3D_TEST_ASSERT(statistics.HeapCount == 1 && statistics.PageCount == kHeapPageCount && statistics.AllocatedPageCount == 4)
		const GpuPageAllocation first = allocations[0];

		// A request no free run holds adds a heap for the missing pages, and takes the longest runs first
		allocations.Clear();
		B3D_TEST_ASSERT(pool.TryAllocate(20, allocations) && allocations.Size() == 2)
		IGpuHeap* const secondHeap = allocations[0].Heap;
		B3D_TEST_ASSERT(secondHeap != firstHeap && IsAllocation(allocations[0], secondHeap, 0, kHeapPageCount))
		B3D_TEST_ASSERT(IsAllocation(allocations[1], firstHeap, 4, 4))
		B3D_TEST_ASSERT(backend.GetCreatedHeapCount() == 2)
		const GpuPageAllocation second = allocations[0];
		const GpuPageAllocation third = allocations[1];

		// Reclaimed pages are free at once, and the smallest free run that holds a request is taken
		pool.FreeAndReclaim(first);
		B3D_TEST_ASSERT(pool.GetStatistics().AllocatedPageCount == 20)

		allocations.Clear();
		B3D_TEST_ASSERT(pool.TryAllocate(3, allocations) && allocations.Size() == 1)
		B3D_TEST_ASSERT(IsAllocation(allocations[0], firstHeap, 0, 3))
		const GpuPageAllocation fourth = allocations[0];

		// Freed pages are free once the GPU completes the frame they were freed in
		const u64 freeFrame = tracker.GetCurrentMarker();
		pool.Free(second);
		pool.ReclaimUnused();
		B3D_TEST_ASSERT(pool.GetStatistics().AllocatedPageCount == 23)

		tracker.AdvanceFrame();
		tracker.CompleteFrame(freeFrame);
		pool.ReclaimUnused();
		B3D_TEST_ASSERT(pool.GetStatistics().AllocatedPageCount == 7)

		// Heaps without allocated pages are destroyed once they went unused for the idle frames and the GPU completed the last frame that
		// used them, except for one
		allocations.Clear();
		B3D_TEST_ASSERT(pool.TryAllocate(kHeapPageCount, allocations) && allocations.Size() == 1 && allocations[0].Heap == secondHeap)
		const GpuPageAllocation fifth = allocations[0];

		const u64 reclaimFrame = tracker.GetCurrentMarker();
		pool.FreeAndReclaim(third);
		pool.FreeAndReclaim(fourth);
		pool.FreeAndReclaim(fifth);
		pool.ReclaimUnused();
		B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 2)

		tracker.AdvanceFrame();
		tracker.AdvanceFrame();
		pool.ReclaimUnused();
		B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 2 && backend.GetLiveHeapCount() == 2)

		tracker.CompleteFrame(reclaimFrame);
		pool.ReclaimUnused();
		statistics = pool.GetStatistics();
		B3D_TEST_ASSERT(statistics.HeapCount == 1 && statistics.PageCount == kHeapPageCount && statistics.AllocatedPageCount == 0)
		B3D_TEST_ASSERT(backend.GetLiveHeapCount() == 1 && backend.GetDestroyedHeapCount() == 1)

		// The kept heap serves the next request, and stays however long it goes unused
		allocations.Clear();
		B3D_TEST_ASSERT(pool.TryAllocate(kHeapPageCount, allocations) && allocations.Size() == 1)
		B3D_TEST_ASSERT(backend.GetCreatedHeapCount() == 2)

		pool.FreeAndReclaim(allocations[0]);
		for(u32 frameIndex = 0; frameIndex < 4; frameIndex++)
		{
			tracker.AdvanceFrame();
			tracker.CompleteFrame(tracker.GetCurrentMarker());
			pool.ReclaimUnused();
		}

		B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 1 && backend.GetLiveHeapCount() == 1)
	}

	// A request fails with nothing allocated if the heaps it needs cannot all be created
	{
		TestHeapBackend backend(2);
		TestCompletionTracker tracker;

		PagePool::Configuration configuration;
		configuration.CompletionTracker = &tracker;
		PagePool pool(&backend, configuration);

		TArray<GpuPageAllocation> allocations;
		B3D_TEST_ASSERT(!pool.TryAllocate(3 * kHeapPageCount, allocations) && allocations.Empty())
		B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 0)
		B3D_TEST_ASSERT(backend.GetCreatedHeapCount() == 2 && backend.GetLiveHeapCount() == 0)

		B3D_TEST_ASSERT(pool.TryAllocate(kHeapPageCount + 4, allocations) && allocations.Size() == 2)
		B3D_TEST_ASSERT(pool.GetStatistics().AllocatedPageCount == kHeapPageCount + 4)

		for(const GpuPageAllocation& allocation : allocations)
			pool.FreeAndReclaim(allocation);
	}

	// Without spares, every heap without allocated pages is destroyed
	{
		TestHeapBackend backend(~0u);
		TestCompletionTracker tracker;

		PagePool::Configuration configuration;
		configuration.CompletionTracker = &tracker;
		configuration.MaxEmptyHeapCount = 0;
		PagePool pool(&backend, configuration);

		TArray<GpuPageAllocation> allocations;
		B3D_TEST_ASSERT(pool.TryAllocate(1, allocations) && allocations.Size() == 1)
		pool.FreeAndReclaim(allocations[0]);

		tracker.AdvanceFrame();
		tracker.AdvanceFrame();
		tracker.CompleteFrame(tracker.GetCurrentMarker());
		pool.ReclaimUnused();
		B3D_TEST_ASSERT(pool.GetStatistics().HeapCount == 0 && backend.GetLiveHeapCount() == 0)
	}
}

void GpuVirtualMemoryTestSuite::TestTextureLayout()
{
	// A 1024x1024 texture of 4 byte texels with two layers, and a tail per layer from the 64x64 mip level down
	GpuVirtualTextureLayout layout;
	layout.Size = Vector3I(1024, 1024, 1);
	layout.MipLevelCount = 11;
	layout.TileSize = Vector3I(128, 128, 1);
	layout.FirstTailMipLevel = 4;
	layout.TailPageCount = 1;
	layout.UpdatePageCounts(2);

	B3D_TEST_ASSERT(layout.GetTileCount(0) == Vector3I(8, 8, 1) && layout.GetTileCount(3) == Vector3I(1, 1, 1))
	B3D_TEST_ASSERT(layout.PageCountPerLayer == 86 && layout.PageCount == 172)

	// Per layer: the tail, then the mip levels from the smallest
	const auto fnIsRange = [](const GpuPageRange& range, u32 firstPageIndex, u32 pageCount)
	{
		return range.FirstPageIndex == firstPageIndex && range.PageCount == pageCount;
	};

	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(0, 7), 0, 1))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(0, 3), 1, 1))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(1, 4), 86, 1))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(1, 0), 108, 64))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipChainPages(1, 2), 86, 6))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipChainPages(1, 0), 86, 86))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipChainPages(0, 9), 0, 1))

	// Tiles are in raster order
	B3D_TEST_ASSERT(layout.GetPageIndex(0, 1, Vector3I(3, 2, 0)) == 6 + 2 * 4 + 3)
	B3D_TEST_ASSERT(layout.GetPageIndex(1, 0, Vector3I(7, 7, 0)) == 108 + 63)

	// A shared tail comes before all layers, and is not part of their mip chains
	layout.HasSharedTail = true;
	layout.UpdatePageCounts(2);
	B3D_TEST_ASSERT(layout.PageCountPerLayer == 85 && layout.PageCount == 171)
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(1, 5), 0, 1))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipLevelPages(1, 3), 86, 1))
	B3D_TEST_ASSERT(fnIsRange(layout.GetMipChainPages(1, 0), 86, 85))

	// Volume tiles split the pages over depth too
	GpuVirtualTextureLayout volumeLayout;
	volumeLayout.Size = Vector3I(64, 64, 64);
	volumeLayout.MipLevelCount = 7;
	volumeLayout.TileSize = Vector3I(32, 32, 16);
	volumeLayout.FirstTailMipLevel = 2;
	volumeLayout.TailPageCount = 1;
	volumeLayout.UpdatePageCounts(1);
	B3D_TEST_ASSERT(volumeLayout.GetTileCount(0) == Vector3I(2, 2, 4) && volumeLayout.GetTileCount(1) == Vector3I(1, 1, 2))
	B3D_TEST_ASSERT(volumeLayout.PageCount == 1 + 2 + 16)
	B3D_TEST_ASSERT(volumeLayout.GetPageIndex(0, 0, Vector3I(1, 0, 3)) == 3 + (3 * 2 + 0) * 2 + 1)

	// The Null device uses the standard tile shapes, and packs the mip levels smaller than a tile in the tail
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	if(device->GetCapabilities().BackendName != "Null" || !device->GetCapabilities().HasCapability(RSC_VIRTUAL_TEXTURES))
		return;

	struct TileSizeCase
	{
		PixelFormat Format;
		TextureType Type;
		Vector3I TileSize;
	};

	const TileSizeCase tileSizeCases[] =
	{
		{ PF_R8, TEX_TYPE_2D, Vector3I(256, 256, 1) },
		{ PF_RG8, TEX_TYPE_2D, Vector3I(256, 128, 1) },
		{ PF_RGBA8, TEX_TYPE_2D, Vector3I(128, 128, 1) },
		{ PF_RGBA16F, TEX_TYPE_2D, Vector3I(128, 64, 1) },
		{ PF_RGBA32F, TEX_TYPE_2D, Vector3I(64, 64, 1) },
		{ PF_BC1, TEX_TYPE_2D, Vector3I(512, 256, 1) },
		{ PF_BC7, TEX_TYPE_2D, Vector3I(256, 256, 1) },
		{ PF_R8, TEX_TYPE_3D, Vector3I(64, 32, 32) },
		{ PF_RGBA8, TEX_TYPE_3D, Vector3I(32, 32, 16) },
		{ PF_RGBA32F, TEX_TYPE_3D, Vector3I(16, 16, 16) },
	};

	for(const TileSizeCase& tileSizeCase : tileSizeCases)
	{
		TextureCreateInformation createInformation;
		createInformation.Type = tileSizeCase.Type;
		createInformation.Format = tileSizeCase.Format;
		createInformation.Width = 256;
		createInformation.Height = 256;
		createInformation.Depth = tileSizeCase.Type == TEX_TYPE_3D ? 64 : 1;

		GpuVirtualTextureLayout deviceLayout;
		B3D_TEST_ASSERT(device->TryGetVirtualTextureLayout(createInformation, deviceLayout))
		B3D_TEST_ASSERT(deviceLayout.TileSize == tileSizeCase.TileSize)
	}

	// 1024x1024 of 4 byte texels as above: the 64x64 mip level and smaller ones, 21844 bytes, share one tail page
	TextureCreateInformation createInformation;
	createInformation.Format = PF_RGBA8;
	createInformation.Width = 1024;
	createInformation.Height = 1024;
	createInformation.MipMapCount = 10;
	createInformation.ArraySliceCount = 2;

	GpuVirtualTextureLayout deviceLayout;
	B3D_TEST_ASSERT(device->TryGetVirtualTextureLayout(createInformation, deviceLayout))
	B3D_TEST_ASSERT(deviceLayout.FirstTailMipLevel == 4 && deviceLayout.TailPageCount == 1 && !deviceLayout.HasSharedTail)
	B3D_TEST_ASSERT(deviceLayout.PageCountPerLayer == 86 && deviceLayout.PageCount == 172)

	// Without mip levels smaller than a tile, there is no tail
	createInformation.MipMapCount = 2;
	B3D_TEST_ASSERT(device->TryGetVirtualTextureLayout(createInformation, deviceLayout))
	B3D_TEST_ASSERT(deviceLayout.FirstTailMipLevel == 3 && deviceLayout.TailPageCount == 0)
	B3D_TEST_ASSERT(deviceLayout.PageCountPerLayer == 64 + 16 + 4)

	// Multisampled textures have no layout
	createInformation.MipMapCount = 0;
	createInformation.SampleCount = 4;
	B3D_TEST_ASSERT(!device->TryGetVirtualTextureLayout(createInformation, deviceLayout))
}

void GpuVirtualMemoryTestSuite::TestPageTable()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	if(!device->GetCapabilities().HasCapability(RSC_VIRTUAL_BUFFERS))
	{
		LoggingScope logs(*this);
		logs.ExpectError("the GPU device doesn't support virtual buffers.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount, GpuBufferFlag::Virtual)) == nullptr)
		return;
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("A virtual buffer must be in GpuBufferMemory::Device memory");

		GpuBufferCreateInformation uploadInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount, GpuBufferFlag::Virtual);
		uploadInformation.Memory = GpuBufferMemory::Mapped;
		B3D_TEST_ASSERT(device->CreateGpuBuffer(uploadInformation) == nullptr)
	}

	GetRenderThread().PostCommand([this, &device]()
	{
		const u32 initialAllocatedPageCount = GetAllocatedPageCount(*device);

		TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount, GpuBufferFlag::Virtual));
		B3D_TEST_ASSERT(buffer != nullptr && buffer->GetPageTable() != nullptr)
		if(buffer == nullptr || buffer->GetPageTable() == nullptr)
			return;

		GpuPageTable& pageTable = *buffer->GetPageTable();
		B3D_TEST_ASSERT(pageTable.GetPageCount() == kPageCount && pageTable.GetCommittedPageCount() == 0)
		B3D_TEST_ASSERT(!pageTable.IsCommitted(GpuPageRange(0, 1)) && pageTable.IsCommitted(GpuPageRange(0, 0)))

		// Committed pages are mapped ahead of the next submission, or by a flush
		B3D_TEST_ASSERT(pageTable.TryCommit(GpuPageRange(2, 3)))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == 3)
		B3D_TEST_ASSERT(pageTable.IsCommitted(GpuPageRange(2, 3)) && !pageTable.IsCommitted(GpuPageRange(1, 3)) && !pageTable.IsCommitted(GpuPageRange(4, 2)))

		device->FlushPageMappings();
		B3D_TEST_ASSERT(!device->GetPageMappingQueue().HasQueuedMappings())

		// Committing committed pages again only gives memory to the ones without
		B3D_TEST_ASSERT(pageTable.TryCommit(GpuPageRange(0, kPageCount)))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount && pageTable.IsCommitted(GpuPageRange(0, kPageCount)))
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

		// Decommitted memory goes back to the pool once the GPU completes the frame of the decommit, unless the pages are committed again
		// before that, which gives them their old memory back
		pageTable.Decommit(GpuPageRange(4, 2));
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 2 && !pageTable.IsCommitted(GpuPageRange(5, 1)))
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

		B3D_TEST_ASSERT(pageTable.TryCommit(GpuPageRange(4, 1)))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 1)
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

		CompleteFrames(*device);
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 1 && pageTable.IsCommitted(GpuPageRange(4, 1)))
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount - 1)

		// Memory from an allocation can't be given to pages with memory, or with a pending removal until the frame of the removal completes
		IGpuPagePool& pagePool = *device->GetPagePool(0);
		TArray<GpuPageAllocation> externalAllocations;
		B3D_TEST_ASSERT(pagePool.TryAllocate(2, externalAllocations) && externalAllocations.Size() == 1)
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount + 1)

		B3D_TEST_ASSERT(!pageTable.TryCommit(5, externalAllocations[0]))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 1)

		pageTable.Decommit(GpuPageRange(6, 1));
		B3D_TEST_ASSERT(!pageTable.TryCommit(5, externalAllocations[0]))

		CompleteFrames(*device);
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

		// Memory from an allocation is seen at once, isn't returned to the pool, and pool memory can't replace it until the frame of its
		// removal completes
		B3D_TEST_ASSERT(pageTable.TryCommit(5, externalAllocations[0]))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount && pageTable.IsCommitted(GpuPageRange(5, 2)))
		B3D_TEST_ASSERT(device->GetPageMappingQueue().HasQueuedMappings())

		pageTable.Decommit(GpuPageRange(5, 2));
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 2 && !pageTable.TryCommit(5, externalAllocations[0]))
		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot commit pages 5 to 6, since memory from a GpuPageAllocation was decommitted from some of them in a frame the GPU hasn't finished.");
			B3D_TEST_ASSERT(!pageTable.TryCommit(GpuPageRange(5, 1)))
			B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 2)
		}

		CompleteFrames(*device);
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

		B3D_TEST_ASSERT(pageTable.TryCommit(5, externalAllocations[0]))
		pageTable.Decommit(GpuPageRange(5, 2));
		CompleteFrames(*device);

		pagePool.Free(externalAllocations[0]);

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot commit pages 9 to 11 of a page table with 10 pages.");
			B3D_TEST_ASSERT(!pageTable.TryCommit(GpuPageRange(9, 2)))
			B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == kPageCount - 2)
		}

		// A destroyed buffer returns its memory once the GPU completes the work submitted before
		buffer = nullptr;
		CompleteFrames(*device);
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount)

		// Commits and decommits of every other page, filled in by commits of whole ranges
		{
			TShared<render::GpuBuffer> scatteredBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount, GpuBufferFlag::Virtual));
			GpuPageTable& scatteredPageTable = *scatteredBuffer->GetPageTable();

			for(u32 pageIndex = 1; pageIndex < kPageCount; pageIndex += 2)
				B3D_TEST_ASSERT(scatteredPageTable.TryCommit(GpuPageRange(pageIndex, 1)))

			B3D_TEST_ASSERT(scatteredPageTable.GetCommittedPageCount() == kPageCount / 2)
			B3D_TEST_ASSERT(scatteredPageTable.IsCommitted(GpuPageRange(3, 1)) && !scatteredPageTable.IsCommitted(GpuPageRange(3, 2)) && !scatteredPageTable.IsCommitted(GpuPageRange(0, 1)))

			B3D_TEST_ASSERT(scatteredPageTable.TryCommit(GpuPageRange(0, kPageCount)))
			B3D_TEST_ASSERT(scatteredPageTable.IsCommitted(GpuPageRange(0, kPageCount)))
			B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

			// Re-commits of the decommitted pages give all of them their old memory back
			for(u32 pageIndex = 1; pageIndex < kPageCount - 1; pageIndex += 2)
				scatteredPageTable.Decommit(GpuPageRange(pageIndex, 1));

			B3D_TEST_ASSERT(scatteredPageTable.GetCommittedPageCount() == kPageCount - 4 && !scatteredPageTable.IsCommitted(GpuPageRange(0, 2)))

			B3D_TEST_ASSERT(scatteredPageTable.TryCommit(GpuPageRange(1, kPageCount - 2)))
			B3D_TEST_ASSERT(scatteredPageTable.GetCommittedPageCount() == kPageCount && scatteredPageTable.IsCommitted(GpuPageRange(0, kPageCount)))
			B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount + kPageCount)

			scatteredPageTable.Decommit(GpuPageRange(0, kPageCount));
			B3D_TEST_ASSERT(scatteredPageTable.GetCommittedPageCount() == 0)

			CompleteFrames(*device);
			B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount)
		}
	}, "GpuVirtualMemoryTestSuite::TestPageTable", true);
}

void GpuVirtualMemoryTestSuite::TestVirtualTexture()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	TextureCreateInformation createInformation;
	createInformation.Name = "Virtual texture";
	createInformation.Format = PF_RGBA8;
	createInformation.Width = 1024;
	createInformation.Height = 1024;
	createInformation.MipMapCount = 10;
	createInformation.Usage = TextureUsageFlag::Default | TextureUsageFlag::Virtual;

	if(!device->GetCapabilities().HasCapability(RSC_VIRTUAL_TEXTURES))
	{
		LoggingScope logs(*this);
		logs.ExpectError("the GPU device doesn't support virtual textures.");
		B3D_TEST_ASSERT(device->CreateTexture(createInformation) == nullptr)
		return;
	}

	GetRenderThread().PostCommand([this, &device, &createInformation]()
	{
		const u32 initialAllocatedPageCount = GetAllocatedPageCount(*device);

		TShared<render::Texture> texture = device->CreateTexture(createInformation);
		B3D_TEST_ASSERT(texture != nullptr && texture->GetPageTable() != nullptr)
		if(texture == nullptr || texture->GetPageTable() == nullptr)
			return;

		GpuPageTable& pageTable = *texture->GetPageTable();
		const GpuVirtualTextureLayout& layout = pageTable.GetTextureLayout();
		B3D_TEST_ASSERT(layout.PageCount > 0 && pageTable.GetPageCount() == layout.PageCount)

		// Streaming in from the smallest mip level commits the pages next to the committed ones
		const GpuPageRange tailPages = layout.GetMipChainPages(0, layout.FirstTailMipLevel);
		const GpuPageRange mipChainPages = layout.GetMipChainPages(0, 1);
		B3D_TEST_ASSERT(pageTable.TryCommit(tailPages) && pageTable.TryCommit(mipChainPages))
		B3D_TEST_ASSERT(pageTable.GetCommittedPageCount() == mipChainPages.PageCount)
		B3D_TEST_ASSERT(!pageTable.IsCommitted(layout.GetMipLevelPages(0, 0)))

		texture = nullptr;
		CompleteFrames(*device);
		B3D_TEST_ASSERT(GetAllocatedPageCount(*device) == initialAllocatedPageCount)
	}, "GpuVirtualMemoryTestSuite::TestVirtualTexture", true);

	{
		LoggingScope logs(*this);
		logs.ExpectError("Virtual texture 'Virtual texture' must be stored on the GPU.");

		TextureCreateInformation cpuInformation = createInformation;
		cpuInformation.Usage |= TextureUsageFlag::StoreOnCPUWithGPUAccess;
		B3D_TEST_ASSERT(device->CreateTexture(cpuInformation) == nullptr)
	}
}
