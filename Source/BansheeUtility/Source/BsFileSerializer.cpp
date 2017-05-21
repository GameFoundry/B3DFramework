//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFileSerializer.h"

#include "BsException.h"
#include "BsIReflectable.h"
#include "BsBinarySerializer.h"
#include "BsFileSystem.h"
#include "BsDataStream.h"
#include "BsDebug.h"
#include <numeric>

using namespace std::placeholders;

namespace bs
{
	FileEncoder::FileEncoder(const Path& fileLocation)
		:mWriteBuffer(nullptr)
	{
		mWriteBuffer = (UINT8*)bs_alloc(WRITE_BUFFER_SIZE);

		Path parentDir = fileLocation.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);
		
		mOutputStream.open(fileLocation.toPlatformString().c_str(), std::ios::out | std::ios::binary);
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

	void FileEncoder::encode(IReflectable* object, const UnorderedMap<String, UINT64>& params)
	{
		if (object == nullptr)
			return;

		UINT64 curPos = (UINT64)mOutputStream.tellp();
		mOutputStream.seekp(sizeof(UINT32), std::ios_base::cur);

		BinarySerializer bs;
		UINT32 totalBytesWritten = 0;
		bs.encode(object, mWriteBuffer, WRITE_BUFFER_SIZE, &totalBytesWritten, 
			std::bind(&FileEncoder::flushBuffer, this, _1, _2, _3), false, params);

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
		mInputStream = FileSystem::openFile(fileLocation, true);

		if (mInputStream == nullptr)
			return;

		if (mInputStream->size() > std::numeric_limits<UINT32>::max())
		{
			BS_EXCEPT(InternalErrorException,
				"File size is larger that UINT32 can hold. Ask a programmer to use a bigger data type.");
		}
	}

	SPtr<IReflectable> FileDecoder::decode(const UnorderedMap<String, UINT64>& params)
	{
		if (mInputStream->eof())
			return nullptr;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));

		BinarySerializer bs;
		SPtr<IReflectable> object = bs.decode(mInputStream, objectSize, params);

		return object;
	}

	void FileDecoder::skip()
	{
		if (mInputStream->eof())
			return;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));
		mInputStream->skip(objectSize);
	}
}
