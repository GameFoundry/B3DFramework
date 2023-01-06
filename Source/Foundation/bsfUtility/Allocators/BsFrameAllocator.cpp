//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Allocators/BsFrameAllocator.h"
#include "Error/BsException.h"

using namespace bs;

u8* FrameAlloc::MemBlock::Alloc(u32 amount)
{
	u8* freePtr = &MData[MFreePtr];
	MFreePtr += amount;

	return freePtr;
}

void FrameAlloc::MemBlock::Clear()
{
	MFreePtr = 0;
}

FrameAlloc::FrameAlloc(u32 blockSize)
	: mBlockSize(blockSize), mFreeBlock(nullptr), mNextBlockIdx(0), mTotalAllocBytes(0), mLastFrame(nullptr)
{
}

FrameAlloc::~FrameAlloc()
{
	for(auto& block : mBlocks)
		DeallocBlock(block);
}

u8* FrameAlloc::Alloc(u32 amount)
{
#if B3D_DEBUG
	amount += sizeof(u32);
#endif
	u32 freeMem = 0;
	if(mFreeBlock != nullptr)
		freeMem = mFreeBlock->MSize - mFreeBlock->MFreePtr;

	if(amount > freeMem)
		AllocBlock(amount);

	u8* data = mFreeBlock->Alloc(amount);

#if B3D_DEBUG
	mTotalAllocBytes += amount;

	u32* storedSize = reinterpret_cast<u32*>(data);
	*storedSize = amount;

	return data + sizeof(u32);
#else
	return data;
#endif
}

u8* FrameAlloc::AllocAligned(u32 amount, u32 alignment)
{
#if B3D_DEBUG
	amount += sizeof(u32);
#endif

	u32 freeMem = 0;
	u32 freePtr = 0;
	if(mFreeBlock != nullptr)
	{
		freeMem = mFreeBlock->MSize - mFreeBlock->MFreePtr;

#if B3D_DEBUG
		freePtr = mFreeBlock->MFreePtr + sizeof(u32);
#else
		freePtr = mFreeBlock->MFreePtr;
#endif
	}

	u32 alignOffset = (alignment - (freePtr & (alignment - 1))) & (alignment - 1);
	if((amount + alignOffset) > freeMem)
	{
		// New blocks are allocated on a 16 byte boundary, ensure enough space is allocated taking into account
		// the requested alignment

#if B3D_DEBUG
		alignOffset = (alignment - (sizeof(u32) & (alignment - 1))) & (alignment - 1);
#else
		if(alignment > 16)
			alignOffset = alignment - 16;
		else
			alignOffset = 0;
#endif

		AllocBlock(amount + alignOffset);
	}

	amount += alignOffset;
	u8* data = mFreeBlock->Alloc(amount);

#if B3D_DEBUG
	mTotalAllocBytes += amount;

	u32* storedSize = reinterpret_cast<u32*>(data + alignOffset);
	*storedSize = amount;

	return data + sizeof(u32) + alignOffset;
#else
	return data + alignOffset;
#endif
}

void FrameAlloc::Free(u8* data)
{
	// Dealloc is only used for debug and can be removed if needed. All the actual deallocation
	// happens in clear()

#if B3D_DEBUG
	if(data)
	{
		data -= sizeof(u32);
		u32* storedSize = reinterpret_cast<u32*>(data);
		mTotalAllocBytes -= *storedSize;
	}
#endif
}

void FrameAlloc::MarkFrame()
{
	void** framePtr = (void**)Alloc(sizeof(void*));
	*framePtr = mLastFrame;
	mLastFrame = framePtr;
}

