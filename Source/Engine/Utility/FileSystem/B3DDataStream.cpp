//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"
#include "String/B3DUnicode.h"
#include "Math/B3DMath.h"

using namespace b3d;

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
DataStream& DataStream::operator>>(T& value)
{
	Read(static_cast<void*>(&value), sizeof(T));

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
			B3D_LOG(Warning, LogGeneric, "UTF-32 big endian decoding not supported");
			return "";
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
			B3D_LOG(Warning, LogGeneric, "UTF-16 big endian decoding not supported");
			return "";
		}
	}

	Seek(dataOffset);

	// Read the entire buffer - ideally in one read, but if the size of the buffer is unknown, do multiple fixed size
	// reads.
	size_t bufferSize = (mSize > 0 ? mSize : 4096);
	auto tempBuffer = B3DStackAllocate<std::stringstream::char_type>((u32)bufferSize);

	std::stringstream result;
	while(!Eof())
	{
		size_t readByteCount = Read(tempBuffer, bufferSize);
		result.write(tempBuffer, readByteCount);
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
			u32 elementCount = (u32)string.length() / 2;

			return UTF8::FromUtF16(U16String((char16_t*)string.data(), elementCount));
		}
	case 4: // UTF-32
		{
			u32 elementCount = (u32)string.length() / 4;

			return UTF8::FromUtF32(U32String((char32_t*)string.data(), elementCount));
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

size_t DataStream::ReadBits(uint8_t* outData, uint32_t count)
{
	uint32_t byteCount = Math::DivideAndRoundUp(count, 8U);
	return Read(outData, byteCount) * 8;
}

size_t DataStream::WriteBits(const uint8_t* data, uint32_t count)
{
	uint32_t byteCount = Math::DivideAndRoundUp(count, 8U);
	return Write(data, byteCount) * 8;
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
	ReallocateBuffer(capacity);
	mCursor = mData;
	mEnd = mCursor + capacity;
}

MemoryDataStream::MemoryDataStream(void* memory, size_t size)
	: DataStream(READ | WRITE), mOwnsMemory(false)
{
	mData = mCursor = static_cast<uint8_t*>(memory);
	mSize = size;
	mCapacity = size;
	mEnd = mData + mSize;
}

MemoryDataStream::MemoryDataStream(const MemoryDataStream& sourceStream)
	: DataStream(READ | WRITE)
{
	// Copy data from incoming stream
	mSize = sourceStream.Size();
	mCapacity = mSize;

	mData = mCursor = static_cast<uint8_t*>(B3DAllocate(mCapacity));
	mEnd = mData + sourceStream.Read(mData, mSize);

	B3D_ASSERT(mEnd >= mCursor);
}

MemoryDataStream::MemoryDataStream(const SPtr<DataStream>& sourceStream)
	: DataStream(READ | WRITE)
{
	// Copy data from incoming stream
	mSize = sourceStream->Size();
	mCapacity = mSize;

	mData = mCursor = static_cast<uint8_t*>(B3DAllocate(mCapacity));
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
		this->mCapacity = other.mCapacity;
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
		mCapacity = 0;
		mData = nullptr;
		mCursor = nullptr;
		mEnd = nullptr;

		this->mOwnsMemory = true;

		ReallocateBuffer(other.mSize);
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
	this->mCapacity = std::exchange(other.mCapacity, 0);
	this->mOwnsMemory = std::exchange(other.mOwnsMemory, false);

	return *this;
}

size_t MemoryDataStream::Read(void* outData, size_t byteCount) const
{
	size_t actualByteCount = byteCount;

	if(mCursor + actualByteCount > mEnd)
		actualByteCount = mEnd - mCursor;

	if(actualByteCount == 0)
		return 0;

	B3D_ASSERT(actualByteCount <= byteCount);

	memcpy(outData, mCursor, actualByteCount);
	mCursor += actualByteCount;

	return actualByteCount;
}

size_t MemoryDataStream::Write(const void* data, size_t byteCount)
{
	const size_t byteCountToWrite = EnsureEnoughSpace(byteCount);
	if(byteCountToWrite == 0)
		return 0;

	memcpy(mCursor, data, byteCountToWrite);
	mCursor += byteCountToWrite;
	mEnd = Math::Max(mCursor, mEnd);
	mSize = mEnd - mData;

	return byteCountToWrite;
}

size_t MemoryDataStream::Skip(size_t byteCount)
{
	const size_t byteCountToSkip = EnsureEnoughSpace(byteCount);

	mCursor += byteCountToSkip;
	mEnd = Math::Max(mCursor, mEnd);
	mSize = mEnd - mData;

	return byteCountToSkip;
}

size_t MemoryDataStream::Seek(size_t position)
{
	i64 byteCountToSkip;
	if(position > mCapacity)
	{
		const size_t requiredExtraSpace = position - mCapacity;
		byteCountToSkip = (i64)EnsureEnoughSpace(requiredExtraSpace);
	}
	else
	{
		const size_t currentOffset = mCursor - mData;
		byteCountToSkip = (i64)position - (i64)currentOffset;
	}

	mCursor += byteCountToSkip;
	mEnd = Math::Max(mCursor, mEnd);
	mSize = mEnd - mData;

	return mCursor - mData;
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

size_t MemoryDataStream::EnsureEnoughSpace(size_t size)
{
	size_t availableByteCount = 0;
	if(IsWriteable())
	{
		availableByteCount = size;

		const size_t currentOffset = mCursor - mData;
		const size_t newOffset = currentOffset + size;
		if(newOffset > mCapacity)
		{
			if(mOwnsMemory)
				ReallocateBuffer(mCapacity + std::max(mCapacity, size));
			else
				availableByteCount = mCapacity - currentOffset;
		}
	}

	return availableByteCount;
}

void MemoryDataStream::ReallocateBuffer(size_t byteCount)
{
	if(byteCount <= mCapacity)
		return;

	B3D_ASSERT(byteCount > mSize);

	auto buffer = B3DAllocateMultiple<uint8_t>(byteCount);
	if(mData != nullptr)
	{
		mCursor = buffer + (mCursor - mData);
		mEnd = buffer + (mEnd - mData);

		memcpy(buffer, mData, mSize);
		B3DFree(mData);
	}
	else
	{
		mCursor = buffer;
		mCursor = buffer;
	}

	mData = buffer;
	mCapacity = byteCount;
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

size_t FileDataStream::Read(void* outData, size_t byteCount) const
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	const_cast<std::fstream&>(mFileStream).read(static_cast<char*>(outData), static_cast<std::streamsize>(byteCount));
	return (size_t)mFileStream.gcount();
}

size_t FileDataStream::Write(const void* data, size_t byteCount)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	size_t written = 0;
	if(B3D_ENSURE(IsWriteable()))
	{
		mFileStream.write(static_cast<const char*>(data), static_cast<std::streamsize>(byteCount));
		written = byteCount;
	}

	return written;
}

size_t FileDataStream::Skip(size_t count)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return 0;

	// Clear fail status in case eof was set
	if(mFileStream.eof() && !mFileStream.bad())
		mFileStream.clear();

	const size_t currentOffset = Tell();

	if(((mAccess & WRITE) != 0))
		mFileStream.seekp(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
	else
		mFileStream.seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);

	const size_t newOffset = Tell();
	return newOffset - currentOffset;
}

size_t FileDataStream::Seek(size_t pos)
{
	if(!B3D_ENSURE(mFileStream.is_open()))
		return Tell();

	// Clear fail status in case eof was set
	if(mFileStream.eof() && !mFileStream.bad())
		mFileStream.clear();

	if(((mAccess & READ) != 0))
		mFileStream.seekg(static_cast<std::streamoff>(pos), std::ios::beg);

	if(((mAccess & WRITE) != 0))
		mFileStream.seekp(static_cast<std::ifstream::pos_type>(pos), std::ios::beg);

	return Tell();
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
