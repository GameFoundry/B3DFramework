
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"
#include "Utility/BsBitwise.h"

namespace bs 
{
	/** @addtogroup Implementation
	 *  @{
	 */

	class Bitfield;

	/** References a single bit in a Bitfield. */
	class BitReferenceConst
	{
	public:
		BitReferenceConst(const uint32_t& data, uint32_t bitMask)
			:mData(data), mBitMask(bitMask)
		{ }

		operator bool() const
		{
			return (mData & mBitMask) != 0;
		}

	protected:
		const uint32_t& mData;
		uint32_t mBitMask;
	};

	/** References a single bit in a Bitfield and allows it to be modified. */
	class BitReference
	{
	public:
		BitReference(uint32_t& data, uint32_t bitMask)
			:mData(data), mBitMask(bitMask)
		{ }

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
		uint32_t& mData;
		uint32_t mBitMask;
	};

	/** Helper template used for specifying types for const and non-const iterator variants for Bitfield. */
	template<bool CONST>
	struct TBitfieldIteratorTypes
	{ };

	template<>
	struct TBitfieldIteratorTypes<true>
	{
		typedef const Bitfield& ArrayType;
		typedef BitReferenceConst ReferenceType;
	};

	template<>
	struct TBitfieldIteratorTypes<false>
	{
		typedef Bitfield& ArrayType;
		typedef BitReference ReferenceType;
	};

	/** Iterator for iterating over individual bits in a Bitfield. */
	template<bool CONST>
	class TBitfieldIterator
	{
	public:
		typedef typename TBitfieldIteratorTypes<CONST>::ArrayType ArrayType;
		typedef typename TBitfieldIteratorTypes<CONST>::ReferenceType ReferenceType;

		TBitfieldIterator(ArrayType owner, uint32_t bitIndex, uint32_t dwordIndex, uint32_t mask)
			: mOwner(owner), mBitIndex(bitIndex), mDwordIndex(dwordIndex), mMask(mask)
		{ }

		TBitfieldIterator& operator++()
		{
			mBitIndex++;
			mMask <<= 1;

			if (!mMask)
			{
				mDwordIndex++;
				mMask = 1;
			}

			return *this;
		}

		operator bool() const
		{
			return mBitIndex < mOwner.size();
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
			assert((bool)*this);

			return ReferenceType(mOwner.mData[mDwordIndex], mMask);
		}

	private:
		ArrayType mOwner;
		uint32_t mBitIndex;
		uint32_t mDwordIndex;
		uint32_t mMask;
	};

	/** @} */

	/** @addtogroup General
	 *  @{
	 */

	/** 
	 * Dynamically sized field that contains a sequential list of bits. The bits are compactly stored and allow for
	 * quick sequential searches (compared to single or multi-byte type sequential searches).
	 */
	class Bitfield
	{
		static constexpr uint32_t BITS_PER_DWORD = sizeof(uint32_t) * 8;
		static constexpr uint32_t BITS_PER_DWORD_LOG2 = 5;

	public:
		using Iterator = TBitfieldIterator<false>;
		using ConstIterator = TBitfieldIterator<true>;

		/** 
		 * Initializes the bitfield with enough storage for @p count bits and sets them to the initial value of @p value. 
		 */
		Bitfield(bool value = false, uint32_t count = 0)
			:mNumBits(count)
		{
			if(count > 0)
			{
				realloc(count);
				reset(value);
			}
		}

		~Bitfield()
		{
			if(mData)
				bs_free(mData);
		}

		Bitfield(const Bitfield& other)
			:mNumBits(other.mNumBits)
		{
			if (other.mMaxBits)
			{
				realloc(other.mMaxBits);

				const uint32_t numBytes = Math::divideAndRoundUp(other.mNumBits, BITS_PER_DWORD) * sizeof(uint32_t);
				memcpy(mData, other.mData, numBytes);
			}
		}

		Bitfield(Bitfield&& other)
			: mData(std::exchange(other.mData, nullptr))
			, mMaxBits(std::exchange(other.mMaxBits, 0))
			, mNumBits(std::exchange(other.mNumBits, 0))
		{ }

