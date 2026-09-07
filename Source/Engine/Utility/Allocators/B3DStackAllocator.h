//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <assert.h>

#include "Prerequisites/B3DTypes.h"
#include "Prerequisites/B3DStdHeaders.h"
#include "Utility/B3DNonCopyable.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

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
	 * separate stacks internally. Backing storage is allocated on demand and released at thread exit. 
	 * All allocations must be freed before thread exit.
	 */
	class MemStack
	{
	public:
		/**
		 * Allocates the requested number of bytes. Each allocation has a 4 byte overhead. Backing blocks are reused when
		 * possible; requests larger than the default block capacity allocate a suitably sized block.
		 */
		static B3D_EXPORT u8* Alloc(u32 amount);

		/** Frees the most recent allocation. Memory must be freed on the allocating thread, in reverse allocation order. */
		static B3D_EXPORT void DeallocLast(u8* data);
	};

	/** @} */
	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** @copydoc MemStack::Alloc() */
	inline void* B3DStackAllocate(u32 amount)
	{
		return (void*)MemStack::Alloc(amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, but does not initialize the object.
	 *
	 * @see	MemStack::Alloc()
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
	 * @see	MemStack::Alloc()
	 */
	template <class T>
	T* B3DStackAllocate(u32 amount)
	{
		return (T*)MemStack::Alloc(sizeof(T) * amount);
	}

	/**
	 * Allocates enough memory to hold the specified type, on the stack, and constructs the object.
	 *
	 * @see	MemStack::Alloc()
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
	 * @see MemStack::Alloc()
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
	 * @see MemStack::DeallocLast()
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
	 * @see	MemStack::DeallocLast()
	 */
	template <class T>
	void B3DStackDelete(T* data, u32 count)
	{
		for(unsigned int i = 0; i < count; i++)
			data[i].~T();

		MemStack::DeallocLast((u8*)data);
	}

	/** @copydoc MemStack::DeallocLast() */
	inline void B3DStackFree(void* data)
	{
		return MemStack::DeallocLast((u8*)data);
	}

	/** Allocates memory on the stack and automatically frees it when it goes out of scope. */
	template <typename T>
	struct StackMemory : INonCopyable
	{
		template<typename... Arguments>
		explicit constexpr StackMemory(Arguments&&... arguments)
			: mData(B3DStackNew<T>(std::forward<Arguments>(arguments)...))
		{}

		~StackMemory()
		{
			if(mData != nullptr)
				B3DStackDelete(mData);
		}

		constexpr operator T*() const& noexcept { return mData; }
		constexpr T* operator->() const noexcept { return mData; }

		constexpr T* Data() const noexcept { return mData; }

	private:
		T* mData = nullptr;
	};

	/** Allocates memory on the stack and automatically frees it when it goes out of scope. */
	template <typename T>
	struct StackMemory<T[]> : INonCopyable
	{
		template<typename... Arguments>
		explicit constexpr StackMemory(Arguments&&... arguments, u32 count)
			: mData(B3DStackNew<T>(std::forward<Arguments>(arguments)..., count)), mCount(count)
		{}

		~StackMemory()
		{
			if(mData != nullptr)
				B3DStackDelete(mData, (u32)mCount);
		}

		constexpr operator T*() const& noexcept { return mData; }
		constexpr T& operator[](size_t index) const noexcept
		{
			B3D_ASSERT(index < mCount);
			return mData[index];
		}

		constexpr T* Data() const noexcept { return mData; }

	private:
		T* mData = nullptr;
		size_t mCount = 0;
	};

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
