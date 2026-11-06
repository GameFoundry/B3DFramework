//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudioClip.h"
#include "Resources/B3DResources.h"
#include "Audio/B3DAudio.h"
#include "RTTI/B3DAudioClipRTTI.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

AudioClip::AudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
	: Resource(false), mInformation(createInformation), mSampleCount(sampleCount), mStreamSize(streamSize), mStreamData(samples)
{
	if(samples != nullptr)
		mStreamOffset = (u32)samples->Tell();

	mKeepSourceData = createInformation.KeepSourceData;
}

void AudioClip::Initialize()
{
	mLength = mSampleCount / mInformation.ChannelCount / (float)mInformation.Frequency;

	Resource::Initialize();
}

HAudioClip AudioClip::Create(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	return B3DStaticResourceCast<AudioClip>(GetResources().CreateResourceHandle(CreateShared(samples, streamSize, sampleCount, createInformation)));
}

TShared<AudioClip> AudioClip::CreateShared(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	TShared<AudioClip> newClip = GetAudio().CreateClip(samples, streamSize, sampleCount, createInformation);
	newClip->SetShared(newClip);
	newClip->Initialize();

	return newClip;
}

void AudioClip::OnPackageFileWillChange(const Path& filePath)
{
	if(mPackageFilePath != filePath)
		return;

	mIsPackageFileChanging = true;

	if(mStreamData == nullptr || !mStreamData->IsFile())
		return;

	const TShared<MemoryDataStream> memoryStream = B3DMakeShared<MemoryDataStream>(mStreamSize);

	mStreamData->Seek(mStreamOffset);
	if(mStreamData->Read(memoryStream->Data(), mStreamSize) != mStreamSize)
	{
		B3D_LOG(Error, LogAudio, "Failed to read audio clip '{0}' into memory. The clip keeps reading from its file.", GetName());
		return;
	}

	mStreamData = memoryStream;
	mStreamOffset = 0;
}

void AudioClip::OnPackageFileDidChange(const Path& filePath, const TOptional<i64>& dataOffsetChange)
{
	if(!mIsPackageFileChanging)
		return;

	mIsPackageFileChanging = false;

	if(!dataOffsetChange.has_value())
	{
		mPackageFilePath = Path::kBlank;
		return;
	}

	mPackageFilePath = filePath;
	mPackageFileOffset = (u32)(mPackageFileOffset + *dataOffsetChange);

	if(mInformation.ReadMode != AudioReadMode::Stream || mStreamData == nullptr)
		return;

	// Back to streaming from the file, unless it cannot be opened in which case the clip remains usable from memory
	const TShared<DataStream> fileStream = FileSystem::OpenFile(mPackageFilePath, FileAccessFlag::Read | FileAccessFlag::Async);
	if(fileStream == nullptr)
		return;

	mStreamData = fileStream;
	mStreamOffset = mPackageFileOffset;
}

TShared<DataStream> AudioClip::OpenStoredData(u32& outSize) const
{
	outSize = 0;

	TShared<DataStream> stream;
	u32 offset = 0;
	u32 size = 0;

	if(mStreamData != nullptr)
	{
		// Not handing out the clip's own stream, as playback might move its read position at any time
		stream = mStreamData->Clone();
		offset = mStreamOffset;
		size = mStreamSize;
	}
	else if(!mPackageFilePath.IsEmpty() && !mIsPackageFileChanging)
	{
		stream = FileSystem::OpenFile(mPackageFilePath);
		offset = mPackageFileOffset;
		size = mPackageFileDataSize;
	}

	// A file stream that failed to open reports a size of zero
	if(stream == nullptr || stream->Size() < (size_t)offset + size)
		return nullptr;

	stream->Seek(offset);
	outSize = size;

	return stream;
}

TShared<AudioClip> AudioClip::CreateEmpty()
{
	AudioClipCreateInformation createInformation;

	TShared<AudioClip> newClip = GetAudio().CreateClip(nullptr, 0, 0, createInformation);
	newClip->SetShared(newClip);

	return newClip;
}

RTTIType* AudioClip::GetRttiStatic()
{
	return AudioClipRTTI::Instance();
}

RTTIType* AudioClip::GetRtti() const
{
	return GetRttiStatic();
}
