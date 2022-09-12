//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

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
	 * can follow using Construct/destruct or Allocate/free methods.
	 */
	class GroupAlloc : INonCopyable
	{
	public:
		GroupAlloc() = default;

		GroupAlloc(GroupAlloc&& other) noexcept
			: mData(std::exchange(other.mData, nullptr)), mDataPtr(std::exchange(other.mDataPtr, nullptr)),
				 mNumBytes(std::exchange(other.mNumBytes, 0))
		{
		}

		~GroupAlloc()
		{
			if (mNumBytes > 0)
				bs_free(mData);
		}

		GroupAlloc& operator=(GroupAlloc&& other) noexcept
		{
			if (this == &other)
				return *this;

			if (mNumBytes > 0)
				bs_free(mData);

			mData = std::exchange(other.mData, nullptr);
			mDataPtr = std::exchange(other.mDataPtr, nullptr);
			mNumBytes = std::exchange(other.mNumBytes, 0);

			return *this;
		}

		/**
		 * Allocates internal memory as reserved by previous calls to reserve(). Must be called before any calls to
		 * Construct or Allocate.
		 */
		void Init()
		{
			assert(mData == nullptr);

			if (mNumBytes > 0)
				mData = (UINT8*)bs_alloc(mNumBytes);

			mDataPtr = mData;
		}

		/**
		 * Reserves the specified amount of bytes to Allocate. Multiple calls to reserve() are cumulative. After all needed
		 * memory is reserved, call init(), followed by actual allocation via Construct() or Allocate() methods.
		 */
		GroupAlloc& Reserve(UINT32 amount)
		{
			assert(mData == nullptr);

			mNumBytes += amount;
			return *this;
		}

		/**
		 * Reserves the specified amount of bytes to Allocate. Multiple calls to reserve() are cumulative. After all needed
		 * memory is reserved, call init(), followed by actual allocation via Construct() or Allocate() methods. If you need
		 * to change the size of your allocation, free your memory by using free(), followed by a call to clear(). Then
		 * reserve(), init() and Allocate() again.
		 */
		template<class T>
		GroupAlloc& Reserve(UINT32 count = 1)
		{
			assert(mData == nullptr);

			mNumBytes += sizeof(T) * count;
			return *this;
		}

		/**
		 * Allocates a new piece of memory of the specified size.
		 *
		 * @param[in]	amount	Amount of memory to Allocate, in bytes.
		 */
		UINT8* Allocate(UINT32 amount)
		{
			assert(mDataPtr + amount <= (mData + mNumBytes));

			UINT8* output = mDataPtr;
			mDataPtr += amount;

			return output;
		}

		/**
		 * Allocates enough memory to hold @p count elements of the specified type.
		 *
		 * @param[in]	amount	Number of elements to Allocate.
		 */
		template<class T>
		T* Allocate(UINT32 amount = 1)
		{
			return (T*) Allocate(sizeof(T) * amount);
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
			if (mData)
				bs_free(mData);

			mNumBytes = 0;
			mData = nullptr;
			mDataPtr = nullptr;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T>
		T* Construct(UINT32 count = 1)
		{
			T* data = (T*) Allocate(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T;

			return data;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T, class... Args>
		T* Construct(Args &&...args, UINT32 count = 1)
		{
			T* data = (T*) Allocate(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates an object allocated with the static allocator. */
		template<class T>
		void Destruct(T* data)
		{
			data->~T();

			free(data);
		}

		/** Destructs and deallocates an array of objects allocated with the static frame allocator. */
		template<class T>
		void Destruct(T* data, UINT32 count)
		{
			for(unsigned int i = 0; i < count; i++)
				data[i].~T();

			free(data);
		}

	private:
		UINT8* mData = nullptr;
		UINT8* mDataPtr = nullptr;
		UINT32 mNumBytes = 0;
	};

	/** @} */
	/** @} */
}
