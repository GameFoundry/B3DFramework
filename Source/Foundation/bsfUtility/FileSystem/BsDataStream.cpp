//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsDataStream.h"
#include "Debug/BsDebug.h"
#include "String/BsUnicode.h"
#include "Math/BsMath.h"

using namespace bs;

const u32 DataStream::kStreamTempSize = 128;

/** Checks does the provided buffer has an UTF32 byte order mark in little endian order. */
bool IsUtF32Le(const u8* buffer)
{
	return buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00;
}

/** Checks does the provided buffer has an UTF32 byte order mark in big endian order. */
bool IsUtF32Be(const u8* buffer)
{
	return buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFE && buffer[3] == 0xFF;
}

/** Checks does the provided buffer has an UTF16 byte order mark in little endian order. */
bool IsUtF16Le(const u8* buffer)
{
	return buffer[0] == 0xFF && buffer[1] == 0xFE;
}

/**	Checks does the provided buffer has an UTF16 byte order mark in big endian order. */
bool IsUtF16Be(const u8* buffer)
{
	return buffer[0] == 0xFE && buffer[1] == 0xFF;
}

/**	Checks does the provided buffer has an UTF8 byte order mark. */
bool IsUtF8(const u8* buffer)
{
	return (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF);
}

template <typename T>
DataStream& DataStream::operator>>(T& val)
{
	read(static_cast<void*>(&val), sizeof(T));

	return *this;
}

void DataStream::WriteString(const String& string, StringEncoding encoding)
{
	if(encoding == StringEncoding::UTF16)
	{
		// Write BOM
		u8 bom[2] = { 0xFF, 0xFE };
		Write(bom, sizeof(bom));

		U16String u16string = UTF8::ToUtF16(string);
		Write(u16string.data(), u16string.length() * sizeof(char16_t));
	}
	else
	{
		// Write BOM
		u8 bom[3] = { 0xEF, 0xBB, 0xBF };
		Write(bom, sizeof(bom));

		Write(string.data(), string.length());
	}
}

void DataStream::WriteString(const WString& string, StringEncoding encoding)
{
	if(encoding == StringEncoding::UTF16)
	{
		// Write BOM
		u8 bom[2] = { 0xFF, 0xFE };
		Write(bom, sizeof(bom));

		String u8string = UTF8::FromWide(string);
		U16String u16string = UTF8::ToUtF16(u8string);
		Write(u16string.data(), u16string.length() * sizeof(char16_t));
	}
	else
	{
		// Write BOM
		u8 bom[3] = { 0xEF, 0xBB, 0xBF };
		Write(bom, sizeof(bom));

		String u8string = UTF8::FromWide(string);
		Write(u8string.data(), u8string.length());
	}
}

String DataStream::GetAsString()
{
	// Ensure read from begin of stream
	Seek(0);

	// Try reading header
	u8 headerBytes[4];
	size_t numHeaderBytes = Read(headerBytes, 4);

	size_t dataOffset = 0;
	if(numHeaderBytes >= 4)
	{
		if(IsUtF32Le(headerBytes))
			dataOffset = 4;
		else if(IsUtF32Be(headerBytes))
		{
			B3D_LOG(Warning, Generic, "UTF-32 big endian decoding not supported");
			return u8"";
		}
	}

	if(dataOffset == 0 && numHeaderBytes >= 3)
	{
		if(IsUtF8(headerBytes))
			dataOffset = 3;
	}

	if(dataOffset == 0 && numHeaderBytes >= 2)
	{
		if(IsUtF16Le(headerBytes))
			dataOffset = 2;
		else if(IsUtF16Be(headerBytes))
		{
			B3D_LOG(Warning, Generic, "UTF-16 big endian decoding not supported");
			return u8"";
		}
	}

	Seek(dataOffset);

	// Read the entire buffer - ideally in one read, but if the size of the buffer is unknown, do multiple fixed size
	// reads.
	size_t bufSize = (mSize > 0 ? mSize : 4096);
	auto tempBuffer = B3DStackAllocate<std::stringstream::char_type>((u32)bufSize);

	std::stringstream result;
	while(!Eof())
	{
		size_t numReadBytes = Read(tempBuffer, bufSize);
		result.write(tempBuffer, numReadBytes);
	}

	B3DStackFree(tempBuffer);

	std::string string = result.str();

	switch(dataOffset)
	{
	default:
	case 0: // No BOM = assumed UTF-8
	case 3: // UTF-8
		return String(string.data(), string.length());
	case 2: // UTF-16
		{
			u32 numElems = (u32)string.length() / 2;

			return UTF8::FromUtF16(U16String((char16_t*)string.data(), numElems));
		}
	case 4: // UTF-32
		{
			u32 numElems = (u32)string.length() / 4;

			return UTF8::FromUtF32(U32String((char32_t*)string.data(), numElems));
		}
	}

	// Note: Never assuming ANSI as there is no ideal way to check for it. If required I need to
	// try reading the data and if all UTF encodings fail, assume it's ANSI. For now it should be
	// fine as most files are UTF-8 encoded.
}

