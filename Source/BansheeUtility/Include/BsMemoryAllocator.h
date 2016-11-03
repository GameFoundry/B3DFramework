//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once
#undef min
#undef max

#include "BsTypes.h"	        // for UINT64

#include <atomic>
#include <limits>
#include <new>                  /* For 'placement new' */
#include <utility>

#if BS_PLATFORM == BS_PLATFORM_LINUX
#  include <malloc.h>
#endif

namespace BansheeEngine
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

		UINT8* alignedData = ((UINT8*)data) + sizeof(void*);
		alignedData += alignment - ((uintptr_t)alignedData) & (alignment - 1);

		((void**)alignedData)[-1] = data;
		return alignedData;
	}

	inline void platformAlignedFree(void* ptr)
	{
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
		static BS_UTILITY_EXPORT UINT64 getNumAllocs()
		{
			return Allocs;
		}

		static BS_UTILITY_EXPORT UINT64 getNumFrees()
		{
			return Frees;
		}
		
	private:
		friend class MemoryAllocatorBase;

		// Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
		static BS_UTILITY_EXPORT void incAllocCount() { Allocs++; }
		static BS_UTILITY_EXPORT void incFreeCount() { Frees++; }

		static BS_THREADLOCAL UINT64 Allocs;
		static BS_THREADLOCAL UINT64 Frees;
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
	inline void* bs_alloc(UINT32 count)
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
	inline T* bs_newN(UINT32 count)
	{
		T* ptr = (T*)MemoryAllocator<Alloc>::allocate(sizeof(T) * count);

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class Alloc, class... Args>
	Type* bs_new(Args &&...args)
	{
		return new (bs_alloc<Alloc>(sizeof(Type))) Type(std::forward<Args>(args)...);
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
	inline void bs_deleteN(T* ptr, UINT32 count)
	{
		for(unsigned int i = 0; i < count; i++)
			ptr[i].~T();

		MemoryAllocator<Alloc>::free(ptr);
	}

	/*****************************************************************************/
	/* Default versions of all alloc/free/new/delete methods which call GenAlloc */
	/*****************************************************************************/

	/** Allocates the specified number of bytes. */
	inline void* bs_alloc(UINT32 count)
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
	inline void* bs_alloc_aligned(UINT32 count, UINT32 align)
	{
		return MemoryAllocator<GenAlloc>::allocateAligned(count, align);
	}


	/** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
	inline void* bs_alloc_aligned16(UINT32 count)
	{
		return MemoryAllocator<GenAlloc>::allocateAligned16(count);
	}

	/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
	template<class T> 
	inline T* bs_allocN(UINT32 count)
	{
		return (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template<class T> 
	inline T* bs_newN(UINT32 count)
	{
		T* ptr = (T*)MemoryAllocator<GenAlloc>::allocate(count * sizeof(T));

		for(unsigned int i = 0; i < count; i++)
			new ((void*)&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template<class Type, class... Args>
	Type* bs_new(Args &&...args)
	{
		return new (bs_alloc<GenAlloc>(sizeof(Type))) Type(std::forward<Args>(args)...);
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
/* 							MACRO VERSIONS                      		*/
/* You will almost always want to use the template versions but in some */
/* cases (private destructor) it is not possible. In which case you may	*/
/* use these instead.												    */
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
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		StdAlloc() noexcept {}
		template<class U, class Alloc2> StdAlloc(const StdAlloc<U, Alloc2>&) noexcept {}
		template<class U, class Alloc2> bool operator==(const StdAlloc<U, Alloc2>&) const noexcept { return true; }
		template<class U, class Alloc2> bool operator!=(const StdAlloc<U, Alloc2>&) const noexcept { return false; }
		template<class U> class rebind { public: typedef StdAlloc<U, Alloc> other; };

		/** Allocate but don't initialize number elements of type T. */
		T* allocate(const size_t num) const
		{
			if (num == 0)
				return nullptr;

			if (num > static_cast<size_t>(-1) / sizeof(T))
				return nullptr; // Error

			void* const pv = bs_alloc<Alloc>((UINT32)(num * sizeof(T)));
			if (!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		void deallocate(T* p, size_t num) const noexcept
		{
			bs_free<Alloc>((void*)p);
		}

		size_t max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
		void construct(pointer p, const_reference t) { new (p) T(t); }
		void destroy(pointer p) { p->~T(); }
		/* This version of construct() (with a varying number of parameters)
		 * seems necessary in order to use some STL data structures from
		 * libstdc++-4.8, but compilation fails on OS X, hence the #if. */
#if BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_WIN32
		template<class U, class... Args>
		void construct(U* p, Args&&... args) { new(p) U(std::forward<Args>(args)...); }
#endif
	};

	/** @} */
	/** @} */
}

#include "BsMemStack.h"
#include "BsGlobalFrameAlloc.h"
#include "BsMemAllocProfiler.h"
