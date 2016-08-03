//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <limits>
#include <new>                  /* For 'placement new' */

#include "BsPrerequisitesUtil.h"

namespace BansheeEngine
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Frame allocator. Performs very fast allocations but can only free all of its memory at once. Perfect for allocations 
	 * that last just a single frame.
	 * 			
	 * @note	Not thread safe with an exception. alloc() and clear() methods need to be called from the same thread.
	 * 			dealloc() is thread safe and can be called from any thread.
	 */
	class BS_UTILITY_EXPORT FrameAlloc
	{
	private:
		/** A single block of memory within a frame allocator. */
		class MemBlock
		{
		public:
			MemBlock(UINT32 size);
			~MemBlock();

			/** Allocates a piece of memory within the block. Caller must ensure the block has enough empty space. */
			UINT8* alloc(UINT32 amount);

			/** Releases all allocations within a block but doesn't actually free the memory. */
			void clear();

			UINT8* mData;
			UINT32 mFreePtr;
			UINT32 mSize;
		};

	public:
		FrameAlloc(UINT32 blockSize = 1024 * 1024);
		~FrameAlloc();

		/**
		 * Allocates a new block of memory of the specified size.
		 *
		 * @param[in]	amount	Amount of memory to allocate, in bytes.
		 * 					
		 * @note	Not thread safe.
		 */
		UINT8* alloc(UINT32 amount);

		/**
		 * Allocates a new block of memory of the specified size aligned to the specified boundary. If the aligment is less
		 * or equal to 16 it is more efficient to use the allocAligned16() alternative of this method.
		 *
		 * @param[in]	amount		Amount of memory to allocate, in bytes.
		 * @param[in]	alignment	Alignment of the allocated memory. Must be power of two.
		 * 					
		 * @note	Not thread safe.
		 */
		UINT8* allocAligned(UINT32 amount, UINT32 alignment);

		/**
		 * Allocates and constructs a new object.
		 *	
		 * @note	Not thread safe.
		 */
		template<class T, class... Args>
		T* alloc(Args &&...args)
		{
			return new ((T*)alloc(sizeof(T))) T(std::forward<Args>(args)...);
		}

		/**
		 * Deallocates a previously allocated block of memory.
		 *
		 * @note
		 * No deallocation is actually done here. This method is only used for debug purposes so it is easier to track 
		 * down memory leaks and corruption.
		 * @note
		 * Thread safe.
		 */
		void dealloc(UINT8* data);

		/**
		 * Deallocates and destructs a previously allocated object.
		 *
		 * @note	
		 * No deallocation is actually done here. This method is only used to call the destructor and for debug purposes 
		 * so it is easier to track down memory leaks and corruption.
		 * @note
		 * Thread safe.
		 */
		template<class T>
		void dealloc(T* obj)
		{
			if (obj != nullptr)
				obj->~T();

			dealloc((UINT8*)obj);
		}

		/** Starts a new frame. Next call to clear() will only clear memory allocated past this point. */
		void markFrame();

		/**
		 * Deallocates all allocated memory since the last call to markFrame() (or all the memory if there was no call 
		 * to markFrame()).
		 * 			
		 * @note	Not thread safe.
		 */
		void clear();

		/**
		 * Changes the frame allocator owner thread. After the owner thread has changed only allocations from that thread 
		 * can be made.
		 */
		void setOwnerThread(ThreadId thread);

	private:
		UINT32 mBlockSize;
		Vector<MemBlock*> mBlocks;
		MemBlock* mFreeBlock;
		UINT32 mNextBlockIdx;
		std::atomic<UINT32> mTotalAllocBytes;
		void* mLastFrame;

#if BS_DEBUG_MODE
		ThreadId mOwnerThread;
#endif

		/**
		 * Allocates a dynamic block of memory of the wanted size. The exact allocation size might be slightly higher in 
		 * order to store block meta data.
		 */
		MemBlock* allocBlock(UINT32 wantedSize);

		/** Frees a memory block. */
		void deallocBlock(MemBlock* block);
	};

	/** Allocator for the standard library that internally uses a frame allocator. */
	template <class T>
	class StdFrameAlloc
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		StdFrameAlloc() noexcept 
			:mFrameAlloc(nullptr)
		{ }

		StdFrameAlloc(FrameAlloc* alloc) noexcept
			:mFrameAlloc(alloc)
		{ }

		template<class U> StdFrameAlloc(const StdFrameAlloc<U>& alloc) noexcept
			:mFrameAlloc(alloc.mFrameAlloc)
		{ }

		template<class U> bool operator==(const StdFrameAlloc<U>&) const noexcept { return true; }
		template<class U> bool operator!=(const StdFrameAlloc<U>&) const noexcept { return false; }
		template<class U> class rebind { public: typedef StdFrameAlloc<U> other; };

		/** Allocate but don't initialize number elements of type T.*/
		T* allocate(const size_t num) const
		{
			if (num == 0)
				return nullptr;

			if (num > static_cast<size_t>(-1) / sizeof(T))
				return nullptr; // Error

			void* const pv = mFrameAlloc->alloc((UINT32)(num * sizeof(T)));
			if (!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		void deallocate(T* p, size_t num) const noexcept
		{
			mFrameAlloc->dealloc((UINT8*)p);
		}

		FrameAlloc* mFrameAlloc;

		size_t max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
		void construct(pointer p, const_reference t) { new (p) T(t); }
		void destroy(pointer p) { p->~T(); }
		template<class U, class... Args>
		void construct(U* p, Args&&... args) { new(p) U(std::forward<Args>(args)...); }

	};

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, class T2>
	bool operator== (const StdFrameAlloc<T1>&,
		const StdFrameAlloc<T2>&) throw() {
		return true;
	}

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, class T2>
	bool operator!= (const StdFrameAlloc<T1>&,
		const StdFrameAlloc<T2>&) throw() {
		return false;
	}

	/** @} */
	/** @} */
}