WString DataStream::GetAsWString()
{
	String u8string = GetAsString();

	return UTF8::ToWide(u8string);
}

size_t DataStream::ReadBits(uint8_t* data, uint32_t count)
{
	uint32_t numBytes = Math::DivideAndRoundUp(count, 8U);
	return Read(data, numBytes) * 8;
}

size_t DataStream::WriteBits(const uint8_t* data, uint32_t count)
{
	uint32_t numBytes = Math::DivideAndRoundUp(count, 8U);
	return Write(data, numBytes) * 8;
}

void DataStream::Align(uint32_t count)
{
	if(count <= 1)
		return;

	u32 alignOffset = (count - (Tell() & (count - 1))) & (count - 1);
	Skip(alignOffset);
}

MemoryDataStream::MemoryDataStream()
	: DataStream(READ | WRITE)
{
}

MemoryDataStream::MemoryDataStream(size_t capacity)
	: DataStream(READ | WRITE)
{
	Realloc(capacity);
	mCursor = mData;
	mEnd = mCursor + capacity;
}

MemoryDataStream::MemoryDataStream(void* memory, size_t size)
	: DataStream(READ | WRITE), mOwnsMemory(false)
{
	mData = mCursor = static_cast<uint8_t*>(memory);
	mSize = size;
	mEnd = mData + mSize;
}

MemoryDataStream::MemoryDataStream(const MemoryDataStream& sourceStream)
	: DataStream(READ | WRITE)
{
	// Copy data from incoming stream
	mSize = sourceStream.Size();

	mData = mCursor = static_cast<uint8_t*>(B3DAllocate(mSize));
	mEnd = mData + sourceStream.Read(mData, mSize);

	B3D_ASSERT(mEnd >= mCursor);
}

MemoryDataStream::MemoryDataStream(const SPtr<DataStream>& sourceStream)
	: DataStream(READ | WRITE)
{
	// Copy data from incoming stream
	mSize = sourceStream->Size();

	mData = mCursor = static_cast<uint8_t*>(B3DAllocate(mSize));
	mEnd = mData + sourceStream->Read(mData, mSize);

	B3D_ASSERT(mEnd >= mCursor);
}

MemoryDataStream::MemoryDataStream(MemoryDataStream&& other)
{
	*this = std::move(other);
}

MemoryDataStream::~MemoryDataStream()
{
	Close();
}

MemoryDataStream& MemoryDataStream::operator=(const MemoryDataStream& other)
{
	if(this == &other)
		return *this;

	this->mName = other.mName;
	this->mAccess = other.mAccess;

	if(!other.mOwnsMemory)
	{
		this->mSize = other.mSize;
		this->mData = other.mData;
		this->mCursor = other.mCursor;
		this->mEnd = other.mEnd;
		this->mOwnsMemory = false;
	}
	else
	{
		if(mData && mOwnsMemory)
			B3DFree(mData);

		mSize = 0;
		mData = nullptr;
		mCursor = nullptr;
		mEnd = nullptr;

		this->mOwnsMemory = true;

		Realloc(other.mSize);
		mEnd = mData + mSize;
		mCursor = mData + (other.mCursor - other.mData);

		if(mSize > 0)
			memcpy(mData, other.mData, mSize);
	}

	return *this;
}

MemoryDataStream& MemoryDataStream::operator=(MemoryDataStream&& other)
{
	if(this == &other)
		return *this;

	if(mData && mOwnsMemory)
		B3DFree(mData);

	this->mName = std::move(other.mName);
	this->mAccess = std::exchange(other.mAccess, 0);
	this->mCursor = std::exchange(other.mCursor, nullptr);
	this->mEnd = std::exchange(other.mEnd, nullptr);
	this->mData = std::exchange(other.mData, nullptr);
	this->mSize = std::exchange(other.mSize, 0);
	this->mOwnsMemory = std::exchange(other.mOwnsMemory, false);

	return *this;
}

size_t MemoryDataStream::Read(void* buf, size_t count) const
{
	size_t cnt = count;

	if(mCursor + cnt > mEnd)
		cnt = mEnd - mCursor;

	if(cnt == 0)
		return 0;

	B3D_ASSERT(cnt <= count);

	memcpy(buf, mCursor, cnt);
	mCursor += cnt;

	return cnt;
}

size_t MemoryDataStream::Write(const void* buf, size_t count)
{
	size_t written = 0;
	if(IsWriteable())
	{
		written = count;

		size_t numUsedBytes = (mCursor - mData);
		size_t newSize = numUsedBytes + count;
		if(newSize > mSize)
		{
			if(mOwnsMemory)
				Realloc(newSize);
			else
				written = mSize - numUsedBytes;
		}

		if(written == 0)
			return 0;

		memcpy(mCursor, buf, written);
		mCursor += written;

		mEnd = std::max(mCursor, mEnd);
	}

	return written;
}

