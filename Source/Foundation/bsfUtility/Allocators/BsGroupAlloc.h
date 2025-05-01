//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Provides an easy way to group multiple allocations under a single (actual) allocation. Requires the user to first
	 * call reserve() methods for all requested data elements, followed by init(), after which allocation/deallocation
	 * can follow using construct/destruct or alloc/free methods.
	 */
	class GroupAlloc : INonCopyable
	{
	public:
		GroupAlloc() = default;

		GroupAlloc(GroupAlloc&& other) noexcept
			: mData(std::exchange(other.mData, nullptr)), mDataPtr(std::exchange(other.mDataPtr, nullptr)), mNumBytes(std::exchange(other.mNumBytes, 0))
		{
		}

		~GroupAlloc()
		{
			if(mNumBytes > 0)
				B3DFree(mData);
		}

		GroupAlloc& operator=(GroupAlloc&& other) noexcept
		{
			if(this == &other)
				return *this;

			if(mNumBytes > 0)
				B3DFree(mData);

			mData = std::exchange(other.mData, nullptr);
			mDataPtr = std::exchange(other.mDataPtr, nullptr);
			mNumBytes = std::exchange(other.mNumBytes, 0);

			return *this;
		}

		/**
		 * Allocates internal memory as reserved by previous calls to reserve(). Must be called before any calls to
		 * construct or alloc.
		 */
		void Init()
		{
			B3D_ASSERT(mData == nullptr);

			if(mNumBytes > 0)
				mData = (u8*)B3DAllocate(mNumBytes);

			mDataPtr = mData;
		}

		/**
		 * Reserves the specified amount of bytes to allocate. Multiple calls to reserve() are cumulative. After all needed
		 * memory is reserved, call init(), followed by actual allocation via construct() or alloc() methods.
		 */
		GroupAlloc& Reserve(u32 amount)
		{
			B3D_ASSERT(mData == nullptr);

			mNumBytes += amount;
			return *this;
		}

		/**
		 * Reserves the specified amount of bytes to allocate. Multiple calls to reserve() are cumulative. After all needed
		 * memory is reserved, call init(), followed by actual allocation via construct() or alloc() methods. If you need
		 * to change the size of your allocation, free your memory by using free(), followed by a call to clear(). Then
		 * reserve(), init() and alloc() again.
		 */
		template <class T>
		GroupAlloc& Reserve(u32 count = 1)
		{
			B3D_ASSERT(mData == nullptr);

			mNumBytes += sizeof(T) * count;
			return *this;
		}

		/**
		 * Allocates a new piece of memory of the specified size.
		 *
		 * @param[in]	amount	Amount of memory to allocate, in bytes.
		 */
		u8* Alloc(u32 amount)
		{
			B3D_ASSERT(mDataPtr + amount <= (mData + mNumBytes));

			u8* output = mDataPtr;
			mDataPtr += amount;

			return output;
		}

		/**
		 * Allocates enough memory to hold @p count elements of the specified type.
		 *
		 * @param[in]	count	Number of elements to allocate.
		 */
		template <class T>
		T* Alloc(u32 count = 1)
		{
			return (T*)Alloc(sizeof(T) * count);
		}

		/** Deallocates a previously allocated piece of memory. */
		void Free(void* data)
		{
			// Do nothing
		}

		/** Frees any internally allocated buffers. All elements must be previously freed by calling free(). */
		void Clear()
		{
			// Note: A debug check if user actually freed the memory could be helpful
			if(mData)
				B3DFree(mData);

			mNumBytes = 0;
			mData = nullptr;
			mDataPtr = nullptr;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template <class T>
		T* Construct(u32 count = 1)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new((void*)&data[i]) T;

			return data;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template <class T, class... Args>
		T* Construct(Args&&... args, u32 count = 1)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new((void*)&data[i]) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates an object allocated with the static allocator. */
		template <class T>
		void Destruct(T* data)
		{
			data->~T();

			free(data);
		}

		/** Destructs and deallocates an array of objects allocated with the static frame allocator. */
		template <class T>
		void Destruct(T* data, u32 count)
		{
			for(unsigned int i = 0; i < count; i++)
				data[i].~T();

			free(data);
		}

	private:
		u8* mData = nullptr;
		u8* mDataPtr = nullptr;
		u32 mNumBytes = 0;
	};

	/** @} */
	/** @} */
} // namespace bs