		Bitfield& operator=(const Bitfield& rhs)
		{
			if(this != &rhs)
			{
				clear(true);
				mNumBits = rhs.mNumBits;

				if (rhs.mMaxBits)
				{
					realloc(rhs.mMaxBits);

					const uint32_t numBytes = Math::divideAndRoundUp(rhs.mNumBits, BITS_PER_DWORD) * sizeof(uint32_t);
					memcpy(mData, rhs.mData, numBytes);
				}
			}

			return *this;
		}

		Bitfield& operator=(Bitfield&& rhs)
		{
			if(this != &rhs)
			{
				if (mData)
					bs_free(mData);

				mData = std::exchange(rhs.mData, nullptr);
				mNumBits = std::exchange(rhs.mNumBits, 0);
				mMaxBits = std::exchange(rhs.mMaxBits, 0);
			}

			return *this;
		}

		BitReference operator[](uint32_t idx)
		{
			assert(idx < mNumBits);

			const uint32_t bitMask = 1 << (idx & (BITS_PER_DWORD - 1));
			uint32_t& data = mData[idx >> BITS_PER_DWORD_LOG2];

			return BitReference(data, bitMask);
		}

		BitReferenceConst operator[](uint32_t idx) const
		{
			assert(idx < mNumBits);

			const uint32_t bitMask = 1 << (idx & (BITS_PER_DWORD - 1));
			uint32_t& data = mData[idx >> BITS_PER_DWORD_LOG2];

			return BitReferenceConst(data, bitMask);
		}

		/** Adds a new bit value to the end of the bitfield and returns the index of the added bit. */
		uint32_t add(bool value)
		{
			if(mNumBits >= mMaxBits)
			{
				// Grow
				const uint32_t newMaxBits = mMaxBits + 4 * BITS_PER_DWORD + mMaxBits / 2;
				realloc(newMaxBits);
			}

			const uint32_t index = mNumBits;
			mNumBits++;

			(*this)[index] = value;
			return index;
		}

		/** Removes a bit at the specified index. */
		void remove(uint32_t index)
		{
			assert(index < mNumBits);

			const uint32_t dwordIndex = index >> BITS_PER_DWORD_LOG2;
			const uint32_t mask = 1 << (index & (BITS_PER_DWORD - 1));

			const uint32_t curDwordBits = mData[dwordIndex];

			// Mask the dword we want to remove the bit from
			const uint32_t firstHalfMask = mask - 1; // These stay the same
			const uint32_t secondHalfMask = ~firstHalfMask; // These get shifted so the removed bit gets moved outside the mask

			mData[dwordIndex] = (curDwordBits & firstHalfMask) | (((curDwordBits >> 1) & secondHalfMask));

			// Grab the last bit from the next dword and put it as the last bit in the current dword. Then shift the
			// next dword and repeat until all following dwords are processed.
			const uint32_t lastDwordIndex = (mNumBits - 1) >> BITS_PER_DWORD_LOG2;
			for(uint32_t i = dwordIndex; i < lastDwordIndex; i++)
			{
				// First bit from next dword goes at the end of the current dword
				mData[i] |= (mData[i + 1] & 0x1) << 31;

				// Following dword gets shifted, removing the bit we just mvoed
				mData[i + 1] >>= 1;
			}

			mNumBits--;
		}

		/** Attempts to find the first non-zero bit in the field. Returns -1 if all bits are zero or the field is empty. */
		uint32_t find(bool value) const
		{
			const uint32_t mask = value ? 0 : (uint32_t)-1;
			const uint32_t numDWords = Math::divideAndRoundUp(mNumBits, BITS_PER_DWORD);

			for(uint32_t i = 0; i < numDWords; i++)
			{
				if(mData[i] == mask)
					continue;

				const uint32_t bits = value ? mData[i] : ~mData[i];
				const uint32_t bitIndex = i * BITS_PER_DWORD + Bitwise::leastSignificantBit(bits);

				if(bitIndex < mNumBits)
					return bitIndex;
			}

			return (uint32_t)-1;
		}

