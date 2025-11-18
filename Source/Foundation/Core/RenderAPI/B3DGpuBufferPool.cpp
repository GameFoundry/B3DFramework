#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

using namespace b3d::render;

void TransientGpuBufferPool::Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 initialBufferCount, u32 suballocationsPerBuffer)
{
	mDevice = &device;
	mBufferCreateInformation = createInfo;
	mSuballocationsPerBuffer = suballocationsPerBuffer;

	B3D_ASSERT(suballocationsPerBuffer > 0, "Suballocations per buffer must be greater than 0");

	// Calculate aligned suballocation size
	mSuballocationSize = b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInfo, device);

	for (u32 bufferIndex = 0; bufferIndex < initialBufferCount; bufferIndex++)
		AddNewBufferToPool();
}

GpuBufferSuballocation TransientGpuBufferPool::Allocate()
{
	B3D_ASSERT(mDevice != nullptr, "GpuBufferPool not initialized");

	// Try to pop from free list
	if (mFreeListHead != ~0u)
	{
		u32 entryIndex = mFreeListHead;
		SuballocationEntry& entry = mSuballocations[entryIndex];

		// Pop from free list
		mFreeListHead = entry.NextFreeIndex;

		entry.LastUsedFrameNumber = mCurrentFrameNumber;

		return GpuBufferSuballocation(entry.Buffer, entry.SuballocationIndex, entry.SuballocationOffset);
	}

	AddNewBufferToPool();
	return Allocate();
}

void TransientGpuBufferPool::AdvanceFrame()
{
	B3D_ASSERT(mDevice != nullptr, "GpuBufferPool not initialized");

	mCurrentFrameNumber++;

	// Rebuild free list from scratch
	// Only entries that are old enough to reuse are added to the free list
	mFreeListHead = ~0u;

	for (u32 entryIndex = 0; entryIndex < mSuballocations.size(); entryIndex++)
	{
		SuballocationEntry& entry = mSuballocations[entryIndex];

		// Add to free list if old enough to reuse
		if ((mCurrentFrameNumber - entry.LastUsedFrameNumber) >= RenderThread::kMaximumFramesInFlight)
		{
			entry.NextFreeIndex = mFreeListHead;
			mFreeListHead = entryIndex;
		}
	}
}

void TransientGpuBufferPool::AddNewBufferToPool()
{
	B3D_ASSERT(mDevice != nullptr, "GpuBufferPool not initialized");

	// Create new GpuBuffer with suballocations
	GpuBufferCreateInformation bufferCreateInformation = mBufferCreateInformation;
	bufferCreateInformation.SuballocationCount = mSuballocationsPerBuffer;

	SPtr<GpuBuffer> newBuffer = mDevice->CreateGpuBuffer(bufferCreateInformation);
	mBuffers.Add(newBuffer);

	// Get the actual aligned stride
	const u32 stride = newBuffer->GetSuballocationSize();

	// Add suballocations to pool and free-list
	const u32 baseIndex = (u32)mSuballocations.size();
	for (u32 subIndex = 0; subIndex < mSuballocationsPerBuffer; subIndex++)
	{
		SuballocationEntry entry;
		entry.Buffer = newBuffer;
		entry.SuballocationIndex = subIndex;
		entry.SuballocationOffset = subIndex * stride;
		entry.LastUsedFrameNumber = 0;

		// Link into free list (prepend)
		entry.NextFreeIndex = mFreeListHead;
		mFreeListHead = baseIndex + subIndex;

		mSuballocations.Add(entry);
	}
}
