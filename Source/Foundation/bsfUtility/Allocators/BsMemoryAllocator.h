//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#undef min
#undef max

#include <new>
#include <limits>
#include <cstdint>
#include <utility>

#if BS_PLATFORM == BS_PLATFORM_LINUX
#  include <malloc.h>
#endif

namespace bs
{
	class MemoryAllocatorBase;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

#if BS_PLATFORM == BS_PLATFORM_WIN32
	inline void* platformAlignedAlloc16(size_t size)
	{
		return _aligned_malloc(size, 16);
	}

	inline void platformAlignedFree16(void* ptr)
	{
		_aligned_free(ptr);
	}

	inline void* platformAlignedAlloc(size_t size, size_t alignment)
	{
		return _aligned_malloc(size, alignment);
	}

	inline void platformAlignedFree(void* ptr)
	{
		_aligned_free(ptr);
	}
#elif BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_ANDROID
	inline void* platformAlignedAlloc16(size_t size)
	{
		return ::memalign(16, size);
	}

	inline void platformAlignedFree16(void* ptr)
	{
		::free(ptr);
	}

	inline void* platformAlignedAlloc(size_t size, size_t alignment)
	{
		return ::memalign(alignment, size);
	}

	inline void platformAlignedFree(void* ptr)
	{
		::free(ptr);
	}
#else // 16 byte aligment by default
	inline void* platformAlignedAlloc16(size_t size)
	{
		return ::malloc(size);
	}

	inline void platformAlignedFree16(void* ptr)
	{
		::free(ptr);
	}

	inline void* platformAlignedAlloc(size_t size, size_t alignment)
	{
		void* data = ::malloc(size + (alignment - 1) + sizeof(void*));
		if (data == nullptr)
			return nullptr;

		char* alignedData = ((char*)data) + sizeof(void*);
		alignedData += (alignment - ((uintptr_t)alignedData) & (alignment - 1)) & (alignment - 1);

		((void**)alignedData)[-1] = data;
		return alignedData;
	}

	inline void platformAlignedFree(void* ptr)
	{
		// TODO: Document how this works.
		::free(((void**)ptr)[-1]);
	}
#endif

	/**
	 * Thread safe class used for storing total number of memory allocations and deallocations, primarily for statistic
	 * purposes.
	 */
	class MemoryCounter
	{
	public:
		static BS_UTILITY_EXPORT uint64_t getNumAllocs()
		{
			return Allocs;
		}

		static BS_UTILITY_EXPORT uint64_t getNumFrees()
		{
			return Frees;
		}

	private:
		friend class MemoryAllocatorBase;

		// Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
		static BS_UTILITY_EXPORT void incAllocCount() { ++Allocs; }
		static BS_UTILITY_EXPORT void incFreeCount() { ++Frees; }

		static BS_THREADLOCAL uint64_t Allocs;
		static BS_THREADLOCAL uint64_t Frees;
	};

	/** Base class all memory allocators need to inherit. Provides allocation and free counting. */
	class MemoryAllocatorBase
	{
	protected:
		static void incAllocCount() { MemoryCounter::incAllocCount(); }
		static void incFreeCount() { MemoryCounter::incFreeCount(); }
	};

	/**
	 * Memory allocator providing a generic implementation. Specialize for specific categories as needed.
	 *
	 * @note	For example you might implement a pool allocator for specific types in order
	 * 			to reduce allocation overhead. By default standard malloc/free are used.
	 */
	template<class T>
	class MemoryAllocator : public MemoryAllocatorBase
	{
	public:
		/** Allocates @p bytes bytes. */
		static void* allocate(size_t bytes)
		{
#if BS_PROFILING_ENABLED
			incAllocCount();
#endif

			return malloc(bytes);
		}

		/**
		 * Allocates @p bytes and aligns them to the specified boundary (in bytes). If the aligment is less or equal to
		 * 16 it is more efficient to use the allocateAligned16() alternative of this method. Alignment must be power of two.
		 */
		static void* allocateAligned(size_t bytes, size_t alignment)
		{
#if BS_PROFILING_ENABLED
			incAllocCount();
#endif

			return platformAlignedAlloc(bytes, alignment);
		}

		/** Allocates @p bytes and aligns them to a 16 byte boundary. */
		static void* allocateAligned16(size_t bytes)
		{
#if BS_PROFILING_ENABLED
			incAllocCount();
#endif

			return platformAlignedAlloc16(bytes);
		}

		/** Frees the memory at the specified location. */
		static void free(void* ptr)
		{
#if BS_PROFILING_ENABLED
			incFreeCount();
#endif

			::free(ptr);
		}

		/** Frees memory allocated with allocateAligned() */
		static void freeAligned(void* ptr)
		{
#if BS_PROFILING_ENABLED
			incFreeCount();
#endif

			platformAlignedFree(ptr);
		}

		/** Frees memory allocated with allocateAligned16() */
		static void freeAligned16(void* ptr)
		{
#if BS_PROFILING_ENABLED
			incFreeCount();
#endif

			platformAlignedFree16(ptr);
		}
	};

