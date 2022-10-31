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
#	include <malloc.h>
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
		if(data == nullptr)
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
		static BS_UTILITY_EXPORT uint64_t GetNumAllocs()
		{
			return Allocs;
		}

		static BS_UTILITY_EXPORT uint64_t GetNumFrees()
		{
			return Frees;
		}

	private:
		friend class MemoryAllocatorBase;

		// Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
		static BS_UTILITY_EXPORT void IncAllocCount() { ++Allocs; }

		static BS_UTILITY_EXPORT void IncFreeCount() { ++Frees; }

		static BS_THREADLOCAL uint64_t Allocs;
		static BS_THREADLOCAL uint64_t Frees;
	};

	/** Base class all memory allocators need to inherit. Provides allocation and free counting. */
	class MemoryAllocatorBase
	{
	protected:
		static void IncAllocCount() { MemoryCounter::IncAllocCount(); }

		static void IncFreeCount() { MemoryCounter::IncFreeCount(); }
	};

	/**
	 * Memory allocator providing a generic implementation. Specialize for specific categories as needed.
	 *
	 * @note	For example you might implement a pool allocator for specific types in order
	 * 			to reduce allocation overhead. By default standard malloc/free are used.
	 */
	template <class T>
	class MemoryAllocator : public MemoryAllocatorBase
	{
	public:
		/** Allocates @p bytes bytes. */
		static void* Allocate(size_t bytes)
		{
#if BS_PROFILING_ENABLED
			IncAllocCount();
#endif

			return malloc(bytes);
		}

		/**
		 * Allocates @p bytes and aligns them to the specified boundary (in bytes). If the aligment is less or equal to
		 * 16 it is more efficient to use the allocateAligned16() alternative of this method. Alignment must be power of two.
		 */
		static void* AllocateAligned(size_t bytes, size_t alignment)
		{
#if BS_PROFILING_ENABLED
			IncAllocCount();
#endif

			return platformAlignedAlloc(bytes, alignment);
		}

		/** Allocates @p bytes and aligns them to a 16 byte boundary. */
		static void* AllocateAligned16(size_t bytes)
		{
#if BS_PROFILING_ENABLED
			IncAllocCount();
#endif

			return platformAlignedAlloc16(bytes);
		}

		/** Frees the memory at the specified location. */
		static void Free(void* ptr)
		{
#if BS_PROFILING_ENABLED
			IncFreeCount();
#endif

			::free(ptr);
		}

		/** Frees memory allocated with allocateAligned() */
		static void FreeAligned(void* ptr)
		{
#if BS_PROFILING_ENABLED
			IncFreeCount();
#endif

			platformAlignedFree(ptr);
		}

		/** Frees memory allocated with allocateAligned16() */
		static void FreeAligned16(void* ptr)
		{
#if BS_PROFILING_ENABLED
			IncFreeCount();
#endif

			platformAlignedFree16(ptr);
		}
	};

	/**
	 * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
	 * happen often.
	 */
	class GenAlloc
	{};

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** Allocates the specified number of bytes. */
	template <class Alloc>
	void* B3DAllocate(size_t count)
	{
		return MemoryAllocator<Alloc>::Allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template <class T, class Alloc>
	T* B3DAllocate()
	{
		return (T*)MemoryAllocator<Alloc>::Allocate(sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template <class T, class Alloc>
	T* B3DNewMultiple(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<Alloc>::Allocate(sizeof(T) * count);

		for(size_t i = 0; i < count; ++i)
			new(&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template <class Type, class Alloc, class... Args>
	Type* B3DNew(Args&&... args)
	{
		return new(B3DAllocate<Type, Alloc>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	template <class Alloc>
	void B3DFree(void* ptr)
	{
		MemoryAllocator<Alloc>::Free(ptr);
	}

	/** Destructs and frees the specified object. */
	template <class T, class Alloc = GenAlloc>
	void B3DDelete(T* ptr)
	{
		(ptr)->~T();

		MemoryAllocator<Alloc>::Free(ptr);
	}

	/** Callable struct that acts as a proxy for B3DDelete */
	template <class T, class Alloc = GenAlloc>
	struct Deleter
	{
		constexpr Deleter() noexcept = default;

		/** Constructor enabling deleter conversion and therefore polymorphism with smart points (if they use the same allocator). */
		template <class T2, std::enable_if_t<std::is_convertible<T2*, T*>::value, int> = 0>
		constexpr Deleter(const Deleter<T2, Alloc>& other) noexcept
		{}

		void operator()(T* ptr) const
		{
			B3DDelete<T, Alloc>(ptr);
		}
	};

	/** Destructs and frees the specified array of objects. */
	template <class T, class Alloc = GenAlloc>
	void B3DDeleteMultiple(T* ptr, size_t count)
	{
		for(size_t i = 0; i < count; ++i)
			ptr[i].~T();

		MemoryAllocator<Alloc>::Free(ptr);
	}

	/*****************************************************************************/
	/* Default versions of all alloc/free/new/delete methods which call GenAlloc */
	/*****************************************************************************/

	/** Allocates the specified number of bytes. */
	inline void* B3DAllocate(size_t count)
	{
		return MemoryAllocator<GenAlloc>::Allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template <class T>
	T* B3DAllocate()
	{
		return (T*)MemoryAllocator<GenAlloc>::Allocate(sizeof(T));
	}

	/**
	 * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
	 * of two.
	 */
	inline void* B3DAllocateAligned(size_t count, size_t align)
	{
		return MemoryAllocator<GenAlloc>::AllocateAligned(count, align);
	}

	/** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
	inline void* B3DAllocateAligned16(size_t count)
	{
		return MemoryAllocator<GenAlloc>::AllocateAligned16(count);
	}

	/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
	template <class T>
	T* B3DAllocateMultiple(size_t count)
	{
		return (T*)MemoryAllocator<GenAlloc>::Allocate(count * sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template <class T>
	T* B3DNewMultiple(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<GenAlloc>::Allocate(count * sizeof(T));

		for(size_t i = 0; i < count; ++i)
			new(&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template <class Type, class... Args>
	Type* B3DNew(Args&&... args)
	{
		return new(B3DAllocate<Type, GenAlloc>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	inline void B3DFree(void* ptr)
	{
		MemoryAllocator<GenAlloc>::Free(ptr);
	}

	/** Frees memory previously allocated with B3DAllocateAligned(). */
	inline void B3DFreeAligned(void* ptr)
	{
		MemoryAllocator<GenAlloc>::FreeAligned(ptr);
	}

	/** Frees memory previously allocated with B3DAllocateAligned16(). */
	inline void B3DFreeAligned16(void* ptr)
	{
		MemoryAllocator<GenAlloc>::FreeAligned16(ptr);
	}

/************************************************************************/
/*			MACRO VERSIONS					*/
/* You will almost always want to use the template versions but in some */
/* cases (private destructor) it is not possible. In which case you may	*/
/* use these instead.							*/
/************************************************************************/
#define BS_PVT_DELETE(T, ptr) \
	(ptr)->~T();              \
	MemoryAllocator<GenAlloc>::free(ptr);

#define BS_PVT_DELETE_A(T, ptr, Alloc) \
	(ptr)->~T();                       \
	MemoryAllocator<Alloc>::free(ptr);

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	// NOLINTBEGIN(readability-identifier-naming)
	/** Allocator for the standard library that internally uses bsf memory allocator. */
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
		template <class U, class Alloc2>
		constexpr StdAlloc(const StdAlloc<U, Alloc2>&){};

		template <class U, class Alloc2>
		constexpr bool operator==(const StdAlloc<U, Alloc2>&) const noexcept
		{
			return true;
		}

		template <class U, class Alloc2>
		constexpr bool operator!=(const StdAlloc<U, Alloc2>&) const noexcept
		{
			return false;
		}

		template <class U>
		class rebind
		{
		public:
			using other = StdAlloc<U, Alloc>;
		};

		/** Allocate but don't initialize number elements of type T. */
		static T* allocate(const size_t num)
		{
			if(num == 0)
				return nullptr;

			if(num > max_size())
				return nullptr; // Error

			void* const pv = B3DAllocate<Alloc>(num * sizeof(T));
			if(!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		static void deallocate(pointer p, size_type)
		{
			B3DFree<Alloc>(p);
		}

		static constexpr size_t max_size() { return std::numeric_limits<size_type>::max() / sizeof(T); }

		static constexpr void destroy(pointer p) { p->~T(); }

		template <class... Args>
		static void construct(pointer p, Args&&... args)
		{
			new(p) T(std::forward<Args>(args)...);
		}
	};

	// NOLINTEND(readability-identifier-naming)

	/** @} */
	/** @} */
} // namespace bs

#include "Allocators/BsStackAlloc.h"
#include "Allocators/BsFreeAlloc.h"
#include "Allocators/BsFrameAllocator.h"
#include "Allocators/BsStaticAlloc.h"
#include "Allocators/BsMemAllocProfiler.h"
