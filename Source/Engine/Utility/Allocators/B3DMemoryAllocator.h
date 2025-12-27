//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#undef min
#undef max

#include <new>
#include <limits>
#include <cstdint>
#include <utility>

#if B3D_PLATFORM_LINUX
#	include <malloc.h>
#endif

namespace b3d
{
	class MemoryAllocatorBase;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

#if B3D_PLATFORM_WIN32
	inline void* PlatformAlignedAlloc16(size_t size)
	{
		return _aligned_malloc(size, 16);
	}

	inline void PlatformAlignedFree16(void* ptr)
	{
		_aligned_free(ptr);
	}

	inline void* PlatformAlignedAlloc(size_t size, size_t alignment)
	{
		return _aligned_malloc(size, alignment);
	}

	inline void PlatformAlignedFree(void* ptr)
	{
		_aligned_free(ptr);
	}
#elif B3D_PLATFORM_LINUX || B3D_PLATFORM_ANDROID
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
		static B3D_EXPORT uint64_t GetAllocationCount()
		{
			return Allocs;
		}

		static B3D_EXPORT uint64_t GetFreeCount()
		{
			return Frees;
		}

	private:
		friend class MemoryAllocatorBase;

		// Threadlocal data can't be exported, so some magic to make it accessible from MemoryAllocator
		static B3D_EXPORT void IncrementAllocationCount() { ++Allocs; }

		static B3D_EXPORT void IncrementFreeCount() { ++Frees; }

