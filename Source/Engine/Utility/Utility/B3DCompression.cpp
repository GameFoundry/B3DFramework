//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DCompression.h"
#include "FileSystem/B3DDataStream.h"

// Third party
#include "B3DBitwise.h"
#include "snappy.h"
#include "snappy-sinksource.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

B3D_LOG_CATEGORY_STATIC(LogCompression, Log)

/** Source accepting a data stream. Used for Snappy compression library. */
class DataStreamSource : public snappy::Source
{
public:
	static constexpr size_t kFileReadBufferSize = 32768;

	DataStreamSource(DataStream& stream, u64 bytesToRead, std::function<void(float)> reportProgress = nullptr)
		: mStream(stream), mReportProgress(std::move(reportProgress))
	{
		const size_t remainingBytesInStream = stream.Size() - stream.Tell();
		mTotalBytesToRead = bytesToRead == 0 ? remainingBytesInStream : std::min(remainingBytesInStream, bytesToRead);
		mRemainingBytes = mTotalBytesToRead;
		mEndAddress = stream.Tell() + mTotalBytesToRead;

		if(mStream.IsFile())
		{
			mReadBuffers[0] = (char*)B3DAllocate(kFileReadBufferSize);
			mReadBuffers[1] = (char*)B3DAllocate(kFileReadBufferSize);

			mFileReadCursor = stream.Tell();

			// Start reading the first chunk right away so it overlaps with whatever the caller does before the first Peek.
			IssueReadAhead();
		}
	}

	~DataStreamSource() override
	{
		// Wait for any in-flight read-ahead before freeing the buffer it's writing into.
		if(mReadAheadInFlight)
			mReadAheadOp.BlockUntilComplete();

		if(mReadBuffers[0] != nullptr)
			B3DFree(mReadBuffers[0]);

		if(mReadBuffers[1] != nullptr)
			B3DFree(mReadBuffers[1]);
	}

	size_t Available() const override
	{
		return mRemainingBytes;
	}

	const char* Peek(size_t* len) override
	{
		if(!mStream.IsFile())
		{
			const auto& memoryStream = static_cast<const MemoryDataStream&>(mStream);

			*len = Available();
			return (char*)memoryStream.Data() + mReadBufferOffset;
		}

		// Advance through buffers until the active one has unconsumed data (a Skip may span more than one buffer).
		while(mReadBufferOffset >= mReadBufferSize)
		{
			if(!mReadAheadInFlight)
			{
				// No more data available from the stream.
				*len = 0;
				return mReadBuffers[mActiveBufferIndex];
			}

			mReadBufferOffset -= mReadBufferSize;

			// Consume the chunk that was being read ahead (cheap if the IO already finished), then immediately start
			// reading the next one so it overlaps with the snappy decompression of this chunk.
			mReadAheadOp.BlockUntilComplete();
			const TShared<MemoryDataStream> readResult = mReadAheadOp.GetReturnValue();

			mActiveBufferIndex = mReadAheadTargetBufferIndex;
			mReadBufferSize = readResult != nullptr ? readResult->Size() : 0;
			mReadAheadInFlight = false;

			IssueReadAhead();

			if(mReadBufferSize == 0)
				break;
		}

		*len = mReadBufferSize - mReadBufferOffset;
		return mReadBuffers[mActiveBufferIndex] + mReadBufferOffset;
	}

	void Skip(size_t n) override
	{
		mReadBufferOffset += n;
		mRemainingBytes -= n;

		if(mReportProgress)
			mReportProgress(1.0f - mRemainingBytes / (float)mTotalBytesToRead);
	}

private:
	/** Issues an asynchronous read for the next chunk into the buffer not currently being consumed. */
	void IssueReadAhead()
	{
		if(mFileReadCursor >= mEndAddress)
		{
			mReadAheadInFlight = false;
			return;
		}

		const size_t sizeToRead = std::min(kFileReadBufferSize, mEndAddress - mFileReadCursor);
		mReadAheadTargetBufferIndex = 1 - mActiveBufferIndex;

		mReadAheadOp = mStream.ReadAsync(mFileReadCursor, sizeToRead, DataRange(mReadBuffers[mReadAheadTargetBufferIndex], sizeToRead));
		mReadAheadInFlight = true;
		mFileReadCursor += sizeToRead;
	}

	DataStream& mStream;
	std::function<void(float)> mReportProgress;

