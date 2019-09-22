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
		Path parentDir = fileLocation.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);
		
		mOutputStream = FileSystem::createAndOpenFile(fileLocation);
	}

	void FileEncoder::encode(IReflectable* object, SerializationContext* context)
	{
		if (object == nullptr)
			return;

		size_t startPos = mOutputStream->tell();
		mOutputStream->skip(sizeof(UINT32));

		BinarySerializer bs;
		bs.encode(object, mOutputStream, BinarySerializerFlag::None, context);

		size_t endPos = mOutputStream->tell();
		auto size = (UINT32)(endPos - startPos - sizeof(UINT32));
		
		mOutputStream->seek(startPos);
		mOutputStream->write((char*)&size, sizeof(size));
		mOutputStream->skip(size);
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
		SPtr<IReflectable> object = bs.decode(mInputStream, objectSize, BinarySerializerFlag::None, context);

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
