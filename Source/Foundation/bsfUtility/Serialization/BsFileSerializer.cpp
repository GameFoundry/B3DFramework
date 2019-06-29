//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsFileSerializer.h"

#include "Error/BsException.h"
#include "Reflection/BsIReflectable.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include "Debug/BsDebug.h"
#include <numeric>

using namespace std::placeholders;

namespace bs
{
	FileEncoder::FileEncoder(const Path& fileLocation)
	{
		mWriteBuffer = (UINT8*)bs_alloc(WRITE_BUFFER_SIZE);

		Path parentDir = fileLocation.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);
		
		mOutputStream.open(fileLocation.toPlatformString().c_str(), std::ios::out | std::ios::binary);
		if (mOutputStream.fail())
		{
			BS_LOG(Warning, FileSystem, "Failed to save file: \"" + fileLocation.toString() + "\". Error: " + strerror(errno) + ".");
		}
	}

	FileEncoder::~FileEncoder()
	{
		bs_free(mWriteBuffer);

		mOutputStream.close();
		mOutputStream.clear();
	}

	void FileEncoder::encode(IReflectable* object, SerializationContext* context)
	{
		if (object == nullptr)
			return;

		UINT64 curPos = (UINT64)mOutputStream.tellp();
		mOutputStream.seekp(sizeof(UINT32), std::ios_base::cur);

		BinarySerializer bs;
		UINT32 totalBytesWritten = 0;
		bs.encode(object, mWriteBuffer, WRITE_BUFFER_SIZE, &totalBytesWritten, 
			std::bind(&FileEncoder::flushBuffer, this, _1, _2, _3), false, context);

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

	SPtr<IReflectable> FileDecoder::decode(SerializationContext* context)
	{
		if (mInputStream->eof())
			return nullptr;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));

		BinarySerializer bs;
		SPtr<IReflectable> object = bs.decode(mInputStream, objectSize, context);

		return object;
	}

	UINT32 FileDecoder::getSize() const
	{
		if (mInputStream->eof())
			return 0;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));
		mInputStream->seek(mInputStream->tell() - sizeof(objectSize));

		return objectSize;
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
