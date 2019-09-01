
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

		/** @copydoc Bitstream::readBytes(T&) */
		template<class T>
		uint32_t readBytes(T& value);

		/** @copydoc Bitstream::readBytes(void*, uint32_t) */
		uint32_t readBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::skip */
		void skip(int32_t count);

		/** @copydoc Bitstream::skipBytes */
		void skipBytes(int32_t count) { return skip(count * 8); }

		/** @copydoc Bitstream::seek */
		void seek(uint32_t pos);
		
		/** @copydoc Bitstream::tell */
		uint32_t tell() const { return mCursor; }

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
		uint32_t mCursor = 0;
		uint32_t mBufferedRangeStart = 0;
		uint32_t mBufferedRangeEnd = 0;
		Bitstream* mBitstream;
		SPtr<DataStream> mDataStream;
		Bitstream mMemBitstream;
		uint32_t mLength;
		uint32_t mPreloadSize;
		uint32_t mMaxBufferSize;
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

		/** @copydoc Bitstream::writeBytes(T&) */
		template<class T>
		uint32_t writeBytes(T& value);

		/** @copydoc Bitstream::writeBytes(void*, uint32_t) */
		uint32_t writeBytes(Bitstream::QuantType* data, uint32_t count);

		/** Flushes the write buffer to the output stream if a certain buffer length is reached. */
		void flush(bool force);

		/** Returns the underlying data stream. */
		const SPtr<DataStream>& getDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& getBitstream() const { return *mBitstream;  }
	private:
		Bitstream* mBitstream;
		SPtr<DataStream> mDataStream;
		uint32_t mFlushAfter;
	};

	/** @} */

	inline BufferedBitstreamReader::BufferedBitstreamReader(Bitstream* bitstream, const SPtr<DataStream>& dataStream,
		uint32_t preloadSize, uint32_t maxBufferSize)
		: mCursor((uint32_t)dataStream->tell() * 8), mBufferedRangeStart(mCursor), mBufferedRangeEnd(mCursor), mBitstream(bitstream)
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
			mBufferedRangeEnd = mLength * 8;
		}
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
		mCursor += count * 8;
		return mBitstream->readBytes(data, count);
	}

	inline void BufferedBitstreamReader::skip(int32_t count)
	{
		seek((uint32_t)std::max(0, (int32_t)mCursor + count));
	}

	inline void BufferedBitstreamReader::seek(uint32_t pos)
	{
		if (!mIsMapped && (pos < mBufferedRangeStart || pos >= mBufferedRangeEnd))
		{
			mBufferedRangeStart = Math::divideAndRoundUp(pos, 8U) * 8U;
			mBufferedRangeEnd = mBufferedRangeStart;
		}

		mCursor = pos;
		mBitstream->seek(pos - mBufferedRangeStart);
	}

	inline void BufferedBitstreamReader::preload(uint32_t count)
	{
		assert(mCursor >= mBufferedRangeStart);
		
		if ((mCursor + count * 8) <= mBufferedRangeEnd)
			return;
		
		// Pre-load the next chunk
		assert((mBufferedRangeEnd % 8) == 0);
		uint32_t remainingBytes = mLength - mBufferedRangeEnd / 8;
		
		uint32_t numBytesToPreload = std::min(std::max(mPreloadSize, count), remainingBytes);

		// Make sure our buffer has enough room for the new data
		uint32_t bufferedLength = mBufferedRangeEnd - mBufferedRangeStart;
		uint32_t newBufferedLength = bufferedLength + numBytesToPreload * 8;
		if (mBitstream->capacity() < newBufferedLength)
			mBitstream->resize(newBufferedLength);

		// Read the data from data stream into the bitstream
		uint32_t orgPos = mBitstream->tell();
		mBitstream->seek(bufferedLength);
		
		mDataStream->seek(mBufferedRangeEnd / 8);
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
		
		uint32_t bufferedLengthBits = mBufferedRangeEnd - mBufferedRangeStart;
		uint32_t bufferedLengthBytes = bufferedLengthBits / 8;

		if (!force && bufferedLengthBytes < mMaxBufferSize)
			return;

		uint32_t offsetBits = mCursor - mBufferedRangeStart;
		uint32_t bytesToClear = offsetBits >> Bitstream::BITS_PER_QUANT_LOG2;

		uint32_t remainingBits = offsetBits - bytesToClear * Bitstream::BITS_PER_QUANT;
		uint32_t remainingBytes = bufferedLengthBytes - bytesToClear;

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
		if (mBitstream->capacity() < bufferSize * 8)
			mBitstream->reserve(bufferSize * 8);
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

	inline void BufferedBitstreamWriter::flush(bool force)
	{
		uint32_t bitsInBuffer = mBitstream->tell();
		if ((bitsInBuffer < (mFlushAfter * 8)) && !force)
			return;

		// Flush all the complete bytes, and leave any sub-byte bits in the write stream
		uint32_t bytesToFlush = bitsInBuffer >> Bitstream::BITS_PER_QUANT_LOG2;
		uint32_t bitsToFlush = bytesToFlush * Bitstream::BITS_PER_QUANT;
		uint32_t leftoverBits = bitsInBuffer - bitsToFlush;

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
		mDataStream->write(mBitstream->cursor(), bytesToFlush);

		mBitstream->writeBits(&quant, leftoverBits);
	}
	
}
