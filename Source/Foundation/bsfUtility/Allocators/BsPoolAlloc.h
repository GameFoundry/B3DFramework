//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include <climits>

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * A memory allocator that allocates elements of the same size. Allows for fairly quick allocations and deallocations.
	 * 
	 * @tparam	ElemSize		Size of a single element in the pool. This will be the exact allocation size. 4 byte minimum.
	 * @tparam	ElemsPerBlock	Determines how much space to reserve for elements. This determines the initial size of the
	 *							pool, and the additional size the pool will be expanded by every time the number of elements
	 *							goes over the available storage limit.
	 * @tparam	Alignment		Memory alignment of each allocated element. Note that alignments that are larger than
	 *							element size, or aren't a multiplier of element size will introduce additionally padding
	 *							for each element, and therefore require more internal memory.
	 */
	template <int ElemSize, int ElemsPerBlock = 512, int Alignment = 4>
	class PoolAlloc
	{
	private:
		/** A single block able to hold ElemsPerBlock elements. */
		class MemBlock
		{
		public:
			MemBlock(UINT8* blockData)
				:blockData(blockData), freePtr(0), freeElems(ElemsPerBlock), nextBlock(nullptr)
			{
				UINT32 offset = 0;
				for(UINT32 i = 0; i < ElemsPerBlock; i++)
				{
					UINT32* entryPtr = (UINT32*)&blockData[offset];

					offset += ActualElemSize;
					*entryPtr = offset;
				}
			}

			~MemBlock()
			{
				assert(freeElems == ElemsPerBlock && "Not all elements were deallocated from a block.");
			}

			/**
			 * Returns the first free address and increments the free pointer. Caller needs to ensure the remaining block 
			 * size is adequate before calling.
			 */
			UINT8* alloc()
			{
				UINT8* freeEntry = &blockData[freePtr];
				freePtr = *(UINT32*)freeEntry;
				--freeElems;

				return freeEntry;
			}

			/** Deallocates the provided pointer. */
			void dealloc(void* data)
			{
				UINT32* entryPtr = (UINT32*)data;
				*entryPtr = freePtr;
				++freeElems;

				freePtr = (UINT32)(((UINT8*)data) - blockData);
			}

			UINT8* blockData;
			UINT32 freePtr;
			UINT32 freeElems;
			MemBlock* nextBlock;
		};

	public:
		PoolAlloc()
		{
			static_assert(ElemSize >= 4, "Pool allocator minimum allowed element size is 4 bytes.");
			static_assert(ElemsPerBlock > 0, "Number of elements per block must be at least 1.");
			static_assert(ElemsPerBlock * ActualElemSize <= UINT_MAX, "Pool allocator block size too large.");
		}

		~PoolAlloc()
		{
			MemBlock* curBlock = mFreeBlock;
			while (curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->nextBlock;
				deallocBlock(curBlock);

				curBlock = nextBlock;
			}
		}

		/** Allocates enough memory for a single element in the pool. */
		UINT8* alloc()
		{
			if(mFreeBlock == nullptr || mFreeBlock->freeElems == 0)
				allocBlock();

			mTotalNumElems++;
			return mFreeBlock->alloc();
		}

		/** Deallocates an element from the pool. */
		void free(void* data)
		{
			MemBlock* curBlock = mFreeBlock;
			while(curBlock)
			{
				constexpr UINT32 blockDataSize = ActualElemSize * ElemsPerBlock;
				if(data >= curBlock->blockData && data < (curBlock->blockData + blockDataSize))
				{
					curBlock->dealloc(data);
					mTotalNumElems--;

					if(curBlock->freeElems == 0 && curBlock->nextBlock)
					{
						// Free the block, but only if there is some extra free space in other blocks
						const UINT32 totalSpace = (mNumBlocks - 1) * ElemsPerBlock;
						const UINT32 freeSpace = totalSpace - mTotalNumElems;

						if(freeSpace > ElemsPerBlock / 2)
						{
							mFreeBlock = curBlock->nextBlock;
							deallocBlock(curBlock);
						}
					}

					return;
				}

				curBlock = curBlock->nextBlock;
			}

			assert(false);
		}

		/** Allocates and constructs a single pool element. */
		template<class T, class... Args>
		T* construct(Args &&...args)
		{
			T* data = (T*)alloc();
			new ((void*)data) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates a single pool element. */
		template<class T>
		void destruct(T* data)
		{
			data->~T();
			free(data);
		}

	private:
		/** Allocates a new block of memory using a heap allocator. */
		MemBlock* allocBlock()
		{
			MemBlock* newBlock = nullptr;
			MemBlock* curBlock = mFreeBlock;

			while (curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->nextBlock;
				if (nextBlock != nullptr && nextBlock->freeElems > 0)
				{
					// Found an existing block with free space
					newBlock = nextBlock;

					curBlock->nextBlock = newBlock->nextBlock;
					newBlock->nextBlock = mFreeBlock;

					break;
				}

				curBlock = nextBlock;
			}

			if (newBlock == nullptr)
			{
				constexpr UINT32 blockDataSize = ActualElemSize * ElemsPerBlock;
				size_t paddedBlockDataSize = blockDataSize + (Alignment - 1); // Padding for potential alignment correction

				UINT8* data = (UINT8*)bs_alloc(sizeof(MemBlock) + (UINT32)paddedBlockDataSize);

				void* blockData = data + sizeof(MemBlock);
				blockData = std::align(Alignment, blockDataSize, blockData, paddedBlockDataSize);

				newBlock = new (data) MemBlock((UINT8*)blockData);
				mNumBlocks++;

				newBlock->nextBlock = mFreeBlock;
			}

			mFreeBlock = newBlock;
			return newBlock;
		}

		/** Deallocates a block of memory. */
		void deallocBlock(MemBlock* block)
		{
			block->~MemBlock();
			bs_free(block);

			mNumBlocks--;
		}

		static constexpr int ActualElemSize = ((ElemSize + Alignment - 1) / Alignment) * Alignment;

		MemBlock* mFreeBlock = nullptr;
		UINT32 mTotalNumElems = 0;
		UINT32 mNumBlocks = 0;
	};

	/** @} */
	/** @} */
}
