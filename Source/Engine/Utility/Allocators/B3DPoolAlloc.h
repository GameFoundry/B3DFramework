//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include <climits>

namespace b3d
{
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
	 * @tparam	Lock			If true the pool allocator will be made thread safe (at the cost of performance).
	 */
	template <int ElemSize, int ElemsPerBlock = 512, int Alignment = 4, bool Lock = false>
	class PoolAlloc
	{
	private:
		/** A single block able to hold ElemsPerBlock elements. */
		class MemBlock
		{
		public:
			MemBlock(u8* blockData)
				: BlockData(blockData), FreePtr(0), FreeElems(ElemsPerBlock), NextBlock(nullptr)
			{
				u32 offset = 0;
				for(u32 i = 0; i < ElemsPerBlock; i++)
				{
					u32* entryPtr = (u32*)&blockData[offset];

					offset += kActualElemSize;
					*entryPtr = offset;
				}
			}

			~MemBlock()
			{
				B3D_ASSERT(FreeElems == ElemsPerBlock && "Not all elements were deallocated from a block.");
			}

			/**
			 * Returns the first free address and increments the free pointer. Caller needs to ensure the remaining block
			 * size is adequate before calling.
			 */
			u8* Alloc()
			{
				u8* freeEntry = &BlockData[FreePtr];
				FreePtr = *(u32*)freeEntry;
				--FreeElems;

				return freeEntry;
			}

			/** Deallocates the provided pointer. */
			void Dealloc(void* data)
			{
				u32* entryPtr = (u32*)data;
				*entryPtr = FreePtr;
				++FreeElems;

				FreePtr = (u32)(((u8*)data) - BlockData);
			}

			u8* BlockData;
			u32 FreePtr;
			u32 FreeElems;
			MemBlock* NextBlock;
		};

	public:
		PoolAlloc()
		{
			static_assert(ElemSize >= 4, "Pool allocator minimum allowed element size is 4 bytes.");
			static_assert(ElemsPerBlock > 0, "Number of elements per block must be at least 1.");
			static_assert(ElemsPerBlock * kActualElemSize <= UINT_MAX, "Pool allocator block size too large.");
		}

		~PoolAlloc()
		{
			ScopedLock<Lock> lock(mLockPolicy);

			MemBlock* curBlock = mFreeBlock;
			while(curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->NextBlock;
				DeallocBlock(curBlock);

				curBlock = nextBlock;
			}
		}

		/** Allocates enough memory for a single element in the pool. */
		u8* Alloc()
		{
			ScopedLock<Lock> lock(mLockPolicy);

			if(mFreeBlock == nullptr || mFreeBlock->FreeElems == 0)
				AllocBlock();

			mTotalNumElems++;
			u8* output = mFreeBlock->Alloc();

			return output;
		}

		/** Deallocates an element from the pool. */
		void Free(void* data)
		{
			ScopedLock<Lock> lock(mLockPolicy);

			MemBlock* curBlock = mFreeBlock;
			while(curBlock)
			{
				constexpr u32 blockDataSize = kActualElemSize * ElemsPerBlock;
				if(data >= curBlock->BlockData && data < (curBlock->BlockData + blockDataSize))
				{
					curBlock->Dealloc(data);
					mTotalNumElems--;

					if(curBlock->FreeElems == 0 && curBlock->NextBlock)
					{
						// Free the block, but only if there is some extra free space in other blocks
						const u32 totalSpace = (mNumBlocks - 1) * ElemsPerBlock;
						const u32 freeSpace = totalSpace - mTotalNumElems;

						if(freeSpace > ElemsPerBlock / 2)
						{
							mFreeBlock = curBlock->NextBlock;
							DeallocBlock(curBlock);
						}
					}

					return;
				}

				curBlock = curBlock->NextBlock;
			}

			B3D_ASSERT(false);
		}

