//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsCompression.h"
#include "FileSystem/BsDataStream.h"

// Third party
#include "snappy.h"
#include "snappy-sinksource.h"
#include "Debug/BsDebug.h"

namespace bs
{
	/** Source accepting a data stream. Used for Snappy compression library. */
	class DataStreamSource : public snappy::Source
	{
	public:
		DataStreamSource(const SPtr<DataStream>& stream, std::function<void(float)> reportProgress = nullptr)
			:mStream(stream), mReportProgress(std::move(reportProgress))
		{
			mTotal = mStream->size() - mStream->tell();
			mRemaining = mTotal;

			if (mStream->isFile())
				mReadBuffer = (char*)bs_alloc(32768);
		}

		virtual ~DataStreamSource()
		{
			if (mReadBuffer != nullptr)
				bs_free(mReadBuffer);
		}

		size_t Available() const override
		{
			return mRemaining;
		}

		const char* Peek(size_t* len) override
		{
			if (!mStream->isFile())
			{
				SPtr<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStream);

				*len = Available();
				return (char*)memStream->data() + mBufferOffset;
			}
			else
			{
				while (mBufferOffset >= mReadBufferContentSize)
				{
					mBufferOffset -= mReadBufferContentSize;
					mReadBufferContentSize = mStream->read(mReadBuffer, 32768);

					if (mReadBufferContentSize == 0)
						break;
				}

				*len = mReadBufferContentSize - mBufferOffset;
				return (char*)(mReadBuffer + mBufferOffset);
			}
		}

		void Skip(size_t n) override
		{
			mBufferOffset += n;
			mRemaining -= n;

			if(mReportProgress)
				mReportProgress(1.0f - mRemaining / (float)mTotal);
		}
	private:
		SPtr<DataStream> mStream;
		std::function<void(float)> mReportProgress;

		size_t mRemaining;
		size_t mTotal;
		size_t mBufferOffset = 0;

		// File streams only
		char* mReadBuffer = nullptr;
		size_t mReadBufferContentSize = 0;
	};

	/** Sink (destination) accepting a data stream. Used for Snappy compression library. */
	class DataStreamSink : public snappy::Sink
	{
		struct BufferPiece
		{
			char* buffer;
			size_t size;
		};

	public:
		DataStreamSink() = default;
		virtual ~DataStreamSink()
		{
			for (auto& entry : mBufferPieces)
				bs_free(entry.buffer);
		}

		void Append(const char* data, size_t n) override
		{
			if(mBufferPieces.empty() || mBufferPieces.back().buffer != data)
			{
				BufferPiece piece;
				piece.buffer = (char*)bs_alloc((UINT32)n);
				piece.size = n;

				memcpy(piece.buffer, data, n);
				mBufferPieces.push_back(piece);
			}
			else
			{
				BufferPiece& piece = mBufferPieces.back();
				assert(piece.buffer == data);

				piece.size = n;
			}
		}

		char* GetAppendBuffer(size_t len, char* scratch) override
		{
			BufferPiece piece;
			piece.buffer = (char*)bs_alloc((UINT32)len);
			piece.size = 0;

			mBufferPieces.push_back(piece);
			return piece.buffer;
		}

		char* GetAppendBufferVariable(size_t min_size, size_t desired_size_hint, char* scratch, size_t scratch_size,
			size_t* allocated_size) override
		{
			BufferPiece piece;
			piece.buffer = (char*)bs_alloc((UINT32)desired_size_hint);
			piece.size = 0;

			mBufferPieces.push_back(piece);

			*allocated_size = desired_size_hint;
			return piece.buffer;
		}

		void AppendAndTakeOwnership(char* bytes, size_t n, void(*deleter)(void*, const char*, size_t),
			void *deleter_arg) override
		{
			BufferPiece& piece = mBufferPieces.back();

			if (piece.buffer != bytes)
			{
				memcpy(piece.buffer, bytes, n);
				(*deleter)(deleter_arg, bytes, n);
			}

			piece.size = n;
		}

		SPtr<MemoryDataStream> GetOutput()
		{
			size_t totalSize = 0;
			for (auto& entry : mBufferPieces)
				totalSize += entry.size;

			SPtr<MemoryDataStream> ds = bs_shared_ptr_new<MemoryDataStream>(totalSize);
			for (auto& entry : mBufferPieces)
				ds->write(entry.buffer, entry.size);

			ds->seek(0);
			return ds;
		}

	private:
		Vector<BufferPiece> mBufferPieces;
	};

	SPtr<MemoryDataStream> Compression::compress(const SPtr<DataStream>& input, std::function<void(float)> reportProgress)
	{
		DataStreamSource src(input, std::move(reportProgress));
		DataStreamSink dst;

		size_t bytesWritten = snappy::Compress(&src, &dst);
		SPtr<MemoryDataStream> output = dst.GetOutput();
		assert(output->size() == bytesWritten);

		return output;
	}

	SPtr<MemoryDataStream> Compression::decompress(const SPtr<DataStream>& input, std::function<void(float)> reportProgress)
	{
		DataStreamSource src(input, std::move(reportProgress));
		DataStreamSink dst;

		if (!snappy::Uncompress(&src, &dst))
		{
			BS_LOG(Error, Generic, "Decompression failed, corrupt data.");
			return nullptr;
		}

		return dst.GetOutput();
	}
}
