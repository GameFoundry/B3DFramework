
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"
#include "Utility/BsBitwise.h"

namespace bs 
{
	/** @addtogroup General
	 *  @{
	 */

	/** 
	 * Allows encoding/decoding of types into a stream of bits. Supports various methods of storing data in a compact form. 
	 * The bitstream can manage its internal memory or a user can provide an external source of data. If using internal 
	 * memory the bitstream will automatically grow the memory storage as needed.
	 */
	class Bitstream
	{
		using QuantType = uint8_t;
	public:
		/** 
		 * Initializes an empty bitstream. As data is written the stream will grow its internal memory storage 
		 * automatically. 
		 */
		Bitstream() = default;

		/** 
		 * Initializes a bitstream with some initial capacity. If more bytes than capacity is written, the bitstream will
		 * grow its internal memory storage.
		 *
		 * @param[in]	capacity	Number of bytes to initially allocate for the internal memory storage.
		 */
		Bitstream(uint32_t capacity);

		/**
		 * Initializes a bitstream with external data storage. The bitstream will not manage internal memory and will not
		 * grow memory storage if capacity is exceeded. The user is responsible for keeping track and not writing outside
		 * of buffer range.
		 * 
		 * @param[in]	data	Address of the external memory buffer. The user is responsible of keeping this memory alive
		 *						for the lifetime of the bitstream, as well as releasing it. Must have enough capacity to 
		 *						store @p count bits.
		 * @param[in]	count	Size of the provided data, in bits.
		 */
		Bitstream(QuantType* data, uint32_t count);

		~Bitstream();

		/** 
		 * Writes bits from the provided buffer into the stream. The bits will be written at the current cursor location
		 * and the cursor will be advanced by the number of bits written. If writing outside of range the internal memory 
		 * buffer will be automatically expanded. Writing outside of range when using external memory is undefined 
		 * behaviour.
		 * 
		 * @param[in]	data	Buffer to write the data from. Must have enough capacity to store @p count bits.
		 * @param[in]	count	Number of bits to write. 
		 */
		void writeBits(const QuantType* data, uint32_t count);

		/**
		 * Reads bits from the stream into the provided buffer. The bits will be read from the current cursor location
		 * and the cursor will be advanced by the number of bits read. Attempting to read past the stream length is
		 * undefined behaviour.
		 * 
		 * @param[out]	data	Buffer to read the data from. Must have enough capacity to store @p count bits.
		 * @param[in]	count	Number of bits to read. 
		 */
		void readBits(QuantType* data, uint32_t count);

		/** 
		 * Writes the provided data into the stream in. The data will be written at the current cursor location and the 
		 * cursor will be advanced by the number of bytes written. If writing outside of range the internal memory 
		 * buffer will be automatically expanded. Writing outside of range when using external memory is undefined 
		 * behaviour.
		 * 
		 * @param[in]	value	Data to write.
		 */
		template<class T>
		void write(const T& value);

		/**
		 * Reads bits from the stream and initializes the provided object with them. The bits will be read from the 
		 * current cursor location and the cursor will be advanced by the number of bits read. Attempting to read past the 
		 * stream length is undefined behaviour.
		 * 
		 * @param[out]	value	Object to initialize with the read bits.
		 */
		template<class T>
		void read(T& value);

		/** @copydoc write(const T&) */
		void write(const bool& value);

		/** @copydoc read(const T&) */
		void read(bool& value);

		/**
		 * Skip a defined number of bits, moving the read/write cursor by this amount. This can also be a negative value, 
		 * in which case the file pointer rewinds a defined number of bits. Note the cursor can never skip past the
		 * capacity of the buffer, and will be clamped.
		 */
		void skip(int32_t count);

		/** 
		 * Repositions the read/write cursor to the specified bit. Note the cursor can never skip past the capacity
		 * of the buffer, and will be clamped.
		 */
		void seek(uint32_t pos);

