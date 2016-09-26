//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFrameAlloc.h"
#include "BsException.h"

namespace BansheeEngine
{
	FrameAlloc::MemBlock::MemBlock(UINT32 size)
		:mData(nullptr), mFreePtr(0), mSize(size)
	{ }

	FrameAlloc::MemBlock::~MemBlock()
	{ }

	UINT8* FrameAlloc::MemBlock::alloc(UINT32 amount)
	{
		UINT8* freePtr = &mData[mFreePtr];
		mFreePtr += amount;

		return freePtr;
	}

	void FrameAlloc::MemBlock::clear()
	{
		mFreePtr = 0;
	}

#if BS_DEBUG_MODE
	FrameAlloc::FrameAlloc(UINT32 blockSize)
		:mBlockSize(blockSize), mFreeBlock(nullptr), mNextBlockIdx(0), mTotalAllocBytes(0),
		mLastFrame(nullptr), mOwnerThread(BS_THREAD_CURRENT_ID)
	{
		allocBlock(mBlockSize);
	}
#else
	FrameAlloc::FrameAlloc(UINT32 blockSize)
		:mTotalAllocBytes(0), mFreeBlock(nullptr), mBlockSize(blockSize),
		mLastFrame(nullptr), mNextBlockIdx(0)
	{
		allocBlock(mBlockSize);
	}
#endif

	FrameAlloc::~FrameAlloc()
	{
		for(auto& block : mBlocks)
			deallocBlock(block);
	}

	UINT8* FrameAlloc::alloc(UINT32 amount)
	{
#if BS_DEBUG_MODE
		assert(mOwnerThread == BS_THREAD_CURRENT_ID && "Frame allocator called from invalid thread.");

		amount += sizeof(UINT32);
#endif

		UINT32 freeMem = mFreeBlock->mSize - mFreeBlock->mFreePtr;
		if(amount > freeMem)
			allocBlock(amount);

		UINT8* data = mFreeBlock->alloc(amount);

#if BS_DEBUG_MODE
		mTotalAllocBytes += amount;

		UINT32* storedSize = reinterpret_cast<UINT32*>(data);
		*storedSize = amount;

		return data + sizeof(UINT32);
#else
		return data;
#endif
	}

	UINT8* FrameAlloc::allocAligned(UINT32 amount, UINT32 alignment)
	{
#if BS_DEBUG_MODE
		assert(mOwnerThread == BS_THREAD_CURRENT_ID && "Frame allocator called from invalid thread.");

		amount += sizeof(UINT32);
		UINT32 freePtr = mFreeBlock->mFreePtr + sizeof(UINT32);
#else
		UINT32 freePtr = mFreeBlock->mFreePtr;
#endif

		UINT32 alignOffset = alignment - freePtr & (alignment - 1);

		UINT32 freeMem = mFreeBlock->mSize - mFreeBlock->mFreePtr;
		if ((amount + alignOffset) > freeMem)
		{
			// New blocks are allocated on a 16 byte boundary, ensure we enough space is allocated taking into account
			// the requested alignment

#if BS_DEBUG_MODE
			alignOffset = alignment - sizeof(UINT32) & (alignment - 1);
#else
			if (alignment > 16)
				alignOffset = alignment - 16;
			else
				alignOffset = 0;
#endif

			allocBlock(amount + alignOffset);
		}

		amount += alignOffset;
		UINT8* data = mFreeBlock->alloc(amount);

#if BS_DEBUG_MODE
		mTotalAllocBytes += amount;

		UINT32* storedSize = reinterpret_cast<UINT32*>(data + alignOffset);
		*storedSize = amount;

		return data + sizeof(UINT32) + alignOffset;
#else
		return data + alignOffset;
#endif
	}

	void FrameAlloc::dealloc(UINT8* data)
	{
		// Dealloc is only used for debug and can be removed if needed. All the actual deallocation
		// happens in clear()
			
#if BS_DEBUG_MODE
		data -= sizeof(UINT32);
		UINT32* storedSize = reinterpret_cast<UINT32*>(data);
		mTotalAllocBytes -= *storedSize;
#endif
	}

	void FrameAlloc::markFrame()
	{
		void** framePtr = (void**)alloc(sizeof(void*));
		*framePtr = mLastFrame;
		mLastFrame = framePtr;
	}

