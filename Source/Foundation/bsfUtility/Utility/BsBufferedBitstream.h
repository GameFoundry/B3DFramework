
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsBitstream.h"
#include "FileSystem/BsDataStream.h"
#include "Error/BsException.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Wraps a Bitstream and a DataStream. Buffers the data from the data stream into the bit stream as required
	 * and then reads from the bitstream.
	 */
	class BufferedBitstreamReader
	{
	public:
		/**
		 * Constructs a new instance of the object.
		 * 
		 * @param[in]	bitstream		Bitstream into which to load the buffered data.
		 * @param[in]	dataStream		Data stream from which to read the data.
		 * @param[in]	preloadSize		Determines the size of the chunk to preload, when we reach the end of
		 *								buffered data. In bytes.
		 * @param[in]	maxBufferSize	Maximum size of the buffer before it is cleared.
		 */
		BufferedBitstreamReader(Bitstream* bitstream, const SPtr<DataStream>& dataStream, uint32_t preloadSize,
			uint32_t maxBufferSize);

		// Note: Perhaps allow reads with no chunk preload (i.e. just the requested count)

		/** @copydoc Bitstream::readBits(Bitstream::QuantType* data, uint32_t count) */
		uint64_t readBits(Bitstream::QuantType* data, uint64_t count);
		
		/** @copydoc Bitstream::readBytes(T&) */
		template<class T>
		uint32_t readBytes(T& value);

		/** @copydoc Bitstream::readBytes(void*, uint32_t) */
		uint32_t readBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::readVarInt(uint32_t&) */
		uint32_t readVarInt(uint32_t& value);
		
		/** @copydoc Bitstream::skip */
		void skip(int64_t count);

		/** @copydoc Bitstream::skipBytes */
		void skipBytes(int32_t count) { return skip((int64_t)count * 8); }

		/** @copydoc Bitstream::seek */
		void seek(uint64_t pos);
		
		/** @copydoc Bitstream::tell */
		uint64_t tell() const { return mCursor; }

		/** @copydoc Bitstream::align() */
		void align(uint32_t count = 1);

		/** Preloads the specified number of bytes into the bitstream from the data stream. */
		void preload(uint32_t count);
		
		/**
		 * Clears buffered data behind the current cursor location.
		 *
		 * @param[in]	force	If false the buffer will only be cleared if its current size is	over the maximum
		 *						buffer size limit. Otherwise it will always be cleared.
		 */
		void clearBuffered(bool force);

		/** Returns the underlying data stream. */
		const SPtr<DataStream>& getDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& getBitstream() const { return *mBitstream;  }
	private:
		uint64_t mCursor = 0;
		uint64_t mBufferedRangeStart = 0;
		uint64_t mBufferedRangeEnd = 0;
		Bitstream* mBitstream;
		SPtr<DataStream> mDataStream;
		Bitstream mMemBitstream;
		uint64_t mLength;
		uint64_t mPreloadSize;
		uint64_t mMaxBufferSize;
		bool mIsMapped = false;
	};

	/**
	 * Wraps a Bitstream and a DataStream. Buffers the written data in the bitstream and then on request flushes the
	 * data into the data stream.
	 */
	class BufferedBitstreamWriter
	{
	public:
		/**
		 * Constructs a new instance of the object.
		 *
		 * @param[in]	bitstream		Bitstream into which the buffered data will be written. 
		 * @param[in]	dataStream		Data stream from which to read the data.
		 * @param[in]	bufferSize		Initial size of the write buffer, in bytes.
		 * @param[in]	flushAfter		Number of bytes after which the write buffer will be flushed to the data stream.
		 */
		BufferedBitstreamWriter(Bitstream* bitstream, const SPtr<DataStream>& dataStream, uint32_t bufferSize, uint32_t flushAfter);

		/** @copydoc Bitstream::writeBits(const Bitstream::QuantType*, uint32_t) */
		uint64_t writeBits(const Bitstream::QuantType* data, uint64_t count);
		
		/** @copydoc Bitstream::writeBytes(T&) */
		template<class T>
		uint32_t writeBytes(T& value);

		/** @copydoc Bitstream::writeBytes(void*, uint32_t) */
		uint32_t writeBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::writeVarInt */
		uint32_t writeVarInt(uint32_t value);
		
		/** @copydoc Bitstream::align() */
		void align(uint32_t count = 1);
		
		/** Flushes the write buffer to the output stream if a certain buffer length is reached. */
		void flush(bool force);

		/** Returns the underlying data stream. */
		const SPtr<DataStream>& getDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& getBitstream() const { return *mBitstream;  }
	private:
		Bitstream* mBitstream;
		SPtr<DataStream> mDataStream;
		uint64_t mFlushAfter;
	};

	/** @} */

	inline BufferedBitstreamReader::BufferedBitstreamReader(Bitstream* bitstream, const SPtr<DataStream>& dataStream,
		uint32_t preloadSize, uint32_t maxBufferSize)
		: mCursor((uint64_t)dataStream->tell() * 8), mBufferedRangeStart(mCursor), mBufferedRangeEnd(mCursor), mBitstream(bitstream)
		, mDataStream(dataStream), mLength((uint32_t)dataStream->size()), mPreloadSize(preloadSize), mMaxBufferSize(maxBufferSize)
		, mIsMapped(!dataStream->isFile())
	{
		// Special case for memory streams, we can just map the memory directly
		if(mIsMapped)
		{
			auto memStream = std::static_pointer_cast<MemoryDataStream>(dataStream);
			mMemBitstream = Bitstream(memStream->data(), (uint32_t)memStream->size());
			mBitstream = &mMemBitstream;

			mBufferedRangeStart = 0;
			mBufferedRangeEnd = (uint64_t)mLength * 8;
		}
	}

	inline uint64_t BufferedBitstreamReader::readBits(Bitstream::QuantType* data, uint64_t count)
	{
		preload((uint32_t)Math::divideAndRoundUp(count, (uint64_t)8));
		mCursor += count;
		return mBitstream->readBits(data, count);
	}

	template <class T>
	uint32_t BufferedBitstreamReader::readBytes(T& value)
	{
		preload(sizeof(T));
		mCursor += sizeof(T) * 8;
		return mBitstream->readBytes(value);
	}

	inline uint32_t BufferedBitstreamReader::readBytes(Bitstream::QuantType* data, uint32_t count)
	{
		preload(count);
		mCursor += (uint64_t)count * 8;
		return mBitstream->readBytes(data, count);
	}

	inline uint32_t BufferedBitstreamReader::readVarInt(uint32_t& value)
	{
		preload(sizeof(value));
		uint32_t readBits = mBitstream->readVarInt(value);
		mCursor += readBits;

		return readBits;
	}

	inline void BufferedBitstreamReader::skip(int64_t count)
	{
		seek((uint64_t)std::max((int64_t)0, (int64_t)mCursor + count));
	}

	inline void BufferedBitstreamReader::align(uint32_t count)
	{
		if (count == 0)
			return;

		uint32_t bits = count * 8;
		skip(bits - (((mCursor - 1) & (bits - 1)) + 1));
	}

	inline void BufferedBitstreamReader::seek(uint64_t pos)
	{
		if (!mIsMapped && (pos < mBufferedRangeStart || pos >= mBufferedRangeEnd))
		{
			mBufferedRangeStart = Math::divideAndRoundUp(pos, (uint64_t)8) * 8;
			mBufferedRangeEnd = mBufferedRangeStart;
		}

		mCursor = pos;
		mBitstream->seek(pos - mBufferedRangeStart);
	}

	inline void BufferedBitstreamReader::preload(uint32_t count)
	{
		assert(mCursor >= mBufferedRangeStart);
		
		if ((mCursor + (uint64_t)count * 8) <= mBufferedRangeEnd)
			return;
		
		// Pre-load the next chunk
		assert((mBufferedRangeEnd % 8) == 0);
		uint64_t remainingBytes = mLength - mBufferedRangeEnd / 8;
		
		uint64_t numBytesToPreload = std::min(std::max(mPreloadSize, (uint64_t)count), remainingBytes);

		// Make sure our buffer has enough room for the new data
		uint64_t bufferedLength = mBufferedRangeEnd - mBufferedRangeStart;
		uint64_t newBufferedLength = bufferedLength + numBytesToPreload * 8;
		if (mBitstream->capacity() < newBufferedLength)
			mBitstream->resize((uint32_t)Math::divideAndRoundUp(newBufferedLength, (uint64_t)Bitstream::BITS_PER_QUANT));

		// Read the data from data stream into the bitstream
		uint64_t orgPos = mBitstream->tell();
		mBitstream->seek(bufferedLength);
		
		mDataStream->seek((size_t)(mBufferedRangeEnd / 8));
		if (mDataStream->read(mBitstream->cursor(), numBytesToPreload) != numBytesToPreload)
			BS_EXCEPT(InternalErrorException, "Error reading data.");
		
		mBitstream->seek(orgPos);
		mBufferedRangeEnd += numBytesToPreload * 8;
	}

	inline void BufferedBitstreamReader::clearBuffered(bool force)
	{
		// If memory stream, there is no buffer and we map the entire stream
		if (mIsMapped)
			return;
		
		uint64_t bufferedLengthBits = mBufferedRangeEnd - mBufferedRangeStart;
		uint64_t bufferedLengthBytes = bufferedLengthBits / 8;

		if (!force && bufferedLengthBytes < mMaxBufferSize)
			return;

		uint64_t offsetBits = mCursor - mBufferedRangeStart;
		uint64_t bytesToClear = offsetBits >> Bitstream::BITS_PER_QUANT_LOG2;

		uint64_t remainingBits = offsetBits - bytesToClear * Bitstream::BITS_PER_QUANT;
		uint32_t remainingBytes = (uint32_t)(bufferedLengthBytes - bytesToClear);

		mBufferedRangeStart += bytesToClear * 8;

		Bitstream::QuantType* remainingData = nullptr;
		if(remainingBytes > 0)
		{
			remainingData = bs_stack_alloc<Bitstream::QuantType>(remainingBytes);
			mBitstream->readBytes(remainingData, remainingBytes);
		}

		mBitstream->seek(0);
		mBitstream->writeBytes(remainingData, remainingBytes);
		mBitstream->seek(remainingBits);

		if (remainingData)
			bs_stack_free(remainingData);
	}

	inline BufferedBitstreamWriter::BufferedBitstreamWriter(Bitstream* bitstream, const SPtr<DataStream>& dataStream,
		uint32_t bufferSize, uint32_t flushAfter)
		: mBitstream(bitstream), mDataStream(dataStream), mFlushAfter(flushAfter)
	{
		if (mBitstream->capacity() < (uint64_t)bufferSize * 8)
			mBitstream->reserve(bufferSize);
	}

	inline uint64_t BufferedBitstreamWriter::writeBits(const Bitstream::QuantType* data, uint64_t count)
	{
		return mBitstream->writeBits(data, count);
	}

	template <class T>
	uint32_t BufferedBitstreamWriter::writeBytes(T& value)
	{
		return mBitstream->writeBytes(value);
	}

	inline uint32_t BufferedBitstreamWriter::writeBytes(Bitstream::QuantType* data, uint32_t count)
	{
		return mBitstream->writeBytes(data, count);
	}

	inline uint32_t BufferedBitstreamWriter::writeVarInt(uint32_t value)
	{
		return mBitstream->writeVarInt(value);
	}

	inline void BufferedBitstreamWriter::align(uint32_t count)
	{
		mBitstream->align(count);
	}

	inline void BufferedBitstreamWriter::flush(bool force)
	{
		uint64_t bitsInBuffer = mBitstream->tell();
		if ((bitsInBuffer < (mFlushAfter * 8)) && !force)
			return;

		// Flush all the complete bytes, and leave any sub-byte bits in the write stream
		uint64_t bytesToFlush = bitsInBuffer >> Bitstream::BITS_PER_QUANT_LOG2;
		uint64_t bitsToFlush = bytesToFlush * Bitstream::BITS_PER_QUANT;
		uint64_t leftoverBits = bitsInBuffer - bitsToFlush;

		assert(leftoverBits < Bitstream::BITS_PER_QUANT);

		Bitstream::QuantType quant = 0;
		if (force && leftoverBits > 0)
		{
			// Pad the last quant
			uint32_t bitsToPad = Bitstream::BITS_PER_QUANT - leftoverBits;
			mBitstream->writeBits(&quant, bitsToPad);
			bitsInBuffer += bitsToPad;
			
			assert((bitsInBuffer % Bitstream::BITS_PER_QUANT) == 0);

			bytesToFlush = bitsInBuffer >> Bitstream::BITS_PER_QUANT_LOG2;
			bitsToFlush = bytesToFlush * Bitstream::BITS_PER_QUANT;
			leftoverBits = 0;
		}

		mBitstream->seek(bitsToFlush);
		mBitstream->readBits(&quant, leftoverBits);

		mBitstream->seek(0);
		mDataStream->write(mBitstream->cursor(), (size_t)bytesToFlush);

		mBitstream->writeBits(&quant, leftoverBits);
	}
	
}
