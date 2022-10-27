//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
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

using namespace bs;

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
	return bs_shared_ptr_new<AudioClipImportOptions>();
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
			reader = bs_unique_ptr_new<WaveDecoder>();
		else if(extension == u8".flac")
			reader = bs_unique_ptr_new<FLACDecoder>();
		else if(extension == u8".ogg")
			reader = bs_unique_ptr_new<OggVorbisDecoder>();

		if(reader == nullptr)
			return nullptr;

		if(!reader->IsValid(stream))
			return nullptr;

		if(!reader->Open(stream, info))
			return nullptr;

		bytesPerSample = info.BitDepth / 8;
		bufferSize = info.NumSamples * bytesPerSample;

		sampleStream = bs_shared_ptr_new<MemoryDataStream>(bufferSize);
		reader->Read(sampleStream->Data(), info.NumSamples);
	}

	SPtr<const AudioClipImportOptions> clipIO = std::static_pointer_cast<const AudioClipImportOptions>(importOptions);

	// If 3D, convert to mono
	if(clipIO->Is3D && info.NumChannels > 1)
	{
		u32 numSamplesPerChannel = info.NumSamples / info.NumChannels;

		u32 monoBufferSize = numSamplesPerChannel * bytesPerSample;
		auto monoStream = bs_shared_ptr_new<MemoryDataStream>(monoBufferSize);

		AudioUtility::ConvertToMono(sampleStream->Data(), monoStream->Data(), info.BitDepth, numSamplesPerChannel, info.NumChannels);

		info.NumSamples = numSamplesPerChannel;
		info.NumChannels = 1;

		sampleStream = monoStream;
		bufferSize = monoBufferSize;
	}

	// Convert bit depth if needed
	if(clipIO->BitDepth != info.BitDepth)
	{
		u32 outBufferSize = info.NumSamples * (clipIO->BitDepth / 8);
		auto outStream = bs_shared_ptr_new<MemoryDataStream>(outBufferSize);

		AudioUtility::ConvertBitDepth(sampleStream->Data(), info.BitDepth, outStream->Data(), clipIO->BitDepth, info.NumSamples);

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

	AUDIO_CLIP_DESC clipDesc;
	clipDesc.BitDepth = info.BitDepth;
	clipDesc.Format = clipIO->Format;
	clipDesc.Frequency = info.SampleRate;
	clipDesc.NumChannels = info.NumChannels;
	clipDesc.ReadMode = clipIO->ReadMode;
	clipDesc.Is3D = clipIO->Is3D;

	SPtr<AudioClip> clip = AudioClip::CreatePtrInternal(sampleStream, bufferSize, info.NumSamples, clipDesc);

	const String fileName = filePath.GetFilename(false);
	clip->SetName(fileName);

	return clip;
}
