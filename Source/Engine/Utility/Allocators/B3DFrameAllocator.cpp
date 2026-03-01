//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Allocators/B3DFrameAllocator.h"

using namespace b3d;

u8* FrameAllocator::MemoryBlock::Alloc(u32 allocationSize)
{
	u8* freePointer = &Data[FreePointer];
	FreePointer += allocationSize;

	return freePointer;
}

void FrameAllocator::MemoryBlock::Clear()
{
	FreePointer = 0;
}

FrameAllocator::FrameAllocator(u32 blockSize)
	: mBlockSize(blockSize), mFreeBlock(nullptr), mNextBlockIndex(0), mTotalAllocatedBytes(0), mLastFrame(nullptr)
	, mCurrentFrameDepth(0)
{
#if B3D_DEBUG
	mOwnerThread = ThreadId();
#endif
}

FrameAllocator::~FrameAllocator()
{
	for(auto& block : mBlocks)
		DeallocateBlock(block);
}

#if B3D_DEBUG
void FrameAllocator::WriteDebugHeader(u8* data, u32 totalSize, u32 frameDepth, u32 userSize)
{
	// Write pre-guard
	u32* preGuard = reinterpret_cast<u32*>(data);
	*preGuard = kGuardPattern;

	// Write header fields after pre-guard
	u32* sizePointer = reinterpret_cast<u32*>(data + kGuardSize);
	u32* depthPointer = sizePointer + 1;
	u32* userSizePointer = depthPointer + 1;

	*sizePointer = totalSize;
	*depthPointer = frameDepth;
	*userSizePointer = userSize;
}

void FrameAllocator::ReadDebugHeader(const u8* data, u32& outTotalSize, u32& outDepth, u32& outUserSize) const
{
	// Read header fields (after pre-guard)
	const u32* sizePointer = reinterpret_cast<const u32*>(data + kGuardSize);
	const u32* depthPointer = sizePointer + 1;
	const u32* userSizePointer = depthPointer + 1;

	outTotalSize = *sizePointer;
	outDepth = *depthPointer;
	outUserSize = *userSizePointer;
}

bool FrameAllocator::ValidateAllocation(const u8* data, u32 totalSize, u32 userSize) const
{
	// Check pre-guard
	const u32* preGuard = reinterpret_cast<const u32*>(data);
	if (*preGuard != kGuardPattern)
		return false;  // Buffer underrun detected

	// Check post-guard
	const u8* userData = data + kDebugHeaderSize;
	const u32* postGuard = reinterpret_cast<const u32*>(userData + userSize);
	if (*postGuard != kGuardPattern)
		return false;  // Buffer overrun detected

	return true;
}
#endif

u8* FrameAllocator::Allocate(u32 allocationSize)
{
#if B3D_DEBUG
	// Validate thread ownership
	B3D_ASSERT(mOwnerThread == ThreadId() && "FrameAllocator accessed from different thread than owner");

	// Store original user-requested size before expanding
	u32 userSize = allocationSize;

	// Expand allocation to include debug header and trailer
	allocationSize += kDebugHeaderSize + kDebugTrailerSize;
#endif

	// Check if current block has enough free memory
	u32 freeMemory = 0;
	if(mFreeBlock != nullptr)
		freeMemory = mFreeBlock->Size - mFreeBlock->FreePointer;

	// Allocate new block if current block doesn't have enough space
	if(allocationSize > freeMemory)
		AllocateBlock(allocationSize);

	// Perform bump pointer allocation within the block
	u8* data = mFreeBlock->Alloc(allocationSize);

#if B3D_DEBUG
	// Update total allocation tracking
	mTotalAllocatedBytes.fetch_add(allocationSize, std::memory_order_relaxed);

	// Write debug header with guard, size, depth, and user size
	WriteDebugHeader(data, allocationSize, mCurrentFrameDepth, userSize);

	// Calculate user data pointer
	u8* userData = data + kDebugHeaderSize;

	// Fill user memory with uninitialized pattern
	std::memset(userData, kUninitPattern, userSize);

	// Write post-guard after user data
	u32* postGuard = reinterpret_cast<u32*>(userData + userSize);
	*postGuard = kGuardPattern;

	// Periodic integrity check
	mAllocationCount++;
	if(mCheckFrequency > 0 && (mAllocationCount % mCheckFrequency) == 0)
		CheckMemory();

	// Return pointer past the debug header to user
	return userData;
#else
	return data;
#endif
}

