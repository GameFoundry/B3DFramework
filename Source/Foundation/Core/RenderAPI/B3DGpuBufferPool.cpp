#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

using namespace b3d::render;

TrackedGpuBufferSuballocation::TrackedGpuBufferSuballocation(GpuBufferPool* owner, const GpuBufferSuballocation& base)
	: GpuBufferSuballocation(base) , mOwner(owner)
{
}

TrackedGpuBufferSuballocation::~TrackedGpuBufferSuballocation()
{
	if (mOwner != nullptr)
		mOwner->Release(*this);
}

TrackedGpuBufferSuballocation::TrackedGpuBufferSuballocation(TrackedGpuBufferSuballocation&& other) noexcept
	: GpuBufferSuballocation(std::move(other))
	, mOwner(other.mOwner)
{
	other.mOwner = nullptr;
}

TrackedGpuBufferSuballocation& TrackedGpuBufferSuballocation::operator=(TrackedGpuBufferSuballocation&& other) noexcept
{
	if (this != &other)
	{
		if (mOwner != nullptr)
			mOwner->Release(*this);

		GpuBufferSuballocation::operator=(std::move(other));

		mOwner = other.mOwner;
		other.mOwner = nullptr;
	}
	return *this;
}

TransientGpuBufferPool::TransientGpuBufferPool(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount)
{
	Initialize(device, createInfo, suballocationsPerBuffer, initialBufferCount);
}

void TransientGpuBufferPool::Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount)
{
	mDevice = &device;
	mBufferCreateInformation = createInfo;
	mBufferCreateInformation.SuballocationCount = suballocationsPerBuffer;
	mSuballocationsPerBuffer = suballocationsPerBuffer;

	B3D_ASSERT(suballocationsPerBuffer > 0 && "Suballocations per buffer must be greater than 0");

	// Calculate aligned suballocation size
	mSuballocationSize = b3d::GpuBuffer::CalculateSuballocatedBufferSize(mBufferCreateInformation, device);

	for (u32 bufferIndex = 0; bufferIndex < initialBufferCount; bufferIndex++)
		AddNewBufferToPool();
}

