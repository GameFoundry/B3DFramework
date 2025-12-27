//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <limits>
#include <new> /* For 'placement new' */

#include "Prerequisites/B3DPlatformDefines.h"
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

	class FrameAllocatorTag;

	/**
	 * Frame allocator. Performs very fast allocations using a bump allocator pattern but can only free all of its memory
	 * at once. Perfect for allocations that last just a single frame.
	 *
	 * Frames can be nested within each other using MarkFrame() and Clear(). Each MarkFrame() increases the frame depth,
	 * and Clear() decreases it. Memory allocated at a specific depth must be freed at the same depth. In debug builds,
	 * attempting to free memory at a different depth than it was allocated will trigger an assertion.
	 *
	 * @par Memory Layout (Debug builds only)
	 * Each allocation includes an 8-byte header: [u32 allocationSize][u32 frameDepth]
	 * Release builds have zero overhead - allocations are returned directly from the bump pointer.
	 *
	 * @par Thread Safety
	 * This allocator is NOT thread-safe for concurrent access. Each thread must use its own allocator instance.
	 * The global frame allocator (GetFrameAllocator()) achieves thread safety through thread-local storage,
	 * ensuring each thread gets its own allocator instance. In debug builds, all methods validate that they're
	 * called from the allocator's owner thread.
	 *
	 * @par Performance Characteristics
	 * - Allocation: O(1) bump pointer (unless new block needed, then O(n) for block search/allocation)
	 * - Deallocation: O(1) counter update in debug builds, no-op in release builds
	 * - Clear: O(blocks) to reset pointers and optionally merge blocks
	 *
	 * @par Example Usage
	 * @code
	 * {
	 *     FrameScope outerFrame;  // Depth 0 → 1
	 *     void* data1 = allocator.Alloc(100);  // Allocated at depth 1
	 *
	 *     {
	 *         FrameScope innerFrame;  // Depth 1 → 2
	 *         void* data2 = allocator.Alloc(200);  // Allocated at depth 2
	 *         allocator.Free(data2);  // OK: freed at depth 2
	 *     }  // innerFrame destroyed, depth 2 → 1
	 *
	 *     allocator.Free(data1);  // OK: freed at depth 1
	 * }  // outerFrame destroyed, depth 1 → 0
	 * @endcode
	 */
	class B3D_EXPORT FrameAllocator
	{
	private:
		/** A single block of memory within a frame allocator. */
		class MemoryBlock
		{
		public:
			MemoryBlock(u32 size)
				: Size(size) {}

			~MemoryBlock() = default;

			/**
			 * Allocates a piece of memory within the block using bump pointer allocation.
			 *
			 * @param	allocationSize	Number of bytes to allocate.
			 * @return	Pointer to the allocated memory.
			 *
			 * @note	Caller must ensure the block has enough free space before calling.
			 */
			u8* Alloc(u32 allocationSize);

			/**
			 * Releases all allocations within a block by resetting the free pointer to zero.
			 * Does not actually free the underlying memory buffer.
			 */
			void Clear();

			u8* Data = nullptr;
			u32 FreePointer = 0;
			u32 Size;
		};

#if B3D_DEBUG
		/** Size of guard pattern (fence post) in bytes. */
		static constexpr u32 kGuardSize = 4;

		/** Pattern written to guard bytes to detect buffer overruns/underruns. */
		static constexpr u32 kGuardPattern = 0xFDFDFDFD;

		/** Pattern used to fill uninitialized memory. */
		static constexpr u8 kUninitPattern = 0xCD;

		/** Pattern used to fill freed memory (detect use-after-free). */
		static constexpr u8 kFreedPattern = 0xDD;

		/** Debug header size: pre_guard(4) + totalSize(4) + depth(4) + userSize(4) = 16 bytes. */
		static constexpr u32 kDebugHeaderSize = kGuardSize + sizeof(u32) * 3;

		/** Debug trailer size (post-guard). */
		static constexpr u32 kDebugTrailerSize = kGuardSize;
#endif
		static constexpr u32 kDefaultBlockSize = 1024 * 1024;  // 1 MB
		static constexpr u32 kBlockAlignment = 16;

	public:
		FrameAllocator(u32 blockSize = kDefaultBlockSize);
		~FrameAllocator();

		/**
		 * Allocates a new block of memory of the specified size.
		 *
		 * @param	allocationSize	Amount of memory to allocate, in bytes.
		 *
		 * @note	Not thread safe.
		 */
		u8* Allocate(u32 allocationSize);

		/**
		 * Allocates a new block of memory of the specified size aligned to the specified boundary. If the aligment is less
		 * or equal to 16 it is more efficient to use the AllocAligned16() alternative of this method.
		 *
		 * @param	allocationSize	Amount of memory to allocate, in bytes.
		 * @param	alignment		Alignment of the allocated memory. Must be power of two.
		 *
		 * @note	Not thread safe.
		 */
		u8* AllocateAligned(u32 allocationSize, u32 alignment);

		/**
		 * Allocates and constructs a new object.
		 *
		 * @note	Not thread safe.
		 */
		template <class T, class... Args>
		T* Construct(Args&&... args)
		{
			return new((T*)Allocate(sizeof(T))) T(std::forward<Args>(args)...);
		}

		/**
		 * Destructs and deallocates an object.
		 *
		 * @note	Not thread safe.
		 */
		template <class T>
		void Destruct(T* data)
		{
			data->~T();
			Free((u8*)data);
		}

		/**
		 * Deallocates a previously allocated block of memory.
		 *
		 * @note
		 * No actual deallocation happens here. This method only updates debug tracking counters and validates
		 * that memory is being freed at the same frame depth it was allocated. The actual memory is bulk-freed
		 * in Clear() using the bump allocator pattern.
		 *
		 * @note
		 * In debug builds, asserts if memory is freed at a different frame depth than it was allocated.
		 *
		 * @note
		 * Thread safe.
		 */
		void Free(u8* data);

		/**
		 * Deallocates and destructs a previously allocated object.
		 *
		 * @note
		 * No deallocation is actually done here. This method is only used to call the destructor and for debug purposes
		 * so it is easier to track down memory leaks and corruption.
		 * @note
		 * Thread safe.
		 */
		template <class T>
		void Free(T* obj)
		{
			if(obj != nullptr)
				obj->~T();

			Free((u8*)obj);
		}

		/**
		 * Marks the beginning of a new frame scope. Increments the frame depth counter.
		 * The next call to Clear() will only clear memory allocated past this point.
		 *
		 * @note
		 * Frames can be nested. Each MarkFrame() must be paired with a corresponding Clear() call.
		 * Use the FrameScope RAII helper to ensure proper pairing.
		 */
		void MarkFrame();

		/**
		 * Deallocates all allocated memory since the last call to MarkFrame() (or all the memory if there was no call
		 * to MarkFrame()). Decrements the frame depth counter.
		 *
		 * @note
		 * Uses a frame marker linked list to track nested scopes. Frame markers are stored within the allocator's
		 * own memory, creating a self-referential structure. When clearing to a frame marker, the allocator rewinds
		 * the allocation pointers to the marker's position.
		 *
		 * @note
		 * If no frame marker exists and debug validation is enabled, asserts that all allocated memory has been
		 * explicitly freed (mTotalAllocatedBytes == 0).
		 *
		 * @note
		 * Optionally merges multiple freed blocks into a single larger block to reduce fragmentation.
		 *
		 * @note	Not thread safe.
		 */
		void Clear();

#if B3D_DEBUG
		/**
		 * Validates integrity of all allocations in the frame allocator.
		 * Walks through all memory blocks and checks guard bytes for buffer overruns/underruns.
		 *
		 * @return	True if all allocations are intact, false if corruption detected.
		 * @note	Only available in debug builds.
		 */
		bool CheckMemory() const;

		/**
		 * Sets the frequency of automatic memory integrity checks.
		 *
		 * @param	frequency	Check every N allocations. Set to 0 to disable automatic checks.
		 */
		void SetCheckFrequency(u32 frequency) { mCheckFrequency = frequency; }
#endif

	private:
		u32 mBlockSize;
		Vector<MemoryBlock*> mBlocks;
		MemoryBlock* mFreeBlock;
		u32 mNextBlockIndex;
		std::atomic<u32> mTotalAllocatedBytes;
		void* mLastFrame;
		u32 mCurrentFrameDepth;

#if B3D_DEBUG
		ThreadId mOwnerThread;
		u32 mAllocationCount = 0;
		u32 mCheckFrequency = 50;
#endif

		/**
		 * Allocates a dynamic block of memory of the wanted size. The exact allocation size might be slightly higher in
		 * order to store block meta data.
		 */
		MemoryBlock* AllocateBlock(u32 wantedSize);

		/** Frees a memory block. */
		void DeallocateBlock(MemoryBlock* block);

#if B3D_DEBUG
		/**
		 * Writes the debug header (guard, allocation size, frame depth, user size) before the user data.
		 *
		 * @param	data			Pointer to the start of the allocation (header location).
		 * @param	totalSize		Total size of the allocation including header and trailer.
		 * @param	frameDepth		Frame depth at which this allocation was made.
		 * @param	userSize		Size of user-requested data (excluding debug overhead).
		 */
		void WriteDebugHeader(u8* data, u32 totalSize, u32 frameDepth, u32 userSize);

		/**
		 * Reads the debug header to retrieve allocation metadata.
		 *
		 * @param	data			Pointer to the start of the allocation (header location).
		 * @param	outTotalSize	[out] Total allocation size stored in header.
		 * @param	outDepth		[out] Frame depth stored in header.
		 * @param	outUserSize		[out] User-requested data size stored in header.
		 */
		void ReadDebugHeader(const u8* data, u32& outTotalSize, u32& outDepth, u32& outUserSize) const;

		/**
		 * Validates the guard bytes of a single allocation.
		 *
		 * @param	data			Pointer to the start of the allocation (header location).
		 * @param	totalSize		Total allocation size from header.
		 * @param	userSize		User data size from header.
		 * @return	True if guards are intact, false if corruption detected.
		 */
		bool ValidateAllocation(const u8* data, u32 totalSize, u32 userSize) const;
#endif
	};

	/**
	 * Version of FrameAllocator that allows blocks size to be provided through the template argument instead of the
	 * constructor. */
	template <int BlockSize>
	class TFrameAllocator : public FrameAllocator
	{
	public:
		TFrameAllocator()
			: FrameAllocator(BlockSize)
		{}
	};

	// NOLINTBEGIN(readability-identifier-naming)
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

		StdFrameAlloc(FrameAllocator* alloc) noexcept
			: mFrameAlloc(alloc)
		{}

		template <class U>
		StdFrameAlloc(const StdFrameAlloc<U>& alloc) noexcept
			: mFrameAlloc(alloc.mFrameAlloc)
		{}

		template <class U>
		bool operator==(const StdFrameAlloc<U>&) const noexcept
		{
			return true;
		}

		template <class U>
		bool operator!=(const StdFrameAlloc<U>&) const noexcept
		{
			return false;
		}

		template <class U>
		class rebind
		{
		public:
			typedef StdFrameAlloc<U> other;
		};

		/** Allocate but don't initialize number elements of type T.*/
		T* allocate(const size_t num) const
		{
			if(num == 0)
				return nullptr;

			if(num > static_cast<size_t>(-1) / sizeof(T))
				return nullptr; // Error

			void* const pv = mFrameAlloc->Allocate((u32)(num * sizeof(T)));
			if(!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		void deallocate(T* p, size_t num) const noexcept
		{
			mFrameAlloc->Free((u8*)p);
		}

		FrameAllocator* mFrameAlloc = nullptr;

		size_t max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }

		void construct(pointer p, const_reference t) { new(p) T(t); }

		void destroy(pointer p) { p->~T(); }

		template <class U, class... Args>
		void construct(U* p, Args&&... args)
		{
			new(p) U(std::forward<Args>(args)...);
		}
	};

	// NOLINTEND(readability-identifier-naming)

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, class T2>
	bool operator==(const StdFrameAlloc<T1>&, const StdFrameAlloc<T2>&) throw()
	{
		return true;
	}

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, class T2>
	bool operator!=(const StdFrameAlloc<T1>&, const StdFrameAlloc<T2>&) throw()
	{
		return false;
	}

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/**
	 * Returns a global, application wide FrameAllocator. Each thread gets its own frame allocator.
	 *
	 * @note	Thread safe.
	 */
	B3D_EXPORT FrameAllocator& GetFrameAllocator();

	/**
	 * Allocates some memory using the global frame allocator.
	 *
	 * @param	byteCount	Number of bytes to allocate.
	 */
	B3D_EXPORT u8* B3DFrameAllocate(u32 byteCount);

	/**
	 * Allocates the specified number of bytes aligned to the provided boundary, using the global frame allocator. Boundary
	 * is in bytes and must be a power of two.
	 */
	B3D_EXPORT u8* B3DFrameAllocateAligned(u32 count, u32 align);

	/**
	 * Deallocates memory allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	B3D_EXPORT void B3DFrameFree(void* data);

	/**
	 * Frees memory previously allocated with B3DFrameAllocateAligned().
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	B3D_EXPORT void B3DFrameFreeAligned(void* data);

	/**
	 * Allocates enough memory to hold the object of specified type using the global frame allocator, but does not
	 * construct the object.
	 */
	template <class T>
	T* B3DFrameAllocate()
	{
		return (T*)B3DFrameAllocate(sizeof(T));
	}

	/**
	 * Allocates enough memory to hold N objects of specified type using the global frame allocator, but does not
	 * construct the object.
	 */
	template <class T>
	T* B3DFrameAllocate(u32 elementCount)
	{
		return (T*)B3DFrameAllocate(sizeof(T) * elementCount);
	}

	/**
	 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator,
	 * and constructs them.
	 */
	template <class T>
	T* B3DFrameNew(u32 elementCount = 0)
	{
		T* data = B3DFrameAllocate<T>(elementCount);

		for(unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
			new((void*)&data[elementIndex]) T;

		return data;
	}

	/**
	 * Allocates enough memory to hold the object(s) of specified type using the global frame allocator, and constructs them.
	 */
	template <class T, class... Args>
	T* B3DFrameNew(Args&&... args, u32 elementCount = 0)
	{
		T* data = B3DFrameAllocate<T>(elementCount);

		for(unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
			new((void*)&data[elementIndex]) T(std::forward<Args>(args)...);

		return data;
	}

	/**
	 * Destructs and deallocates an object allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	template <class T>
	void B3DFrameDelete(T* data)
	{
		data->~T();

		B3DFrameFree((u8*)data);
	}

	/**
	 * Destructs and deallocates an array of objects allocated with the global frame allocator.
	 *
	 * @note	Must be called on the same thread the memory was allocated on.
	 */
	template <class T>
	void B3DFrameDelete(T* data, u32 elementCount)
	{
		for(unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
			data[elementIndex].~T();

		B3DFrameFree((u8*)data);
	}

	/** @copydoc FrameAllocator::MarkFrame */
	B3D_EXPORT void B3DMarkAllocatorFrame();

	/** @copydoc FrameAllocator::Clear */
	B3D_EXPORT void B3DClearAllocatorFrame();

#if B3D_DEBUG
	/**
	 * Checks memory integrity of the global frame allocator.
	 *
	 * @return	True if all allocations are intact, false if corruption detected.
	 */
	B3D_EXPORT bool B3DFrameCheckMemory();

	/**
	 * Sets the automatic check frequency for the global frame allocator.
	 *
	 * @param	frequency	Check every N allocations. 0 disables automatic checks.
	 */
	B3D_EXPORT void B3DFrameSetCheckFrequency(u32 frequency);
#endif

	/** Opens a frame scope on construction and closes it on destruction. See B3DMarkAllocatorFrame(). */
	struct FrameAllocatorScope
	{
		FrameAllocatorScope()
		{
			mAllocator = &GetFrameAllocator();
			mAllocator->MarkFrame();
		}

		FrameAllocatorScope(FrameAllocator* allocator)
			: mAllocator(allocator)
		{
			B3D_ASSERT(allocator != nullptr);
			mAllocator->MarkFrame();
		}

		~FrameAllocatorScope()
		{
			mAllocator->Clear();
		}

	private:
		FrameAllocator* mAllocator;
	};

	/** String allocated with a frame allocator. */
	typedef std::basic_string<char, std::char_traits<char>, StdAlloc<char, FrameAllocatorTag>> FrameString;

	/** WString allocated with a frame allocator. */
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, StdAlloc<wchar_t, FrameAllocatorTag>> FrameWString;

	/** Vector allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAllocatorTag>>
	using FrameVector = std::vector<T, A>;

	/** Stack allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAllocatorTag>>
	using FrameStack = std::stack<T, std::deque<T, A>>;

	/** Queue allocated with a frame allocator. */
	template <typename T, typename A = StdAlloc<T, FrameAllocatorTag>>
	using FrameQueue = std::queue<T, std::deque<T, A>>;

	/** Set allocated with a frame allocator. */
	template <typename T, typename P = std::less<T>, typename A = StdAlloc<T, FrameAllocatorTag>>
	using FrameSet = std::set<T, P, A>;

	/** Map allocated with a frame allocator. */
	template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>, FrameAllocatorTag>>
	using FrameMap = std::map<K, V, P, A>;

	/** UnorderedSet allocated with a frame allocator. */
	template <typename T, typename H = std::hash<T>, typename C = std::equal_to<T>, typename A = StdAlloc<T, FrameAllocatorTag>>
	using FrameUnorderedSet = std::unordered_set<T, H, C, A>;

	/** UnorderedMap allocated with a frame allocator. */
	template <typename K, typename V, typename H = std::hash<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>, FrameAllocatorTag>>
	using FrameUnorderedMap = std::unordered_map<K, V, H, C, A>;

	/** @} */
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	extern B3D_THREADLOCAL FrameAllocator* _GlobalFrameAlloc;

	/**
	 * Specialized memory allocator implementations that allows use of a global frame allocator in normal
	 * new/delete/free/dealloc operators.
	 */
	template <>
	class MemoryAllocator<FrameAllocatorTag> : public MemoryAllocatorBase
	{
	public:
		/** @copydoc MemoryAllocator::Allocate */
		static void* Allocate(size_t bytes)
		{
			return B3DFrameAllocate((u32)bytes);
		}

		/** @copydoc MemoryAllocator::AllocateAligned */
		static void* AllocateAligned(size_t bytes, size_t alignment)
		{
#if B3D_PROFILING_ENABLED
			IncrementAllocationCount();
#endif

			return B3DFrameAllocateAligned((u32)bytes, (u32)alignment);
		}

		/** @copydoc MemoryAllocator::AllocateAligned16 */
		static void* AllocateAligned16(size_t bytes)
		{
#if B3D_PROFILING_ENABLED
			IncrementAllocationCount();
#endif

			return B3DFrameAllocateAligned((u32)bytes, 16);
		}

		/** @copydoc MemoryAllocator::Free */
		static void Free(void* ptr)
		{
			B3DFrameFree(ptr);
		}

		/** @copydoc MemoryAllocator::FreeAligned */
		static void FreeAligned(void* ptr)
		{
#if B3D_PROFILING_ENABLED
			IncrementFreeCount();
#endif

			B3DFrameFreeAligned(ptr);
		}

		/** @copydoc MemoryAllocator::FreeAligned16 */
		static void FreeAligned16(void* ptr)
		{
#if B3D_PROFILING_ENABLED
			IncrementFreeCount();
#endif

			B3DFrameFreeAligned(ptr);
		}
	};

	/** @} */
	/** @} */
} // namespace b3d
