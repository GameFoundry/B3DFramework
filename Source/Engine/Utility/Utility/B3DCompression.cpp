//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DCompression.h"
#include "FileSystem/B3DDataStream.h"

// Third party
#include "B3DBitwise.h"
#include "snappy.h"
#include "snappy-sinksource.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

B3D_LOG_CATEGORY_STATIC(Compression, Log)

/** Source accepting a data stream. Used for Snappy compression library. */
class DataStreamSource : public snappy::Source
{
public:
	static constexpr size_t kFileReadBufferSize = 32768;

	DataStreamSource(const DataStream& stream, u64 bytesToRead, std::function<void(float)> reportProgress = nullptr)
		: mStream(stream), mReportProgress(std::move(reportProgress))
	{
		const size_t remainingBytesInStream = stream.Size() - stream.Tell();
		mTotalBytesToRead = bytesToRead == 0 ? remainingBytesInStream : std::min(remainingBytesInStream, bytesToRead);
		mRemainingBytes = mTotalBytesToRead;
		mEndAddress = stream.Tell() + mTotalBytesToRead;

		if(mStream.IsFile())
			mReadBuffer = (char*)B3DAllocate(kFileReadBufferSize);
	}

	~DataStreamSource() override
	{
		if(mReadBuffer != nullptr)
			B3DFree(mReadBuffer);
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
		else
		{
			while(mReadBufferOffset >= mReadBufferSize)
			{
				mReadBufferOffset -= mReadBufferSize;

				const size_t sizeToRead = std::min(kFileReadBufferSize, mEndAddress - mStream.Tell());
				mReadBufferSize = mStream.Read(mReadBuffer, sizeToRead);

				if(mReadBufferSize == 0)
					break;
			}

			*len = mReadBufferSize - mReadBufferOffset;
			return (char*)(mReadBuffer + mReadBufferOffset);
		}
	}

	void Skip(size_t n) override
	{
		mReadBufferOffset += n;
		mRemainingBytes -= n;

		if(mReportProgress)
			mReportProgress(1.0f - mRemainingBytes / (float)mTotalBytesToRead);
	}

private:
	const DataStream& mStream;
	std::function<void(float)> mReportProgress;

	size_t mRemainingBytes;
	size_t mTotalBytesToRead;
	size_t mEndAddress = 0;

	// File streams only
	char* mReadBuffer = nullptr;
	size_t mReadBufferOffset = 0;
	size_t mReadBufferSize = 0;
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
		B3D_LOG(Error, Compression, "Cannot compress data. Unsupported compression type provided: {0}.", (u32)compressionType);
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
		B3D_LOG(Error, Compression, "Cannot decompress data. Unsupported compression type provided: {0}.", (u32)compressionType);
		return false;
	}

	DataStreamSource dataSource(input, inputDataSize, std::move(reportProgress));
	DataStreamSink dataSink(output);

	if(!snappy::Uncompress(&dataSource, &dataSink))
	{
		B3D_LOG(Error, Compression, "Cannot decompress data. Corrupt input data.");
		return false;
	}

	return true;
}
