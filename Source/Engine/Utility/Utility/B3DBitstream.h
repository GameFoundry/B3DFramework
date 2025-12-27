
//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMath.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DVector3.h"
#include "Utility/B3DBitwise.h"

namespace b3d
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
		static constexpr uint32_t kBytesPerQuant = sizeof(QuantType);
		static constexpr uint32_t kBitsPerQuant = kBytesPerQuant * 8;
		static constexpr uint32_t kBitsPerQuantLoG2 = Bitwise::BitsLog2(kBitsPerQuant);

		/**
		 * Initializes an empty bitstream. As data is written the stream will grow its internal memory storage
		 * automatically.
		 */
		Bitstream() = default;

		/**
		 * Initializes a bitstream with some initial capacity. If more bytes than capacity is written, the bitstream will
		 * grow its internal memory storage.
		 *
		 * @param	capacity	Number of bytes to initially allocate for the internal memory storage.
		 */
		Bitstream(uint32_t capacity);

		/**
		 * Initializes a bitstream with external data storage. The bitstream will not manage internal memory and will not
		 * grow memory storage if capacity is exceeded. The user is responsible for keeping track and not writing outside
		 * of buffer range.
		 *
		 * @param	data	Address of the external memory buffer. The user is responsible of keeping this memory alive
		 *					for the lifetime of the bitstream, as well as releasing it. Must have enough capacity to
		 *					store @p count bits.
		 * @param	count	Size of the provided data, in bytes.
		 */
		Bitstream(QuantType* data, uint32_t count);

		Bitstream(const Bitstream& other);
		Bitstream(Bitstream&& other);
		~Bitstream();

		Bitstream& operator=(const Bitstream& other);
		Bitstream& operator=(Bitstream&& other);

		/**
		 * Writes bits from the provided buffer into the stream at the current cursor location and advances the cursor.
		 *
		 * @param	data	Buffer to write the data from. Must have enough capacity to store @p count bits.
		 * @param	count	Number of bits to write.
		 * @return			Number of bits written.
		 */
		uint64_t WriteBits(const QuantType* data, uint64_t count);

		/**
		 * Reads bits from the stream into the provided buffer from the current cursor location and advances the cursor.
		 *
		 * @param	outData	Buffer to read the data from. Must have enough capacity to store @p count bits.
		 * @param	count	Number of bits to read.
		 * @return			Number of bits read.
		 */
		uint64_t ReadBits(QuantType* outData, uint64_t count);

		/**
		 * Writes the provided data into the stream at the current cursor location and advances the cursor. This
		 * will compress certain data types if possible (i.e. a boolean will be written as a single bit).
		 *
		 * @param	value	Data to write.
		 * @return			Number of bits written.
		 */
		template <class T>
		uint64_t Write(const T& value);

		/**
		 * Reads bits from the stream previously written by write() and stores them into the provided object. Data is
		 * read from the current cursor location and advances the cursor.
		 *
		 * @param	outValue	Object to initialize with the read bits.
		 * @return				Number of bits read.
		 */
		template <class T>
		uint64_t Read(T& outValue);

		/** @copydoc Write(const T&) */
		uint64_t Write(const bool& value);

		/** @copydoc Read(T&) */
		uint64_t Read(bool& outValue);

		/** @copydoc Write(const T&) */
		uint64_t Write(const String& value);

		/** @copydoc Read(T&) */
		uint64_t Read(String& outValue);

		/**
		 * Writes the provided data into the stream at the current cursor location and advances the cursor.
		 * Unlike write() this function always writes the full object size (i.e. sizeof(T)).
		 *
		 * @param	value	Data to write.
		 * @return			Number of bytes written.
		 */
		template <class T>
		uint32_t WriteBytes(const T& value);

		/**
		 * Reads bits from the stream previously written by writeBytes() and stores them into the provided object. Data is
		 * read from the current cursor location and advances the cursor.
		 *
		 * @param	outValue	Object to initialize with the read bits.
		 * @return				Number of bytes read.
		 */
		template <class T>
		uint32_t ReadBytes(T& outValue);

		/**
		 * Writes bytes from the provided buffer into the stream at the current cursor location and advances the cursor.
		 *
		 * @param	data	Buffer to write the data from. Must have enough capacity to store @p count bytes.
		 * @param	count	Number of bytes to write.
		 * @return			Number of bytes written.
		 */
		uint32_t WriteBytes(const QuantType* data, uint32_t count);

		/**
		 * Reads bytes from the stream into the provided buffer from the current cursor location and advances the cursor.
		 *
		 * @param	outData	Buffer to read the data from. Must have enough capacity to store @p count bytes.
		 * @param	count	Number of bytes to read.
		 * @return			Number of bytes read.
		 */
		uint32_t ReadBytes(QuantType* outData, uint32_t count);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling write().
		 */
		template <class T>
		uint64_t WriteDelta(const T& value, const T& lastValue);

		/** Reads the data written by writeDelta() from the current cursor location and advances the cursor. */
		template <class T>
		uint64_t ReadDelta(T& outValue, const T& lastValue);

		/** @copydoc WriteDelta(const T&, const T&) */
		uint32_t WriteDelta(bool value, bool lastValue);

		/** @copydoc ReadDelta(T&, const T&) */
		uint32_t ReadDelta(bool& outValue, bool lastValue);

		/**
		 * Encodes a 32-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t WriteVarInt(uint32_t value);

		/**
		 * Encodes a 32-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t WriteVarInt(int32_t value);

		/**
		 * Encodes a 64-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t WriteVarInt(uint64_t value);

		/**
		 * Encodes a 64-bit integer value as a base-128 varint and writes it to the stream. Write is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t WriteVarInt(int64_t value);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t ReadVarInt(uint32_t& outValue);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t ReadVarInt(int32_t& outValue);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t ReadVarInt(uint64_t& outValue);

		/**
		 * Decodes a 32-bit integer value encoded as a base-128 varint from the stream. Read is performed at the
		 * current cursor location and advances the cursor. Varints are a method of serializing integers using one or
		 * more bytes, where smaller values use less bytes. Returns the number of bits written.
		 */
		uint32_t ReadVarInt(int64_t& outValue);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling writeVarInt(). Returns the number
		 * of bits written.
		 */
		template <class T>
		uint32_t WriteVarIntDelta(const T& value, const T& lastValue);

		/**
		 * Reads the data written by writeVarIntDelta() from the current cursor location and advances the cursor.
		 * Returns the number of bits read.
		 */
		template <class T>
		uint32_t ReadVarIntDelta(T& outValue, const T& lastValue);

		/**
		 * Encodes a float in range [0, 1] into a fixed point representation using a specific number of bits, and writes it
		 * to the stream. Write is performed at the current cursor location and advances the cursor.
		 */
		void WriteNorm(float value, uint32_t bits = 16);

		/**
		 * Decodes a float encoded using writeNorm(float, uint32_t). Read is performed at the current cursor location and
		 * advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void ReadNorm(float& outValue, uint32_t bits = 16);

		/**
		 * Encodes a 3D vector with individual components in range [-1, 1] into a fixed point representation where each
		 * component uses a specific number of bits, and writes it to the stream. Write is performed at the current cursor
		 * location and advances the cursor.
		 */
		void WriteNorm(const Vector3& value, uint32_t bits = 16);

		/**
		 * Decodes a 3D vector encoded using writeNorm(Vector3, uint32_t). Read is performed at the current cursor location
		 * and advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void ReadNorm(Vector3& outValue, uint32_t bits = 16);

		/**
		 * Encodes a quaternion with individual components in range [-1, 1] into a fixed point representation where each
		 * component uses a specific number of bits, and writes it to the stream. Write is performed at the current cursor
		 * location and advances the cursor.
		 */
		void WriteNorm(const Quaternion& value, uint32_t bits = 16);

		/**
		 * Decodes a quaternion encoded using writeNorm(Quaternion, uint32_t). Read is performed at the current cursor
		 * location and advances the cursor. Same number of bits need to be used as when the float was encoded.
		 */
		void ReadNorm(Quaternion& outValue, uint32_t bits = 16);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling writeNorm().
		 */
		template <class T>
		void WriteNormDelta(const T& value, const T& lastValue, uint32_t bits = 16);

		/** Reads the data written by writeNormDelta() from the current cursor location and advances the cursor. */
		template <class T>
		void ReadNormDelta(T& outValue, const T& lastValue, uint32_t bits = 16);

		/**
		 * Encodes an integer in a specific range, using the range the reduce the number of bits required, and writes it
		 * to the stream. Write is performed at the current cursor location and advances the cursor. Returns the number
		 * of bits written.
		 */
		template <class T>
		uint32_t WriteRange(const T& value, const T& min, const T& max);

		/**
		 * Decodes an integer encoded using writeRange(const T&, const T&, const T&). Read is performed at the current
		 * cursor location and advances the cursor. Same needs to be used as when the value was encoded. Returns the
		 * number of bits read.
		 */
		template <class T>
		uint32_t ReadRange(T& outValue, const T& min, const T& max);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling
		 * writeRange(const T&, const T&, const T&). Returns the number of bits written.
		 */
		template <class T>
		uint32_t WriteRangeDelta(const T& value, const T& lastValue, const T& min, const T& max);

		/**
		 * Reads the data written by writeRangeDelta(const T&, const T&, const T&, const T&) from the current cursor
		 * location and advances the cursor. Returns the number of bits read.
		 */
		template <class T>
		uint32_t ReadRangeDelta(T& outValue, const T& lastValue, const T& min, const T& max);

		/**
		 * Encodes a float in a specific range into a fixed point representation using a specific number of bits, and
		 * writes it to the stream. Write is performed at the current cursor location and advances the cursor.
		 */
		void WriteRange(float value, float min, float max, uint32_t bits = 16);

		/**
		 * Decodes a float encoded using writeRange(float, float, float, uint32_t). Read is performed at the current cursor
		 * location and advances the cursor. Same number of bits, and the same range needs to be used as when the float was
		 * encoded.
		 */
		void ReadRange(float& outValue, float min, float max, uint32_t bits = 16);

		/**
		 * Checks if the provided value differs from the last provided value, and if they are equivalent writes just a
		 * single bit signifying no change. Otherwise the value is encoded as if calling
		 * writeRange(float, float, float, uint32_t).
		 */
		void WriteRangeDelta(float value, float lastValue, float min, float max, uint32_t bits = 16);

		/**
		 * Reads the data written by writeRangeDelta(float, float, float, float, uint32_t) from the current cursor
		 * location and advances the cursor.
		 */
		void ReadRangeDelta(float& outValue, float lastValue, float min, float max, uint32_t bits = 16);

		/**
		 * Skip a defined number of bits, moving the read/write cursor by this amount. This can also be a negative value,
		 * in which case the file pointer rewinds a defined number of bits. Note the cursor can never skip past the
		 * capacity of the buffer, and will be clamped.
		 */
		void Skip(int64_t count);

		/** Same as skip() except is uses number of bytes instead of number of bits as the parameter. */
		void SkipBytes(int32_t count) { return Skip((uint64_t)count * 8); }

		/**
		 * Repositions the read/write cursor to the specified bit. Note the cursor can never skip past the capacity
		 * of the buffer, and will be clamped.
		 */
		void Seek(uint64_t pos);

		/**
		 * Aligns the read/write cursor to a byte boundary. @p count determines the alignment in bytes. Note the
		 * requested alignment might not be achieved if count > 1 and it would move the cursor past the capacity of the
		 * buffer, as the cursor will be clamped to buffer end regardless of alignment.
		 *
		 * Returns number of bits skipped due to alignment.
		 */
		u64 Align(uint32_t count = 1);

		/** Expands the capacity to the specified number of bytes, unless already equal or greater. */
		void Reserve(uint32_t count);

		/**
		 * Expands the capacity and size to the specified number of bytes. Capacity will not be reduced if already
		 * equal or larger.
		 */
		void Resize(uint32_t count);

		/** Returns the current read/write cursor position, in bits. */
		uint64_t Tell() const { return mCursor; }

		/** Returns true if the stream has reached the end. */
		bool Eof() const { return mCursor >= mNumBits; }

		/** Returns the total number of bits available in the stream. */
		uint64_t Size() const { return mNumBits; }

		/** Returns the total number of bits the stream can store without needing to allocate more memory. */
		uint64_t Capacity() const { return mMaxBits; }

		/** Returns the internal data buffer. */
		QuantType* Data() const { return mData; }

		/** Returns the byte the read/write cursor is currently positioned on. */
		QuantType* Cursor() const;

	private:
		/** Checks if the internal memory buffer needs to grow in order to accomodate @p numBits bits. */
		void ReallocIfNeeded(uint64_t numBits);

		/** Reallocates the internal buffer making enough room for @p numBits (rounded to a multiple of BYTES_PER_QUANT. */
		void Realloc(uint64_t numBits);

		QuantType* mData = nullptr;
		uint64_t mMaxBits = 0;
		uint64_t mNumBits = 0;
		bool mOwnsMemory = true;

		uint64_t mCursor = 0;
	};

	/** @} */

	inline Bitstream::Bitstream(uint32_t capacity)
	{
		Realloc((uint64_t)(capacity)*8);
	}

	inline Bitstream::Bitstream(QuantType* data, uint32_t count)
		: mData(data), mMaxBits((uint64_t)count * 8), mNumBits((uint64_t)count * 8), mOwnsMemory(false) {}

	inline Bitstream::~Bitstream()
	{
		if(mData && mOwnsMemory)
			B3DFree(mData);
	}

	inline Bitstream::Bitstream(const Bitstream& other)
	{
		*this = other;
	}

	inline Bitstream::Bitstream(Bitstream&& other)
	{
		*this = std::move(other);
	}

	inline Bitstream& Bitstream::operator=(const Bitstream& other)
	{
		if(this == &other)
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
			if(mData && mOwnsMemory)
				B3DFree(mData);

			mData = nullptr;
			mMaxBits = 0;

			this->mOwnsMemory = true;
			Realloc(other.mMaxBits);

			if(mMaxBits > 0)
			{
				const uint32_t numBytes = (uint32_t)Math::DivideAndRoundUp(mMaxBits, (uint64_t)kBitsPerQuant) * kBytesPerQuant;
				memcpy(mData, other.mData, numBytes);
			}
		}

		return *this;
	}

	inline Bitstream& Bitstream::operator=(Bitstream&& other)
	{
		if(this == &other)
			return *this;

		if(mData && mOwnsMemory)
			B3DFree(mData);

		this->mCursor = std::exchange(other.mCursor, 0);
		this->mNumBits = std::exchange(other.mNumBits, 0);
		this->mMaxBits = std::exchange(other.mMaxBits, 0);
		this->mData = std::exchange(other.mData, nullptr);
		this->mOwnsMemory = std::exchange(other.mOwnsMemory, false);

		return *this;
	}

	inline uint64_t Bitstream::WriteBits(const QuantType* data, uint64_t count)
	{
		if(count == 0)
			return 0;

		uint64_t newCursor = mCursor + count;
		ReallocIfNeeded(newCursor);

		uint64_t remaining = count;
		uint64_t destBitsMod = (mCursor & (kBitsPerQuant - 1));
		uint64_t destQuant = mCursor >> kBitsPerQuantLoG2;
		uint64_t destMask = (1 << destBitsMod) - 1;

		// If destination is aligned, memcpy everything except the last quant (unless it is also aligned)
		if(destBitsMod == 0)
		{
			uint64_t numQuants = remaining >> kBitsPerQuantLoG2;
			memcpy(&mData[destQuant], data, numQuants * kBytesPerQuant);

			data += numQuants;
			remaining -= numQuants * kBitsPerQuant;
			destQuant += numQuants;
		}

		// Write remaining bits (or all bits if destination wasn't aligned)
		while(remaining > 0)
		{
			QuantType quant = *data;
			data++;

			mData[destQuant] = (quant << destBitsMod) | (mData[destQuant] & destMask);

			uint32_t writtenBits = (uint32_t)(kBitsPerQuant - destBitsMod);
			if(remaining > writtenBits)
				mData[destQuant + 1] = (quant >> writtenBits) | (mData[destQuant + 1] & ~destMask);

			destQuant++;
			remaining -= std::min((uint64_t)kBitsPerQuant, remaining);
		}

		mCursor = newCursor;
		mNumBits = std::max(mNumBits, newCursor);

		return count;
	}

	inline uint64_t Bitstream::ReadBits(QuantType* outData, uint64_t count)
	{
		if(count == 0)
			return 0;

		B3D_ASSERT((mCursor + count) <= mNumBits);

		uint64_t remaining = count;
		uint64_t newCursor = mCursor + count;
		uint64_t srcBitsMod = mCursor & (kBitsPerQuant - 1);
		uint64_t srcQuant = mCursor >> kBitsPerQuantLoG2;

		// If source is aligned, memcpy everything except the last quant (unless it is also aligned)
		if(srcBitsMod == 0)
		{
			uint64_t numQuants = remaining >> kBitsPerQuantLoG2;
			memcpy(outData, &mData[srcQuant], numQuants * kBytesPerQuant);

			outData += numQuants;
			remaining -= numQuants * kBitsPerQuant;
			srcQuant += numQuants;
		}

		// Read remaining bits (or all bits if source wasn't aligned)
		while(remaining > 0)
		{
			QuantType& quant = *outData;
			outData++;

			quant = 0;
			quant |= mData[srcQuant] >> srcBitsMod;

			uint32_t readBits = (uint32_t)(kBitsPerQuant - srcBitsMod);
			if(remaining > readBits)
				quant |= mData[srcQuant + 1] << readBits;

			srcQuant++;
			remaining -= std::min((uint64_t)kBitsPerQuant, remaining);
		}

		mCursor = newCursor;
		return count;
	}

	template <class T>
	uint64_t Bitstream::Write(const T& value)
	{
		return WriteBits((QuantType*)&value, sizeof(value) * 8);
	}

	template <class T>
	uint64_t Bitstream::Read(T& outValue)
	{
		QuantType* temp = (QuantType*)&outValue;
		return ReadBits(temp, sizeof(outValue) * 8);
	}

	inline uint64_t Bitstream::Write(const bool& value)
	{
		ReallocIfNeeded(mCursor + 1);

		uint64_t destBitsMod = mCursor & (kBitsPerQuant - 1);
		uint64_t destQuant = mCursor >> kBitsPerQuantLoG2;

		if(value)
			mData[destQuant] |= 1U << destBitsMod;
		else
			mData[destQuant] &= ~(1U << destBitsMod);

		mCursor++;
		mNumBits = std::max(mNumBits, mCursor);

		return 1;
	}

	inline uint64_t Bitstream::Read(bool& outValue)
	{
		B3D_ASSERT((mCursor + 1) <= mNumBits);

		uint64_t srcBitsMod = mCursor & (kBitsPerQuant - 1);
		uint64_t srcQuant = mCursor >> kBitsPerQuantLoG2;

		outValue = (mData[srcQuant] >> srcBitsMod) & 0x1;
		mCursor++;

		return 1;
	}

	inline uint64_t Bitstream::Write(const String& value)
	{
		uint32_t length = (uint32_t)value.size();
		uint64_t written = WriteVarInt(length);
		written += WriteBits((QuantType*)value.data(), (uint64_t)length * 8);

		return written;
	}

	inline uint64_t Bitstream::Read(String& outValue)
	{
		uint32_t length;
		uint64_t read = ReadVarInt(length);

		outValue.resize(length);

		QuantType* temp = (QuantType*)outValue.data();
		read += ReadBits(temp, (uint64_t)length * 8);

		return read;
	}

	template <class T>
	uint32_t Bitstream::WriteBytes(const T& value)
	{
		uint64_t numBits = WriteBits((QuantType*)&value, sizeof(value) * 8);
		B3D_ASSERT((numBits % 8) == 0);

		return (uint32_t)(numBits / 8);
	}

	template <class T>
	uint32_t Bitstream::ReadBytes(T& outValue)
	{
		QuantType* temp = (QuantType*)&outValue;
		uint64_t numBits = ReadBits(temp, sizeof(outValue) * 8);
		B3D_ASSERT((numBits % 8) == 0);

		return (uint32_t)(numBits / 8);
	}

	inline uint32_t Bitstream::WriteBytes(const QuantType* data, uint32_t count)
	{
		return (uint32_t)(WriteBits(data, (uint64_t)count * 8) / 8);
	}

	inline uint32_t Bitstream::ReadBytes(QuantType* outData, uint32_t count)
	{
		return (uint32_t)(ReadBits(outData, (uint64_t)count * 8) / 8);
	}

	template <class T>
	uint64_t Bitstream::WriteDelta(const T& value, const T& lastValue)
	{
		if(value == lastValue)
			return Write(true);
		else
			return Write(false) + Write(value);
	}

	template <class T>
	uint64_t Bitstream::ReadDelta(T& outValue, const T& lastValue)
	{
		bool clean;
		Read(clean);

		if(clean)
		{
			outValue = lastValue;
			return 1;
		}
		else
			return Read(outValue) + 1;
	}

	inline uint32_t Bitstream::WriteDelta(bool value, bool lastValue)
	{
		return (uint32_t)Write(value);
	}

	inline uint32_t Bitstream::ReadDelta(bool& outValue, bool lastValue)
	{
		return (uint32_t)Read(outValue);
	}

	inline uint32_t Bitstream::WriteVarInt(uint32_t value)
	{
		uint8_t output[5];
		uint32_t count = Bitwise::EncodeVarInt(value, output);

		return (uint32_t)WriteBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::WriteVarInt(int32_t value)
	{
		uint8_t output[5];
		uint32_t count = Bitwise::EncodeVarInt(value, output);

		return (uint32_t)WriteBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::WriteVarInt(uint64_t value)
	{
		uint8_t output[10];
		uint32_t count = Bitwise::EncodeVarInt(value, output);

		return (uint32_t)WriteBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::WriteVarInt(int64_t value)
	{
		uint8_t output[10];
		uint32_t count = Bitwise::EncodeVarInt(value, output);

		return (uint32_t)WriteBits(output, (uint64_t)count * 8);
	}

	inline uint32_t Bitstream::ReadVarInt(uint32_t& outValue)
	{
		uint32_t read = 0;
		uint8_t output[5];
		for(uint32_t outputIndex = 0; outputIndex < 5; outputIndex++)
		{
			read += (uint32_t)ReadBits(&output[outputIndex], 8);
			if((output[outputIndex] & 0x80) == 0)
				break;
		}

		Bitwise::DecodeVarInt(outValue, output, 5);
		return read;
	}

	inline uint32_t Bitstream::ReadVarInt(int32_t& outValue)
	{
		uint32_t read = 0;
		uint8_t output[5];
		for(uint32_t outputIndex = 0; outputIndex < 5; outputIndex++)
		{
			read += (uint32_t)ReadBits(&output[outputIndex], 8);
			if((output[outputIndex] & 0x80) == 0)
				break;
		}

		Bitwise::DecodeVarInt(outValue, output, 5);
		return read;
	}

	inline uint32_t Bitstream::ReadVarInt(uint64_t& outValue)
	{
		uint32_t read = 0;
		uint8_t output[10];
		for(uint32_t outputIndex = 0; outputIndex < 10; outputIndex++)
		{
			read += (uint32_t)ReadBits(&output[outputIndex], 8);
			if((output[outputIndex] & 0x80) == 0)
				break;
		}

		Bitwise::DecodeVarInt(outValue, output, 10);
		return read;
	}

	inline uint32_t Bitstream::ReadVarInt(int64_t& outValue)
	{
		uint32_t read = 0;
		uint8_t output[10];
		for(uint32_t outputIndex = 0; outputIndex < 10; outputIndex++)
		{
			read += (uint32_t)ReadBits(&output[outputIndex], 8);
			if((output[outputIndex] & 0x80) == 0)
				break;
		}

		Bitwise::DecodeVarInt(outValue, output, 10);
		return read;
	}

	template <class T>
	uint32_t Bitstream::WriteVarIntDelta(const T& value, const T& lastValue)
	{
		if(value == lastValue)
		{
			Write(true);
			return 1;
		}
		else
		{
			Write(false);
			return WriteVarInt(value) + 1;
		}
	}

	template <class T>
	uint32_t Bitstream::ReadVarIntDelta(T& outValue, const T& lastValue)
	{
		bool clean;
		Read(clean);

		if(clean)
		{
			outValue = lastValue;
			return 1;
		}
		else
			return ReadVarInt(outValue) + 1;
	}

	inline void Bitstream::WriteNorm(float value, uint32_t bits)
	{
		uint32_t encodedVal = Bitwise::UnormToUint(value, bits);
		WriteBits((QuantType*)&encodedVal, bits);
	}

	inline void Bitstream::ReadNorm(float& outValue, uint32_t bits)
	{
		uint32_t encodedVal = 0;
		ReadBits((QuantType*)&encodedVal, bits);
		outValue = Bitwise::UintToUnorm(encodedVal, bits);
	}

	inline void Bitstream::WriteNorm(const Vector3& value, uint32_t bits)
	{
		WriteRange(value.X, -1.0f, 1.0f, bits);
		WriteRange(value.Y, -1.0f, 1.0f, bits);
		WriteRange(value.Z, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::ReadNorm(Vector3& outValue, uint32_t bits)
	{
		ReadRange(outValue.X, -1.0f, 1.0f, bits);
		ReadRange(outValue.Y, -1.0f, 1.0f, bits);
		ReadRange(outValue.Z, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::WriteNorm(const Quaternion& value, uint32_t bits)
	{
		WriteRange(value.X, -1.0f, 1.0f, bits);
		WriteRange(value.Y, -1.0f, 1.0f, bits);
		WriteRange(value.Z, -1.0f, 1.0f, bits);
		WriteRange(value.W, -1.0f, 1.0f, bits);
	}

	inline void Bitstream::ReadNorm(Quaternion& outValue, uint32_t bits)
	{
		ReadRange(outValue.X, -1.0f, 1.0f, bits);
		ReadRange(outValue.Y, -1.0f, 1.0f, bits);
		ReadRange(outValue.Z, -1.0f, 1.0f, bits);
		ReadRange(outValue.W, -1.0f, 1.0f, bits);
	}

	template <class T>
	void Bitstream::WriteNormDelta(const T& value, const T& lastValue, uint32_t bits)
	{
		if(value == lastValue)
			Write(true);
		else
		{
			Write(false);
			WriteNorm(value, bits);
		}
	}

	template <class T>
	void Bitstream::ReadNormDelta(T& outValue, const T& lastValue, uint32_t bits)
	{
		bool clean;
		Read(clean);

		if(clean)
			outValue = lastValue;
		else
			ReadNorm(outValue, bits);
	}

	template <class T>
	uint32_t Bitstream::WriteRange(const T& value, const T& min, const T& max)
	{
		T range = max - min;
		uint32_t bits = Bitwise::MostSignificantBit(range) + 1;

		T rangeVal = value - min;
		WriteBits((QuantType*)&rangeVal, bits);

		return bits;
	}

	template <class T>
	uint32_t Bitstream::ReadRange(T& outValue, const T& min, const T& max)
	{
		T range = max - min;
		uint32_t bits = Bitwise::MostSignificantBit(range) + 1;

		outValue = 0;
		ReadBits((QuantType*)&outValue, bits);
		outValue += min;

		return bits;
	}

	template <class T>
	uint32_t Bitstream::WriteRangeDelta(const T& value, const T& lastValue, const T& min, const T& max)
	{
		if(value == lastValue)
		{
			Write(true);
			return 1;
		}
		else
		{
			Write(false);
			return WriteRange(value, min, max) + 1;
		}
	}

	template <class T>
	uint32_t Bitstream::ReadRangeDelta(T& outValue, const T& lastValue, const T& min, const T& max)
	{
		bool clean;
		Read(clean);

		if(clean)
		{
			outValue = lastValue;
			return 1;
		}
		else
			return ReadRange(outValue, min, max) + 1;
	}

	inline void Bitstream::WriteRange(float value, float min, float max, uint32_t bits)
	{
		float pct = Math::Clamp01((value - min) / (max - min));
		WriteNorm(pct, bits);
	}

	inline void Bitstream::ReadRange(float& outValue, float min, float max, uint32_t bits)
	{
		float pct;
		ReadNorm(pct, bits);

		outValue = min + (max - min) * pct;
	}

	inline void Bitstream::WriteRangeDelta(float value, float lastValue, float min, float max, uint32_t bits)
	{
		if(value == lastValue)
			Write(true);
		else
		{
			Write(false);
			WriteRange(value, min, max, bits);
		}
	}

	inline void Bitstream::ReadRangeDelta(float& outValue, float lastValue, float min, float max, uint32_t bits)
	{
		bool clean;
		Read(clean);

		if(clean)
			outValue = lastValue;
		else
			ReadRange(outValue, min, max, bits);
	}

	inline void Bitstream::Skip(int64_t count)
	{
		mCursor = (uint64_t)Math::Clamp((int64_t)mCursor + count, (int64_t)0, (int64_t)mMaxBits);
	}

	inline void Bitstream::Seek(uint64_t pos)
	{
		mCursor = std::min(pos, mMaxBits);
	}

	inline u64 Bitstream::Align(uint32_t count)
	{
		if(count == 0)
			return 0;

		const u64 bitsToAlign = count * 8;
		const u64 bitsToSkip = bitsToAlign - (((mCursor - 1) & (bitsToAlign - 1)) + 1);
		Skip((int64_t)bitsToSkip);

		return bitsToSkip;
	}

	inline void Bitstream::Reserve(uint32_t count)
	{
		if(Capacity() < ((uint64_t)count * 8))
			Realloc((uint64_t)count * 8);
	}

	inline void Bitstream::Resize(uint32_t count)
	{
		Reserve(count);
		mNumBits = (uint64_t)count * 8;
	}

	inline Bitstream::QuantType* Bitstream::Cursor() const
	{
		return &mData[mCursor >> kBitsPerQuantLoG2];
	}

	inline void Bitstream::ReallocIfNeeded(uint64_t numBits)
	{
		if(numBits > mMaxBits)
		{
			if(mOwnsMemory)
			{
				// Grow
				const uint64_t newMaxBits = numBits + 4 * kBitsPerQuant + numBits / 2;
				Realloc(newMaxBits);
			}
			else
			{
				// Caller accessing bits outside of external memory range
				B3D_ASSERT(false);
			}
		}
	}

	inline void Bitstream::Realloc(uint64_t numBits)
	{
		numBits = Math::DivideAndRoundUp(numBits, (uint64_t)kBitsPerQuant) * kBitsPerQuant;

		if(numBits != mMaxBits)
		{
			B3D_ASSERT(numBits > mMaxBits);

			const uint32_t numQuants = (uint32_t)Math::DivideAndRoundUp(numBits, (uint64_t)kBitsPerQuant);

			// Note: Eventually add support for custom allocators
			auto buffer = B3DAllocateMultiple<uint8_t>(numQuants);
			if(mData)
			{
				const uint32_t numBytes = (uint32_t)Math::DivideAndRoundUp(mMaxBits, (uint64_t)kBitsPerQuant) * kBytesPerQuant;
				memcpy(buffer, mData, numBytes);
				B3DFree(mData);
			}

			mData = buffer;
			mMaxBits = numBits;
		}
	}
} // namespace b3d