GpuBufferSuballocation TransientGpuBufferPool::Allocate()
{
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");

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
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");

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
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");

	// Create new GpuBuffer with suballocations
	SPtr<GpuBuffer> newBuffer = mDevice->CreateGpuBuffer(mBufferCreateInformation);
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

void GpuBufferPool::Initialize(GpuDevice& device, const GpuBufferCreateInformation& createInfo, u32 suballocationsPerBuffer, u32 initialBufferCount)
{
	mDevice = &device;
	mBufferCreateInformation = createInfo;
	mBufferCreateInformation.SuballocationCount = suballocationsPerBuffer;
	mSuballocationsPerBuffer = suballocationsPerBuffer;

	B3D_ASSERT(suballocationsPerBuffer > 0 && "Suballocations per buffer must be greater than 0");

	// Calculate aligned suballocation size
	mSuballocationSize = b3d::GpuBuffer::CalculateSuballocatedBufferSize(mBufferCreateInformation, device);

	for (u32 bufferIndex = 0; bufferIndex < initialBufferCount; bufferIndex++)
		AddNewBufferToPool();
}

GpuBufferSuballocation GpuBufferPool::Allocate()
{
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");

	// Find first buffer with free entries
	for (u32 bufferIndex = 0; bufferIndex < mBuffers.size(); bufferIndex++)
	{
		BufferEntry& bufferEntry = mBuffers[bufferIndex];

		if (bufferEntry.Buffer == nullptr)
			continue;

		if (bufferEntry.FreeListHead != ~0u)
		{
			const u32 globalSuballocationIndex = bufferEntry.FreeListHead;
			SuballocationEntry& suballocationEntry = mSuballocations[globalSuballocationIndex];

			bufferEntry.FreeListHead = suballocationEntry.NextFreeIndex;
			bufferEntry.AllocatedCount++;

			const u32 baseGlobalSuballocationIndex = bufferIndex * mSuballocationsPerBuffer;
			const u32 suballocationIndex = globalSuballocationIndex - baseGlobalSuballocationIndex;
			const u32 suballocationOffset = suballocationIndex * mSuballocationSize;
			return GpuBufferSuballocation(bufferEntry.Buffer, suballocationIndex, suballocationOffset);
		}
	}

	// No free entries in any buffer, grow
	AddNewBufferToPool();
	return Allocate();
}

b3d::UPtr<TrackedGpuBufferSuballocation> GpuBufferPool::AllocateTracked()
{
	GpuBufferSuballocation allocation = Allocate();
	return b3d::B3DMakeUnique<TrackedGpuBufferSuballocation>(this, allocation);
}

void GpuBufferPool::Release(const GpuBufferSuballocation& allocation)
{
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");
	B3D_ASSERT(allocation.IsValid() && "Cannot release invalid allocation");

	// Find the buffer entry. Not using a map since number of buffers is expected to be small (increase suballocation count if that's not the case).
	const SPtr<GpuBuffer>& buffer = allocation.GetBuffer();
	u32 foundBufferIndex = ~0u;

	for (u32 bufferIndex = 0; bufferIndex < mBuffers.size(); bufferIndex++)
	{
		if (mBuffers[bufferIndex].Buffer == buffer)
		{
			foundBufferIndex = bufferIndex;
			break;
		}
	}

	B3D_ASSERT(foundBufferIndex != ~0u && "Buffer not found in pool");

	// Calculate entry index directly from base + offset
	BufferEntry& bufferEntry = mBuffers[foundBufferIndex];

	const u32 baseGlobalSuballocationIndex = foundBufferIndex * mSuballocationsPerBuffer;
	const u32 globalSuballocationIndex = baseGlobalSuballocationIndex + allocation.GetSuballocationIndex();

	SuballocationEntry& suballocationEntry = mSuballocations[globalSuballocationIndex];
	B3D_ASSERT(bufferEntry.AllocatedCount > 0 && "Buffer allocation count underflow");

	bufferEntry.AllocatedCount--;

	suballocationEntry.NextFreeIndex = bufferEntry.FreeListHead;
	bufferEntry.FreeListHead = globalSuballocationIndex;

	// If buffer is now empty, mark it as deleted (keep in array for reuse)
	if (bufferEntry.AllocatedCount == 0)
	{
		// Mark buffer slot as deleted (keep in mBuffers array for reuse)
		bufferEntry.Buffer = nullptr;
	}
}

void GpuBufferPool::AddNewBufferToPool()
{
	B3D_ASSERT(mDevice != nullptr && "GpuBufferPool not initialized");

	// Create new GpuBuffer with suballocations
	SPtr<GpuBuffer> newBuffer = mDevice->CreateGpuBuffer(mBufferCreateInformation);

	// Lambda to initialize suballocations for a buffer entry
	auto fnInitializeBufferEntry = [this, &newBuffer](u32 bufferIndex)
	{
		BufferEntry& bufferEntry = mBuffers[bufferIndex];
		const u32 baseGlobalSuballocationIndex = bufferIndex * mSuballocationsPerBuffer;

		bufferEntry.Buffer = newBuffer;
		bufferEntry.AllocatedCount = 0;
		bufferEntry.FreeListHead = baseGlobalSuballocationIndex;

		// Initialize all suballocations for this buffer
		for (u32 subIndex = 0; subIndex < mSuballocationsPerBuffer; subIndex++)
		{
			const u32 entryIndex = baseGlobalSuballocationIndex + subIndex;
			SuballocationEntry& entry = mSuballocations[entryIndex];

			// Link into this buffer's free-list
			entry.NextFreeIndex = (subIndex + 1 < mSuballocationsPerBuffer) ? entryIndex + 1 : ~0u;
		}
	};

	// Try to reuse a deleted buffer slot first
	for (u32 bufferIndex = 0; bufferIndex < (u32)mBuffers.size(); bufferIndex++)
	{
		BufferEntry& bufferEntry = mBuffers[bufferIndex];
		if (bufferEntry.Buffer == nullptr)  // Found deleted slot
		{
			fnInitializeBufferEntry(bufferIndex);
			return;
		}
	}

	// No deleted slots, create new buffer entry
	BufferEntry newBufferEntry;

	// Add placeholder entries to mSuballocations
	mSuballocations.Reserve((u32)mSuballocations.Size() + mSuballocationsPerBuffer);

	for (u32 suballocationIndex = 0; suballocationIndex < mSuballocationsPerBuffer; suballocationIndex++)
		mSuballocations.Add(SuballocationEntry{});

	mBuffers.Add(newBufferEntry);

	// Initialize the new buffer entry
	fnInitializeBufferEntry((u32)(mBuffers.Size() - 1));
}