		/** 
		 * Aligns the read/write cursor to a byte boundary. @p count determines the alignment in bytes. Note the
		 * requested alignment might not be achieved if count > 1 and it would move the cursor past the capacity of the 
		 * buffer, as the cursor will be clamped to buffer end regardless of alignment.
		 */
		void align(uint32_t count = 1);

		/** Returns the current read/write cursor position, in bits. */
		uint32_t tell() const { return mCursor; }

		/** Returns true if the stream has reached the end. */
		bool eof() const { return mCursor >= mNumBits; }

		/** Returns the total number of bits available in the stream. */
		uint32_t size() const { return mNumBits; }

		/** Returns the total number of bits the stream can store without needing to allocate more memory. */
		uint32_t capacity() const { return mMaxBits; }

		/** Returns the internal data buffer. */
		QuantType* data() const { return mData; }

	private:
		/** Helper function that returns 2-base logarithm for some common type sizes. */
		constexpr static uint32_t LOG2(uint32_t v)
		{
			switch(v)
			{
			default:
			case 1: return 3;
			case 2: return 4;
			case 4: return 5;
			case 8: return 6;
			}
		}

		static constexpr uint32_t BYTES_PER_QUANT = sizeof(QuantType);
		static constexpr uint32_t BITS_PER_QUANT = BYTES_PER_QUANT * 8;
		static constexpr uint32_t BITS_PER_QUANT_LOG2 = Bitwise::bitsLog2(BITS_PER_QUANT);

		/** Checks if the internal memory buffer needs to grow in order to accomodate @p numBits bits. */
		void reallocIfNeeded(uint32_t numBits);

		/** Reallocates the internal buffer making enough room for @p numBits (rounded to a multiple of BYTES_PER_QUANT. */
		void realloc(uint32_t numBits);

		QuantType* mData = nullptr;
		uint32_t mMaxBits = 0;
		uint32_t mNumBits = 0;
		bool mOwnsMemory = true;

		uint32_t mCursor = 0;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	inline Bitstream::Bitstream(uint32_t capacity)
	{
		realloc(capacity * 8);
	}

	inline Bitstream::Bitstream(QuantType* data, uint32_t count)
		: mData(data), mMaxBits(count), mNumBits(count), mOwnsMemory(false) { }

	inline Bitstream::~Bitstream()
	{
		if (mData && mOwnsMemory)
			bs_free(mData);
	}

	inline void Bitstream::writeBits(const QuantType* data, uint32_t count)
	{
		if (count == 0)
			return;

		uint32_t newCursor = mCursor + count;
		reallocIfNeeded(newCursor);

		uint32_t destBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint32_t destQuant = mCursor >> BITS_PER_QUANT_LOG2;
		uint32_t destMask = (1 << BITS_PER_QUANT) - 1;

		// If destination is aligned, memcpy everything except the last quant (unless it is also aligned)
		if (destBitsMod == 0)
		{
			uint32_t numQuants = count >> BITS_PER_QUANT_LOG2;
			memcpy(&mData[destQuant], data, numQuants * BYTES_PER_QUANT);

			data += numQuants;
			count -= numQuants * BITS_PER_QUANT;
			destQuant += numQuants;
		}

		// Write remaining bits (or all bits if destination wasn't aligned)
		while (count > 0)
		{
			QuantType quant = *data;
			data++;

			mData[destQuant] = (quant << destBitsMod) | (mData[destQuant] & destMask);

			uint32_t writtenBits = BITS_PER_QUANT - destBitsMod;
			if (count > writtenBits)
				mData[destQuant + 1] = (quant >> writtenBits) | (mData[destQuant + 1] & ~destMask);

			destQuant++;
			count -= std::min(BITS_PER_QUANT, count);
		}

		mCursor = newCursor;
		mNumBits = std::max(mNumBits, newCursor);
	}

