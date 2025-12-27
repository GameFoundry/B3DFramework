//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Provides an easy way to group multiple allocations under a single (actual) allocation. Requires the user to first
	 * call Reserve() methods for all requested data elements, followed by Initialize(), after which allocation/deallocation
	 * can follow using Allocate/Free methods.
	 */
	class GroupAllocator : INonCopyable
	{
#if B3D_DEBUG
		static constexpr u32 kGuardSize = 4;
		static constexpr u32 kGuardPattern = 0xFDFDFDFD;
		static constexpr u8 kUninitPattern = 0xCD;
		static constexpr u8 kFreedPattern = 0xDD;
		static constexpr u32 kDebugHeaderSize = kGuardSize + sizeof(u32) * 2;  // pre-guard + totalSize + userSize
		static constexpr u32 kDebugTrailerSize = kGuardSize;  // post-guard
#endif

	public:
		GroupAllocator() = default;

		GroupAllocator(GroupAllocator&& other) noexcept
			: mBuffer(std::exchange(other.mBuffer, nullptr)), mNextAllocation(std::exchange(other.mNextAllocation, nullptr)), mByteCount(std::exchange(other.mByteCount, 0))
		{
		}

		~GroupAllocator()
		{
			if(mByteCount > 0)
				B3DFree(mBuffer);
		}

		GroupAllocator& operator=(GroupAllocator&& other) noexcept
		{
			if(this == &other)
				return *this;

			if(mByteCount > 0)
				B3DFree(mBuffer);

			mBuffer = std::exchange(other.mBuffer, nullptr);
			mNextAllocation = std::exchange(other.mNextAllocation, nullptr);
			mByteCount = std::exchange(other.mByteCount, 0);

			return *this;
		}

		/** Allocates internal memory as reserved by previous calls to Reserve(). Must be called before any calls to Allocate. */
		void Initialize()
		{
			B3D_ASSERT(mBuffer == nullptr);

			if(mByteCount > 0)
				mBuffer = (u8*)B3DAllocate(mByteCount);

			mNextAllocation = mBuffer;
		}

		/**
		 * Reserves the specified amount of bytes to allocate. Multiple calls to Reserve() are cumulative. After all needed
		 * memory is reserved, call Initialize(), followed by actual allocation via Allocate().
		 */
		GroupAllocator& Reserve(u32 amount)
		{
			B3D_ASSERT(mBuffer == nullptr);

#if B3D_DEBUG
			amount += kDebugHeaderSize + kDebugTrailerSize;
#endif
			mByteCount += amount;
			return *this;
		}

		/**
		 * Reserves the specified amount of bytes to allocate. Multiple calls to Reserve() are cumulative. After all needed
		 * memory is reserved, call Initialize(), followed by actual allocation via Allocate(). If you need to change the size
		 * of your allocation, free your memory by using Free(), followed by a call to Clear(). Then Reserve(), Initialize()
		 * and Allocate() again.
		 */
		template <class T>
		GroupAllocator& Reserve(u32 count = 1)
		{
			B3D_ASSERT(mBuffer == nullptr);

			u32 amount = sizeof(T) * count;
#if B3D_DEBUG
			amount += kDebugHeaderSize + kDebugTrailerSize;
#endif
			mByteCount += amount;
			return *this;
		}

		/**
		 * Allocates a new piece of memory of the specified size.
		 *
		 * @param	amount	Amount of memory to allocate, in bytes.
		 */
		u8* Allocate(u32 amount)
		{
#if B3D_DEBUG
			u32 userSize = amount;
			u32 totalSize = amount + kDebugHeaderSize + kDebugTrailerSize;

			B3D_ASSERT(mNextAllocation + totalSize <= (mBuffer + mByteCount));

			// Check integrity of all existing allocations before making new one
			B3D_ASSERT(CheckMemory() && "Memory corruption detected before allocation");

			u8* output = mNextAllocation;
			mNextAllocation += totalSize;

			// Write pre-guard
			*reinterpret_cast<u32*>(output) = kGuardPattern;

			// Write sizes
			*reinterpret_cast<u32*>(output + kGuardSize) = totalSize;
			*reinterpret_cast<u32*>(output + kGuardSize + sizeof(u32)) = userSize;

			// Get user data pointer
			u8* userData = output + kDebugHeaderSize;

			// Fill with uninitialized pattern
			std::memset(userData, kUninitPattern, userSize);

			// Write post-guard
			*reinterpret_cast<u32*>(userData + userSize) = kGuardPattern;

			return userData;
#else
			B3D_ASSERT(mNextAllocation + amount <= (mBuffer + mByteCount));

			u8* output = mNextAllocation;
			mNextAllocation += amount;

			return output;
#endif
		}

		/**
		 * Allocates enough memory to hold @p count elements of the specified type and returns an array view to the allocated memory.
		 *
		 * @param	count		Number of elements to allocate.
		 * @param	clearToZero	If true, memory is cleared to zero. Otherwise filled with debug pattern (0xCD) in debug builds.
		 * @return				Array view over the allocated memory.
		 */
		template <class T>
		TArrayView<T> Allocate(u32 count, bool clearToZero = false)
		{
			T* const data = (T*)Allocate(sizeof(T) * count);

			if(clearToZero && data != nullptr)
				B3DZeroOut(data, count);

			return TArrayView<T>(data, count);
		}

		/** Deallocates a previously allocated piece of memory. */
		void Free(void* data)
		{
#if B3D_DEBUG
			if(data)
			{
				// Check integrity of all allocations before freeing
				B3D_ASSERT(CheckMemory() && "Memory corruption detected before free");

				u8* header = static_cast<u8*>(data) - kDebugHeaderSize;

				u32 userSize = *reinterpret_cast<u32*>(header + kGuardSize + sizeof(u32));

				// Validate guards for this specific allocation
				u32 preGuard = *reinterpret_cast<u32*>(header);
				u32 postGuard = *reinterpret_cast<u32*>(static_cast<u8*>(data) + userSize);

				B3D_ASSERT(preGuard == kGuardPattern && "Memory corruption: pre-guard overwritten (buffer underrun)");
				B3D_ASSERT(postGuard == kGuardPattern && "Memory corruption: post-guard overwritten (buffer overrun)");

				// Fill with freed pattern
				std::memset(data, kFreedPattern, userSize);
			}
#endif
		}

		/** Frees any internally allocated buffers. */
		void Clear()
		{
			if(mBuffer)
				B3DFree(mBuffer);

			mByteCount = 0;
			mBuffer = nullptr;
			mNextAllocation = nullptr;
		}

#if B3D_DEBUG
		/**
		 * Validates integrity of all allocations by checking guard bytes.
		 * Called automatically on each Alloc() and Free() operation.
		 *
		 * @return	True if all guards are intact, false if corruption detected.
		 */
		bool CheckMemory() const
		{
			if(!mBuffer)
				return true;

			u8* current = mBuffer;
			u8* end = mNextAllocation;

			while(current < end)
			{
				u32 preGuard = *reinterpret_cast<const u32*>(current);
				if(preGuard != kGuardPattern)
					return false;

				u32 totalSize = *reinterpret_cast<const u32*>(current + kGuardSize);
				u32 userSize = *reinterpret_cast<const u32*>(current + kGuardSize + sizeof(u32));

				u8* userData = current + kDebugHeaderSize;
				u32 postGuard = *reinterpret_cast<const u32*>(userData + userSize);
				if(postGuard != kGuardPattern)
					return false;

				current += totalSize;
			}

			return true;
		}
#endif

	private:
		u8* mBuffer = nullptr;
		u8* mNextAllocation = nullptr;
		u32 mByteCount = 0;
	};

	/** @} */
	/** @} */
} // namespace b3d
