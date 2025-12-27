
//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMath.h"
#include "Utility/B3DBitwise.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	template<class Allocator>
	class TBitfield;

	/** References a single bit in a TBitfield. */
	class BitReferenceConst
	{
	public:
		BitReferenceConst(const u32& data, u32 bitMask)
			: mData(data), mBitMask(bitMask)
		{}

		operator bool() const
		{
			return (mData & mBitMask) != 0;
		}

	protected:
		const u32& mData;
		u32 mBitMask;
	};

	/** References a single bit in a TBitfield and allows it to be modified. */
	class BitReference
	{
	public:
		BitReference(u32& data, u32 bitMask)
			: mData(data), mBitMask(bitMask)
		{}

		operator bool() const
		{
			return (mData & mBitMask) != 0;
		}

		BitReference& operator=(bool value)
		{
			if(value)
				mData |= mBitMask;
			else
				mData &= ~mBitMask;

			return *this;
		}

		BitReference& operator=(const BitReference& rhs)
		{
			*this = (bool)rhs;
			return *this;
		}

	protected:
		u32& mData;
		u32 mBitMask;
	};

	/** Helper template used for specifying types for const and non-const iterator variants for Bitfield. */
	template <bool CONST, class Allocator>
	struct TBitfieldIteratorTypes
	{};

	template <class Allocator>
	struct TBitfieldIteratorTypes<true, Allocator>
	{
		typedef const TBitfield<Allocator>& ArrayType;
		typedef BitReferenceConst ReferenceType;
	};

	template <class Allocator>
	struct TBitfieldIteratorTypes<false, Allocator>
	{
		typedef TBitfield<Allocator>& ArrayType;
		typedef BitReference ReferenceType;
	};

	/** Iterator for iterating over individual bits in a Bitfield. */
	template <bool CONST, class Allocator>
	class TBitfieldIterator
	{
	public:
		typedef typename TBitfieldIteratorTypes<CONST, Allocator>::ArrayType ArrayType;
		typedef typename TBitfieldIteratorTypes<CONST, Allocator>::ReferenceType ReferenceType;

		TBitfieldIterator(ArrayType owner, u64 bitIndex, u64 dwordIndex, u32 mask)
			: mOwner(owner), mBitIndex(bitIndex), mDwordIndex(dwordIndex), mMask(mask)
		{}

		TBitfieldIterator& operator++()
		{
			mBitIndex++;
			mMask <<= 1;

			if(!mMask)
			{
				mDwordIndex++;
				mMask = 1;
			}

			return *this;
		}

		operator bool() const
		{
			return mBitIndex < mOwner.Size();
		}

		bool operator!() const
		{
			return !(bool)*this;
		}

		bool operator!=(const TBitfieldIterator& rhs)
		{
			return mBitIndex != rhs.mBitIndex;
		}

		ReferenceType operator*() const
		{
			B3D_ASSERT((bool)*this);

			return ReferenceType(mOwner.mAllocator.GetElements()[mDwordIndex], mMask);
		}

	private:
		ArrayType mOwner;
		u64 mBitIndex;
		u64 mDwordIndex;
		u32 mMask;
	};

	/** @} */

	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Dynamically sized field that contains a sequential list of bits. The bits are compactly stored and allow for
	 * quick sequential searches (compared to single or multi-byte type sequential searches).
	 */
	template<class Allocator = DefaultContainerAllocator>
	class TBitfield
	{
	public:
		static constexpr u64 kBitsPerDword = sizeof(u32) * 8;
		static constexpr u64 kBitsPerDwordLoG2 = 5;

		using Iterator = TBitfieldIterator<false, Allocator>;
		using ConstIterator = TBitfieldIterator<true, Allocator>;

		/**
		 * Initializes the bitfield with enough storage for @p count bits and sets them to the initial value of @p value.
		 */
		TBitfield(bool value = false, u64 count = 0)
		{
			Realloc(count);

			// Must assign this after reallocating, as reallocation uses this to determine current element count
			mNumBits = count;

			if(count > 0)
				Reset(value);
		}

		~TBitfield() = default;

		TBitfield(const TBitfield& other)
		{
			*this = other;
		}

		TBitfield(TBitfield&& other)
		{
			*this = std::move(other);
		}

		TBitfield& operator=(const TBitfield& rhs)
		{
			if(this != &rhs)
			{
				Clear(true);
				mNumBits = rhs.mNumBits;

				if(rhs.mMaxBits)
				{
					Realloc(rhs.mMaxBits);

					const u64 numBytes = Math::DivideAndRoundUp(rhs.mNumBits, kBitsPerDword) * sizeof(u32);
					memcpy(mAllocator.GetElements(), rhs.mAllocator.GetElements(), numBytes);
				}
			}

			return *this;
		}

		TBitfield& operator=(TBitfield&& rhs)
		{
			if(this != &rhs)
			{
				const u64 myDwordCount = Math::DivideAndRoundUp(mNumBits, kBitsPerDword);
				const u64 otherDwordCount = Math::DivideAndRoundUp(rhs.mNumBits, kBitsPerDword);

				mAllocator.Move(myDwordCount, otherDwordCount, std::move(rhs.mAllocator));
				mNumBits = std::exchange(rhs.mNumBits, 0);
				mMaxBits = std::exchange(rhs.mMaxBits, rhs.mAllocator.GetMinimumCapacity() * kBitsPerDword);
			}

			return *this;
		}

		BitReference operator[](u64 index)
		{
			B3D_ASSERT(index < mNumBits);

			const u32 bitMask = 1 << (index & (kBitsPerDword - 1));
			u32& data = mAllocator.GetElements()[index >> kBitsPerDwordLoG2];

			return BitReference(data, bitMask);
		}

		BitReferenceConst operator[](u64 index) const
		{
			B3D_ASSERT(index < mNumBits);

			const u32 bitMask = 1 << (index & (kBitsPerDword - 1));
			u32& data = mAllocator.GetElements()[index >> kBitsPerDwordLoG2];

			return BitReferenceConst(data, bitMask);
		}

		/** Adds a new bit value to the end of the bitfield and returns the index of the added bit. */
		u64 Add(bool value)
		{
			if(mNumBits >= mMaxBits)
			{
				// Grow
				const u64 newMaxBits = mMaxBits + 4 * kBitsPerDword + mMaxBits / 2;
				Realloc(newMaxBits);
			}

			const u64 index = mNumBits;
			mNumBits++;

			(*this)[index] = value;
			return index;
		}

		/** Removes a bit at the specified index. */
		void Remove(u64 index)
		{
			B3D_ASSERT(index < mNumBits);

			const u64 dwordIndex = index >> kBitsPerDwordLoG2;
			const u32 mask = 1 << (index & (kBitsPerDword - 1));

			const u32 curDwordBits = mAllocator.GetElements()[dwordIndex];

			// Mask the dword we want to remove the bit from
			const u32 firstHalfMask = mask - 1; // These stay the same
			const u32 secondHalfMask = ~firstHalfMask; // These get shifted so the removed bit gets moved outside the mask

			mAllocator.GetElements()[dwordIndex] = (curDwordBits & firstHalfMask) | (((curDwordBits >> 1) & secondHalfMask));

			// Grab the last bit from the next dword and put it as the last bit in the current dword. Then shift the
			// next dword and repeat until all following dwords are processed.
			const u64 lastDwordIndex = (mNumBits - 1) >> kBitsPerDwordLoG2;
			for(u64 currentDwordIndex = dwordIndex; currentDwordIndex < lastDwordIndex; currentDwordIndex++)
			{
				// First bit from next dword goes at the end of the current dword
				mAllocator.GetElements()[currentDwordIndex] |= (mAllocator.GetElements()[currentDwordIndex + 1] & 0x1) << 31;

				// Following dword gets shifted, removing the bit we just mvoed
				mAllocator.GetElements()[currentDwordIndex + 1] >>= 1;
			}

			mNumBits--;
		}

		/** Attempts to find the first non-zero bit in the field. Returns -1 if all bits are zero or the field is empty. */
		u64 Find(bool value) const
		{
			const u32 mask = value ? 0 : ~0u;
			const u64 numDWords = Math::DivideAndRoundUp(mNumBits, kBitsPerDword);

			for(u64 dwordIndex = 0; dwordIndex < numDWords; dwordIndex++)
			{
				if(mAllocator.GetElements()[dwordIndex] == mask)
					continue;

				const u32 bits = value ? mAllocator.GetElements()[dwordIndex] : ~mAllocator.GetElements()[dwordIndex];
				const u64 bitIndex = dwordIndex * kBitsPerDword + Bitwise::LeastSignificantBit(bits);

				if(bitIndex < mNumBits)
					return bitIndex;
			}

			return ~0ULL;
		}

		/** Counts the number of values in the bit field. */
		u64 Count(bool value) const
		{
			// TODO: Implement this faster via popcnt and similar instructions

			u64 counter = 0;
			for(const auto& entry : *this)
			{
				if(entry == value)
					counter++;
			}

			return counter;
		}

		/** Resizes the bitfield to the specified number of elements, initializing any new elements with @p value. */
		void Resize(u64 size, bool value = false)
		{
			if(size > mMaxBits)
				Realloc(size);

			const u64 oldSize = mNumBits;
			mNumBits = size;

			// TODO: Assign the values more efficiently
			for(u64 index = oldSize; index < size; ++index)
				(*this)[index] = value;
		}

		/**
		 * Resizes the internal buffer to the specified size, but doesn't add any new elements. If existing capacity is equal or larger than the
		 * requested size, no operation is performed.
		 */
		void Reserve(u64 size)
		{
			if(mMaxBits >= size)
				return;

			Realloc(size);
		}

		/** Resets all the bits in the field to the specified value. */
		void Reset(bool value = false)
		{
			if(mNumBits == 0)
				return;

			const u32 mask = value ? 0xFF : 0;
			const u64 numBytes = Math::DivideAndRoundUp(mNumBits, kBitsPerDword) * sizeof(u32);
			memset(mAllocator.GetElements(), mask, numBytes);
		}

		/**
		 * Removes all the bits from the field. If @p free is true then the underlying memory buffers will be freed as
		 * well.
		 */
		void Clear(bool free = false)
		{
			mNumBits = 0;

			if(free)
			{
				const u64 currentDwordCount = Math::DivideAndRoundUp(mNumBits, kBitsPerDword);
				mAllocator.Resize(currentDwordCount, 0);
				mMaxBits = mAllocator.GetMinimumCapacity() * kBitsPerDword;
			}
		}

		/** Returns the number of bits in the bitfield. */
		u64 Size() const
		{
			return mNumBits;
		}

		/** Returns the total allocated capacity of the bitfield, in number of bits. */
		u64 Capacity() const
		{
			return mMaxBits;
		}

		/** Returns the underlying bitfield data. Data is always sequential and allocated using 32-bit alignment. */
		const u32* Data() const
		{
			return mAllocator.GetElements();
		}

		/** Returns the underlying bitfield data. */
		u32* Data() 
		{
			return mAllocator.GetElements();
		}

		/** Returns a non-const iterator pointing to the first bit in the bitfield. */
		Iterator Begin()
		{
			return Iterator(*this, 0, 0, 1);
		}

		/** Returns a non-const interator pointing past the last bit in the bitfield. */
		Iterator End()
		{
			u64 bitIndex = mNumBits;
			u64 dwordIndex = bitIndex >> kBitsPerDwordLoG2;
			u32 mask = 1 << (bitIndex & (kBitsPerDword - 1));

			return Iterator(*this, bitIndex, dwordIndex, mask);
		}

		/** Returns a const iterator pointing to the first bit in the bitfield. */
		ConstIterator Begin() const
		{
			return ConstIterator(*this, 0, 0, 1);
		}

		/** Returns a const interator pointing past the last bit in the bitfield. */
		ConstIterator End() const
		{
			u64 bitIndex = mNumBits;
			u64 dwordIndex = bitIndex >> kBitsPerDwordLoG2;
			u32 mask = 1 << (bitIndex & (kBitsPerDword - 1));

			return ConstIterator(*this, bitIndex, dwordIndex, mask);
		}

		// NOLINTBEGIN
		/** @copydoc Begin */
		Iterator begin() { return Begin(); }

		/** @copydoc End */
		Iterator end() { return End(); }

		/** @copydoc Begin */
		ConstIterator begin() const { return Begin(); }

		/** @copydoc End */
		ConstIterator end() const { return End(); }
		// NOLINTEND

	private:
		template <bool CONST, class Allocator2>
		friend class TBitfieldIterator;

		/** Reallocates the internal buffer making enough room for @p bitCapacity (rounded to a multiple of 32-bits). */
		void Realloc(u64 bitCapacity)
		{
			bitCapacity = Math::DivideAndRoundUp(bitCapacity, kBitsPerDword) * kBitsPerDword;
			bitCapacity = Math::Max(mAllocator.GetMinimumCapacity() * kBitsPerDword, bitCapacity);

			if(bitCapacity != mMaxBits)
			{
				const u64 currentDwordCount = Math::DivideAndRoundUp(mNumBits, kBitsPerDword);
				const u64 newDwordCount = Math::DivideAndRoundUp(bitCapacity, kBitsPerDword);

				mAllocator.Resize(currentDwordCount, newDwordCount);
				mMaxBits = bitCapacity;
			}
		}

		typename Allocator::template ForElementType<u32> mAllocator;
		u64 mMaxBits = mAllocator.GetMinimumCapacity() * kBitsPerDword;
		u64 mNumBits = 0;
	};

} // namespace b3d

/** @cond SPECIALIZATIONS */
/** @addtogroup Implementation
 *  @{
 */

namespace std
{
	template <>
	inline void swap(b3d::BitReference& lhs, b3d::BitReference& rhs)
	{
		const bool temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	inline void Swap(b3d::BitReference&& lhs, b3d::BitReference&& rhs)
	{
		const bool temp = lhs;
		lhs = rhs;
		rhs = temp;
	}
}; // namespace std

/** @endgroup */
/** @endcond */