void MemoryDataStream::Skip(size_t count)
{
	B3D_ASSERT((mCursor + count) <= mEnd);
	mCursor = std::min(mCursor + count, mEnd);
}

void MemoryDataStream::Seek(size_t pos)
{
	B3D_ASSERT((mData + pos) <= mEnd);
	mCursor = std::min(mData + pos, mEnd);
}

size_t MemoryDataStream::Tell() const
{
	return mCursor - mData;
}

bool MemoryDataStream::Eof() const
{
	return mCursor >= mEnd;
}

SPtr<DataStream> MemoryDataStream::Clone(bool copyData) const
{
	if(!copyData)
		return B3DMakeShared<MemoryDataStream>(mData, mSize);

	return B3DMakeShared<MemoryDataStream>(*this);
}

bool MemoryDataStream::Close()
{
	if(mData != nullptr)
	{
		if(mOwnsMemory)
			B3DFree(mData);

		mData = nullptr;
	}

	return true;
}

void MemoryDataStream::Realloc(size_t numBytes)
{
	if(numBytes != mSize)
	{
		B3D_ASSERT(numBytes > mSize);

		// Note: Eventually add support for custom allocators
		auto buffer = B3DAllocateMultiple<uint8_t>(numBytes);
		if(mData)
		{
			mCursor = buffer + (mCursor - mData);
			mEnd = buffer + (mEnd - mData);

			memcpy(buffer, mData, mSize);
			B3DFree(mData);
		}
		else
		{
			mCursor = buffer;
			mEnd = buffer;
		}

		mData = buffer;
		mSize = numBytes;
	}
}

FileDataStream::FileDataStream(const Path& path, AccessMode accessMode)
	: DataStream(accessMode), mPath(path)
{ }

FileDataStream::~FileDataStream()
{
	if(!mFileStream.is_open())
		return;

	Close();
}

bool FileDataStream::Open()
{
	// Always open in binary mode
	std::ios::openmode mode = std::ios::binary;

	if((mAccess & READ) != 0)
		mode |= std::ios::in;

	if(((mAccess & WRITE) != 0))
		mode |= std::ios::out;

	mFileStream.open(mPath.ToPlatformString().c_str(), mode);

	if(mFileStream.fail())
		return false;

	mFileStream.seekg(0, std::ios_base::end);
	mSize = (size_t)mFileStream.tellg();
	mFileStream.seekg(0, std::ios_base::beg);

	return true;
}

size_t FileDataStream::Read(void* buf, size_t count) const
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	const_cast<std::fstream&>(mFileStream).read(static_cast<char*>(buf), static_cast<std::streamsize>(count));
	return (size_t)mFileStream.gcount();
}

size_t FileDataStream::Write(const void* buf, size_t count)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	size_t written = 0;
	if(B3D_ENSURE(IsWriteable()))
	{
		mFileStream.write(static_cast<const char*>(buf), static_cast<std::streamsize>(count));
		written = count;
	}

	return written;
}

void FileDataStream::Skip(size_t count)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return;

	// Clear fail status in case eof was set
	if(mFileStream.eof() && !mFileStream.bad())
		mFileStream.clear();

	if(((mAccess & READ) != 0))
		mFileStream.seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);

	if(((mAccess & WRITE) != 0))
		mFileStream.seekp(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
}

void FileDataStream::Seek(size_t pos)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return;

	// Clear fail status in case eof was set
	if(mFileStream.eof() && !mFileStream.bad())
		mFileStream.clear();

	if(((mAccess & READ) != 0))
		mFileStream.seekg(static_cast<std::streamoff>(pos), std::ios::beg);

	if(((mAccess & WRITE) != 0))
		mFileStream.seekp(static_cast<std::ifstream::pos_type>(pos), std::ios::beg);
}

size_t FileDataStream::Tell() const
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	// Clear fail status in case eof was set
	if(mFileStream.eof() && !mFileStream.bad())
		const_cast<std::fstream&>(mFileStream).clear();

	if(((mAccess & WRITE) != 0))
		return (size_t)const_cast<std::fstream&>(mFileStream).tellp();

	return (size_t)const_cast<std::fstream&>(mFileStream).tellg();
}

bool FileDataStream::Eof() const
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return false;

	return mFileStream.eof();
}

SPtr<DataStream> FileDataStream::Clone(bool copyData) const
{
	return B3DMakeShared<FileDataStream>(mPath, (AccessMode)GetAccessMode());
}

bool FileDataStream::Flush()
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return false;

	mFileStream.flush();
	return !mFileStream.bad();
}


bool FileDataStream::Close()
{
	const bool flushResult = Flush();
	if(mFileStream.is_open())
		mFileStream.close();

	return flushResult;
}
