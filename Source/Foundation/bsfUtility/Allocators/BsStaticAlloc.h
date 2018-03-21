//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Static allocator that attempts to perform zero heap (dynamic) allocations by always keeping an active preallocated 
	 * buffer. The allocator provides a fixed amount of preallocated memory, and if the size of the allocated data goes over
	 * that limit the allocator will fall back to dynamic heap allocations using the selected allocator.
	 * 
	 * @note		Static allocations can only be freed if memory is deallocated in opposite order it is allocated.
	 *				Otherwise static memory gets orphaned until a call to clear(). Dynamic memory allocations behave
	 *				depending on the selected allocator.
	 * 
	 * @tparam	BlockSize			Size of the initially allocated static block, and minimum size of any dynamically 
	 *								allocated memory.
	 * @tparam	DynamicAllocator	Allocator to fall-back to when static buffer is full.
	 */
	template<int BlockSize = 512, class DynamicAllocator = TFrameAlloc<BlockSize>>
	class StaticAlloc
	{
	private:
		/** A single block of memory within a static allocator. */
		class MemBlock
		{
		public:
			MemBlock(UINT8* data, UINT32 size) 
				:mData(data), mSize(size)
			{ }

			/** Allocates a piece of memory within the block. Caller must ensure the block has enough empty space. */
			UINT8* alloc(UINT32 amount)
			{
				UINT8* freePtr = &mData[mFreePtr];
				mFreePtr += amount;

				return freePtr;
			}

			/** 
			 * Frees a piece of memory within the block. If the memory isn't the last allocated memory, no deallocation
			 * happens and that memory is instead orphaned.
			 */
			void free(UINT8* data, UINT32 allocSize)
			{
				if((data + allocSize) == (mData + mFreePtr))
					mFreePtr -= allocSize;
			}

			/** Releases all allocations within a block but doesn't actually free the memory. */
			void clear()
			{
				mFreePtr = 0;
			}

			UINT8* mData = nullptr;
			UINT32 mFreePtr = 0;
			UINT32 mSize = 0;
			MemBlock* mNextBlock = nullptr;
		};

	public:
		StaticAlloc() = default;
		~StaticAlloc() = default;

		/**
		 * Allocates a new piece of memory of the specified size.
		 *
		 * @param[in]	amount	Amount of memory to allocate, in bytes.
		 */
		UINT8* alloc(UINT32 amount)
		{
			if (amount == 0)
				return nullptr;

#if BS_DEBUG_MODE
			amount += sizeof(UINT32);
#endif

			UINT32 freeMem = BlockSize - mFreePtr;
			
			UINT8* data;
			if (amount > freeMem)
				data = mDynamicAlloc.alloc(amount);
			else
			{
				data = &mStaticData[mFreePtr];
				mFreePtr += amount;
			}

#if BS_DEBUG_MODE
			mTotalAllocBytes += amount;

			UINT32* storedSize = reinterpret_cast<UINT32*>(data);
			*storedSize = amount;

			return data + sizeof(UINT32);
#else
			return data;
#endif
		}

		/** Deallocates a previously allocated piece of memory. */
		void free(void* data, UINT32 allocSize)
		{
			if (data == nullptr)
				return;

			UINT8* dataPtr = (UINT8*)data;
#if BS_DEBUG_MODE
			dataPtr -= sizeof(UINT32);

			UINT32* storedSize = (UINT32*)(dataPtr);
			mTotalAllocBytes -= *storedSize;
#endif

			if(data > mStaticData && data < (mStaticData + BlockSize))
			{
				if((((UINT8*)data) + allocSize) == (mStaticData + mFreePtr))
					mFreePtr -= allocSize;
			}
			else
				mDynamicAlloc.free(dataPtr);
		}

		/** Deallocates a previously allocated piece of memory. */
		void free(void* data)
		{
			if (data == nullptr)
				return;

			UINT8* dataPtr = (UINT8*)data;
#if BS_DEBUG_MODE
			dataPtr -= sizeof(UINT32);

			UINT32* storedSize = (UINT32*)(dataPtr);
			mTotalAllocBytes -= *storedSize;
#endif
			if(data < mStaticData || data >= (mStaticData + BlockSize))
				mDynamicAlloc.free(dataPtr);
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T>
		T* construct(UINT32 count = 0)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T;

			return data;
		}

		/**
		 * Allocates enough memory to hold the object(s) of specified type using the static allocator, and constructs them.
		 */
		template<class T, class... Args>
		T* construct(Args &&...args, UINT32 count = 0)
		{
			T* data = (T*)alloc(sizeof(T) * count);

			for(unsigned int i = 0; i < count; i++)
				new ((void*)&data[i]) T(std::forward<Args>(args)...);

			return data;
		}

		/** Destructs and deallocates an object allocated with the static allocator. */
		template<class T>
		void destruct(T* data)
		{
			data->~T();

			free(data, sizeof(T));
		}

		/** Destructs and deallocates an array of objects allocated with the static frame allocator. */
		template<class T>
		void destruct(T* data, UINT32 count)
		{
			for(unsigned int i = 0; i < count; i++)
				data[i].~T();

			free(data, sizeof(T) * count);
		}

		/** Frees the internal memory buffers. All external allocations must be freed before calling this. */
		void clear()
		{
			assert(mTotalAllocBytes == 0);

			mFreePtr = 0;
			mDynamicAlloc.clear();
		}

	private:
		UINT8 mStaticData[BlockSize];
		UINT32 mFreePtr = 0;
		DynamicAllocator mDynamicAlloc;

		UINT32 mTotalAllocBytes = 0;
	};

	/** Allocator for the standard library that internally uses a static allocator. */
	template <int BlockSize, class T>
	class StdStaticAlloc
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		StdStaticAlloc() = default; 

		StdStaticAlloc(StaticAlloc<BlockSize, FreeAlloc>* alloc) noexcept
			:mStaticAlloc(alloc)
		{ }

		template<class U> StdStaticAlloc(const StdStaticAlloc<BlockSize, U>& alloc) noexcept
			:mStaticAlloc(alloc.mStaticAlloc)
		{ }

		template<class U> class rebind { public: typedef StdStaticAlloc<BlockSize, U> other; };

		/** Allocate but don't initialize number elements of type T.*/
		T* allocate(const size_t num) const
		{
			if (num == 0)
				return nullptr;

			if (num > static_cast<size_t>(-1) / sizeof(T))
				return nullptr; // Error

			void* const pv = mStaticAlloc->alloc((UINT32)(num * sizeof(T)));
			if (!pv)
				return nullptr; // Error

			return static_cast<T*>(pv);
		}

		/** Deallocate storage p of deleted elements. */
		void deallocate(T* p, size_t num) const noexcept
		{
			mStaticAlloc->free((UINT8*)p, (UINT32)num);
		}

		StaticAlloc<BlockSize, FreeAlloc>* mStaticAlloc = nullptr;

		size_t max_size() const { return std::numeric_limits<UINT32>::max() / sizeof(T); }
		void construct(pointer p, const_reference t) { new (p) T(t); }
		void destroy(pointer p) { p->~T(); }
		template<class U, class... Args>
		void construct(U* p, Args&&... args) { new(p) U(std::forward<Args>(args)...); }

		template <class T1, int N1, class T2, int N2>
		friend bool operator== (const StdStaticAlloc<N1, T1>& a, const StdStaticAlloc<N2, T2>& b) throw();
	
	};

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, int N1, class T2, int N2>
	bool operator== (const StdStaticAlloc<N1, T1>& a, const StdStaticAlloc<N2, T2>& b) throw() 
	{
		return N1 == N2 && a.mStaticAlloc == b.mStaticAlloc;
	}

	/** Return that all specializations of this allocator are interchangeable. */
	template <class T1, int N1, class T2, int N2>
	bool operator!= (const StdStaticAlloc<N1, T1>& a, const StdStaticAlloc<N2, T2>& b) throw() 
	{
		return !(a == b);
	}

	/** @} */
	/** @} */


	/** @addtogroup Memory
	 *  @{
	 */

	/** 
	 * Equivalent to Vector, except it avoids any dynamic allocations until the number of elements exceeds @p Count. 
	 * Requires allocator to be explicitly provided.
	 */
	template <typename T, int Count> 
	using StaticVector = std::vector<T, StdStaticAlloc<sizeof(T) * Count, T>>;

	/** @} */
}
