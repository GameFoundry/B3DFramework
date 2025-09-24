//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <stack>
#include <assert.h>

#include "Prerequisites/B3DTypes.h"
#include "Prerequisites/B3DStdHeaders.h"

#include "Threading/B3DThreading.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Describes a memory stack of a certain block capacity. See MemStack for more information.
	 *
	 * @tparam	BlockCapacity Minimum size of a block. Larger blocks mean less memory allocations, but also potentially
	 *						  more wasted memory. If an allocation requests more bytes than BlockCapacity, first largest
	 *						  multiple is used instead.
	 */
	template <int BlockCapacity = 1024 * 1024>
	class MemStackInternal
	{
	private:
		/**
		 * A single block of memory of BlockCapacity size. A pointer to the first free address is stored, and a remaining
		 * size.
		 */
		class MemBlock
		{
		public:
			MemBlock(u32 size)
				: MSize(size) {}

			~MemBlock() = default;

			/**
			 * Returns the first free address and increments the free pointer. Caller needs to ensure the remaining block
			 * size is adequate before calling.
			 */
			u8* Alloc(u32 amount)
			{
				u8* freePtr = &MData[MFreePtr];
				MFreePtr += amount;

				return freePtr;
			}

			/**
			 * Deallocates the provided pointer. Deallocation must happen in opposite order from allocation otherwise
			 * corruption will occur.
			 *
			 * @note	Pointer to @p data isn't actually needed, but is provided for debug purposes in order to more
			 * 			easily track out-of-order deallocations.
			 */
			void Dealloc(u8* data, u32 amount)
			{
				MFreePtr -= amount;
				B3D_ASSERT((&MData[MFreePtr]) == data && "Out of order stack deallocation detected. Deallocations need to happen in order opposite of allocations.");
			}

			u8* MData = nullptr;
			u32 MFreePtr = 0;
			u32 MSize = 0;
			MemBlock* MNextBlock = nullptr;
			MemBlock* MPrevBlock = nullptr;
		};

	public:
		MemStackInternal()
		{
			mFreeBlock = AllocBlock(BlockCapacity);
		}

		~MemStackInternal()
		{
			B3D_ASSERT(mFreeBlock->MFreePtr == 0 && "Not all blocks were released before shutting down the stack allocator.");

			MemBlock* curBlock = mFreeBlock;
			while(curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->MNextBlock;
				DeallocBlock(curBlock);

				curBlock = nextBlock;
			}
		}

		/**
		 * Allocates the given amount of memory on the stack.
		 *
		 * @param[in]	amount	The amount to allocate in bytes.
		 *
		 * @note
		 * Allocates the memory in the currently active block if it is large enough, otherwise a new block is allocated.
		 * If the allocation is larger than default block size a separate block will be allocated only for that allocation,
		 * making it essentially a slower heap allocator.
		 * @note
		 * Each allocation comes with a 4 byte overhead.
		 */
		u8* Alloc(u32 amount)
		{
			amount += sizeof(u32);

			u32 freeMem = mFreeBlock->MSize - mFreeBlock->MFreePtr;
			if(amount > freeMem)
				AllocBlock(amount);

			u8* data = mFreeBlock->Alloc(amount);

			u32* storedSize = reinterpret_cast<u32*>(data);
			*storedSize = amount;

			return data + sizeof(u32);
		}

		/** Deallocates the given memory. Data must be deallocated in opposite order then when it was allocated. */
		void Dealloc(u8* data)
		{
			data -= sizeof(u32);

			u32* storedSize = reinterpret_cast<u32*>(data);
			mFreeBlock->Dealloc(data, *storedSize);

			if(mFreeBlock->MFreePtr == 0)
			{
				MemBlock* emptyBlock = mFreeBlock;

				if(emptyBlock->MPrevBlock != nullptr)
					mFreeBlock = emptyBlock->MPrevBlock;

				// Merge with next block
				if(emptyBlock->MNextBlock != nullptr)
				{
					u32 totalSize = emptyBlock->MSize + emptyBlock->MNextBlock->MSize;

					if(emptyBlock->MPrevBlock != nullptr)
						emptyBlock->MPrevBlock->MNextBlock = nullptr;
					else
						mFreeBlock = nullptr;

					DeallocBlock(emptyBlock->MNextBlock);
					DeallocBlock(emptyBlock);

					AllocBlock(totalSize);
				}
			}
		}

	private:
		MemBlock* mFreeBlock = nullptr;

		/**
		 * Allocates a new block of memory using a heap allocator. Block will never be smaller than BlockCapacity no matter
		 * the @p wantedSize.
		 */
		MemBlock* AllocBlock(u32 wantedSize)
		{
			u32 blockSize = BlockCapacity;
			if(wantedSize > blockSize)
				blockSize = wantedSize;

			MemBlock* newBlock = nullptr;
			MemBlock* curBlock = mFreeBlock;

			while(curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->MNextBlock;
				if(nextBlock != nullptr && nextBlock->MSize >= blockSize)
				{
					newBlock = nextBlock;
					break;
				}

				curBlock = nextBlock;
			}

			if(newBlock == nullptr)
			{
				u8* data = (u8*)reinterpret_cast<u8*>(B3DAllocate(blockSize + sizeof(MemBlock)));
				newBlock = new(data) MemBlock(blockSize);
				data += sizeof(MemBlock);

				newBlock->MData = data;
				newBlock->MPrevBlock = mFreeBlock;

				if(mFreeBlock != nullptr)
				{
					if(mFreeBlock->MNextBlock != nullptr)
						mFreeBlock->MNextBlock->MPrevBlock = newBlock;

					newBlock->MNextBlock = mFreeBlock->MNextBlock;
					mFreeBlock->MNextBlock = newBlock;
				}
			}

			mFreeBlock = newBlock;
			return newBlock;
		}

		/** Deallocates a block of memory. */
		void DeallocBlock(MemBlock* block)
		{
			block->~MemBlock();
			B3DFree(block);
		}
	};

	/**
	 * One of the fastest, but also very limiting type of allocator. All deallocations must happen in opposite order from
	 * allocations.
	 *
	 * @note
	 * It's mostly useful when you need to allocate something temporarily on the heap, usually something that gets
	 * allocated and freed within the same method.
	 * @note
	 * Each allocation comes with a pretty hefty 4 byte memory overhead, so don't use it for small allocations.
	 * @note
	 * Thread safe. But you cannot allocate on one thread and deallocate on another. Threads will keep
	 * separate stacks internally. Make sure to call beginThread()/endThread() for any thread this stack is used on.
	 */
	class MemStack
	{
	public:
		/**
		 * Sets up the stack with the currently active thread. You need to call this on any thread before doing any
		 * allocations or deallocations.
		 */
		static B3D_UTILITY_EXPORT void BeginThread();

		/**
		 * Cleans up the stack for the current thread. You may not perform any allocations or deallocations after this is
		 * called, unless you call beginThread again.
		 */
		static B3D_UTILITY_EXPORT void EndThread();

		/** @copydoc MemStackInternal::Alloc() */
		static B3D_UTILITY_EXPORT u8* Alloc(u32 amount);

		/** @copydoc MemStackInternal::Dealloc() */
		static B3D_UTILITY_EXPORT void DeallocLast(u8* data);

	private:
		static B3D_THREADLOCAL MemStackInternal<1024 * 1024>* ThreadMemStack;
	};

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** @copydoc MemStackInternal::Alloc() */
	inline void* B3DStackAllocate(u32 amount)
	{
		return (void*)MemStack::Alloc(amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, but does not initialize the object.
	 *
	 * @see	MemStackInternal::Alloc()
	 */
	template <class T>
	T* B3DStackAllocate()
	{
		return (T*)MemStack::Alloc(sizeof(T));
	}

	/**
	 * Allocates enough memory to hold N objects of the specified type, on the stack, but does not initialize the objects.
	 *
	 * @param[in]	amount	Number of entries of the requested type to allocate.
	 *
	 * @see	MemStackInternal::Alloc()
	 */
	template <class T>
	T* B3DStackAllocate(u32 amount)
	{
		return (T*)MemStack::Alloc(sizeof(T) * amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, and constructs the object.
	 *
	 * @see	MemStackInternal::Alloc()
	 */
	template <class T>
	T* B3DStackNew(u32 count = 0)
	{
		T* data = B3DStackAllocate<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new((void*)&data[i]) T;

		return data;
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, and constructs the object.
	 *
	 * @see MemStackInternal::Alloc()
	 */
	template <class T, class... Args>
	T* B3DStackNew(Args&&... args, u32 count = 0)
	{
		T* data = B3DStackAllocate<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new((void*)&data[i]) T(std::forward<Args>(args)...);

		return data;
	}

	/**
	 * Destructs and deallocates last allocated entry currently located on stack.
	 *
	 * @see MemStackInternal::Dealloc()
	 */
	template <class T>
	void B3DStackDelete(T* data)
	{
		data->~T();

		MemStack::DeallocLast((u8*)data);
	}

	/**
	 * Destructs an array of objects and deallocates last allocated entry currently located on stack.
	 *
	 * @see	MemStackInternal::Dealloc()
	 */
	template <class T>
	void B3DStackDelete(T* data, u32 count)
	{
		for(unsigned int i = 0; i < count; i++)
			data[i].~T();

		MemStack::DeallocLast((u8*)data);
	}

	inline void B3DStackDelete(void* data, u32 count)
	{
		MemStack::DeallocLast((u8*)data);
	}

	/** @copydoc MemStackInternal::Dealloc() */
	inline void B3DStackFree(void* data)
	{
		return MemStack::DeallocLast((u8*)data);
	}

	/**
	 * An object used to transparently clean up a stack allocation when it's no longer in scope. Make sure to take great
	 * care not to free non-managed stack allocations out of order or to free the stack allocation managed by this object!
	 */
	template <typename T>
	struct StackMemory
	{
		/*
		 * Provide implicit conversion to the allocated buffer so that users of this code can "pretend" this object is a
		 * pointer to the stack buffer that they wanted.
		 */
		constexpr operator T*() const& noexcept
		{
			return mPtr;
		}

		/*
		 * This ensures that the result of B3DManagedStackAllocate() doesn't get passed to a function call as a temporary,
		 * or immediately assigned as a T*. Instead, the user of this class is forced to deal with this class as itself,
		 * when handling the return value of B3DManagedStackAllocate() preventing an immediate (and erroneous) call to
		 * B3DStackFree().
		 */
		constexpr operator T*() const&& noexcept = delete;

		explicit constexpr StackMemory(T* p, size_t count = 1)
			: mPtr(p), mCount(count)
		{}

		/** Needed until c++17 */
		StackMemory(StackMemory&& other)
			: mPtr(std::exchange(other.mPtr, nullptr))
			, mCount(std::exchange(other.mCount, 0))
		{}

		StackMemory(StackMemory const&) = delete;
		StackMemory& operator=(StackMemory&&) = delete;
		StackMemory& operator=(StackMemory const&) = delete;

		/** Frees the stack allocation. */
		~StackMemory()
		{
			if(mPtr != nullptr)
			{
				if(mCount >= 1)
					B3DStackDelete(mPtr, (u32)mCount);
				else
					B3DStackFree(mPtr);
			}
		}

	private:
		T* mPtr = nullptr;
		size_t mCount = 0;
	};

	/**
	 * Same as B3DStackAllocate() except the returned object takes care of automatically cleaning up when it goes out of
	 * scope.
	 */
	inline StackMemory<void> B3DManagedStackAllocate(u32 amount)
	{
		return StackMemory<void>(B3DStackAllocate(amount));
	}

	/**
	 * Same as B3DStackAllocate() except the returned object takes care of automatically cleaning up when it goes out of
	 * scope.
	 */
	template <class T>
	StackMemory<T> B3DManagedStackAllocate()
	{
		return StackMemory<T>(B3DStackAllocate<T>());
	}

	/**
	 * Same as B3DStackAllocate() except the returned object takes care of automatically cleaning up when it goes out of
	 * scope.
	 */
	template <class T>
	StackMemory<T> B3DManagedStackAllocate(u32 amount)
	{
		return StackMemory<T>(B3DStackAllocate<T>(amount));
	}

	/**
	 * Same as B3DStackNew() except the returned object takes care of automatically cleaning up when it goes out of
	 * scope.
	 */
	template <class T>
	StackMemory<T> B3DManagedStackNew(size_t count = 1)
	{
		return StackMemory<T>(B3DStackNew<T>(count), count);
	}

	/**
	 * Same as B3DStackNew() except the returned object takes care of automatically cleaning up when it goes out of
	 * scope.
	 */
	template <class T, class... Args>
	StackMemory<T> B3DManagedStackNew(Args&&... args, size_t count = 1)
	{
		return StackMemory<T>(B3DStackNew<T>(std::forward<Args>(args)..., count), count);
	}

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Allows use of a stack allocator by using normal new/delete/free/dealloc operators.
	 *
	 * @see	MemStack
	 */
	class StackAllocatorTag
	{};

	/**
	 * Specialized memory allocator implementations that allows use of a stack allocator in normal new/delete/free/dealloc
	 * operators.
	 *
	 * @see MemStack
	 */
	template <>
	class MemoryAllocator<StackAllocatorTag> : public MemoryAllocatorBase
	{
	public:
		static void* Allocate(size_t bytes)
		{
			return B3DStackAllocate((u32)bytes);
		}

		static void Free(void* ptr)
		{
			B3DStackFree(ptr);
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
