//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFileSerializer.h"

#include "BsException.h"
#include "BsIReflectable.h"
#include "BsBinarySerializer.h"
#include "BsFileSystem.h"
#include "BsDebug.h"
#include <numeric>

using namespace std::placeholders;

namespace BansheeEngine
{
	FileEncoder::FileEncoder(const Path& fileLocation)
		:mWriteBuffer(nullptr)
	{
		mWriteBuffer = (UINT8*)bs_alloc(WRITE_BUFFER_SIZE);

		Path parentDir = fileLocation.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);

		mOutputStream.open(fileLocation.toWString().c_str(), std::ios::out | std::ios::binary);
		if (mOutputStream.fail())
		{
			LOGWRN("Failed to save file: \"" + fileLocation.toString() + "\". Error: " + strerror(errno) + ".");
		}
	}

	FileEncoder::~FileEncoder()
	{
		bs_free(mWriteBuffer);

		mOutputStream.close();
		mOutputStream.clear();
	}

	void FileEncoder::encode(IReflectable* object)
	{
		if (object == nullptr)
			return;

		UINT64 curPos = (UINT64)mOutputStream.tellp();
		mOutputStream.seekp(sizeof(UINT32), std::ios_base::cur);

		BinarySerializer bs;
		UINT32 totalBytesWritten = 0;
		bs.encode(object, mWriteBuffer, WRITE_BUFFER_SIZE, &totalBytesWritten, std::bind(&FileEncoder::flushBuffer, this, _1, _2, _3));

		mOutputStream.seekp(curPos);
		mOutputStream.write((char*)&totalBytesWritten, sizeof(totalBytesWritten));
		mOutputStream.seekp(totalBytesWritten, std::ios_base::cur);
	}

	UINT8* FileEncoder::flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize)
	{
		mOutputStream.write((const char*)bufferStart, bytesWritten);

		return bufferStart;
	}

	FileDecoder::FileDecoder(const Path& fileLocation)
	{
		mInputStream.open(fileLocation.toWString().c_str(), std::ios::in | std::ios::ate | std::ios::binary);

		if (mInputStream.fail())
		{
			LOGWRN("Failed to open file: \"" + fileLocation.toString() + "\". Error: " + strerror(errno) + ".");
		}

		std::streamoff fileSize = mInputStream.tellg();
		if (fileSize > std::numeric_limits<UINT32>::max())
		{
			BS_EXCEPT(InternalErrorException,
				"File size is larger that UINT32 can hold. Ask a programmer to use a bigger data type.");
		}

		mInputStream.seekg(0, std::ios::beg);
	}

	FileDecoder::~FileDecoder()
	{
		mInputStream.close();
		mInputStream.clear();
	}

	SPtr<IReflectable> FileDecoder::decode()
	{
		if (mInputStream.fail() || mInputStream.eof())
			return nullptr;

		UINT32 objectSize = 0;
		mInputStream.read((char*)&objectSize, sizeof(objectSize));

		UINT8* readBuffer = (UINT8*)bs_alloc((UINT32)objectSize); // TODO - Low priority. Consider upgrading BinarySerializer so we don't have to read everything at once
		mInputStream.read((char*)readBuffer, objectSize);

		BinarySerializer bs;
		SPtr<IReflectable> object = bs.decode(readBuffer, objectSize);

		bs_free(readBuffer);

		return object;
	}

	void FileDecoder::skip()
	{
		if (mInputStream.eof())
			return;

		UINT32 objectSize = 0;
		mInputStream.read((char*)&objectSize, sizeof(objectSize));

		mInputStream.seekg(objectSize, std::ios::cur);
	}
}