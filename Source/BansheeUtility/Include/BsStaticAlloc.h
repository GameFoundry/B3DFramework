//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Static allocator that attempts to perform zero heap (dynamic) allocations by always keeping an active preallocated 
	 * buffer. The allocator provides a fixed amount of preallocated memory, and if the size of the allocated data goes over
	 * that limit the allocator will fall back to dynamic heap allocations.
	 * 
	 * @note	This kind of allocator is only able to free all of its memory at once. Freeing individual elements
	 *			will not free the memory until a call to clear().
	 * 			
	 * @tparam	BlockSize			Size of the initially allocated static block, and minimum size of any dynamically 
	 *								allocated memory.
	 * @tparam	MaxDynamicMemory	Maximum amount of unused memory allowed in the buffer after a call to clear(). Keeping 
	 *								active dynamic buffers can help prevent further memory allocations at the cost of 
	 *								memory. This is not relevant if you stay within the bounds of the statically allocated 
	 *								memory.
	 */
	template<int BlockSize = 512, int MaxDynamicMemory = 512>
	class StaticAlloc
	{
	private:
		/** A single block of memory within a static allocator. */
		class MemBlock
		{
		public:
			MemBlock(UINT8* data, UINT32 size) 
				:mData(data), mFreePtr(0), mSize(size),
				mPrevBlock(nullptr), mNextBlock(nullptr)
			{ }

			/** Allocates a piece of memory within the block. Caller must ensure the block has enough empty space. */
			UINT8* alloc(UINT32 amount)
			{
				UINT8* freePtr = &mData[mFreePtr];
				mFreePtr += amount;

				return freePtr;
			}

			/** Releases all allocations within a block but doesn't actually free the memory. */
			void clear()
			{
				mFreePtr = 0;
			}

			UINT8* mData;
			UINT32 mFreePtr;
			UINT32 mSize;
			MemBlock* mPrevBlock;
			MemBlock* mNextBlock;
		};

	public:
		StaticAlloc()
			:mStaticBlock(mStaticData, BlockSize), mFreeBlock(&mStaticBlock),
			mTotalAllocBytes(0)
		{

		}

		~StaticAlloc()
		{
			assert(mFreeBlock == &mStaticBlock && mStaticBlock.mFreePtr == 0);

			freeBlocks(mFreeBlock);
		}

		/**
		 * Allocates a new piece of memory of the specified size.
		 *
		 * @param[in]	amount	Amount of memory to allocate, in bytes.
		 */
		UINT8* alloc(UINT32 amount)
		{
			if (amount == 0)
				return nullptr;

#if BS_DEBUG_MODE
			amount += sizeof(UINT32);
#endif

			UINT32 freeMem = mFreeBlock->mSize - mFreeBlock->mFreePtr;
			if (amount > freeMem)
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

		/** Deallocates a previously allocated piece of memory. */
		void free(void* data)
		{
			if (data == nullptr)
				return;

			// Dealloc is only used for debug and can be removed if needed. All the actual deallocation
			// happens in clear()

#if BS_DEBUG_MODE
			UINT8* dataPtr = (UINT8*)data;
			dataPtr -= sizeof(UINT32);

			UINT32* storedSize = (UINT32*)(dataPtr);
			mTotalAllocBytes -= *storedSize;
#endif
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T>
		T* construct(UINT32 count = 0)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T;

			return data;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T, class... Args>
		T* construct(Args &&...args, UINT32 count = 0)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates an object allocated with the static allocator. */
		template<class T>
		void destruct(T* data)
		{
			data->~T();

			free(data);
		}

		/** Destructs and deallocates an array of objects allocated with the static frame allocator. */
		template<class T>
		void destruct(T* data, UINT32 count)
		{
			for(unsigned int i = 0; i < count; i++)
				data[i].~T();

			free(data);
		}

		/** Frees the internal memory buffers. All external allocations must be freed before calling this. */
		void clear()
		{
			assert(mTotalAllocBytes == 0);

			MemBlock* dynamicBlock = mStaticBlock.mNextBlock;
			INT32 totalDynamicMemAmount = 0;
			UINT32 numDynamicBlocks = 0;

			while (dynamicBlock != nullptr)
			{
				totalDynamicMemAmount += dynamicBlock->mFreePtr;
				dynamicBlock->clear();
			
				dynamicBlock = dynamicBlock->mNextBlock;
				numDynamicBlocks++;
			}

			mFreeBlock = &mStaticBlock;
			mStaticBlock.clear();

			if (numDynamicBlocks > 1)
			{
				freeBlocks(&mStaticBlock);
				allocBlock(std::min(totalDynamicMemAmount, MaxDynamicMemory));
				mFreeBlock = &mStaticBlock;
			}
			else if (numDynamicBlocks == 1 && MaxDynamicMemory == 0)
			{
				freeBlocks(&mStaticBlock);
			}
		}

	private:
		UINT8 mStaticData[BlockSize];
		MemBlock mStaticBlock;

		MemBlock* mFreeBlock;
		UINT32 mTotalAllocBytes;

		/**
		 * Allocates a dynamic block of memory of the wanted size. The exact allocation size might be slightly higher in 
		 * order to store block meta data.
		 */
		MemBlock* allocBlock(UINT32 wantedSize)
		{
			UINT32 blockSize = BlockSize;
			if (wantedSize > blockSize)
				blockSize = wantedSize;

			MemBlock* dynamicBlock = mFreeBlock->mNextBlock;
			MemBlock* newBlock = nullptr;
			while (dynamicBlock != nullptr)
			{
				if (dynamicBlock->mSize >= blockSize)
				{
					newBlock = dynamicBlock;
					break;
				}

				dynamicBlock = dynamicBlock->mNextBlock;
			}

			if (newBlock == nullptr)
			{
				UINT8* data = (UINT8*)reinterpret_cast<UINT8*>(bs_alloc(blockSize + sizeof(MemBlock)));
				newBlock = new (data)MemBlock(data + sizeof(MemBlock), blockSize);
				newBlock->mPrevBlock = mFreeBlock;
				mFreeBlock->mNextBlock = newBlock;
			}

			mFreeBlock = newBlock;
			return newBlock;
		}

		/** Releases memory for any dynamic blocks following the provided block (if there are any). */
		void freeBlocks(MemBlock* start)
		{
			MemBlock* dynamicBlock = start->mNextBlock;
			while (dynamicBlock != nullptr)
			{
				MemBlock* nextBlock = dynamicBlock->mNextBlock;

				dynamicBlock->~MemBlock();
				bs_free(dynamicBlock);

				dynamicBlock = nextBlock;
			}

			start->mNextBlock = nullptr;
		}
	};

	/** @} */
	/** @} */
}