//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <limits>
#include <new>                  /* For 'placement new' */

#include "Prerequisites/BsPlatformDefines.h"
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
			MemBlock(UINT32 size) :mSize(size) { }

			~MemBlock() = default;

			/** Allocates a piece of memory within the block. Caller must ensure the block has enough empty space. */
			UINT8* alloc(UINT32 amount);

			/** Releases all allocations within a block but doesn't actually free the memory. */
			void clear();

			UINT8* mData = nullptr;
			UINT32 mFreePtr = 0;
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
		T* construct(Args &&...args)
		{
			return new ((T*)alloc(sizeof(T))) T(std::forward<Args>(args)...);
		}

		/**
		 * Destructs and deallocates an object.
		 *	
		 * @note	Not thread safe.
		 */
		template<class T>
		void destruct(T* data)
		{
			data->~T();
			free((UINT8*)data);
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
		void free(UINT8* data);

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
		void free(T* obj)
		{
			if (obj != nullptr)
				obj->~T();

			free((UINT8*)obj);
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

	/** 
	 * Version of FrameAlloc that allows blocks size to be provided through the template argument instead of the 
	 * constructor. */
	template<int BlockSize>
	class TFrameAlloc : public FrameAlloc
	{
	public:
		TFrameAlloc()
			:FrameAlloc(BlockSize)
		{ }
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

		StdFrameAlloc() noexcept = default;

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
			mFrameAlloc->free((UINT8*)p);
		}

		FrameAlloc* mFrameAlloc = nullptr;

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

	/** @addtogroup Memory
	 *  @{
	 */

	/**
	 * Returns a global, application wide FrameAlloc. Each thread gets its own frame allocator.
	 *
	 * @note	Thread safe.
	 */
	BS_UTILITY_EXPORT FrameAlloc& gFrameAlloc();

	/**
	 * Allocates some memory using the global frame allocator.
	 *
	 * @param[in]	numBytes	Number of bytes to allocate.
	 */
	BS_UTILITY_EXPORT UINT8* bs_frame_alloc(UINT32 numBytes);

	/** 
	 * Allocates the specified number of bytes aligned to the provided boundary, using the global frame allocator. Boundary
	 * is in bytes and must be a power of two.
	 */
	BS_UTILITY_EXPORT UINT8* bs_frame_alloc_aligned(UINT32 count, UINT32 align);

	/**
	 * Deallocates memory allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	BS_UTILITY_EXPORT void bs_frame_free(void* data);

	/** 
	 * Frees memory previously allocated with bs_frame_alloc_aligned(). 
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	BS_UTILITY_EXPORT void bs_frame_free_aligned(void* data);

	/**
	 * Allocates enough memory to hold the object of specified type using the global frame allocator, but does not 
	 * construct the object. 
	 */
	template<class T>
	T* bs_frame_alloc()
	{
		return (T*)bs_frame_alloc(sizeof(T));
	}

	/**
	 * Allocates enough memory to hold N objects of specified type using the global frame allocator, but does not 
	 * construct the object. 
	 */
	template<class T>
	T* bs_frame_alloc(UINT32 count)
	{
		return (T*)bs_frame_alloc(sizeof(T) * count);
	}

	/**
	 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator, 
	 * and constructs them.
	 */
	template<class T>
	T* bs_frame_new(UINT32 count = 0)
	{
		T* data = bs_frame_alloc<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&data[i]) T;

		return data;
	}

	/**
	 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator, and constructs them.
	 */
	template<class T, class... Args>
	T* bs_frame_new(Args &&...args, UINT32 count = 0)
	{
		T* data = bs_frame_alloc<T>(count);

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&data[i]) T(std::forward<Args>(args)...);

		return data;
	}

	/**
	 * Destructs and deallocates an object allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	template<class T>
	void bs_frame_delete(T* data)
	{
		data->~T();

		bs_frame_free((UINT8*)data);
	}

	/**
	 * Destructs and deallocates an array of objects allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	template<class T>
	void bs_frame_delete(T* data, UINT32 count)
	{
		for(unsigned int i = 0; i < count; i++)
			data[i].~T();

		bs_frame_free((UINT8*)data);
	}

	/** @copydoc FrameAlloc::markFrame */
	BS_UTILITY_EXPORT void bs_frame_mark();

	/** @copydoc FrameAlloc::clear */
	BS_UTILITY_EXPORT void bs_frame_clear();

	/** String allocated with a frame allocator. */
	typedef std::basic_string<char, std::char_traits<char>, StdAlloc<char, FrameAlloc>> FrameString;

	/** WString allocated with a frame allocator. */
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, StdAlloc<wchar_t, FrameAlloc>> FrameWString;

	/** Vector allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAlloc>>
	using FrameVector = std::vector < T, A > ;

	/** Stack allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAlloc>>
	using FrameStack = std::stack < T, std::deque<T, A> > ;

	/** Queue allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAlloc>>
	using FrameQueue = std::queue<T, std::deque<T, A>>;

	/** Set allocated with a frame allocator. */
	template <typename T, typename P = std::less<T>, typename A = StdAlloc<T, FrameAlloc>>
	using FrameSet = std::set < T, P, A > ;

	/** Map allocated with a frame allocator. */
	template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>, FrameAlloc>>
	using FrameMap = std::map < K, V, P, A >;

	/** UnorderedSet allocated with a frame allocator. */
	template <typename T, typename H = std::hash<T>, typename C = std::equal_to<T>, typename A = StdAlloc<T, FrameAlloc>>
	using FrameUnorderedSet = std::unordered_set < T, H, C, A >;

	/** UnorderedMap allocated with a frame allocator. */
	template <typename K, typename V, typename H = std::hash<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>, FrameAlloc>>
	using FrameUnorderedMap = std::unordered_map < K, V, H, C, A >;

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	extern BS_THREADLOCAL FrameAlloc* _GlobalFrameAlloc;

	/**
	 * Specialized memory allocator implementations that allows use of a global frame allocator in normal 
	 * new/delete/free/dealloc operators.
	 */
	template<>
	class MemoryAllocator<FrameAlloc> : public MemoryAllocatorBase
	{
	public:
		/** @copydoc MemoryAllocator::allocate */
		static void* allocate(size_t bytes)
		{
			return bs_frame_alloc((UINT32)bytes);
		}

		/** @copydoc MemoryAllocator::allocateAligned */
		static void* allocateAligned(size_t bytes, size_t alignment)
		{
#if BS_PROFILING_ENABLED
			incAllocCount();
#endif

			return bs_frame_alloc_aligned((UINT32)bytes, (UINT32)alignment);
		}

		/** @copydoc MemoryAllocator::allocateAligned16 */
		static void* allocateAligned16(size_t bytes)
		{
#if BS_PROFILING_ENABLED
			incAllocCount();
#endif

			return bs_frame_alloc_aligned((UINT32)bytes, 16);
		}

		/** @copydoc MemoryAllocator::free */
		static void free(void* ptr)
		{
			bs_frame_free(ptr);
		}

		/** @copydoc MemoryAllocator::freeAligned */
		static void freeAligned(void* ptr)
		{
#if BS_PROFILING_ENABLED
			incFreeCount();
#endif

			bs_frame_free_aligned(ptr);
		}

		/** @copydoc MemoryAllocator::freeAligned16 */
		static void freeAligned16(void* ptr)
		{
#if BS_PROFILING_ENABLED
			incFreeCount();
#endif

			bs_frame_free_aligned(ptr);
		}
	};

	/** @} */
	/** @} */
}