	inline void Bitstream::readBits(QuantType* data, uint32_t count)
	{
		if (count == 0)
			return;

		assert((mCursor + count) <= mNumBits);

		uint32_t newCursor = mCursor + count;
		uint32_t srcBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint32_t srcQuant = mCursor >> BITS_PER_QUANT_LOG2;

		// If source is aligned, memcpy everything except the last quant (unless it is also aligned)
		if (srcBitsMod == 0)
		{
			uint32_t numQuants = count >> BITS_PER_QUANT_LOG2;
			memcpy(data, &mData[srcQuant], numQuants * BYTES_PER_QUANT);

			data += numQuants;
			count -= numQuants * BITS_PER_QUANT;
			srcQuant += numQuants;
		}

		// Read remaining bits (or all bits if source wasn't aligned)
		while (count > 0)
		{
			QuantType& quant = *data;
			data++;

			quant = 0;
			quant |= mData[srcQuant] >> srcBitsMod;

			uint32_t readBits = BITS_PER_QUANT - srcBitsMod;
			if (count > readBits)
				quant |= mData[srcQuant + 1] << readBits;

			srcQuant++;
			count -= std::min(BITS_PER_QUANT, count);
		}

		mCursor = newCursor;
	}

	template <class T>
	void Bitstream::write(const T& value)
	{
		writeBits((QuantType*)&value, sizeof(value) * 8);
	}

	template <class T>
	void Bitstream::read(T& value)
	{
		QuantType* temp = (QuantType*)&value;
		readBits(temp, sizeof(value) * 8);
	}

	inline void Bitstream::write(const bool& value)
	{
		reallocIfNeeded(mCursor + 1);

		uint32_t destBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint32_t destQuant = mCursor >> BITS_PER_QUANT_LOG2;

		if (value)
			mData[destQuant] |= 1U << destBitsMod;
		else
			mData[destQuant] &= ~(1U << destBitsMod);

		mCursor++;
		mNumBits = std::max(mNumBits, mCursor);
	}

	inline void Bitstream::read(bool& value)
	{
		assert((mCursor + 1) <= mNumBits);

		uint32_t srcBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint32_t srcQuant = mCursor >> BITS_PER_QUANT_LOG2;

		value = (mData[srcQuant] >> srcBitsMod) & 0x1;
		mCursor++;
	}

	inline void Bitstream::skip(int32_t count)
	{
		mCursor = (uint32_t)Math::clamp((int32_t)mCursor + count, 0, (int32_t)mMaxBits);
	}

	inline void Bitstream::seek(uint32_t pos)
	{
		mCursor = std::min(pos, mMaxBits);
	}

	inline void Bitstream::align(uint32_t count)
	{
		if (count == 0)
			return;

		uint32_t bits = count * 8;
		skip(bits - (((mCursor - 1) & (bits - 1)) + 1));
	}

	inline void Bitstream::reallocIfNeeded(uint32_t numBits)
	{
		if (numBits > mMaxBits)
		{
			if (mOwnsMemory)
			{
				// Grow
				const uint32_t newMaxBits = numBits + 4 * BITS_PER_QUANT + numBits / 2;
				realloc(newMaxBits);
			}
			else
			{
				// Caller accessing bits outside of external memory range
				assert(false);
			}
		}
	}

	inline void Bitstream::realloc(uint32_t numBits)
	{
		numBits = Math::divideAndRoundUp(numBits, BITS_PER_QUANT) * BITS_PER_QUANT;

		if (numBits != mMaxBits)
		{
			assert(numBits > mMaxBits);

			const uint32_t numQuants = Math::divideAndRoundUp(numBits, BITS_PER_QUANT);

			// Note: Eventually add support for custom allocators
			auto buffer = bs_allocN<uint8_t>(numQuants);
			if (mData)
			{
				const uint32_t numBytes = Math::divideAndRoundUp(mMaxBits, BITS_PER_QUANT) * BYTES_PER_QUANT;
				memcpy(buffer, mData, numBytes);
				bs_free(mData);
			}

			mData = buffer;
			mMaxBits = numBits;
		}
	}
}
