//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsDataStream.h"
#include "Debug/BsDebug.h"
#include "String/BsUnicode.h"

namespace bs
{
	const UINT32 DataStream::StreamTempSize = 128;

	/** Checks does the provided buffer has an UTF32 byte order mark in little endian order. */
	bool isUTF32LE(const UINT8* buffer)
	{
		return buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00;
	}

	/** Checks does the provided buffer has an UTF32 byte order mark in big endian order. */
	bool isUTF32BE(const UINT8* buffer)
	{
		return buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFE && buffer[3] == 0xFF;
	}

	/** Checks does the provided buffer has an UTF16 byte order mark in little endian order. */
	bool isUTF16LE(const UINT8* buffer)
	{
		return buffer[0] == 0xFF && buffer[1] == 0xFE;
	}

	/**	Checks does the provided buffer has an UTF16 byte order mark in big endian order. */
	bool isUTF16BE(const UINT8* buffer)
	{
		return buffer[0] == 0xFE && buffer[1] == 0xFF;
	}

	/**	Checks does the provided buffer has an UTF8 byte order mark. */
	bool isUTF8(const UINT8* buffer)
	{
		return (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF);
	}

	template <typename T> DataStream& DataStream::operator>> (T& val)
	{
		read(static_cast<void*>(&val), sizeof(T));

		return *this;
	}

	void DataStream::writeString(const String& string, StringEncoding encoding)
	{
		if (encoding == StringEncoding::UTF16)
		{
			// Write BOM
			UINT8 bom[2] = { 0xFF, 0xFE };
			write(bom, sizeof(bom));

			U16String u16string = UTF8::toUTF16(string);
			write(u16string.data(), u16string.length() * sizeof(char16_t));
		}
		else
		{
			// Write BOM
			UINT8 bom[3] = { 0xEF, 0xBB, 0xBF };
			write(bom, sizeof(bom));

			write(string.data(), string.length());
		}
	}

	void DataStream::writeString(const WString& string, StringEncoding encoding)
	{
		if (encoding == StringEncoding::UTF16)
		{
			// Write BOM
			UINT8 bom[2] = { 0xFF, 0xFE };
			write(bom, sizeof(bom));

			String u8string = UTF8::fromWide(string);
			U16String u16string = UTF8::toUTF16(u8string);
			write(u16string.data(), u16string.length() * sizeof(char16_t));
		}
		else
		{
			// Write BOM
			UINT8 bom[3] = { 0xEF, 0xBB, 0xBF };
			write(bom, sizeof(bom));

			String u8string = UTF8::fromWide(string);
			write(u8string.data(), u8string.length());
		}
	}

	String DataStream::getAsString()
	{
		// Ensure read from begin of stream
		seek(0);

		// Try reading header
		UINT8 headerBytes[4];
		size_t numHeaderBytes = read(headerBytes, 4);

		size_t dataOffset = 0;
		if(numHeaderBytes >= 4)
		{
			if (isUTF32LE(headerBytes))
				dataOffset = 4;
			else if (isUTF32BE(headerBytes))
			{
				LOGWRN("UTF-32 big endian decoding not supported");
				return u8"";
			}
		}

		if(dataOffset == 0 && numHeaderBytes >= 3)
		{
			if (isUTF8(headerBytes))
				dataOffset = 3;
		}

		if(dataOffset == 0 && numHeaderBytes >= 2)
		{
			if (isUTF16LE(headerBytes))
				dataOffset = 2;
			else if (isUTF16BE(headerBytes))
			{
				LOGWRN("UTF-16 big endian decoding not supported");
				return u8"";
			}
		}

		seek(dataOffset);

		// Read the entire buffer - ideally in one read, but if the size of the buffer is unknown, do multiple fixed size
		// reads.
		size_t bufSize = (mSize > 0 ? mSize : 4096);
		auto tempBuffer = bs_stack_alloc<std::stringstream::char_type>((UINT32)mSize);

		std::stringstream result;
		while (!eof())
		{
			size_t numReadBytes = read(tempBuffer, bufSize);
			result.write(tempBuffer, numReadBytes);
		}

		bs_stack_free(tempBuffer);

		std::string string = result.str();

		switch(dataOffset)
		{
		default:
		case 0: // No BOM = assumed UTF-8
		case 3: // UTF-8
			return String(string.data(), string.length());
		case 2: // UTF-16
			{
			UINT32 numElems = (UINT32)string.length() / 2;

			return UTF8::fromUTF16(U16String((char16_t*)string.data(), numElems));
			}
		case 4: // UTF-32
			{
			UINT32 numElems = (UINT32)string.length() / 4;

			return UTF8::fromUTF32(U32String((char32_t*)string.data(), numElems));
			}
		}

		// Note: Never assuming ANSI as there is no ideal way to check for it. If required I need to
		// try reading the data and if all UTF encodings fail, assume it's ANSI. For now it should be
		// fine as most files are UTF-8 encoded.
	}

	WString DataStream::getAsWString()
	{
		String u8string = getAsString();

		return UTF8::toWide(u8string);
	}