		static B3D_THREADLOCAL uint64_t Allocs;
		static B3D_THREADLOCAL uint64_t Frees;
	};

	/**
	 * General allocator provided by the OS. Use for persistent long term allocations, and allocations that don't
	 * happen often.
	 */
	class DefaultAllocatorTag
	{};

	/** Base class all memory allocators need to inherit. Provides allocation and free counting. */
	class MemoryAllocatorBase
	{
	protected:
		static void IncrementAllocationCount() { MemoryCounter::IncrementAllocationCount(); }
		static void IncrementFreeCount() { MemoryCounter::IncrementFreeCount(); }
	};

	template <class T>
	class MemoryAllocator : public MemoryAllocatorBase
	{};

	/**
	 * Memory allocator providing a generic implementation. Specialize for specific categories as needed.
	 *
	 * @note	For example you might implement a pool allocator for specific types in order
	 * 			to reduce allocation overhead. By default standard malloc/free are used.
	 */
	template <>
	class MemoryAllocator<DefaultAllocatorTag> : public MemoryAllocatorBase
	{
	public:
		/** Allocates @p bytes bytes. */
		static void* Allocate(size_t bytes)
		{
#if B3D_PROFILING_ENABLED
			IncrementAllocationCount();
#endif

			return malloc(bytes);
		}

		/**
		 * Allocates @p bytes and aligns them to the specified boundary (in bytes). If the aligment is less or equal to
		 * 16 it is more efficient to use the allocateAligned16() alternative of this method. Alignment must be power of two.
		 */
		static void* AllocateAligned(size_t bytes, size_t alignment)
		{
#if B3D_PROFILING_ENABLED
			IncrementAllocationCount();
#endif

			return PlatformAlignedAlloc(bytes, alignment);
		}

		/** Allocates @p bytes and aligns them to a 16 byte boundary. */
		static void* AllocateAligned16(size_t bytes)
		{
#if B3D_PROFILING_ENABLED
			IncrementAllocationCount();
#endif

			return PlatformAlignedAlloc16(bytes);
		}

		/** Frees the memory at the specified location. */
		static void Free(void* ptr)
		{
#if B3D_PROFILING_ENABLED
			IncrementFreeCount();
#endif

			::free(ptr);
		}

		/** Frees memory allocated with allocateAligned() */
		static void FreeAligned(void* ptr)
		{
#if B3D_PROFILING_ENABLED
			IncrementFreeCount();
#endif

			PlatformAlignedFree(ptr);
		}

		/** Frees memory allocated with allocateAligned16() */
		static void FreeAligned16(void* ptr)
		{
#if B3D_PROFILING_ENABLED
			IncrementFreeCount();
#endif

			PlatformAlignedFree16(ptr);
		}
	};

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** Allocates the specified number of bytes. */
	template <class AllocatorTag>
	void* B3DAllocate(size_t count)
	{
		return MemoryAllocator<AllocatorTag>::Allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template <class T, class AllocatorTag>
	T* B3DAllocate()
	{
		return (T*)MemoryAllocator<AllocatorTag>::Allocate(sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template <class T, class AllocatorTag>
	T* B3DNewMultiple(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<AllocatorTag>::Allocate(sizeof(T) * count);

		for(size_t i = 0; i < count; ++i)
			new(&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template <class Type, class AllocatorTag, class... Args>
	Type* B3DNew(Args&&... args)
	{
		return new(B3DAllocate<Type, AllocatorTag>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	template <class AllocatorTag = DefaultAllocatorTag>
	void B3DFree(void* ptr)
	{
		MemoryAllocator<AllocatorTag>::Free(ptr);
	}

	/** Destructs and frees the specified object. */
	template <class T, class AllocatorTag = DefaultAllocatorTag>
	void B3DDelete(T* ptr)
	{
		(ptr)->~T();

		MemoryAllocator<AllocatorTag>::Free(ptr);
	}

	/** Callable struct that acts as a proxy for B3DDelete */
	template <class T, class AllocatorTag = DefaultAllocatorTag>
	struct Deleter
	{
		constexpr Deleter() noexcept = default;

		/** Constructor enabling deleter conversion and therefore polymorphism with smart points (if they use the same allocator). */
		template <class T2, std::enable_if_t<std::is_convertible<T2*, T*>::value, int> = 0>
		constexpr Deleter(const Deleter<T2, AllocatorTag>& other) noexcept
		{}

		void operator()(T* ptr) const
		{
			B3DDelete<T, AllocatorTag>(ptr);
		}
	};

	/** Destructs and frees the specified array of objects. */
	template <class T, class AllocatorTag = DefaultAllocatorTag>
	void B3DDeleteMultiple(T* ptr, size_t count)
	{
		for(size_t i = 0; i < count; ++i)
			ptr[i].~T();

		MemoryAllocator<AllocatorTag>::Free(ptr);
	}

	/*****************************************************************************/
	/* Default versions of all alloc/free/new/delete methods which use DefaultAllocatorTag */
	/*****************************************************************************/

	/** Allocates the specified number of bytes. */
	inline void* B3DAllocate(size_t count)
	{
		return MemoryAllocator<DefaultAllocatorTag>::Allocate(count);
	}

	/** Allocates enough bytes to hold the specified type, but doesn't construct it. */
	template <class T>
	T* B3DAllocate()
	{
		return (T*)MemoryAllocator<DefaultAllocatorTag>::Allocate(sizeof(T));
	}

	/**
	 * Allocates the specified number of bytes aligned to the provided boundary. Boundary is in bytes and must be a power
	 * of two.
	 */
	inline void* B3DAllocateAligned(size_t count, size_t align)
	{
		return MemoryAllocator<DefaultAllocatorTag>::AllocateAligned(count, align);
	}

	/** Allocates the specified number of bytes aligned to a 16 bytes boundary. */
	inline void* B3DAllocateAligned16(size_t count)
	{
		return MemoryAllocator<DefaultAllocatorTag>::AllocateAligned16(count);
	}

	/** Allocates enough bytes to hold an array of @p count elements the specified type, but doesn't construct them. */
	template <class T>
	T* B3DAllocateMultiple(size_t count)
	{
		return (T*)MemoryAllocator<DefaultAllocatorTag>::Allocate(count * sizeof(T));
	}

	/** Creates and constructs an array of @p count elements. */
	template <class T>
	T* B3DNewMultiple(size_t count)
	{
		T* ptr = (T*)MemoryAllocator<DefaultAllocatorTag>::Allocate(count * sizeof(T));

		for(size_t i = 0; i < count; ++i)
			new(&ptr[i]) T;

		return ptr;
	}

	/** Create a new object with the specified allocator and the specified parameters. */
	template <class Type, class... Args>
	Type* B3DNew(Args&&... args)
	{
		return new(B3DAllocate<Type, DefaultAllocatorTag>()) Type(std::forward<Args>(args)...);
	}

	/** Frees all the bytes allocated at the specified location. */
	inline void B3DFree(void* ptr)
	{
		MemoryAllocator<DefaultAllocatorTag>::Free(ptr);
	}

	/** Frees memory previously allocated with B3DAllocateAligned(). */
	inline void B3DFreeAligned(void* ptr)
	{
		MemoryAllocator<DefaultAllocatorTag>::FreeAligned(ptr);
	}

	/** Frees memory previously allocated with B3DAllocateAligned16(). */
	inline void B3DFreeAligned16(void* ptr)
	{
		MemoryAllocator<DefaultAllocatorTag>::FreeAligned16(ptr);
	}

/************************************************************************/
/*			MACRO VERSIONS					*/
/* You will almost always want to use the template versions but in some */
/* cases (private destructor) it is not possible. In which case you may	*/
/* use these instead.							*/
/************************************************************************/
#define B3D_PVT_DELETE(T, ptr) \
	(ptr)->~T();              \
	MemoryAllocator<DefaultAllocatorTag>::free(ptr);

#define B3D_PVT_DELETE_A(T, ptr, Alloc) \
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
	template <class T, class AllocatorTag = DefaultAllocatorTag>
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
			using other = StdAlloc<U, AllocatorTag>;
		};

		/** Allocate but don't initialize number elements of type T. */
		static T* allocate(const size_t num)
		{
			if(num == 0)
				return nullptr;

			if(num > max_size())
				return nullptr; // Error

			void* const pv = B3DAllocate<AllocatorTag>(num * sizeof(T));
			if(!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		static void deallocate(pointer p, size_type)
		{
			B3DFree<AllocatorTag>(p);
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
} // namespace b3d

#include "Allocators/B3DStackAlloc.h"
#include "Allocators/B3DFreeAlloc.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Allocators/B3DStaticAlloc.h"
#include "Allocators/B3DMemAllocProfiler.h"
#include "Allocators/B3DContainerAllocators.h"