	/**
	 * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
	 * happen often.
	 */
	class GenAlloc
	{ };

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** Allocates the specified number of bytes. */
	template<class Alloc>
	inline void* bs_alloc(size_t count)
	{
		return MemoryAllocator<Alloc>::allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template<class T, class Alloc>
	inline T* bs_alloc()
	{
		return (T*)MemoryAllocator<Alloc>::allocate(sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template<class T, class Alloc>
	inline T* bs_newN(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<Alloc>::allocate(sizeof(T) * count);

		for(size_t i = 0; i < count; ++i)
			new (&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class Alloc, class... Args>
	Type* bs_new(Args &&...args)
	{
		return new (bs_alloc<Type, Alloc>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	template<class Alloc>
	inline void bs_free(void* ptr)
	{
		MemoryAllocator<Alloc>::free(ptr);
	}

	/** Destructs and frees the specified object. */
	template<class T, class Alloc = GenAlloc>
	inline void bs_delete(T* ptr)
	{
		(ptr)->~T();

		MemoryAllocator<Alloc>::free(ptr);
	}

	/** Destructs and frees the specified array of objects. */
	template<class T, class Alloc = GenAlloc>
	inline void bs_deleteN(T* ptr, size_t count)
	{
		for(size_t i = 0; i < count; ++i)
			ptr[i].~T();

		MemoryAllocator<Alloc>::free(ptr);
	}

	/*****************************************************************************/
	/* Default versions of all alloc/free/new/delete methods which call GenAlloc */
	/*****************************************************************************/

	/** Allocates the specified number of bytes. */
	inline void* bs_alloc(size_t count)
	{
		return MemoryAllocator<GenAlloc>::allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template<class T>
	inline T* bs_alloc()
	{
		return (T*)MemoryAllocator<GenAlloc>::allocate(sizeof(T));
	}

	/**
	 * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
	 * of two.
	 */
	inline void* bs_alloc_aligned(size_t count, size_t align)
	{
		return MemoryAllocator<GenAlloc>::allocateAligned(count, align);
	}


	/** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
	inline void* bs_alloc_aligned16(size_t count)
	{
		return MemoryAllocator<GenAlloc>::allocateAligned16(count);
	}

	/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
	template<class T>
	inline T* bs_allocN(size_t count)
	{
		return (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template<class T>
	inline T* bs_newN(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));

		for(size_t i = 0; i < count; ++i)
			new (&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class... Args>
	Type* bs_new(Args &&...args)
	{
		return new (bs_alloc<Type, GenAlloc>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	inline void bs_free(void* ptr)
	{
		MemoryAllocator<GenAlloc>::free(ptr);
	}

	/** Frees memory previously allocated with bs_alloc_aligned(). */
	inline void bs_free_aligned(void* ptr)
	{
		MemoryAllocator<GenAlloc>::freeAligned(ptr);
	}

	/** Frees memory previously allocated with bs_alloc_aligned16(). */
	inline void bs_free_aligned16(void* ptr)
	{
		MemoryAllocator<GenAlloc>::freeAligned16(ptr);
	}

/************************************************************************/
/*			MACRO VERSIONS					*/
/* You will almost always want to use the template versions but in some */
/* cases (private destructor) it is not possible. In which case you may	*/
/* use these instead.							*/
/************************************************************************/
#define BS_PVT_DELETE(T, ptr) \
	(ptr)->~T(); \
	MemoryAllocator<GenAlloc>::free(ptr);

#define BS_PVT_DELETE_A(T, ptr, Alloc) \
	(ptr)->~T(); \
	MemoryAllocator<Alloc>::free(ptr);

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/** Allocator for the standard library that internally uses Banshee memory allocator. */
	template <class T, class Alloc = GenAlloc>
	class StdAlloc
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		constexpr StdAlloc() = default;
		constexpr StdAlloc(StdAlloc&&) = default;
		constexpr StdAlloc(const StdAlloc&) = default;
		template<class U, class Alloc2> constexpr StdAlloc(const StdAlloc<U, Alloc2>&) { };
		template<class U, class Alloc2> constexpr bool operator==(const StdAlloc<U, Alloc2>&) const noexcept { return true; }
		template<class U, class Alloc2> constexpr bool operator!=(const StdAlloc<U, Alloc2>&) const noexcept { return false; }

		template<class U> class rebind { public: using other = StdAlloc<U, Alloc>; };

		/** Allocate but don't initialize number elements of type T. */
		static T* allocate(const size_t num)
		{
			if (num == 0)
				return nullptr;

			if (num > std::numeric_limits<size_t>::max() / sizeof(T))
				return nullptr; // Error

			void* const pv = bs_alloc<Alloc>(num * sizeof(T));
			if (!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		static void deallocate(pointer p, size_type)
		{
			bs_free<Alloc>(p);
		}

		static constexpr size_t max_size() { return std::numeric_limits<size_type>::max() / sizeof(T); }
		static constexpr void destroy(pointer p) { p->~T(); }

		template<class... Args>
		static void construct(pointer p, Args&&... args) { new(p) T(std::forward<Args>(args)...); }
	};

	/** @} */
	/** @} */
}

#include "Allocators/BsStackAlloc.h"
#include "Allocators/BsFreeAlloc.h"
#include "Allocators/BsFrameAlloc.h"
#include "Allocators/BsStaticAlloc.h"
#include "Allocators/BsMemAllocProfiler.h"
