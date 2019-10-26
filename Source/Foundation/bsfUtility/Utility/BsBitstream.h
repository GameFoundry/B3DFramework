
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"
#include "Math/BsQuaternion.h"
#include "Math/BsVector3.h"
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
	 *
	 * The stream keeps an internal cursor that represents the bit at which to perform read & write operations.
	 * Read & write operations will operate at the current cursor location and the cursor will be advanced by the number of
	 * bits read or written. If writing outside of range the internal memory buffer will be automatically expanded, except
	 * when external memory buffer is used, in which case it is undefined behaviour. Reading outside of range is always
	 * undefined behaviour.
	 */
	class Bitstream
	{
	public:
		using QuantType = uint8_t;
		static constexpr uint32_t BYTES_PER_QUANT = sizeof(QuantType);
		static constexpr uint32_t BITS_PER_QUANT = BYTES_PER_QUANT * 8;
		static constexpr uint32_t BITS_PER_QUANT_LOG2 = Bitwise::bitsLog2(BITS_PER_QUANT);

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
		 * @param[in]	count	Size of the provided data, in bytes.
		 */
		Bitstream(QuantType* data, uint32_t count);

		Bitstream(const Bitstream& other);
		Bitstream(Bitstream&& other);
		~Bitstream();
		
		Bitstream& operator= (const Bitstream& other);
		Bitstream& operator= (Bitstream&& other);

		/**
		 * Writes bits from the provided buffer into the stream at the current cursor location and advances the cursor.
		 *
		 * @param[in]	data	Buffer to write the data from. Must have enough capacity to store @p count bits.
		 * @param[in]	count	Number of bits to write.
		 * @return				Number of bits written.
		 */
		uint64_t writeBits(const QuantType* data, uint64_t count);

		/**
		 * Reads bits from the stream into the provided buffer from the current cursor location and advances the cursor.
		 *
		 * @param[out]	data	Buffer to read the data from. Must have enough capacity to store @p count bits.
		 * @param[in]	count	Number of bits to read.
		 * @return				Number of bits read.
		 */
		uint64_t readBits(QuantType* data, uint64_t count);

		/**
		 * Writes the provided data into the stream at the current cursor location and advances the cursor. This
		 * will compress certain data types if possible (i.e. a boolean will be written as a single bit).
		 *
		 * @param[in]	value	Data to write.
		 * @return				Number of bits written.
		 */
		template<class T>
		uint64_t write(const T& value);

		/**
		 * Reads bits from the stream previously written by write() and stores them into the provided object. Data is
		 * read from the current cursor location and advances the cursor.
		 *
		 * @param[out]	value	Object to initialize with the read bits.
		 * @return				Number of bits read.
		 */
		template<class T>
		uint64_t read(T& value);

		/** @copydoc write(const T&) */
		uint64_t write(const bool& value);

		/** @copydoc read(T&) */
		uint64_t read(bool& value);

		/** @copydoc write(const T&) */
		uint64_t write(const String& value);

		/** @copydoc read(T&) */
		uint64_t read(String& value);

		/**
		 * Writes the provided data into the stream at the current cursor location and advances the cursor.
		 * Unlike write() this function always writes the full object size (i.e. sizeof(T)).
		 *
		 * @param[in]	value	Data to write.
		 * @return				Number of bytes written.
		 */
		template<class T>
		uint32_t writeBytes(const T& value);

		/**
		 * Reads bits from the stream previously written by writeBytes() and stores them into the provided object. Data is
		 * read from the current cursor location and advances the cursor.
		 *
		 * @param[out]	value	Object to initialize with the read bits.
		 * @return				Number of bytes read.
		 */
		template<class T>
		uint32_t readBytes(T& value);

		/**
		 * Writes bytes from the provided buffer into the stream at the current cursor location and advances the cursor.
		 *
		 * @param[in]	data	Buffer to write the data from. Must have enough capacity to store @p count bytes.
		 * @param[in]	count	Number of bytes to write.
		 * @return				Number of bytes written.
		 */
		uint32_t writeBytes(const QuantType* data, uint32_t count);

		/**
		 * Reads bytes from the stream into the provided buffer from the current cursor location and advances the cursor.
		 *
		 * @param[out]	data	Buffer to read the data from. Must have enough capacity to store @p count bytes.
		 * @param[in]	count	Number of bytes to read.
		 * @return				Number of bytes read.
		 */
		uint32_t readBytes(QuantType* data, uint32_t count);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling write().
		 */
		template<class T>
		uint64_t writeDelta(const T& value, const T& lastValue);

		/** Reads the data written by writeDelta() from the current cursor location and advances the cursor. */
		template<class T>
		uint64_t readDelta(T& value, const T& lastValue);

		/** @copydoc writeDelta(const T&, const T&) */
		uint32_t writeDelta(bool value, bool lastValue);

		/** @copydoc readDelta(T&, const T&) */
		uint32_t readDelta(bool& value, bool lastValue);

		/**
		 * Encodes a 32-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t writeVarInt(uint32_t value);

		/**
		 * Encodes a 32-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t writeVarInt(int32_t value);

		/**
		 * Encodes a 64-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t writeVarInt(uint64_t value);

		/**
		 * Encodes a 64-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t writeVarInt(int64_t value);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t readVarInt(uint32_t& value);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t readVarInt(int32_t& value);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t readVarInt(uint64_t& value);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t readVarInt(int64_t& value);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling writeVarInt(). Returns the number
		 * of bits written.
		 */
		template<class T>
		uint32_t writeVarIntDelta(const T& value, const T& lastValue);

		/**
		 * Reads the data written by writeVarIntDelta() from the current cursor location and advances the cursor.
		 * Returns the number of bits read.
		 */
		template<class T>
		uint32_t readVarIntDelta(T& value, const T& lastValue);

		/**
		 * Encodes a float in range [0, 1] into a fixed point representation using a specific number of bits, and writes it
		 * to the stream. Write is performed at the current cursor location and advances the cursor.
		 */
		void writeNorm(float value, uint32_t bits = 16);

		/**
		 * Decodes a float encoded using writeNorm(float, uint32_t). Read is performed at the current cursor location and
		 * advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void readNorm(float& value, uint32_t bits = 16);

		/**
		 * Encodes a 3D vector with individual components in range [-1, 1] into a fixed point representation where each
		 * component uses a specific number of bits, and writes it to the stream. Write is performed at the current cursor
		 * location and advances the cursor.
		 */
		void writeNorm(const Vector3& value, uint32_t bits = 16);

		/**
		 * Decodes a 3D vector encoded using writeNorm(Vector3, uint32_t). Read is performed at the current cursor location
		 * and advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void readNorm(Vector3& value, uint32_t bits = 16);

		/**
		 * Encodes a quaternion with individual components in range [-1, 1] into a fixed point representation where each
		 * component uses a specific number of bits, and writes it to the stream. Write is performed at the current cursor
		 * location and advances the cursor.
		 */
		void writeNorm(const Quaternion& value, uint32_t bits = 16);

		/**
		 * Decodes a quaternion encoded using writeNorm(Quaternion, uint32_t). Read is performed at the current cursor
		 * location and advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void readNorm(Quaternion& value, uint32_t bits = 16);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling writeNorm().
		 */
		template<class T>
		void writeNormDelta(const T& value, const T& lastValue, uint32_t bits = 16);

		/** Reads the data written by writeNormDelta() from the current cursor location and advances the cursor. */
		template<class T>
		void readNormDelta(T& value, const T& lastValue, uint32_t bits = 16);

		/**
		 * Encodes an integer in a specific range, using the range the reduce the number of bits required, and writes it
		 * to the stream. Write is performed at the current cursor location and advances the cursor. Returns the number
		 * of bits written.
		 */
		template<class T>
		uint32_t writeRange(const T& value, const T& min, const T& max);

		/**
		 * Decodes an integer encoded using writeRange(const T&, const T&, const T&). Read is performed at the current
		 * cursor location and advances the cursor. Same needs to be used as when the value was encoded. Returns the
		 * number of bits read.
		 */
		template<class T>
		uint32_t readRange(T& value, const T& min, const T& max);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling
		 * writeRange(const T&, const T&, const T&). Returns the number of bits written.
		 */
		template<class T>
		uint32_t writeRangeDelta(const T& value, const T& lastValue, const T& min, const T& max);

		/**
		 * Reads the data written by writeRangeDelta(const T&, const T&, const T&, const T&) from the current cursor
		 * location and advances the cursor. Returns the number of bits read.
		 */
		template<class T>
		uint32_t readRangeDelta(T& value, const T& lastValue, const T& min, const T& max);

		/**
		 * Encodes a float in a specific range into a fixed point representation using a specific number of bits, and
		 * writes it to the stream. Write is performed at the current cursor location and advances the cursor.
		 */
		void writeRange(float value, float min, float max, uint32_t bits = 16);

		/**
		 * Decodes a float encoded using writeRange(float, float, float, uint32_t). Read is performed at the current cursor
		 * location and advances the cursor. Same number of bits, and the same range needs to be used as when the float was
		 * encoded.
		 */
		void readRange(float& value, float min, float max, uint32_t bits = 16);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling
		 * writeRange(float, float, float, uint32_t).
		 */
		void writeRangeDelta(float value, float lastValue, float min, float max, uint32_t bits = 16);

		/**
		 * Reads the data written by writeRangeDelta(float, float, float, float, uint32_t) from the current cursor
		 * location and advances the cursor.
		 */
		void readRangeDelta(float& value, float lastValue, float min, float max, uint32_t bits = 16);

		/**
		 * Skip a defined number of bits, moving the read/write cursor by this amount. This can also be a negative value,
		 * in which case the file pointer rewinds a defined number of bits. Note the cursor can never skip past the
		 * capacity of the buffer, and will be clamped.
		 */
		void skip(int64_t count);

		/** Same as skip() except is uses number of bytes instead of number of bits as the parameter. */
		void skipBytes(int32_t count) { return skip((uint64_t)count * 8); }

		/**
		 * Repositions the read/write cursor to the specified bit. Note the cursor can never skip past the capacity
		 * of the buffer, and will be clamped.
		 */
		void seek(uint64_t pos);

		/**
		 * Aligns the read/write cursor to a byte boundary. @p count determines the alignment in bytes. Note the
		 * requested alignment might not be achieved if count > 1 and it would move the cursor past the capacity of the
		 * buffer, as the cursor will be clamped to buffer end regardless of alignment.
		 */
		void align(uint32_t count = 1);

		/** Expands the capacity to the specified number of bytes, unless already equal or greater. */
		void reserve(uint32_t count);
		
		/**
		 * Expands the capacity and size to the specified number of bytes. Capacity will not be reduced if already
		 * equal or larger.
		 */
		void resize(uint32_t count);

		/** Returns the current read/write cursor position, in bits. */
		uint64_t tell() const { return mCursor; }

		/** Returns true if the stream has reached the end. */
		bool eof() const { return mCursor >= mNumBits; }

		/** Returns the total number of bits available in the stream. */
		uint64_t size() const { return mNumBits; }

		/** Returns the total number of bits the stream can store without needing to allocate more memory. */
		uint64_t capacity() const { return mMaxBits; }

		/** Returns the internal data buffer. */
		QuantType* data() const { return mData; }

		/** Returns the byte the read/write cursor is currently positioned on. */
		QuantType* cursor() const;

	private:
		/** Checks if the internal memory buffer needs to grow in order to accomodate @p numBits bits. */
		void reallocIfNeeded(uint64_t numBits);

		/** Reallocates the internal buffer making enough room for @p numBits (rounded to a multiple of BYTES_PER_QUANT. */
		void realloc(uint64_t numBits);

		QuantType* mData = nullptr;
		uint64_t mMaxBits = 0;
		uint64_t mNumBits = 0;
		bool mOwnsMemory = true;

		uint64_t mCursor = 0;
	};

	/** @} */

	inline Bitstream::Bitstream(uint32_t capacity)
	{
		realloc((uint64_t)(capacity) * 8);
	}

	inline Bitstream::Bitstream(QuantType* data, uint32_t count)
		: mData(data), mMaxBits((uint64_t)count * 8), mNumBits((uint64_t)count * 8), mOwnsMemory(false) { }

	inline Bitstream::~Bitstream()
	{
		if (mData && mOwnsMemory)
			bs_free(mData);
	}

	inline Bitstream::Bitstream(const Bitstream& other)
	{
		*this = other;
	}

	inline Bitstream::Bitstream(Bitstream&& other)
	{
		*this = std::move(other);
	}

	inline Bitstream& Bitstream::operator= (const Bitstream& other)
	{
		if (this == &other)
			return *this;

		this->mCursor = other.mCursor;
		this->mNumBits = other.mNumBits;
		
		if(!other.mOwnsMemory)
		{
			this->mData = other.mData;
			this->mMaxBits = other.mMaxBits;
			this->mOwnsMemory = false;
		}
		else
		{
			if (mData && mOwnsMemory)
				bs_free(mData);

			mData = nullptr;
			mMaxBits = 0;

			this->mOwnsMemory = true;
			realloc(other.mMaxBits);

			if(mMaxBits > 0)
			{
				const uint32_t numBytes = (uint32_t)Math::divideAndRoundUp(mMaxBits, (uint64_t)BITS_PER_QUANT) * BYTES_PER_QUANT;
				memcpy(mData, other.mData, numBytes);
			}
		}

		return *this;
	}

	inline Bitstream& Bitstream::operator= (Bitstream&& other)
	{
		if (this == &other)
			return *this;

		if (mData && mOwnsMemory)
			bs_free(mData);

		this->mCursor = std::exchange(other.mCursor, 0);
		this->mNumBits = std::exchange(other.mNumBits, 0);
		this->mMaxBits = std::exchange(other.mMaxBits, 0);
		this->mData = std::exchange(other.mData, nullptr);
		this->mOwnsMemory = std::exchange(other.mOwnsMemory, false);

		return *this;
	}

	inline uint64_t Bitstream::writeBits(const QuantType* data, uint64_t count)
	{
		if (count == 0)
			return 0;

		uint64_t newCursor = mCursor + count;
		reallocIfNeeded(newCursor);

		uint64_t remaining = count;
		uint64_t destBitsMod = (mCursor & (BITS_PER_QUANT - 1));
		uint64_t destQuant = mCursor >> BITS_PER_QUANT_LOG2;
		uint64_t destMask = (1 << destBitsMod) - 1;

		// If destination is aligned, memcpy everything except the last quant (unless it is also aligned)
		if (destBitsMod == 0)
		{
			uint64_t numQuants = remaining >> BITS_PER_QUANT_LOG2;
			memcpy(&mData[destQuant], data, numQuants * BYTES_PER_QUANT);

			data += numQuants;
			remaining -= numQuants * BITS_PER_QUANT;
			destQuant += numQuants;
		}

		// Write remaining bits (or all bits if destination wasn't aligned)
		while (remaining > 0)
		{
			QuantType quant = *data;
			data++;

			mData[destQuant] = (quant << destBitsMod) | (mData[destQuant] & destMask);

			uint32_t writtenBits = (uint32_t)(BITS_PER_QUANT - destBitsMod);
			if (remaining > writtenBits)
				mData[destQuant + 1] = (quant >> writtenBits) | (mData[destQuant + 1] & ~destMask);

			destQuant++;
			remaining -= std::min((uint64_t)BITS_PER_QUANT, remaining);
		}

		mCursor = newCursor;
		mNumBits = std::max(mNumBits, newCursor);

		return count;
	}

	inline uint64_t Bitstream::readBits(QuantType* data, uint64_t count)
	{
		if (count == 0)
			return 0;

		assert((mCursor + count) <= mNumBits);

		uint64_t remaining = count;
		uint64_t newCursor = mCursor + count;
		uint64_t srcBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint64_t srcQuant = mCursor >> BITS_PER_QUANT_LOG2;

		// If source is aligned, memcpy everything except the last quant (unless it is also aligned)
		if (srcBitsMod == 0)
		{
			uint64_t numQuants = remaining >> BITS_PER_QUANT_LOG2;
			memcpy(data, &mData[srcQuant], numQuants * BYTES_PER_QUANT);

			data += numQuants;
			remaining -= numQuants * BITS_PER_QUANT;
			srcQuant += numQuants;
		}

		// Read remaining bits (or all bits if source wasn't aligned)
		while (remaining > 0)
		{
			QuantType& quant = *data;
			data++;

			quant = 0;
			quant |= mData[srcQuant] >> srcBitsMod;

			uint32_t readBits = (uint32_t)(BITS_PER_QUANT - srcBitsMod);
			if (remaining > readBits)
				quant |= mData[srcQuant + 1] << readBits;

			srcQuant++;
			remaining -= std::min((uint64_t)BITS_PER_QUANT, remaining);
		}

		mCursor = newCursor;
		return count;
	}

	template <class T>
	uint64_t Bitstream::write(const T& value)
	{
		return writeBits((QuantType*)&value, sizeof(value) * 8);
	}

	template <class T>
	uint64_t Bitstream::read(T& value)
	{
		QuantType* temp = (QuantType*)&value;
		return readBits(temp, sizeof(value) * 8);
	}

	inline uint64_t Bitstream::write(const bool& value)
	{
		reallocIfNeeded(mCursor + 1);

		uint64_t destBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint64_t destQuant = mCursor >> BITS_PER_QUANT_LOG2;

		if (value)
			mData[destQuant] |= 1U << destBitsMod;
		else
			mData[destQuant] &= ~(1U << destBitsMod);

		mCursor++;
		mNumBits = std::max(mNumBits, mCursor);

		return 1;
	}

	inline uint64_t Bitstream::read(bool& value)
	{
		assert((mCursor + 1) <= mNumBits);

		uint64_t srcBitsMod = mCursor & (BITS_PER_QUANT - 1);
		uint64_t srcQuant = mCursor >> BITS_PER_QUANT_LOG2;

		value = (mData[srcQuant] >> srcBitsMod) & 0x1;
		mCursor++;

		return 1;
	}

	inline uint64_t Bitstream::write(const String& value)
	{
		uint32_t length = (uint32_t)value.size();
		uint64_t written = writeVarInt(length);
		written += writeBits((QuantType*)value.data(), (uint64_t)length * 8);

		return written;
	}

	inline uint64_t Bitstream::read(String& value)
	{
		uint32_t length;
		uint64_t read = readVarInt(length);

		value.resize(length);

		QuantType* temp = (QuantType*)value.data();
		read += readBits(temp, (uint64_t)length * 8);

		return read;
	}

	template <class T>
	uint32_t Bitstream::writeBytes(const T& value)
	{
		uint64_t numBits = writeBits((QuantType*)& value, sizeof(value) * 8);
		assert((numBits % 8) == 0);
		
		return (uint32_t)(numBits / 8);
	}

	template <class T>
	uint32_t Bitstream::readBytes(T& value)
	{
		QuantType* temp = (QuantType*)& value;
		uint64_t numBits = readBits(temp, sizeof(value) * 8);
		assert((numBits % 8) == 0);

		return (uint32_t)(numBits / 8);
	}

	inline uint32_t Bitstream::writeBytes(const QuantType* data, uint32_t count)
	{
		return (uint32_t)(writeBits(data, (uint64_t)count * 8) / 8);
	}

	inline uint32_t Bitstream::readBytes(QuantType* data, uint32_t count)
	{
		return (uint32_t)(readBits(data, (uint64_t)count * 8) / 8);
	}

	template <class T>
	uint64_t Bitstream::writeDelta(const T& value, const T& lastValue)
	{
		if (value == lastValue)
			return write(true);
		else
			return write(false) + write(value);
	}

	template <class T>
	uint64_t Bitstream::readDelta(T& value, const T& lastValue)
	{
		bool clean;
		read(clean);

		if (clean)
		{
			value = lastValue;
			return 1;
		}
		else
			return read(value) + 1;
	}

	inline uint32_t Bitstream::writeDelta(bool value, bool lastValue)
	{
		return (uint32_t)write(value);
	}

	inline uint32_t Bitstream::readDelta(bool& value, bool lastValue)
	{
		return (uint32_t)read(value);
	}

	inline uint32_t Bitstream::writeVarInt(uint32_t value)
	{
		uint8_t output[5];
		uint32_t count = Bitwise::encodeVarInt(value, output);

		return (uint32_t)writeBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::writeVarInt(int32_t value)
	{
		uint8_t output[5];
		uint32_t count = Bitwise::encodeVarInt(value, output);

		return (uint32_t)writeBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::writeVarInt(uint64_t value)
	{
		uint8_t output[10];
		uint32_t count = Bitwise::encodeVarInt(value, output);

		return (uint32_t)writeBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::writeVarInt(int64_t value)
	{
		uint8_t output[10];
		uint32_t count = Bitwise::encodeVarInt(value, output);

		return (uint32_t)writeBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::readVarInt(uint32_t& value)
	{
		uint32_t read = 0;
		uint8_t output[5];
		for(uint32_t i = 0; i < 5; i++)
		{
			read += (uint32_t)readBits(&output[i], 8);
			if((output[i] & 0x80) == 0)
				break;
		}

		Bitwise::decodeVarInt(value, output, 5);
		return read;
	}

	inline uint32_t Bitstream::readVarInt(int32_t& value)
	{
		uint32_t read = 0;
		uint8_t output[5];
		for(uint32_t i = 0; i < 5; i++)
		{
			read += (uint32_t)readBits(&output[i], 8);
			if((output[i] & 0x80) == 0)
				break;
		}

		Bitwise::decodeVarInt(value, output, 5);
		return read;
	}

	inline uint32_t Bitstream::readVarInt(uint64_t& value)
	{
		uint32_t read = 0;
		uint8_t output[10];
		for(uint32_t i = 0; i < 10; i++)
		{
			read += (uint32_t)readBits(&output[i], 8);
			if((output[i] & 0x80) == 0)
				break;
		}

		Bitwise::decodeVarInt(value, output, 10);
		return read;
	}

	inline uint32_t Bitstream::readVarInt(int64_t& value)
	{
		uint32_t read = 0;
		uint8_t output[10];
		for(uint32_t i = 0; i < 10; i++)
		{
			read += (uint32_t)readBits(&output[i], 8);
			if((output[i] & 0x80) == 0)
				break;
		}

		Bitwise::decodeVarInt(value, output, 10);
		return read;
	}

	template <class T>
	uint32_t Bitstream::writeVarIntDelta(const T& value, const T& lastValue)
	{
		if (value == lastValue)
		{
			write(true);
			return 1;
		}
		else
		{
			write(false);
			return writeVarInt(value) + 1;
		}
	}

	template <class T>
	uint32_t Bitstream::readVarIntDelta(T& value, const T& lastValue)
	{
		bool clean;
		read(clean);

		if (clean)
		{
			value = lastValue;
			return 1;
		}
		else
			return readVarInt(value) + 1;
	}

	inline void Bitstream::writeNorm(float value, uint32_t bits)
	{
		uint32_t encodedVal = Bitwise::unormToUint(value, bits);
		writeBits((QuantType*)&encodedVal, bits);
	}

	inline void Bitstream::readNorm(float& value, uint32_t bits)
	{
		uint32_t encodedVal = 0;
		readBits((QuantType*)&encodedVal, bits);
		value = Bitwise::uintToUnorm(encodedVal, bits);
	}

	inline void Bitstream::writeNorm(const Vector3& value, uint32_t bits)
	{
		writeRange(value.x, -1.0f, 1.0f, bits);
		writeRange(value.y, -1.0f, 1.0f, bits);
		writeRange(value.z, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::readNorm(Vector3& value, uint32_t bits)
	{
		readRange(value.x, -1.0f, 1.0f, bits);
		readRange(value.y, -1.0f, 1.0f, bits);
		readRange(value.z, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::writeNorm(const Quaternion& value, uint32_t bits)
	{
		writeRange(value.x, -1.0f, 1.0f, bits);
		writeRange(value.y, -1.0f, 1.0f, bits);
		writeRange(value.z, -1.0f, 1.0f, bits);
		writeRange(value.w, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::readNorm(Quaternion& value, uint32_t bits)
	{
		readRange(value.x, -1.0f, 1.0f, bits);
		readRange(value.y, -1.0f, 1.0f, bits);
		readRange(value.z, -1.0f, 1.0f, bits);
		readRange(value.w, -1.0f, 1.0f, bits);
	}

	template <class T>
	void Bitstream::writeNormDelta(const T& value, const T& lastValue, uint32_t bits)
	{
		if(value == lastValue)
			write(true);
		else
		{
			write(false);
			writeNorm(value, bits);
		}
	}

	template <class T>
	void Bitstream::readNormDelta(T& value, const T& lastValue, uint32_t bits)
	{
		bool clean;
		read(clean);

		if(clean)
			value = lastValue;
		else
			readNorm(value, bits);
	}


	template <class T>
	uint32_t Bitstream::writeRange(const T& value, const T& min, const T& max)
	{
		T range = max - min;
		uint32_t bits = Bitwise::mostSignificantBit(range) + 1;

		T rangeVal = value - min;
		writeBits((QuantType*)&rangeVal, bits);

		return bits;
	}

	template <class T>
	uint32_t Bitstream::readRange(T& value, const T& min, const T& max)
	{
		T range = max - min;
		uint32_t bits = Bitwise::mostSignificantBit(range) + 1;

		value = 0;
		readBits((QuantType*)&value, bits);
		value += min;

		return bits;
	}

	template <class T>
	uint32_t Bitstream::writeRangeDelta(const T& value, const T& lastValue, const T& min, const T& max)
	{
		if (value == lastValue)
		{
			write(true);
			return 1;
		}
		else
		{
			write(false);
			return writeRange(value, min, max) + 1;
		}
	}

	template <class T>
	uint32_t Bitstream::readRangeDelta(T& value, const T& lastValue, const T& min, const T& max)
	{
		bool clean;
		read(clean);

		if (clean)
		{
			value = lastValue;
			return 1;
		}
		else
			return readRange(value, min, max) + 1;
	}

	inline void Bitstream::writeRange(float value, float min, float max, uint32_t bits)
	{
		float pct = Math::clamp01((value - min)/(max - min));
		writeNorm(pct, bits);
	}

	inline void Bitstream::readRange(float& value, float min, float max, uint32_t bits)
	{
		float pct;
		readNorm(pct, bits);

		value = min + (max - min) * pct;
	}

	inline void Bitstream::writeRangeDelta(float value, float lastValue, float min, float max, uint32_t bits)
	{
		if(value == lastValue)
			write(true);
		else
		{
			write(false);
			writeRange(value, min, max, bits);
		}
	}

	inline void Bitstream::readRangeDelta(float& value, float lastValue, float min, float max, uint32_t bits)
	{
		bool clean;
		read(clean);

		if(clean)
			value = lastValue;
		else
			readRange(value, min, max, bits);
	}

	inline void Bitstream::skip(int64_t count)
	{
		mCursor = (uint64_t)Math::clamp((int64_t)mCursor + count, (int64_t)0, (int64_t)mMaxBits);
	}

	inline void Bitstream::seek(uint64_t pos)
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

	inline void Bitstream::reserve(uint32_t count)
	{
		if (capacity() < ((uint64_t)count * 8))
			realloc((uint64_t)count * 8);
	}

	inline void Bitstream::resize(uint32_t count)
	{
		reserve(count);
		mNumBits = (uint64_t)count * 8;
	}

	inline Bitstream::QuantType* Bitstream::cursor() const
	{
		return &mData[mCursor >> BITS_PER_QUANT_LOG2];
	}

	inline void Bitstream::reallocIfNeeded(uint64_t numBits)
	{
		if (numBits > mMaxBits)
		{
			if (mOwnsMemory)
			{
				// Grow
				const uint64_t newMaxBits = numBits + 4 * BITS_PER_QUANT + numBits / 2;
				realloc(newMaxBits);
			}
			else
			{
				// Caller accessing bits outside of external memory range
				assert(false);
			}
		}
	}

	inline void Bitstream::realloc(uint64_t numBits)
	{
		numBits = Math::divideAndRoundUp(numBits, (uint64_t)BITS_PER_QUANT) * BITS_PER_QUANT;

		if (numBits != mMaxBits)
		{
			assert(numBits > mMaxBits);

			const uint32_t numQuants = (uint32_t)Math::divideAndRoundUp(numBits, (uint64_t)BITS_PER_QUANT);

			// Note: Eventually add support for custom allocators
			auto buffer = bs_allocN<uint8_t>(numQuants);
			if (mData)
			{
				const uint32_t numBytes = (uint32_t)Math::divideAndRoundUp(mMaxBits, (uint64_t)BITS_PER_QUANT) * BYTES_PER_QUANT;
				memcpy(buffer, mData, numBytes);
				bs_free(mData);
			}

			mData = buffer;
			mMaxBits = numBits;
		}
	}
}