u8* FrameAllocator::AllocateAligned(u32 allocationSize, u32 alignment)
{
#if B3D_DEBUG
	// Validate thread ownership
	B3D_ASSERT(mOwnerThread == ThreadId() && "FrameAllocator accessed from different thread than owner");

	// Store original user-requested size before expanding
	u32 userSize = allocationSize;

	// Expand allocation to include debug header and trailer
	allocationSize += kDebugHeaderSize + kDebugTrailerSize;
#endif

	u32 freeMemory = 0;
	u32 freePointer = 0;
	if(mFreeBlock != nullptr)
	{
		freeMemory = mFreeBlock->Size - mFreeBlock->FreePointer;

#if B3D_DEBUG
		// In debug builds, account for header when calculating alignment
		freePointer = mFreeBlock->FreePointer + kDebugHeaderSize;
#else
		freePointer = mFreeBlock->FreePointer;
#endif
	}

	// Calculate alignment offset using bit manipulation
	// Formula: Rounds up freePointer to next alignment boundary
	// Example: freePointer=10, alignment=16 → offset=6 (10+6=16)
	u32 alignmentOffset = (alignment - (freePointer & (alignment - 1))) & (alignment - 1);

	if((allocationSize + alignmentOffset) > freeMemory)
	{
		// Need to allocate a new block
		// New blocks are allocated on a 16-byte boundary (kBlockAlignment)
		// Calculate worst-case alignment offset for the new block

#if B3D_DEBUG
		// In debug, header comes first, so align relative to header size
		alignmentOffset = (alignment - (kDebugHeaderSize & (alignment - 1))) & (alignment - 1);
#else
		// In release, if alignment > 16, we need extra space since blocks are 16-byte aligned
		if(alignment > kBlockAlignment)
			alignmentOffset = alignment - kBlockAlignment;
		else
			alignmentOffset = 0;
#endif

		AllocateBlock(allocationSize + alignmentOffset);
	}

	// Include alignment offset in total allocation
	allocationSize += alignmentOffset;
	u8* data = mFreeBlock->Alloc(allocationSize);

#if B3D_DEBUG
	// Update total allocation tracking
	mTotalAllocatedBytes.fetch_add(allocationSize, std::memory_order_relaxed);

	// Write debug header at the aligned position (after alignment offset)
	WriteDebugHeader(data + alignmentOffset, allocationSize, mCurrentFrameDepth, userSize);

	// Calculate user data pointer
	u8* userData = data + alignmentOffset + kDebugHeaderSize;

	// Fill user memory with uninitialized pattern
	std::memset(userData, kUninitPattern, userSize);

	// Write post-guard after user data
	u32* postGuard = reinterpret_cast<u32*>(userData + userSize);
	*postGuard = kGuardPattern;

	// Periodic integrity check
	mAllocationCount++;
	if(mCheckFrequency > 0 && (mAllocationCount % mCheckFrequency) == 0)
		CheckMemory();

	// Return pointer past both alignment offset and debug header
	return userData;
#else
	return data + alignmentOffset;
#endif
}

void FrameAllocator::Free(u8* data)
{
	// Note: Deallocation doesn't actually free memory - all memory is bulk-freed in Clear()
	// This method only updates debug tracking and validates cross-frame allocation/deallocation

#if B3D_DEBUG
	if(data)
	{
		// Step back to the start of the allocation (before debug header)
		data -= kDebugHeaderSize;

		// Read debug header to get size, frame depth, and user size
		u32 totalSize;
		u32 allocationDepth;
		u32 userSize;
		ReadDebugHeader(data, totalSize, allocationDepth, userSize);

		// Validate guards before freeing
		if(!ValidateAllocation(data, totalSize, userSize))
		{
			B3D_ASSERT(false && "Memory corruption detected on Free(): guard bytes overwritten");
		}

		// Validate that memory is being freed at the same frame depth it was allocated
		B3D_ASSERT(allocationDepth == mCurrentFrameDepth &&
			"Memory allocated at one frame depth is being freed at a different depth. "
			"This violates the frame allocator contract.");

		// Fill user memory with freed pattern to detect use-after-free
		u8* userData = data + kDebugHeaderSize;
		std::memset(userData, kFreedPattern, userSize);

		// Update total allocation tracking
		mTotalAllocatedBytes.fetch_sub(totalSize, std::memory_order_relaxed);
	}
#endif
}

void FrameAllocator::MarkFrame()
{
#if B3D_DEBUG
	// Validate thread ownership
	B3D_ASSERT(mOwnerThread == ThreadId() && "FrameAllocator accessed from different thread than owner");
#endif

	// Allocate space for a frame marker (pointer to previous frame marker)
	// This creates a linked list of frame markers stored within the allocator's own memory
	void** framePointer = (void**)Allocate(sizeof(void*));
	*framePointer = mLastFrame;
	mLastFrame = framePointer;

	// Increment frame depth to track nesting level
	mCurrentFrameDepth++;
}

