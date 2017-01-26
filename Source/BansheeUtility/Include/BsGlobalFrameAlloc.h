//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsStdHeaders.h"
#include "BsThreadDefines.h"

namespace bs
{
	/** @addtogroup Memory
	 *  @{
	 */

	class FrameAlloc;

	/**
	 * Returns a global, application wide FrameAlloc. Each thread gets its own frame allocator.
	 *
	 * @note	Thread safe.
	 */
	inline BS_UTILITY_EXPORT FrameAlloc& gFrameAlloc();

	/**
	 * Allocates some memory using the global frame allocator.
	 *
	 * @param[in]	numBytes	Number of bytes to allocate.
	 */
	inline BS_UTILITY_EXPORT UINT8* bs_frame_alloc(UINT32 numBytes);

	/** 
	 * Allocates the specified number of bytes aligned to the provided boundary, using the global frame allocator. Boundary
	 * is in bytes and must be a power of two.
	 */
	inline BS_UTILITY_EXPORT UINT8* bs_frame_alloc_aligned(UINT32 count, UINT32 align);

	/**
	 * Deallocates memory allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	inline BS_UTILITY_EXPORT void bs_frame_free(void* data);

	/** 
	 * Frees memory previously allocated with bs_frame_alloc_aligned(). 
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	inline BS_UTILITY_EXPORT void bs_frame_free_aligned(void* data);

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
	inline BS_UTILITY_EXPORT void bs_frame_mark();

	/** @copydoc FrameAlloc::clear */
	inline BS_UTILITY_EXPORT void bs_frame_clear();

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