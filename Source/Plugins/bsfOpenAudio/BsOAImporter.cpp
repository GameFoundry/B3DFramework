//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAImporter.h"
#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"
#include "BsWaveDecoder.h"
#include "BsFLACDecoder.h"
#include "BsOggVorbisDecoder.h"
#include "BsOggVorbisEncoder.h"
#include "Audio/BsAudioClipImportOptions.h"
#include "Audio/BsAudioUtility.h"

using namespace b3d;

OAImporter::OAImporter()
	: SpecificImporter()
{
}

bool OAImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtil::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"wav" || lowerCaseExt == u8"flac" || lowerCaseExt == u8"ogg";
}

bool OAImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	// Don't check for magic number, rely on extension
	return true;
}

SPtr<ImportOptions> OAImporter::CreateImportOptions() const
{
	return B3DMakeShared<AudioClipImportOptions>();
}

SPtr<Resource> OAImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	AudioDataInfo info;
	u32 bytesPerSample;
	u32 bufferSize;
	SPtr<MemoryDataStream> sampleStream;
	{
		Lock fileLock = FileScheduler::GetLock(filePath);
		SPtr<DataStream> stream = FileSystem::OpenFile(filePath);

		String extension = filePath.GetExtension();
		StringUtil::ToLowerCase(extension);

		UPtr<AudioDecoder> reader;
		if(extension == u8".wav")
			reader = B3DMakeUnique<WaveDecoder>();
		else if(extension == u8".flac")
			reader = B3DMakeUnique<FLACDecoder>();
		else if(extension == u8".ogg")
			reader = B3DMakeUnique<OggVorbisDecoder>();

		if(reader == nullptr)
			return nullptr;

		if(!reader->IsValid(stream))
			return nullptr;

		if(!reader->Open(stream, info))
			return nullptr;

		bytesPerSample = info.BitDepth / 8;
		bufferSize = info.SampleCount * bytesPerSample;

		sampleStream = B3DMakeShared<MemoryDataStream>(bufferSize);
		reader->Read(sampleStream->Data(), info.SampleCount);
	}

	SPtr<const AudioClipImportOptions> clipIO = std::static_pointer_cast<const AudioClipImportOptions>(importOptions);

	// If 3D, convert to mono
	if(clipIO->Is3D && info.ChannelCount > 1)
	{
		u32 numSamplesPerChannel = info.SampleCount / info.ChannelCount;

		u32 monoBufferSize = numSamplesPerChannel * bytesPerSample;
		auto monoStream = B3DMakeShared<MemoryDataStream>(monoBufferSize);

		AudioUtility::ConvertToMono(sampleStream->Data(), monoStream->Data(), info.BitDepth, numSamplesPerChannel, info.ChannelCount);

		info.SampleCount = numSamplesPerChannel;
		info.ChannelCount = 1;

		sampleStream = monoStream;
		bufferSize = monoBufferSize;
	}

	// Convert bit depth if needed
	if(clipIO->BitDepth != info.BitDepth)
	{
		u32 outBufferSize = info.SampleCount * (clipIO->BitDepth / 8);
		auto outStream = B3DMakeShared<MemoryDataStream>(outBufferSize);

		AudioUtility::ConvertBitDepth(sampleStream->Data(), info.BitDepth, outStream->Data(), clipIO->BitDepth, info.SampleCount);

		info.BitDepth = clipIO->BitDepth;

		sampleStream = outStream;
		bufferSize = outBufferSize;
	}

	// Encode to Ogg Vorbis if needed
	if(clipIO->Format == AudioFormat::VORBIS)
	{
		// Note: If the original source was in Ogg Vorbis we could just copy it here, but instead we decode to PCM and
		// then re-encode which is redundant. If later we decide to copy be aware that the engine encodes Ogg in a
		// specific quality, and the the import source might have lower or higher bitrate/quality.
		sampleStream = OggVorbisEncoder::PCMToOggVorbis(sampleStream->Data(), info, bufferSize);
	}

	AudioClipCreateInformation clipDesc;
	clipDesc.BitDepth = info.BitDepth;
	clipDesc.Format = clipIO->Format;
	clipDesc.Frequency = info.SampleRate;
	clipDesc.ChannelCount = info.ChannelCount;
	clipDesc.ReadMode = clipIO->ReadMode;
	clipDesc.Is3D = clipIO->Is3D;

	SPtr<AudioClip> clip = AudioClip::CreateShared(sampleStream, bufferSize, info.SampleCount, clipDesc);

	const String fileName = filePath.GetFilename(false);
	clip->SetName(fileName);

	return clip;
}
