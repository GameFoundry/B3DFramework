//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DFileSerializer.h"

#include "Reflection/B3DIReflectable.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"
#include <numeric>

using namespace b3d;

FileEncoder::FileEncoder(const Path& fileLocation)
{
	Path parentDir = fileLocation.GetDirectory();
	if(!FileSystem::Exists(parentDir))
		FileSystem::CreateFolder(parentDir);

	mOutputStream = FileSystem::CreateAndOpenFile(fileLocation);
}

FileEncoder::FileEncoder(const TShared<DataStream>& stream)
	: mOutputStream(stream)
{ }


void FileEncoder::Encode(IReflectable* object, RTTIOperationContext& context)
{
	if(object == nullptr)
		return;

	size_t startPos = mOutputStream->Tell();
	mOutputStream->Skip(sizeof(u32));

	BinarySerializer bs;
	bs.Encode(object, mOutputStream, context);

	size_t endPos = mOutputStream->Tell();
	auto size = (u32)(endPos - startPos - sizeof(u32));

	mOutputStream->Seek(startPos);
	mOutputStream->Write((char*)&size, sizeof(size));
	mOutputStream->Skip(size);
}

void FileEncoder::Encode(IReflectable* object)
{
	RTTIOperationContext rttiOperationContext;
	Encode(object, rttiOperationContext);
}

FileDecoder::FileDecoder(const Path& fileLocation)
{
	mInputStream = FileSystem::OpenFile(fileLocation);

	if(mInputStream == nullptr)
		return;

	if(mInputStream->Size() > std::numeric_limits<u32>::max())
		B3D_LOG(Fatal, LogSerialization, "File size is larger that u32 can hold. Ask a programmer to use a bigger data type.");
}

FileDecoder::FileDecoder(const TShared<DataStream>& stream)
	: mInputStream(stream)
{ }

TShared<IReflectable> FileDecoder::Decode(RTTIOperationContext& context)
{
	if(mInputStream->Eof())
		return nullptr;

	u32 objectSize = 0;
	mInputStream->Read(&objectSize, sizeof(objectSize));

	BinarySerializer bs;
	TShared<IReflectable> object = bs.Decode(mInputStream, objectSize, context);

	return object;
}

TShared<IReflectable> FileDecoder::Decode()
{
	RTTIOperationContext rttiOperationContext;
	return Decode(rttiOperationContext);
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
