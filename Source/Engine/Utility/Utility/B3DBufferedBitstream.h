//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DBitstream.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @addtogroup Serialization
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
		 * @param	bitstream		Bitstream into which to load the buffered data.
		 * @param	dataStream		Data stream from which to read the data.
		 * @param	dataLength		Number of bytes that will be read, starting from the stream's current position. 
		 * @param	preloadSize		Determines the size of the chunk to preload, when we reach the end of
		 *							buffered data. In bytes.
		 * @param	maxBufferSize	Maximum size of the buffer before it is cleared.
		 */
		BufferedBitstreamReader(Bitstream* bitstream, const TShared<DataStream>& dataStream, uint64_t dataLength, uint32_t preloadSize, uint32_t maxBufferSize);

		// Note: Perhaps allow reads with no chunk preload (i.e. just the requested count)

		/** @copydoc Bitstream::ReadBits(Bitstream::QuantType* data, uint32_t count) */
		uint64_t ReadBits(Bitstream::QuantType* data, uint64_t count);

		/** @copydoc Bitstream::ReadBytes(T&) */
		template <class T>
		uint32_t ReadBytes(T& value);

		/** @copydoc Bitstream::ReadBytes(void*, uint32_t) */
		uint32_t ReadBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::ReadVarInt(uint32_t&) */
		uint32_t ReadVarInt(uint32_t& value);

		/** @copydoc Bitstream::Skip */
		void Skip(int64_t count);

		/** @copydoc Bitstream::SkipBytes */
		void SkipBytes(int32_t count) { return Skip((int64_t)count * 8); }

		/** @copydoc Bitstream::Seek */
		void Seek(uint64_t pos);

		/** @copydoc Bitstream::Tell */
		uint64_t Tell() const { return mCursor; }

		/** @copydoc Bitstream::Align() */
		void Align(uint32_t count = 1);

		/** Ensures that at least @p count bytes starting at the current cursor are loaded into the bitstream from the data stream. */
		void EnsureLoadedToBitstream(uint32_t count);

		/**
		 * Clears buffered data behind the current cursor location.
		 *
		 * @param	force	If false the buffer will only be cleared if its current size is	over the maximum
		 *					buffer size limit. Otherwise it will always be cleared.
		 */
		void ClearBuffered(bool force);

		/** Returns the underlying data stream. */
		const TShared<DataStream>& GetDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& GetBitstream() const { return *mBitstream; }

	private:
		/** An in-flight read-ahead: an asynchronous read of [RangeStart, RangeStart + ByteCount). */
		struct ReadAheadRequest
		{
			TAsyncOp<TShared<MemoryDataStream>> Op { AsyncOpEmpty{} }; /**< Empty until assigned the real read operation. */
			uint64_t RangeStart = 0; /**< Byte offset in the stream of the chunk being read ahead. */
			uint64_t ByteCount = 0;  /**< Number of bytes being read ahead. */
		};

		/** Caps how many read-aheads may be in flight at once. */
		static constexpr size_t kMaxInFlightReadAheads = 4;

		/**
		 * Starts an asynchronous read of the chunk immediately following the currently buffered range. A later
		 * EnsureLoadedToBitstream that needs that chunk consumes the (likely already finished) operation instead of issuing a fresh blocking read,
		 * overlapping the IO with the deserialization work in between. Does nothing for memory-backed streams, when there's
		 * no more data to read, when a read-ahead already covers the chunk, or when too many are in flight.
		 */
		void IssueReadAhead();

		/** Drops completed read-aheads that were never consumed (e.g. abandoned by a seek), freeing their buffers. */
		void ReclaimCompletedReadAheads();

		/** Returns the index of an in-flight read ahead request covering @p byteOffset, or ~0u if none. */
		u32 HasReadAheadRequestForOffset(uint64_t byteOffset) const;

		/** Removes the read-ahead slot at @p index by shifting the trailing active slots down. */
		void EraseReadAheadAt(size_t index);

		uint64_t mCursor = 0;
		uint64_t mBufferedRangeStart = 0;
		uint64_t mBufferedRangeEnd = 0;
		Bitstream* mBitstream;
		TShared<DataStream> mDataStream;
		Bitstream mMemBitstream;
		uint64_t mLength;
		uint64_t mPreloadSize;
		uint64_t mMaxBufferSize;
		bool mIsMapped = false;

		/** In-flight read-aheads packed at the front; slots [0, mNumActiveReadAheads) are valid. */
		Array<ReadAheadRequest, kMaxInFlightReadAheads> mReadAheadRequests;
		size_t mActiveReadAheadCount = 0;
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
		 * @param	bitstream		Bitstream into which the buffered data will be written.
		 * @param	dataStream		Data stream from which to read the data.
		 * @param	bufferSize		Initial size of the write buffer, in bytes.
		 * @param	flushAfter		Number of bytes after which the write buffer will be flushed to the data stream.
		 */
		BufferedBitstreamWriter(Bitstream* bitstream, const TShared<DataStream>& dataStream, uint32_t bufferSize, uint32_t flushAfter);

		/** @copydoc Bitstream::WriteBits(const Bitstream::QuantType*, uint32_t) */
		uint64_t WriteBits(const Bitstream::QuantType* data, uint64_t count);

		/** @copydoc Bitstream::WriteBytes(T&) */
		template <class T>
		uint32_t WriteBytes(T& value);

		/** @copydoc Bitstream::WriteBytes(void*, uint32_t) */
		uint32_t WriteBytes(Bitstream::QuantType* data, uint32_t count);

		/** @copydoc Bitstream::WriteVarInt */
		uint32_t WriteVarInt(uint32_t value);

		/** @copydoc Bitstream::Align() */
		void Align(uint32_t count = 1);

		/** Flushes the write buffer to the output stream if a certain buffer length is reached. */
		void Flush(bool force);

		/** Returns the underlying data stream. */
		const TShared<DataStream>& GetDataStream() const { return mDataStream; }

		/** Returns the underlying bitstream. */
		Bitstream& GetBitstream() const { return *mBitstream; }

	private:
		Bitstream* mBitstream;
		TShared<DataStream> mDataStream;
		uint64_t mFlushAfter;
	};

	/** @} */

	inline BufferedBitstreamReader::BufferedBitstreamReader(Bitstream* bitstream, const TShared<DataStream>& dataStream, uint64_t dataLength, uint32_t preloadSize, uint32_t maxBufferSize)
		: mCursor((uint64_t)dataStream->Tell() * 8), mBufferedRangeStart(mCursor), mBufferedRangeEnd(mCursor), mBitstream(bitstream), mDataStream(dataStream), mLength(std::min((uint64_t)dataStream->Size(), (uint64_t)dataStream->Tell() + dataLength)), mPreloadSize(preloadSize), mMaxBufferSize(maxBufferSize), mIsMapped(!dataStream->IsFile())
	{
		// Special case for memory streams, we can just map the memory directly
		if(mIsMapped)
		{
			auto memStream = std::static_pointer_cast<MemoryDataStream>(dataStream);
			mMemBitstream = Bitstream(memStream->Data(), (uint32_t)memStream->Size());
			mMemBitstream.Seek(mCursor);

			mBitstream = &mMemBitstream;

			mBufferedRangeStart = 0;
			mBufferedRangeEnd = (uint64_t)mLength * 8;
		}
	}

	inline uint64_t BufferedBitstreamReader::ReadBits(Bitstream::QuantType* data, uint64_t count)
	{
		EnsureLoadedToBitstream((uint32_t)Math::DivideAndRoundUp(count, (uint64_t)8));
		mCursor += count;
		return mBitstream->ReadBits(data, count);
	}

	template <class T>
	uint32_t BufferedBitstreamReader::ReadBytes(T& value)
	{
		EnsureLoadedToBitstream(sizeof(T));
		mCursor += sizeof(T) * 8;
		return mBitstream->ReadBytes(value);
	}

	inline uint32_t BufferedBitstreamReader::ReadBytes(Bitstream::QuantType* data, uint32_t count)
	{
		EnsureLoadedToBitstream(count);
		mCursor += (uint64_t)count * 8;
		return mBitstream->ReadBytes(data, count);
	}

	inline uint32_t BufferedBitstreamReader::ReadVarInt(uint32_t& value)
	{
		EnsureLoadedToBitstream(sizeof(value));
		uint32_t readBits = mBitstream->ReadVarInt(value);
		mCursor += readBits;

		return readBits;
	}

	inline void BufferedBitstreamReader::Skip(int64_t count)
	{
		Seek((uint64_t)std::max((int64_t)0, (int64_t)mCursor + count));
	}

	inline void BufferedBitstreamReader::Align(uint32_t count)
	{
		if(count == 0)
			return;

		uint32_t bits = count * 8;
		Skip(bits - (((mCursor - 1) & (bits - 1)) + 1));
	}

	inline void BufferedBitstreamReader::Seek(uint64_t pos)
	{
		if(!mIsMapped && (pos < mBufferedRangeStart || pos >= mBufferedRangeEnd))
		{
			mBufferedRangeStart = Math::DivideAndRoundUp(pos, (uint64_t)8) * 8;
			mBufferedRangeEnd = mBufferedRangeStart;

			IssueReadAhead();
		}

		mCursor = pos;
		mBitstream->Seek(pos - mBufferedRangeStart);
	}

	inline void BufferedBitstreamReader::IssueReadAhead()
	{
		// Memory-backed streams are mapped in full up front (no chunk buffering), so there is never anything to read ahead.
		if(mIsMapped)
			return;

		const u64 nextOffset = mBufferedRangeEnd / 8;
		if(nextOffset >= mLength)
			return; // At (or past) the end of the readable range; nothing to read ahead.

		// Nothing to do if an in-flight read-ahead already covers the upcoming chunk.
		if(HasReadAheadRequestForOffset(nextOffset) != ~0u)
			return;

		// Nothing covers the upcoming chunk, so any in-flight read-aheads are leftovers from a previous location; drop the
		// finished ones (freeing their buffers and a concurrency slot) before issuing a new one below.
		ReclaimCompletedReadAheads();

		// Bound the number of concurrent read-aheads; just skip prefetching if at the limit.
		if(mActiveReadAheadCount >= kMaxInFlightReadAheads)
			return;

		const u64 byteCount = std::min(mPreloadSize, mLength - nextOffset);

		ReadAheadRequest& request = mReadAheadRequests[mActiveReadAheadCount++];
		request.RangeStart = nextOffset;
		request.ByteCount = byteCount;
		request.Op = mDataStream->ReadAsync(nextOffset, (size_t)byteCount);
	}

	inline void BufferedBitstreamReader::ReclaimCompletedReadAheads()
	{
		for(size_t requestIndex = 0; requestIndex < mActiveReadAheadCount; )
		{
			// Erasing a completed read-ahead's slot releases its operation (and its owned result buffer).
			if(mReadAheadRequests[requestIndex].Op.HasCompleted())
				EraseReadAheadAt(requestIndex);
			else
				++requestIndex;
		}
	}

	inline u32 BufferedBitstreamReader::HasReadAheadRequestForOffset(uint64_t byteOffset) const
	{
		for(size_t requestIndex = 0; requestIndex < mActiveReadAheadCount; requestIndex++)
		{
			const ReadAheadRequest& request = mReadAheadRequests[requestIndex];
			if(byteOffset >= request.RangeStart && byteOffset < (request.RangeStart + request.ByteCount))
				return (u32)requestIndex;
		}

		return ~0u;
	}

	inline void BufferedBitstreamReader::EraseReadAheadAt(size_t index)
	{
		B3D_ASSERT(index < mActiveReadAheadCount);

		for(size_t requestIndex = index + 1; requestIndex < mActiveReadAheadCount; ++requestIndex)
			mReadAheadRequests[requestIndex - 1] = std::move(mReadAheadRequests[requestIndex]);

		--mActiveReadAheadCount;
		mReadAheadRequests[mActiveReadAheadCount] = ReadAheadRequest{};
	}

	inline void BufferedBitstreamReader::EnsureLoadedToBitstream(uint32_t count)
	{
		B3D_ASSERT(mCursor >= mBufferedRangeStart);

		if((mCursor + (uint64_t)count * 8) <= mBufferedRangeEnd)
			return;

		// Buffer chunks until the requested range is available. A single chunk normally suffices, but a request larger
		// than the read-ahead chunk may need more than one.
		while((mCursor + (uint64_t)count * 8) > mBufferedRangeEnd)
		{
			B3D_ASSERT((mBufferedRangeEnd % 8) == 0);

			const uint64_t readOffset = mBufferedRangeEnd / 8;
			const uint64_t remainingBytes = mLength - readOffset;
			if(remainingBytes == 0)
				break; // End of stream reached; nothing more can be buffered.

			// Consume an in-flight read-ahead if one covers this offset. A sequential read-ahead starts exactly at
			// readOffset, but after a seek into the middle of a read-ahead chunk we consume only its tail (skipping the
			// bytes before readOffset).
			const u32 readAheadIndex = HasReadAheadRequestForOffset(readOffset);
			const bool consumeReadAhead = readAheadIndex != ~0u;

			uint64_t byteCountToRead;
			uint64_t readAheadSkipBytes = 0;
			if(consumeReadAhead)
			{
				readAheadSkipBytes = readOffset - mReadAheadRequests[readAheadIndex].RangeStart;
				byteCountToRead = mReadAheadRequests[readAheadIndex].ByteCount - readAheadSkipBytes;
			}
			else
			{
				// No read-ahead covers this offset (e.g. right after a seek to a fresh location) - read it directly here.
				byteCountToRead = std::min(std::max(mPreloadSize, (uint64_t)count), remainingBytes);
			}

			// Make sure our buffer has enough room for the new data
			const uint64_t bufferedLength = mBufferedRangeEnd - mBufferedRangeStart;
			const uint64_t newBufferedLength = bufferedLength + byteCountToRead * 8;
			if(mBitstream->Capacity() < newBufferedLength)
				mBitstream->Resize((uint32_t)Math::DivideAndRoundUp(newBufferedLength, (uint64_t)Bitstream::kBitsPerQuant));

			const uint64_t orgPos = mBitstream->Tell();
			mBitstream->Seek(bufferedLength);

			if(consumeReadAhead)
			{
				// The chunk was already being read into the operation's own buffer; block on it (cheap if the IO finished
				// while we were consuming the previous chunk) and copy the needed tail into the bitstream buffer.
				ReadAheadRequest& readAheadRequest = mReadAheadRequests[readAheadIndex];
				readAheadRequest.Op.BlockUntilComplete();

				const TShared<MemoryDataStream> readResult = readAheadRequest.Op.GetReturnValue();

				if(readResult == nullptr || readResult->Size() != readAheadRequest.ByteCount)
					B3D_LOG(Fatal, LogSerialization, "Error reading data.");

				mBitstream->WriteBytes((Bitstream::QuantType*)readResult->Data() + readAheadSkipBytes, (uint32_t)byteCountToRead);

				// Done with this read-ahead; drop the request
				EraseReadAheadAt(readAheadIndex);
			}
			else
			{
				// Read straight into the bitstream buffer. 
				TAsyncOp<TShared<MemoryDataStream>> readOp = mDataStream->ReadAsync(readOffset, (size_t)byteCountToRead, DataRange(mBitstream->Cursor(), byteCountToRead));
				readOp.BlockUntilComplete();

				const TShared<MemoryDataStream> readResult = readOp.GetReturnValue();
				if(readResult == nullptr || readResult->Size() != byteCountToRead)
					B3D_LOG(Fatal, LogSerialization, "Error reading data.");
			}

			mBitstream->Seek(orgPos);
			mBufferedRangeEnd += byteCountToRead * 8;
		}

		// Start reading ahead the chunk after the one we just buffered, so the IO overlaps with the deserialization work
		IssueReadAhead();
	}

	inline void BufferedBitstreamReader::ClearBuffered(bool force)
	{
		// If memory stream, there is no buffer and we map the entire stream
		if(mIsMapped)
			return;

		uint64_t bufferedLengthBits = mBufferedRangeEnd - mBufferedRangeStart;
		uint64_t bufferedLengthBytes = bufferedLengthBits / 8;

		if(!force && bufferedLengthBytes < mMaxBufferSize)
			return;

		uint64_t offsetBits = mCursor - mBufferedRangeStart;
		uint64_t bytesToClear = offsetBits >> Bitstream::kBitsPerQuantLoG2;

		uint64_t remainingBits = offsetBits - bytesToClear * Bitstream::kBitsPerQuant;
		uint32_t remainingBytes = (uint32_t)(bufferedLengthBytes - bytesToClear);

		mBufferedRangeStart += bytesToClear * 8;

		Bitstream::QuantType* remainingData = nullptr;
		if(remainingBytes > 0)
		{
			remainingData = B3DStackAllocate<Bitstream::QuantType>(remainingBytes);
			mBitstream->ReadBytes(remainingData, remainingBytes);
		}

		mBitstream->Seek(0);
		mBitstream->WriteBytes(remainingData, remainingBytes);
		mBitstream->Seek(remainingBits);

		if(remainingData)
			B3DStackFree(remainingData);
	}

	inline BufferedBitstreamWriter::BufferedBitstreamWriter(Bitstream* bitstream, const TShared<DataStream>& dataStream, uint32_t bufferSize, uint32_t flushAfter)
		: mBitstream(bitstream), mDataStream(dataStream), mFlushAfter(flushAfter)
	{
		if(mBitstream->Capacity() < (uint64_t)bufferSize * 8)
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
		if((bitsInBuffer < (mFlushAfter * 8)) && !force)
			return;

		// Flush all the complete bytes, and leave any sub-byte bits in the write stream
		uint64_t bytesToFlush = bitsInBuffer >> Bitstream::kBitsPerQuantLoG2;
		uint64_t bitsToFlush = bytesToFlush * Bitstream::kBitsPerQuant;
		uint64_t leftoverBits = bitsInBuffer - bitsToFlush;

		B3D_ASSERT(leftoverBits < Bitstream::kBitsPerQuant);

		Bitstream::QuantType quant = 0;
		if(force && leftoverBits > 0)
		{
			// Pad the last quant
			uint32_t bitsToPad = (u32)(((u64)Bitstream::kBitsPerQuant) - leftoverBits);
			mBitstream->WriteBits(&quant, bitsToPad);
			bitsInBuffer += bitsToPad;

			B3D_ASSERT((bitsInBuffer % Bitstream::kBitsPerQuant) == 0);

			bytesToFlush = bitsInBuffer >> Bitstream::kBitsPerQuantLoG2;
			bitsToFlush = bytesToFlush * Bitstream::kBitsPerQuant;
			leftoverBits = 0;
		}

		mBitstream->Seek(bitsToFlush);
		mBitstream->ReadBits(&quant, leftoverBits);

		mBitstream->Seek(0);
		mDataStream->Write(mBitstream->Cursor(), (size_t)bytesToFlush);

		mBitstream->WriteBits(&quant, leftoverBits);
	}

} // namespace b3d