void FrameAllocator::Clear()
{
#if B3D_DEBUG
	// Validate thread ownership
	B3D_ASSERT(mOwnerThread == ThreadId() && "FrameAllocator accessed from different thread than owner");
#endif

	// Check if we have a frame marker
	// If mLastFrame is not null, we're doing a frame-scoped clear (pop to last MarkFrame)
	// If mLastFrame is null, we're doing a full clear (deallocate everything)

	if(mLastFrame != nullptr)
	{
		// Frame marker exists - perform frame-scoped clear

		B3D_ASSERT(mBlocks.size() > 0 && mNextBlockIndex > 0);

		// Decrement frame depth since we're exiting a frame scope. Need to do this before the Free call below.
		mCurrentFrameDepth--;

		// Save the frame pointer and follow linked list BEFORE calling Free,
		// because Free fills memory with 0xDD pattern in debug builds
		u8* framePointer = (u8*)mLastFrame;
		void* previousFrame = *(void**)mLastFrame;

		// Free the frame marker allocation (this will fill memory with freed pattern)
		Free((u8*)mLastFrame);

		// Update to previous frame marker
		mLastFrame = previousFrame;

#if B3D_DEBUG
		// In debug builds, step back past the debug header to get to the actual allocation start
		framePointer -= kDebugHeaderSize;
#endif

		// Find which block contains the frame marker and rewind allocations
		// Walk backwards through blocks to find the one containing our frame marker

		u32 startBlockIndex = mNextBlockIndex - 1;
		u32 numberOfFreedBlocks = 0;

		for(i32 blockIndex = startBlockIndex; blockIndex >= 0; blockIndex--)
		{
			MemoryBlock* currentBlock = mBlocks[blockIndex];
			u8* blockEnd = currentBlock->Data + currentBlock->Size;

			// Check if frame marker is within this block's memory range
			if(framePointer >= currentBlock->Data && framePointer < blockEnd)
			{
				// Found the block containing the frame marker
				// Calculate how much memory to free from this block
				u8* dataEnd = currentBlock->Data + currentBlock->FreePointer;
				u32 sizeInBlock = (u32)(dataEnd - framePointer);
				B3D_ASSERT(sizeInBlock <= currentBlock->FreePointer);

				// Rewind the block's free pointer to the frame marker position
				currentBlock->FreePointer -= sizeInBlock;

				if(currentBlock->FreePointer == 0)
				{
					numberOfFreedBlocks++;

					// If we're freeing more than one block, reset the block counter
					if(numberOfFreedBlocks > 1)
						mNextBlockIndex = (u32)blockIndex;
				}

				break;
			}
			else
			{
				// Frame marker is not in this block - this entire block was allocated after the marker
				// Mark it as completely freed
				currentBlock->FreePointer = 0;
				mNextBlockIndex = (u32)blockIndex;
				numberOfFreedBlocks++;
			}
		}

		// Optionally merge freed blocks to reduce fragmentation
		// If we freed multiple blocks, merge them into one large block

		if(numberOfFreedBlocks > 1)
		{
			// Calculate total size of all freed blocks
			u32 totalBytes = 0;
			for(u32 blockIndex = 0; blockIndex < numberOfFreedBlocks; blockIndex++)
			{
				MemoryBlock* currentBlock = mBlocks[mNextBlockIndex];
				totalBytes += currentBlock->Size;

				// Deallocate the block
				DeallocateBlock(currentBlock);
				mBlocks.erase(mBlocks.begin() + mNextBlockIndex);
			}

			// Allocate a single new block with the combined size
			u32 oldNextBlockIndex = mNextBlockIndex;
			AllocateBlock(totalBytes);

			// Point to the first non-full block, or the block we just allocated if none available
			if(oldNextBlockIndex > 0)
				mFreeBlock = mBlocks[oldNextBlockIndex - 1];
		}
		else
		{
			// Only one block was affected, just point to it as the free block
			mFreeBlock = mBlocks[mNextBlockIndex - 1];
		}
	}
	else
	{
		// No frame marker - full clear with debug validation

#if B3D_DEBUG
		// In debug builds, validate that all allocated memory was explicitly freed
		if(mTotalAllocatedBytes.load(std::memory_order_relaxed) > 0)
		{
			B3D_ASSERT(false &&
				"Not all frame allocated bytes were properly released. "
				"This indicates a memory leak - memory was allocated but not freed before Clear().");
		}
#endif

		// Merge all blocks into one large block to reduce fragmentation

		if(mBlocks.size() > 1)
		{
			// Calculate total size of all blocks
			u32 totalBytes = 0;
			for(auto& block : mBlocks)
			{
				totalBytes += block->Size;
				DeallocateBlock(block);
			}

			// Clear the block list and reset counters
			mBlocks.clear();
			mNextBlockIndex = 0;

			// Allocate a single merged block
			AllocateBlock(totalBytes);
		}
		else if(mBlocks.size() > 0)
		{
			// Only one block exists - just reset its free pointer
			mBlocks[0]->FreePointer = 0;
		}
	}
}