	size_t mRemainingBytes;
	size_t mTotalBytesToRead;
	size_t mEndAddress = 0;

	// File streams only: double-buffered read-ahead.
	char* mReadBuffers[2] = { nullptr, nullptr };
	u32 mActiveBufferIndex = 0;
	size_t mReadBufferOffset = 0;
	size_t mReadBufferSize = 0;

	TAsyncOp<TShared<MemoryDataStream>> mReadAheadOp;
	bool mReadAheadInFlight = false;
	u32 mReadAheadTargetBufferIndex = 0;
	u64 mFileReadCursor = 0;
};

/** Sink (destination) accepting a data stream. Used for Snappy compression library. */
class DataStreamSink : public snappy::Sink
{
public:
	DataStreamSink(DataStream& outputStream)
		:mOutputStream(outputStream)
	{ }

	virtual ~DataStreamSink()
	{
		if(mBuffer != nullptr)
		{
			B3D_ASSERT(mWasLastBufferAppended);
			B3DFree(mBuffer);
		}
	}

	void Append(const char* data, size_t n) override
	{
		if(mBuffer == data)
		{
			B3D_ASSERT(n <= mBufferCapacity);
			mWasLastBufferAppended = true;
		}

		mOutputStream.Write(data, n);
	}

	char* GetAppendBuffer(size_t len, char* scratch) override
	{
		B3D_ASSERT(mBuffer == nullptr || mWasLastBufferAppended);
		ReallocateBufferIfNeeded(len);

		mWasLastBufferAppended = false;
		return mBuffer;
	}

	char* GetAppendBufferVariable(size_t min_size, size_t desired_size_hint, char* scratch, size_t scratch_size, size_t* allocated_size) override
	{
		B3D_ASSERT(mBuffer == nullptr || mWasLastBufferAppended);

		const size_t requiredCapacity = std::max(desired_size_hint, min_size);
		ReallocateBufferIfNeeded(requiredCapacity);

		*allocated_size = requiredCapacity;
		mWasLastBufferAppended = false;
		return mBuffer;
	}

	void AppendAndTakeOwnership(char* bytes, size_t n, void (*deleter)(void*, const char*, size_t), void* deleter_arg) override
	{
		mOutputStream.Write(bytes, n);

		if(mBuffer != bytes)
		{
			(*deleter)(deleter_arg, bytes, n);
		}
	}

private:
	/** Reallocates the internal buffer if it doesn't have enough capacity. Does not perserve current buffer data. */
	void ReallocateBufferIfNeeded(size_t requiredCapacity)
	{
		const bool reallocateBuffer = mBuffer == nullptr || mBufferCapacity < requiredCapacity;
		if(!reallocateBuffer)
			return;

		if(mBuffer != nullptr)
		{
			B3DFree(mBuffer);
			mBuffer = nullptr;
			mBufferCapacity = 0;
		}

		mBuffer = (char*)B3DAllocate(requiredCapacity);
		mBufferCapacity = requiredCapacity;
	}

	DataStream& mOutputStream;
	char* mBuffer = nullptr;
	size_t mBufferCapacity = 0;
	bool mWasLastBufferAppended = false;
};

u64 Compression::Compress(DataStream& input, DataStream& output, u64 inputDataSize, CompressionType compressionType, std::function<void(float)> reportProgress)
{
	if(compressionType != CompressionType::Snappy)
	{
		B3D_LOG(Error, LogCompression, "Cannot compress data. Unsupported compression type provided: {0}.", (u32)compressionType);
		return false;
	}

	DataStreamSource dataSource(input, inputDataSize, std::move(reportProgress));
	DataStreamSink dataSink(output);

	return (u64)snappy::Compress(&dataSource, &dataSink);
}

bool Compression::Decompress(DataStream& input, DataStream& output, u64 inputDataSize, CompressionType compressionType,  std::function<void(float)> reportProgress)
{
	if(compressionType != CompressionType::Snappy)
	{
		B3D_LOG(Error, LogCompression, "Cannot decompress data. Unsupported compression type provided: {0}.", (u32)compressionType);
		return false;
	}

	DataStreamSource dataSource(input, inputDataSize, std::move(reportProgress));
	DataStreamSink dataSink(output);

	if(!snappy::Uncompress(&dataSource, &dataSink))
	{
		B3D_LOG(Error, LogCompression, "Cannot decompress data. Corrupt input data.");
		return false;
	}

	return true;
}