	void FrameAlloc::clear()
	{
#if BS_DEBUG_MODE
		assert(mOwnerThread == BS_THREAD_CURRENT_ID && "Frame allocator called from invalid thread.");
#endif

		if(mLastFrame != nullptr)
		{
			assert(mBlocks.size() > 0 && mNextBlockIdx > 0);

			dealloc((UINT8*)mLastFrame);

			UINT8* framePtr = (UINT8*)mLastFrame;
			mLastFrame = *(void**)mLastFrame;

#if BS_DEBUG_MODE
			framePtr -= sizeof(UINT32);
#endif

			UINT32 startBlockIdx = mNextBlockIdx - 1;
			UINT32 numFreedBlocks = 0;
			for (INT32 i = startBlockIdx; i >= 0; i--)
			{
				MemBlock* curBlock = mBlocks[i];
				UINT8* blockEnd = curBlock->mData + curBlock->mSize;
				if (framePtr >= curBlock->mData && framePtr < blockEnd)
				{
					UINT8* dataEnd = curBlock->mData + curBlock->mFreePtr;
					UINT32 sizeInBlock = (UINT32)(dataEnd - framePtr);
					assert(sizeInBlock <= curBlock->mFreePtr);

					curBlock->mFreePtr -= sizeInBlock;
					if (curBlock->mFreePtr == 0)
					{
						numFreedBlocks++;

						// Reset block counter if we're gonna reallocate this one
						if (numFreedBlocks > 1)
							mNextBlockIdx = (UINT32)i;
					}

					break;
				}
				else
				{
					curBlock->mFreePtr = 0;
					mNextBlockIdx = (UINT32)i;
					numFreedBlocks++;
				}
			}

			if (numFreedBlocks > 1)
			{
				UINT32 totalBytes = 0;
				for (UINT32 i = 0; i < numFreedBlocks; i++)
				{
					MemBlock* curBlock = mBlocks[mNextBlockIdx];
					totalBytes += curBlock->mSize;

					deallocBlock(curBlock);
					mBlocks.erase(mBlocks.begin() + mNextBlockIdx);
				}
				
				UINT32 oldNextBlockIdx = mNextBlockIdx;
				allocBlock(totalBytes);

				// Point to the first non-full block, or if none available then point the the block we just allocated
				if (oldNextBlockIdx > 0)
					mFreeBlock = mBlocks[oldNextBlockIdx - 1];
			}
			else
			{
				mFreeBlock = mBlocks[mNextBlockIdx - 1];
			}
		}
		else
		{
#if BS_DEBUG_MODE
			if (mTotalAllocBytes.load() > 0)
				BS_EXCEPT(InvalidStateException, "Not all frame allocated bytes were properly released.");
#endif

			if (mBlocks.size() > 1)
			{
				// Merge all blocks into one
				UINT32 totalBytes = 0;
				for (auto& block : mBlocks)
				{
					totalBytes += block->mSize;
					deallocBlock(block);
				}

				mBlocks.clear();
				mNextBlockIdx = 0;

				allocBlock(totalBytes);
			}
		}
	}

	FrameAlloc::MemBlock* FrameAlloc::allocBlock(UINT32 wantedSize)
	{
		UINT32 blockSize = mBlockSize;
		if(wantedSize > blockSize)
			blockSize = wantedSize;

		MemBlock* newBlock = nullptr;
		while (mNextBlockIdx < mBlocks.size())
		{
			MemBlock* curBlock = mBlocks[mNextBlockIdx];
			if (blockSize <= curBlock->mSize)
			{
				newBlock = curBlock;
				mNextBlockIdx++;
				break;
			}
			else
			{
				// Found an empty block that doesn't fit our data, delete it
				deallocBlock(curBlock);
				mBlocks.erase(mBlocks.begin() + mNextBlockIdx);
			}
		}

		if (newBlock == nullptr)
		{
			UINT32 alignOffset = 16 - sizeof(MemBlock) & (16 - 1);

			UINT8* data = (UINT8*)reinterpret_cast<UINT8*>(bs_alloc_aligned16(blockSize + sizeof(MemBlock) + alignOffset));
			newBlock = new (data) MemBlock(blockSize);
			data += sizeof(MemBlock) + alignOffset;
			newBlock->mData = data;

			mBlocks.push_back(newBlock);
			mNextBlockIdx++;
		}

		mFreeBlock = newBlock; // If previous block had some empty space it is lost until next "clear"

		return newBlock;
	}

	void FrameAlloc::deallocBlock(MemBlock* block)
	{
		block->~MemBlock();
		bs_free_aligned(block);
	}

	void FrameAlloc::setOwnerThread(ThreadId thread)
	{
#if BS_DEBUG_MODE
		mOwnerThread = thread;
#endif
	}
}