	MemoryDataStream::MemoryDataStream(size_t size)
		: DataStream(READ | WRITE), mData(nullptr), mFreeOnClose(true)
	{
		mData = mPos = (UINT8*)bs_alloc((UINT32)size);
		mSize = size;
		mEnd = mData + mSize;

		assert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(void* memory, size_t inSize, bool freeOnClose)
		: DataStream(READ | WRITE), mData(nullptr), mFreeOnClose(freeOnClose)
	{
		mData = mPos = static_cast<UINT8*>(memory);
		mSize = inSize;
		mEnd = mData + mSize;

		assert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(DataStream& sourceStream)
		: DataStream(READ | WRITE), mData(nullptr)
	{
		// Copy data from incoming stream
		mSize = sourceStream.size();

		mData = (UINT8*)bs_alloc((UINT32)mSize);
		mPos = mData;
		mEnd = mData + sourceStream.read(mData, mSize);
		mFreeOnClose = true;

		assert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const SPtr<DataStream>& sourceStream)
		:DataStream(READ | WRITE), mData(nullptr)
	{
		// Copy data from incoming stream
		mSize = sourceStream->size();

		mData = (UINT8*)bs_alloc((UINT32)mSize);
		mPos = mData;
		mEnd = mData + sourceStream->read(mData, mSize);
		mFreeOnClose = true;

		assert(mEnd >= mPos);
	}

	MemoryDataStream::~MemoryDataStream()
	{
		close();
	}

	size_t MemoryDataStream::read(void* buf, size_t count)
	{
		size_t cnt = count;

		if (mPos + cnt > mEnd)
			cnt = mEnd - mPos;
		if (cnt == 0)
			return 0;

		assert (cnt <= count);

		memcpy(buf, mPos, cnt);
		mPos += cnt;

		return cnt;
	}

	size_t MemoryDataStream::write(const void* buf, size_t count)
	{
		size_t written = 0;
		if (isWriteable())
		{
			written = count;

			if (mPos + written > mEnd)
				written = mEnd - mPos;
			if (written == 0)
				return 0;

			memcpy(mPos, buf, written);
			mPos += written;
		}

		return written;
	}

	void MemoryDataStream::skip(size_t count)
	{
		size_t newpos = (size_t)( (mPos - mData) + count );
		assert(mData + newpos <= mEnd);

		mPos = mData + newpos;
	}

	void MemoryDataStream::seek(size_t pos)
	{
		assert(mData + pos <= mEnd);
		mPos = mData + pos;
	}

	size_t MemoryDataStream::tell() const
	{
		return mPos - mData;
	}

	bool MemoryDataStream::eof() const
	{
		return mPos >= mEnd;
	}

	SPtr<DataStream> MemoryDataStream::clone(bool copyData) const
	{
		if (!copyData)
			return bs_shared_ptr_new<MemoryDataStream>(mData, mSize, false);

		return bs_shared_ptr_new<MemoryDataStream>(*this);
	}

	void MemoryDataStream::close()
	{
		if (mData != nullptr)
		{
			if(mFreeOnClose)
				bs_free(mData);

			mData = nullptr;
		}
	}

	FileDataStream::FileDataStream(const Path& path, AccessMode accessMode, bool freeOnClose)
		: DataStream(accessMode), mPath(path), mFreeOnClose(freeOnClose)
	{
		// Always open in binary mode
		// Also, always include reading
		std::ios::openmode mode = std::ios::binary;

		if ((accessMode & READ) != 0)
			mode |= std::ios::in;

		if (((accessMode & WRITE) != 0))
		{
			mode |= std::ios::out;
			mFStream = bs_shared_ptr_new<std::fstream>();
			mFStream->open(path.toPlatformString().c_str(), mode);
			mInStream = mFStream;
		}
		else
		{
			mFStreamRO = bs_shared_ptr_new<std::ifstream>();
			mFStreamRO->open(path.toPlatformString().c_str(), mode);
			mInStream = mFStreamRO;
		}

		// Should check ensure open succeeded, in case fail for some reason.
		if (mInStream->fail())
		{
			LOGWRN("Cannot open file: " + path.toString());
			return;
		}

		mInStream->seekg(0, std::ios_base::end);
		mSize = (size_t)mInStream->tellg();
		mInStream->seekg(0, std::ios_base::beg);
	}

	FileDataStream::~FileDataStream()
	{
		close();
	}

	size_t FileDataStream::read(void* buf, size_t count)
	{
		mInStream->read(static_cast<char*>(buf), static_cast<std::streamsize>(count));

		return (size_t)mInStream->gcount();
	}

	size_t FileDataStream::write(const void* buf, size_t count)
	{
		size_t written = 0;
		if (isWriteable() && mFStream)
		{
			mFStream->write(static_cast<const char*>(buf), static_cast<std::streamsize>(count));
			written = count;
		}

		return written;
	}
	void FileDataStream::skip(size_t count)
	{
		mInStream->clear(); // Clear fail status in case eof was set
		mInStream->seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
	}

	void FileDataStream::seek(size_t pos)
	{
		mInStream->clear(); // Clear fail status in case eof was set
		mInStream->seekg(static_cast<std::streamoff>(pos), std::ios::beg);
	}

	size_t FileDataStream::tell() const
	{
		mInStream->clear(); // Clear fail status in case eof was set

		return (size_t)mInStream->tellg();
	}

	bool FileDataStream::eof() const
	{
		return mInStream->eof();
	}

	SPtr<DataStream> FileDataStream::clone(bool copyData) const
	{
		return bs_shared_ptr_new<FileDataStream>(mPath, (AccessMode)getAccessMode(), true);
	}

	void FileDataStream::close()
	{
		if (mInStream)
		{
			if (mFStreamRO)
				mFStreamRO->close();

			if (mFStream)
			{
				mFStream->flush();
				mFStream->close();
			}

			if (mFreeOnClose)
			{
				mInStream = nullptr;
				mFStreamRO = nullptr;
				mFStream = nullptr;
			}
		}
	}
}