		/** Counts the number of values in the bit field. */
		uint32_t count(bool value) const
		{
			// Note: Implement this faster via popcnt and similar instructions

			uint32_t counter = 0;
			for(const auto& entry : *this)
			{
				if(entry == value)
					counter++;
			}

			return counter;
		}

		/** Resets all the bits in the field to the specified value. */
		void reset(bool value = false)
		{
			if(mNumBits == 0)
				return;

			const int32_t mask = value ? 0xFF : 0;
			const uint32_t numBytes = Math::divideAndRoundUp(mNumBits, BITS_PER_DWORD) * sizeof(uint32_t);
			memset(mData, mask, numBytes);
		}

		/** 
		 * Removes all the bits from the field. If @p free is true then the underlying memory buffers will be freed as 
		 * well. 
		 */
		void clear(bool free = false)
		{
			mNumBits = 0;

			if(free)
			{
				if (mData)
				{
					bs_free(mData);
					mData = nullptr;
				}

				mMaxBits = 0;
			}
		}

		/** Returns the number of bits in the bitfield */
		uint32_t size() const
		{
			return mNumBits;
		}

		/** Returns a non-const iterator pointing to the first bit in the bitfield. */
		Iterator begin()
		{
			return Iterator(*this, 0, 0, 1);
		}

		/** Returns a non-const interator pointing past the last bit in the bitfield. */
		Iterator end()
		{
			uint32_t bitIndex = mNumBits;
			uint32_t dwordIndex = bitIndex >> BITS_PER_DWORD_LOG2;
			uint32_t mask = 1 << (bitIndex & (BITS_PER_DWORD - 1));

			return Iterator(*this, bitIndex, dwordIndex, mask);
		}

		/** Returns a const iterator pointing to the first bit in the bitfield. */
		ConstIterator begin() const
		{
			return ConstIterator(*this, 0, 0, 1);
		}

		/** Returns a const interator pointing past the last bit in the bitfield. */
		ConstIterator end() const
		{
			uint32_t bitIndex = mNumBits;
			uint32_t dwordIndex = bitIndex >> BITS_PER_DWORD_LOG2;
			uint32_t mask = 1 << (bitIndex & (BITS_PER_DWORD - 1));

			return ConstIterator(*this, bitIndex, dwordIndex, mask);
		}
		
	private:
		template<bool CONST>
		friend class TBitfieldIterator;

		/** Reallocates the internal buffer making enough room for @p numBits (rounded to a multiple of DWORD). */
		void realloc(uint32_t numBits)
		{
			numBits = Math::divideAndRoundUp(numBits, BITS_PER_DWORD) * BITS_PER_DWORD;

			if(numBits != mMaxBits)
			{
				assert(numBits > mMaxBits);

				const uint32_t numDwords = Math::divideAndRoundUp(numBits, BITS_PER_DWORD); 
				
				// Note: Eventually add support for custom allocators
				auto buffer = bs_allocN<uint32_t>(numDwords);
				if(mData)
				{
					const uint32_t numBytes = Math::divideAndRoundUp(mMaxBits, BITS_PER_DWORD) * sizeof(uint32_t);
					memcpy(buffer, mData, numBytes);
					bs_free(mData);
				}

				mData = buffer;
				mMaxBits = numBits;
			}
		}

		uint32_t* mData = nullptr;
		uint32_t mMaxBits = 0;
		uint32_t mNumBits;
	};


}

/** @cond SPECIALIZATIONS */
/** @addtogroup Implementation
 *  @{
 */

namespace std
{
	template <> inline void swap(bs::BitReference& lhs, bs::BitReference& rhs)
	{
		const bool temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	inline void swap(bs::BitReference&& lhs, bs::BitReference&& rhs)
	{
		const bool temp = lhs;
		lhs = rhs;
		rhs = temp;
	}
};

/** @endgroup */
/** @endcond */