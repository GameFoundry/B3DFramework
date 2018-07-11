//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <stack>
#include <assert.h>

#include "Prerequisites/BsTypes.h"
#include "Prerequisites/BsStdHeaders.h"

#include "Threading/BsThreading.h"

namespace bs
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
			MemBlock(UINT32 size) :mSize(size) { }

			~MemBlock() = default;

			/**
			 * Returns the first free address and increments the free pointer. Caller needs to ensure the remaining block
			 * size is adequate before calling.
			 */
			UINT8* alloc(UINT32 amount)
			{
				UINT8* freePtr = &mData[mFreePtr];
				mFreePtr += amount;

				return freePtr;
			}

			/**
			 * Deallocates the provided pointer. Deallocation must happen in opposite order from allocation otherwise
			 * corruption will occur.
			 *
			 * @note	Pointer to @p data isn't actually needed, but is provided for debug purposes in order to more
			 * 			easily track out-of-order deallocations.
			 */
			void dealloc(UINT8* data, UINT32 amount)
			{
				mFreePtr -= amount;
				assert((&mData[mFreePtr]) == data && "Out of order stack deallocation detected. Deallocations need to happen in order opposite of allocations.");
			}

			UINT8* mData = nullptr;
			UINT32 mFreePtr = 0;
			UINT32 mSize = 0;
			MemBlock* mNextBlock = nullptr;
			MemBlock* mPrevBlock = nullptr;
		};

	public:
		MemStackInternal()
		{
			mFreeBlock = allocBlock(BlockCapacity);
		}

		~MemStackInternal()
		{
			assert(mFreeBlock->mFreePtr == 0 && "Not all blocks were released before shutting down the stack allocator.");

			MemBlock* curBlock = mFreeBlock;
			while (curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->mNextBlock;
				deallocBlock(curBlock);

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
		UINT8* alloc(UINT32 amount)
		{
			amount += sizeof(UINT32);

			UINT32 freeMem = mFreeBlock->mSize - mFreeBlock->mFreePtr;
			if(amount > freeMem)
				allocBlock(amount);

			UINT8* data = mFreeBlock->alloc(amount);

			UINT32* storedSize = reinterpret_cast<UINT32*>(data);
			*storedSize = amount;

			return data + sizeof(UINT32);
		}

		/** Deallocates the given memory. Data must be deallocated in opposite order then when it was allocated. */
		void dealloc(UINT8* data)
		{
			data -= sizeof(UINT32);

			UINT32* storedSize = reinterpret_cast<UINT32*>(data);
			mFreeBlock->dealloc(data, *storedSize);

			if (mFreeBlock->mFreePtr == 0)
			{
				MemBlock* emptyBlock = mFreeBlock;

				if (emptyBlock->mPrevBlock != nullptr)
					mFreeBlock = emptyBlock->mPrevBlock;

				// Merge with next block
				if (emptyBlock->mNextBlock != nullptr)
				{
					UINT32 totalSize = emptyBlock->mSize + emptyBlock->mNextBlock->mSize;

					if (emptyBlock->mPrevBlock != nullptr)
						emptyBlock->mPrevBlock->mNextBlock = nullptr;
					else
						mFreeBlock = nullptr;

					deallocBlock(emptyBlock->mNextBlock);
					deallocBlock(emptyBlock);

					allocBlock(totalSize);
				}
			}
		}

	private:
		MemBlock* mFreeBlock = nullptr;

		/**
		 * Allocates a new block of memory using a heap allocator. Block will never be smaller than BlockCapacity no matter
		 * the @p wantedSize.
		 */
		MemBlock* allocBlock(UINT32 wantedSize)
		{
			UINT32 blockSize = BlockCapacity;
			if(wantedSize > blockSize)
				blockSize = wantedSize;

			MemBlock* newBlock = nullptr;
			MemBlock* curBlock = mFreeBlock;

			while (curBlock != nullptr)
			{
				MemBlock* nextBlock = curBlock->mNextBlock;
				if (nextBlock != nullptr && nextBlock->mSize >= blockSize)
				{
					newBlock = nextBlock;
					break;
				}

				curBlock = nextBlock;
			}

			if (newBlock == nullptr)
			{
				UINT8* data = (UINT8*)reinterpret_cast<UINT8*>(bs_alloc(blockSize + sizeof(MemBlock)));
				newBlock = new (data)MemBlock(blockSize);
				data += sizeof(MemBlock);

				newBlock->mData = data;
				newBlock->mPrevBlock = mFreeBlock;

				if (mFreeBlock != nullptr)
				{
					if(mFreeBlock->mNextBlock != nullptr)
						mFreeBlock->mNextBlock->mPrevBlock = newBlock;

					newBlock->mNextBlock = mFreeBlock->mNextBlock;
					mFreeBlock->mNextBlock = newBlock;
				}
			}

			mFreeBlock = newBlock;
			return newBlock;
		}

		/** Deallocates a block of memory. */
		void deallocBlock(MemBlock* block)
		{
			block->~MemBlock();
			bs_free(block);
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
		static BS_UTILITY_EXPORT void beginThread();

		/**
		 * Cleans up the stack for the current thread. You may not perform any allocations or deallocations after this is
		 * called, unless you call beginThread again.
		 */
		static BS_UTILITY_EXPORT void endThread();

		/** @copydoc MemStackInternal::alloc() */
		static BS_UTILITY_EXPORT UINT8* alloc(UINT32 amount);

		/** @copydoc MemStackInternal::dealloc() */
		static BS_UTILITY_EXPORT void deallocLast(UINT8* data);

	private:
		static BS_THREADLOCAL MemStackInternal<1024 * 1024>* ThreadMemStack;
	};

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** @copydoc MemStackInternal::alloc() */
	inline void* bs_stack_alloc(UINT32 amount)
	{
		return (void*)MemStack::alloc(amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, but does not initialize the object.
	 *
	 * @see	MemStackInternal::alloc()
	 */
	template<class T>
	T* bs_stack_alloc()
	{
		return (T*)MemStack::alloc(sizeof(T));
	}

	/**
	 * Allocates enough memory to hold N objects of the specified type, on the stack, but does not initialize the objects.
	 *
	 * @param[in]	amount	Number of entries of the requested type to allocate.
	 *
	 * @see	MemStackInternal::alloc()
	 */
	template<class T>
	T* bs_stack_alloc(UINT32 amount)
	{
		return (T*)MemStack::alloc(sizeof(T) * amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, and constructs the object.
	 *
	 * @see	MemStackInternal::alloc()
	 */
	template<class T>
	T* bs_stack_new(UINT32 count = 0)
	{
		T* data = bs_stack_alloc<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&data[i]) T;

		return data;
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, and constructs the object.
	 *
	 * @see MemStackInternal::alloc()
	 */
	template<class T, class... Args>
	T* bs_stack_new(Args &&...args, UINT32 count = 0)
	{
		T* data = bs_stack_alloc<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&data[i]) T(std::forward<Args>(args)...);

		return data;
	}

	/**
	 * Destructs and deallocates last allocated entry currently located on stack.
	 *
	 * @see MemStackInternal::dealloc()
	 */
	template<class T>
	void bs_stack_delete(T* data)
	{
		data->~T();

		MemStack::deallocLast((UINT8*)data);
	}

	/**
	 * Destructs an array of objects and deallocates last allocated entry currently located on stack.
	 *
	 * @see	MemStackInternal::dealloc()
	 */
	template<class T>
	void bs_stack_delete(T* data, UINT32 count)
	{
		for(unsigned int i = 0; i < count; i++)
			data[i].~T();

		MemStack::deallocLast((UINT8*)data);
	}

	inline void bs_stack_delete(void* data, UINT32 count)
	{
		MemStack::deallocLast((UINT8*)data);
	}

	/** @copydoc MemStackInternal::dealloc() */
	inline void bs_stack_free(void* data)
	{
		return MemStack::deallocLast((UINT8*)data);
	}

	/**
	 * An object used to transparently clean up a stack allocation when it's no longer in scope. Make sure to take great 
	 * care not to free non-managed stack allocations out of order or to free the stack allocation managed by this object!
	 */
	template<typename T>
	struct StackMemory
	{
		/*
		 * Provide implicit conversion to the allocated buffer so that users of this code can "pretend" this object is a 
		 * pointer to the stack buffer that they wanted.
		 */
		constexpr operator T*() const & noexcept
		{
			return mPtr;
		}

		/*
		 * This ensures that the result of bs_managed_stack_alloc() doesn't get passed to a function call as a temporary, 
		 * or immediately assigned as a T*. Instead, the user of this class is forced to deal with this class as itself, 
		 * when handling the return value of bs_managed_stack_alloc() preventing an immediate (and erroneous) call to 
		 * bs_stack_free().
		 */
		constexpr operator T*() const && noexcept = delete;

		explicit constexpr StackMemory(T* p, size_t count = 1)
		 : mPtr(p), mCount(count)
		{ }

		/** Needed until c++17 */
		StackMemory(StackMemory && other)
		 : mPtr(std::exchange(other.mPtr, nullptr))
		 , mCount(std::exchange(other.mCount, 0))
		{ }

		StackMemory(StackMemory const&) = delete;
		StackMemory& operator=(StackMemory &&)     = delete;
		StackMemory& operator=(StackMemory const&) = delete;

		/** Frees the stack allocation. */
		~StackMemory()
		{
			if(mPtr != nullptr)
			{
				if(mCount >= 1)
					bs_stack_delete(mPtr, (UINT32)mCount);
				else
					bs_stack_free(mPtr);
			}
		}

	private:
		T* mPtr = nullptr;
		size_t mCount = 0;
	};

	/** 
	 * Same as bs_stack_alloc() except the returned object takes care of automatically cleaning up when it goes out of 
	 * scope. 
	 */
	inline StackMemory<void> bs_managed_stack_alloc(UINT32 amount)
	{
		return StackMemory<void>(bs_stack_alloc(amount));
	}

	/** 
	 * Same as bs_stack_alloc() except the returned object takes care of automatically cleaning up when it goes out of 
	 * scope. 
	 */
	template<class T>
	StackMemory<T> bs_managed_stack_alloc()
	{
		return StackMemory<T>(bs_stack_alloc<T>());
	}

	/** 
	 * Same as bs_stack_alloc() except the returned object takes care of automatically cleaning up when it goes out of 
	 * scope. 
	 */
	template<class T>
	StackMemory<T> bs_managed_stack_alloc(UINT32 amount)
	{
		return StackMemory<T>(bs_stack_alloc<T>(amount));
	}

	/** 
	 * Same as bs_stack_new() except the returned object takes care of automatically cleaning up when it goes out of 
	 * scope. 
	 */
	template<class T>
	StackMemory<T> bs_managed_stack_new(size_t count = 1)
	{
		return StackMemory<T>(bs_stack_new<T>(count), count);
	}

	/** 
	 * Same as bs_stack_new() except the returned object takes care of automatically cleaning up when it goes out of 
	 * scope. 
	 */
	template<class T, class... Args>
	StackMemory<T> bs_managed_stack_new(Args && ... args, size_t count = 1)
	{
		return StackMemory<T>(bs_stack_new<T>(std::forward<Args>(args)..., count), count);
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
	class StackAlloc
	{ };

	/**
	* Specialized memory allocator implementations that allows use of a stack allocator in normal new/delete/free/dealloc
	* operators.
	*
	* @see MemStack
	*/
	template<>
	class MemoryAllocator<StackAlloc> : public MemoryAllocatorBase
	{
	public:
		static void* allocate(size_t bytes)
		{
			return bs_stack_alloc((UINT32)bytes);
		}

		static void free(void* ptr)
		{
			bs_stack_free(ptr);
		}
	};

	/** @} */
	/** @} */
}
