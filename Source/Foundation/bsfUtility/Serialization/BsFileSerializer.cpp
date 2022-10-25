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

using namespace bs;

FileEncoder::FileEncoder(const Path& fileLocation)
{
	Path parentDir = fileLocation.GetDirectory();
	if(!FileSystem::Exists(parentDir))
		FileSystem::CreateDir(parentDir);

	mOutputStream = FileSystem::CreateAndOpenFile(fileLocation);
}

void FileEncoder::Encode(IReflectable* object, SerializationContext* context)
{
	if(object == nullptr)
		return;

	size_t startPos = mOutputStream->Tell();
	mOutputStream->Skip(sizeof(u32));

	BinarySerializer bs;
	bs.Encode(object, mOutputStream, BinarySerializerFlag::None, context);

	size_t endPos = mOutputStream->Tell();
	auto size = (u32)(endPos - startPos - sizeof(u32));

	mOutputStream->Seek(startPos);
	mOutputStream->Write((char*)&size, sizeof(size));
	mOutputStream->Skip(size);
}

FileDecoder::FileDecoder(const Path& fileLocation)
{
	mInputStream = FileSystem::OpenFile(fileLocation, true);

	if(mInputStream == nullptr)
		return;

	if(mInputStream->Size() > std::numeric_limits<u32>::max())
	{
		BS_EXCEPT(InternalErrorException, "File size is larger that u32 can hold. Ask a programmer to use a bigger data type.");
	}
}

SPtr<IReflectable> FileDecoder::Decode(SerializationContext* context)
{
	if(mInputStream->Eof())
		return nullptr;

	u32 objectSize = 0;
	mInputStream->Read(&objectSize, sizeof(objectSize));

	BinarySerializer bs;
	SPtr<IReflectable> object = bs.Decode(mInputStream, objectSize, BinarySerializerFlag::None, context);

	return object;
}

u32 FileDecoder::GetSize() const
{
	if(mInputStream->Eof())
		return 0;

	u32 objectSize = 0;
	mInputStream->Read(&objectSize, sizeof(objectSize));
	mInputStream->Seek(mInputStream->Tell() - sizeof(objectSize));

	return objectSize;
}

void FileDecoder::Skip()
{
	if(mInputStream->Eof())
		return;

	u32 objectSize = 0;
	mInputStream->Read(&objectSize, sizeof(objectSize));
	mInputStream->Skip(objectSize);
}
