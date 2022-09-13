
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

		/** @copydoc Bitstream::readBits(Bitstream::QuantType* Data, uint32_t count) */
		uint64_t ReadBits(Bitstream::QuantType* data, uint64_t count);
		
		/** @copydoc Bitstream::readBytes(T&) */
		template<class T>
		uint32_t ReadBytes(T& value);

		/** @copydoc Bitstream::readBytes(void*, uint32_t) */
		uint32_t ReadBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::readVarInt(uint32_t&) */
		uint32_t ReadVarInt(uint32_t& value);
		
		/** @copydoc Bitstream::skip */
		void Skip(int64_t count);

		/** @copydoc Bitstream::skipBytes */
		void SkipBytes(int32_t count) { return skip((int64_t)count * 8); }

		/** @copydoc Bitstream::seek */
		void Seek(uint64_t pos);
		
		/** @copydoc Bitstream::tell */
		uint64_t Tell() const { return mCursor; }

		/** @copydoc Bitstream::align() */
		void Align(uint32_t count = 1);

		/** Preloads the specified number of bytes into the bitstream from the data stream. */
		void Preload(uint32_t count);
		
		/**
		 * Clears buffered data behind the current cursor location.
		 *
		 * @param[in]	force	If false the buffer will only be cleared if its current size is	over the maximum
		 *						buffer size limit. Otherwise it will always be cleared.
		 */
		void ClearBuffered(bool force);

		/** Returns the underlying data stream. */
		const SPtr<DataStream>& GetDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& GetBitstream() const { return *mBitstream;  }
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
	 * Data into the data stream.
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
		uint64_t WriteBits(const Bitstream::QuantType* data, uint64_t count);
		
		/** @copydoc Bitstream::writeBytes(T&) */
		template<class T>
		uint32_t WriteBytes(T& value);

		/** @copydoc Bitstream::writeBytes(void*, uint32_t) */
		uint32_t WriteBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::writeVarInt */
		uint32_t WriteVarInt(uint32_t value);
		
		/** @copydoc Bitstream::align() */
		void Align(uint32_t count = 1);
		
		/** Flushes the write buffer to the output stream if a certain buffer length is reached. */
		void Flush(bool force);

		/** Returns the underlying data stream. */
		const SPtr<DataStream>& GetDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& GetBitstream() const { return *mBitstream;  }
	private:
		Bitstream* mBitstream;
		SPtr<DataStream> mDataStream;
		uint64_t mFlushAfter;
	};

	/** @} */

	inline BufferedBitstreamReader::BufferedBitstreamReader(Bitstream* bitstream, const SPtr<DataStream>& dataStream,
		uint32_t preloadSize, uint32_t maxBufferSize)
		: MCursor((uint64_t)dataStream->Tell() * 8), mBufferedRangeStart(mCursor), mBufferedRangeEnd(mCursor), mBitstream(bitstream)
		, MDataStream(dataStream), mLength((uint32_t)dataStream->Size()), mPreloadSize(preloadSize), mMaxBufferSize(maxBufferSize)
		, MIsMapped(!dataStream->IsFile())
	{
		// Special case for memory streams, we can just map the memory directly
		if(mIsMapped)
		{
			auto memStream = std::static_pointer_cast<MemoryDataStream>(dataStream);
			mMemBitstream = Bitstream(memStream->Data(), (uint32_t)memStream->size());
			mBitstream = &mMemBitstream;

			mBufferedRangeStart = 0;
			mBufferedRangeEnd = (uint64_t)mLength * 8;
		}
	}

	inline uint64_t BufferedBitstreamReader::ReadBits(Bitstream::QuantType* data, uint64_t count)
	{
		preload((uint32_t)Math::DivideAndRoundUp(count, (uint64_t)8));
		mCursor += count;
		return mBitstream->ReadBits(data, count);
	}

	template <class T>
	uint32_t BufferedBitstreamReader::ReadBytes(T& value)
	{
		preload(sizeof(T));
		mCursor += sizeof(T) * 8;
		return mBitstream->ReadBytes(value);
	}

	inline uint32_t BufferedBitstreamReader::ReadBytes(Bitstream::QuantType* data, uint32_t count)
	{
		preload(count);
		mCursor += (uint64_t)count * 8;
		return mBitstream->ReadBytes(data, count);
	}

	inline uint32_t BufferedBitstreamReader::ReadVarInt(uint32_t& value)
	{
		preload(sizeof(value));
		uint32_t readBits = mBitstream->ReadVarInt(value);
		mCursor += readBits;

		return readBits;
	}

	inline void BufferedBitstreamReader::Skip(int64_t count)
	{
		seek((uint64_t)std::max((int64_t)0, (int64_t)mCursor + count));
	}

	inline void BufferedBitstreamReader::Align(uint32_t count)
	{
		if (count == 0)
			return;

		uint32_t bits = count * 8;
		skip(bits - (((mCursor - 1) & (bits - 1)) + 1));
	}

	inline void BufferedBitstreamReader::Seek(uint64_t pos)
	{
		if (!mIsMapped && (pos < mBufferedRangeStart || pos >= mBufferedRangeEnd))
		{
			mBufferedRangeStart = Math::DivideAndRoundUp(pos, (uint64_t)8) * 8;
			mBufferedRangeEnd = mBufferedRangeStart;
		}

		mCursor = pos;
		mBitstream->Seek(pos - mBufferedRangeStart);
	}

	inline void BufferedBitstreamReader::Preload(uint32_t count)
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
		if (mBitstream->Capacity() < newBufferedLength)
			mBitstream->Resize((uint32_t)Math::DivideAndRoundUp(newBufferedLength, (uint64_t)Bitstream::BITS_PER_QUANT));

		// Read the data from data stream into the bitstream
		uint64_t orgPos = mBitstream->Tell();
		mBitstream->Seek(bufferedLength);
		
		mDataStream->Seek((size_t)(mBufferedRangeEnd / 8));
		if (mDataStream->Read(mBitstream->cursor(), numBytesToPreload) != numBytesToPreload)
			BS_EXCEPT(InternalErrorException, "Error reading data.");
		
		mBitstream->Seek(orgPos);
		mBufferedRangeEnd += numBytesToPreload * 8;
	}

	inline void BufferedBitstreamReader::ClearBuffered(bool force)
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
			mBitstream->ReadBytes(remainingData, remainingBytes);
		}

		mBitstream->Seek(0);
		mBitstream->WriteBytes(remainingData, remainingBytes);
		mBitstream->Seek(remainingBits);

		if (remainingData)
			bs_stack_free(remainingData);
	}

	inline BufferedBitstreamWriter::BufferedBitstreamWriter(Bitstream* bitstream, const SPtr<DataStream>& dataStream,
		uint32_t bufferSize, uint32_t flushAfter)
		: MBitstream(bitstream), mDataStream(dataStream), mFlushAfter(flushAfter)
	{
		if (mBitstream->Capacity() < (uint64_t)bufferSize * 8)
			mBitstream->Reserve(bufferSize);
	}

	inline uint64_t BufferedBitstreamWriter::WriteBits(const Bitstream::QuantType* data, uint64_t count)
	{
		return mBitstream->WriteBits(data, count);
	}

	template <class T>
	uint32_t BufferedBitstreamWriter::WriteBytes(T& value)
	{
		return mBitstream->WriteBytes(value);
	}

	inline uint32_t BufferedBitstreamWriter::WriteBytes(Bitstream::QuantType* data, uint32_t count)
	{
		return mBitstream->WriteBytes(data, count);
	}

	inline uint32_t BufferedBitstreamWriter::WriteVarInt(uint32_t value)
	{
		return mBitstream->WriteVarInt(value);
	}

	inline void BufferedBitstreamWriter::Align(uint32_t count)
	{
		mBitstream->Align(count);
	}

	inline void BufferedBitstreamWriter::Flush(bool force)
	{
		uint64_t bitsInBuffer = mBitstream->Tell();
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
			mBitstream->WriteBits(&quant, bitsToPad);
			bitsInBuffer += bitsToPad;
			
			assert((bitsInBuffer % Bitstream::BITS_PER_QUANT) == 0);

			bytesToFlush = bitsInBuffer >> Bitstream::BITS_PER_QUANT_LOG2;
			bitsToFlush = bytesToFlush * Bitstream::BITS_PER_QUANT;
			leftoverBits = 0;
		}

		mBitstream->Seek(bitsToFlush);
		mBitstream->ReadBits(&quant, leftoverBits);

		mBitstream->Seek(0);
		mDataStream->Write(mBitstream->cursor(), (size_t)bytesToFlush);

		mBitstream->WriteBits(&quant, leftoverBits);
	}
	
}