		/** Allocates and constructs a single pool element. */
		template <class T, class... Args>
		T* Construct(Args&&... args)
		{
			T* data = (T*)Alloc();
			new((void*)data) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates a single pool element. */
		template <class T>
		void Destruct(T* data)
		{
			data->~T();
			Free(data);
		}

	private:
		/** Allocates a new block of memory using a heap allocator. */
		MemBlock* AllocBlock()
		{
			MemBlock* newBlock = nullptr;
			MemBlock* curBlock = mFreeBlock;

			while(curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->NextBlock;
				if(nextBlock != nullptr && nextBlock->FreeElems > 0)
				{
					// Found an existing block with free space
					newBlock = nextBlock;

					curBlock->NextBlock = newBlock->NextBlock;
					newBlock->NextBlock = mFreeBlock;

					break;
				}

				curBlock = nextBlock;
			}

			if(newBlock == nullptr)
			{
				constexpr u32 blockDataSize = kActualElemSize * ElemsPerBlock;
				size_t paddedBlockDataSize = blockDataSize + (Alignment - 1); // Padding for potential alignment correction

				u8* data = (u8*)B3DAllocate(sizeof(MemBlock) + (u32)paddedBlockDataSize);

				void* blockData = data + sizeof(MemBlock);
				blockData = std::align(Alignment, blockDataSize, blockData, paddedBlockDataSize);

				newBlock = new(data) MemBlock((u8*)blockData);
				mNumBlocks++;

				newBlock->NextBlock = mFreeBlock;
			}

			mFreeBlock = newBlock;
			return newBlock;
		}

		/** Deallocates a block of memory. */
		void DeallocBlock(MemBlock* block)
		{
			block->~MemBlock();
			B3DFree(block);

			mNumBlocks--;
		}

		static constexpr int kActualElemSize = ((ElemSize + Alignment - 1) / Alignment) * Alignment;

		LockingPolicy<Lock> mLockPolicy;
		MemBlock* mFreeBlock = nullptr;
		u32 mTotalNumElems = 0;
		u32 mNumBlocks = 0;
	};

	/**
	 * Helper class used by GlobalPoolAlloc that allocates a static pool allocator. GlobalPoolAlloc cannot do it
	 * directly since it gets specialized which means the static members would need to be defined in the implementation
	 * file, which complicates its usage.
	 */
	template <class T, int ElemsPerBlock = 512, int Alignment = 4, bool Lock = true>
	class StaticPoolAlloc
	{
	public:
		static PoolAlloc<sizeof(T), ElemsPerBlock, Alignment, Lock> m;
	};

	template <class T, int ElemsPerBlock, int Alignment, bool Lock>
	PoolAlloc<sizeof(T), ElemsPerBlock, Alignment, Lock> StaticPoolAlloc<T, ElemsPerBlock, Alignment, Lock>::m;

	/** Specializable template that allows users to implement globally accessible pool allocators for custom types. */
	template <class T>
	class GlobalPoolAlloc : std::false_type
	{
		template <typename T2>
		struct AlwaysFalse : std::false_type
		{};

		static_assert(AlwaysFalse<T>::value, "No global pool allocator exists for the type.");
	};

	/**
	 * Implements a global pool for the specified type. The pool will initially have enough room for ElemsPerBlock and
	 * will grow by that amount when exceeded. Global pools are thread safe by default.
	 */
#define B3D_IMPLEMENT_GLOBAL_POOL(Type, ElemsPerBlock)                            \
	template <>                                                               \
	class GlobalPoolAlloc<Type> : public StaticPoolAlloc<Type, ElemsPerBlock> \
	{};

	/** Allocates a new object of type T using the global pool allocator, without constructing it. */
	template <class T>
	T* B3DPoolAllocate()
	{
		return (T*)GlobalPoolAlloc<T>::m.Alloc();
	}

	/** Allocates and constructs a new object of type T using the global pool allocator. */
	template <class T, class... Args>
	T* B3DPoolNew(Args&&... args)
	{
		T* data = B3DPoolAllocate<T>();
		new((void*)data) T(std::forward<Args>(args)...);

		return data;
	}

	/** Frees the provided object using its global pool allocator, without destructing it. */
	template <class T>
	void B3DPoolFree(T* ptr)
	{
		GlobalPoolAlloc<T>::m.Free(ptr);
	}

	/** Frees and destructs the provided object using its global pool allocator. */
	template <class T>
	void B3DPoolDelete(T* ptr)
	{
		ptr->~T();
		B3DPoolFree(ptr);
	}

	/** @} */
} // namespace b3d