FrameAllocator::MemoryBlock* FrameAllocator::AllocateBlock(u32 wantedSize)
{
	// Determine the size of the block to allocate
	// Use the default block size unless the wanted size is larger
	u32 blockSize = mBlockSize;
	if(wantedSize > blockSize)
		blockSize = wantedSize;

	// Try to reuse an existing block from the pool
	MemoryBlock* newBlock = nullptr;
	while(mNextBlockIndex < mBlocks.size())
	{
		MemoryBlock* currentBlock = mBlocks[mNextBlockIndex];

		if(blockSize <= currentBlock->Size)
		{
			// Found a suitable block to reuse
			newBlock = currentBlock;
			mNextBlockIndex++;
			break;
		}
		else
		{
			// Found an empty block that's too small - delete it
			DeallocateBlock(currentBlock);
			mBlocks.erase(mBlocks.begin() + mNextBlockIndex);
		}
	}

	// If no suitable block was found, allocate a new one
	if(newBlock == nullptr)
	{
		// Calculate alignment offset for the MemoryBlock object itself
		// We want the data buffer to be 16-byte aligned, so we need to account for
		// the MemoryBlock header size when calculating the offset
		u32 alignmentOffset = kBlockAlignment - (sizeof(MemoryBlock) & (kBlockAlignment - 1));

		// Allocate memory for both the MemoryBlock object and the data buffer
		u8* data = (u8*)reinterpret_cast<u8*>(B3DAllocateAligned16(blockSize + sizeof(MemoryBlock) + alignmentOffset));

		// Construct MemoryBlock object at the start using placement new
		newBlock = new(data) MemoryBlock(blockSize);

		// Data buffer starts after the MemoryBlock object and alignment offset
		data += sizeof(MemoryBlock) + alignmentOffset;
		newBlock->Data = data;

		// Add the new block to our pool
		mBlocks.push_back(newBlock);
		mNextBlockIndex++;
	}

	// Set this as the current free block
	// Note: Any remaining space in the previous free block is lost until the next Clear()
	mFreeBlock = newBlock;

	return newBlock;
}

void FrameAllocator::DeallocateBlock(MemoryBlock* block)
{
	block->~MemoryBlock();
	B3DFreeAligned16(block);
}

#if B3D_DEBUG
bool FrameAllocator::CheckMemory() const
{
	B3D_ASSERT(mOwnerThread == ThreadId() && "FrameAllocator accessed from different thread than owner");

	bool allValid = true;

	// Walk through all active blocks
	for(u32 blockIndex = 0; blockIndex < mNextBlockIndex; ++blockIndex)
	{
		MemoryBlock* block = mBlocks[blockIndex];
		u8* current = block->Data;
		u8* end = block->Data + block->FreePointer;

		// Walk through allocations in this block
		while(current < end)
		{
			u32 totalSize;
			u32 frameDepth;
			u32 userSize;
			ReadDebugHeader(current, totalSize, frameDepth, userSize);

			if(!ValidateAllocation(current, totalSize, userSize))
				allValid = false;

			current += totalSize;
		}
	}

	return allValid;
}
#endif

namespace b3d
{
B3D_THREADLOCAL FrameAllocator* _GlobalFrameAlloc = nullptr;

B3D_EXPORT FrameAllocator& GetFrameAllocator()
{
	if(_GlobalFrameAlloc == nullptr)
	{
		// Note: This will leak memory but since it should exist throughout the entirety
		// of runtime it should only leak on shutdown when the OS will free it anyway.
		_GlobalFrameAlloc = new FrameAllocator();
	}

	return *_GlobalFrameAlloc;
}

B3D_EXPORT u8* B3DFrameAllocate(u32 byteCount)
{
	return GetFrameAllocator().Allocate(byteCount);
}

B3D_EXPORT u8* B3DFrameAllocateAligned(u32 count, u32 align)
{
	return GetFrameAllocator().AllocateAligned(count, align);
}

B3D_EXPORT void B3DFrameFree(void* data)
{
	GetFrameAllocator().Free((u8*)data);
}

B3D_EXPORT void B3DFrameFreeAligned(void* data)
{
	GetFrameAllocator().Free((u8*)data);
}

B3D_EXPORT void B3DMarkAllocatorFrame()
{
	GetFrameAllocator().MarkFrame();
}

B3D_EXPORT void B3DClearAllocatorFrame()
{
	GetFrameAllocator().Clear();
}

#if B3D_DEBUG
B3D_EXPORT bool B3DFrameCheckMemory()
{
	return GetFrameAllocator().CheckMemory();
}

B3D_EXPORT void B3DFrameSetCheckFrequency(u32 frequency)
{
	GetFrameAllocator().SetCheckFrequency(frequency);
}
#endif
} // namespace b3d