void FrameAlloc::Clear()
{
	if(mLastFrame != nullptr)
	{
		B3D_ASSERT(mBlocks.size() > 0 && mNextBlockIdx > 0);

		Free((u8*)mLastFrame);

		u8* framePtr = (u8*)mLastFrame;
		mLastFrame = *(void**)mLastFrame;

#if B3D_DEBUG
		framePtr -= sizeof(u32);
#endif

		u32 startBlockIdx = mNextBlockIdx - 1;
		u32 numFreedBlocks = 0;
		for(i32 i = startBlockIdx; i >= 0; i--)
		{
			MemBlock* curBlock = mBlocks[i];
			u8* blockEnd = curBlock->MData + curBlock->MSize;
			if(framePtr >= curBlock->MData && framePtr < blockEnd)
			{
				u8* dataEnd = curBlock->MData + curBlock->MFreePtr;
				u32 sizeInBlock = (u32)(dataEnd - framePtr);
				B3D_ASSERT(sizeInBlock <= curBlock->MFreePtr);

				curBlock->MFreePtr -= sizeInBlock;
				if(curBlock->MFreePtr == 0)
				{
					numFreedBlocks++;

					// Reset block counter if we're gonna reallocate this one
					if(numFreedBlocks > 1)
						mNextBlockIdx = (u32)i;
				}

				break;
			}
			else
			{
				curBlock->MFreePtr = 0;
				mNextBlockIdx = (u32)i;
				numFreedBlocks++;
			}
		}

		if(numFreedBlocks > 1)
		{
			u32 totalBytes = 0;
			for(u32 i = 0; i < numFreedBlocks; i++)
			{
				MemBlock* curBlock = mBlocks[mNextBlockIdx];
				totalBytes += curBlock->MSize;

				DeallocBlock(curBlock);
				mBlocks.erase(mBlocks.begin() + mNextBlockIdx);
			}

			u32 oldNextBlockIdx = mNextBlockIdx;
			AllocBlock(totalBytes);

			// Point to the first non-full block, or if none available then point the the block we just allocated
			if(oldNextBlockIdx > 0)
				mFreeBlock = mBlocks[oldNextBlockIdx - 1];
		}
		else
		{
			mFreeBlock = mBlocks[mNextBlockIdx - 1];
		}
	}
	else
	{
#if B3D_DEBUG
		if(mTotalAllocBytes.load() > 0)
			B3D_EXCEPT(InvalidStateException, "Not all frame allocated bytes were properly released.");
#endif

		if(mBlocks.size() > 1)
		{
			// Merge all blocks into one
			u32 totalBytes = 0;
			for(auto& block : mBlocks)
			{
				totalBytes += block->MSize;
				DeallocBlock(block);
			}

			mBlocks.clear();
			mNextBlockIdx = 0;

			AllocBlock(totalBytes);
		}
		else if(mBlocks.size() > 0)
			mBlocks[0]->MFreePtr = 0;
	}
}

FrameAlloc::MemBlock* FrameAlloc::AllocBlock(u32 wantedSize)
{
	u32 blockSize = mBlockSize;
	if(wantedSize > blockSize)
		blockSize = wantedSize;

	MemBlock* newBlock = nullptr;
	while(mNextBlockIdx < mBlocks.size())
	{
		MemBlock* curBlock = mBlocks[mNextBlockIdx];
		if(blockSize <= curBlock->MSize)
		{
			newBlock = curBlock;
			mNextBlockIdx++;
			break;
		}
		else
		{
			// Found an empty block that doesn't fit our data, delete it
			DeallocBlock(curBlock);
			mBlocks.erase(mBlocks.begin() + mNextBlockIdx);
		}
	}

	if(newBlock == nullptr)
	{
		u32 alignOffset = 16 - (sizeof(MemBlock) & (16 - 1));

		u8* data = (u8*)reinterpret_cast<u8*>(B3DAllocateAligned16(blockSize + sizeof(MemBlock) + alignOffset));
		newBlock = new(data) MemBlock(blockSize);
		data += sizeof(MemBlock) + alignOffset;
		newBlock->MData = data;

		mBlocks.push_back(newBlock);
		mNextBlockIdx++;
	}

	mFreeBlock = newBlock; // If previous block had some empty space it is lost until next "clear"

	return newBlock;
}

void FrameAlloc::DeallocBlock(MemBlock* block)
{
	block->~MemBlock();
	B3DFreeAligned16(block);
}

void FrameAlloc::SetOwnerThread(ThreadId thread)
{
}

namespace bs
{
B3D_THREADLOCAL FrameAlloc* _GlobalFrameAlloc = nullptr;

B3D_UTILITY_EXPORT FrameAlloc& GetFrameAllocator()
{
	if(_GlobalFrameAlloc == nullptr)
	{
		// Note: This will leak memory but since it should exist throughout the entirety
		// of runtime it should only leak on shutdown when the OS will free it anyway.
		_GlobalFrameAlloc = new FrameAlloc();
	}

	return *_GlobalFrameAlloc;
}

B3D_UTILITY_EXPORT u8* B3DFrameAllocate(u32 numBytes)
{
	return GetFrameAllocator().Alloc(numBytes);
}

B3D_UTILITY_EXPORT u8* B3DFrameAllocateAligned(u32 count, u32 align)
{
	return GetFrameAllocator().AllocAligned(count, align);
}

B3D_UTILITY_EXPORT void B3DFrameFree(void* data)
{
	GetFrameAllocator().Free((u8*)data);
}

B3D_UTILITY_EXPORT void B3DFrameFreeAligned(void* data)
{
	GetFrameAllocator().Free((u8*)data);
}

B3D_UTILITY_EXPORT void B3DMarkAllocatorFrame()
{
	GetFrameAllocator().MarkFrame();
}

B3D_UTILITY_EXPORT void B3DClearAllocatorFrame()
{
	GetFrameAllocator().Clear();
}
} // namespace bs
