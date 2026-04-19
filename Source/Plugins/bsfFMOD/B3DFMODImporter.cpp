//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODImporter.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Audio/B3DAudioClipImportOptions.h"
#include "Audio/B3DAudioUtility.h"
#include "B3DFMODAudio.h"
#include "B3DOggVorbisEncoder.h"

#include <fmod.hpp>

using namespace b3d;

FMODImporter::FMODImporter()
	: SpecificImporter()
{
}

bool FMODImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtil::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"wav" || lowerCaseExt == u8"flac" || lowerCaseExt == u8"ogg" || lowerCaseExt == u8"mp3" ||
		lowerCaseExt == u8"wma" || lowerCaseExt == u8"asf" || lowerCaseExt == u8"wmv" || lowerCaseExt == u8"midi" ||
		lowerCaseExt == u8"fsb" || lowerCaseExt == u8"aif" || lowerCaseExt == u8"aiff";
}

bool FMODImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	// Don't check for magic number, rely on extension
	return true;
}

SPtr<ImportOptions> FMODImporter::CreateImportOptions() const
{
	return B3DMakeShared<AudioClipImportOptions>();
}

SPtr<Resource> FMODImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	AudioDataInfo info;

	FMOD::Sound* sound;
	{
		Lock fileLock = FileScheduler::GetLock(filePath);

		String pathStr = filePath.ToString();
		if(GetFMODAudio().GetFMODInternal()->createSound(pathStr.c_str(), FMOD_CREATESAMPLE, nullptr, &sound) != FMOD_OK)
		{
			B3D_LOG(Error, LogAudio, "Failed importing audio file: {0}", pathStr);
			return nullptr;
		}
	}

	FMOD_SOUND_FORMAT format;
	i32 numChannels = 0;
	i32 numBits = 0;

	sound->getFormat(nullptr, &format, &numChannels, &numBits);

	if(format != FMOD_SOUND_FORMAT_PCM8 && format != FMOD_SOUND_FORMAT_PCM16 && format != FMOD_SOUND_FORMAT_PCM24 && format != FMOD_SOUND_FORMAT_PCM32 && format != FMOD_SOUND_FORMAT_PCMFLOAT)
	{
		B3D_LOG(Error, LogAudio, "Failed importing audio file, invalid imported format: ", filePath);
		return nullptr;
	}

	float frequency = 0.0f;
	sound->getDefaults(&frequency, nullptr);

	u32 size;
	sound->getLength(&size, FMOD_TIMEUNIT_PCMBYTES);

	info.BitDepth = numBits;
	info.ChannelCount = numChannels;
	info.SampleRate = (u32)frequency;
	info.NumSamples = size / (info.BitDepth / 8);

	u32 bytesPerSample = info.BitDepth / 8;
	u32 bufferSize = info.NumSamples * bytesPerSample;
	u8* sampleBuffer = (u8*)B3DAllocate(bufferSize);
	B3D_ASSERT(bufferSize == size);

	u8* startData = nullptr;
	u8* endData = nullptr;
	u32 startSize = 0;
	u32 endSize = 0;
	sound->lock(0, size, (void**)&startData, (void**)&endData, &startSize, &endSize);

	if(format == FMOD_SOUND_FORMAT_PCMFLOAT)
	{
		B3D_ASSERT(info.BitDepth == 32);

		u32* output = (u32*)sampleBuffer;
		for(u32 i = 0; i < info.NumSamples; i++)
		{
			float value = *(((float*)startData) + i);
			*output = (u32)(value * 2147483647.0f);
			output++;
		}
	}
	else
	{
		memcpy(sampleBuffer, startData, bufferSize);
	}

	sound->unlock((void**)&startData, (void**)&endData, startSize, endSize);
	sound->release();

	SPtr<const AudioClipImportOptions> clipIO = std::static_pointer_cast<const AudioClipImportOptions>(importOptions);

	// If 3D, convert to mono
	if(clipIO->Is3D && info.ChannelCount > 1)
	{
		u32 numSamplesPerChannel = info.NumSamples / info.ChannelCount;

		u32 monoBufferSize = numSamplesPerChannel * bytesPerSample;
		u8* monoBuffer = (u8*)B3DAllocate(monoBufferSize);

		AudioUtility::ConvertToMono(sampleBuffer, monoBuffer, info.BitDepth, numSamplesPerChannel, info.ChannelCount);

		info.NumSamples = numSamplesPerChannel;
		info.ChannelCount = 1;

		B3DFree(sampleBuffer);

		sampleBuffer = monoBuffer;
		bufferSize = monoBufferSize;
	}

	// Convert bit depth if needed
	if(clipIO->BitDepth != info.BitDepth)
	{
		u32 outBufferSize = info.NumSamples * (clipIO->BitDepth / 8);
		u8* outBuffer = (u8*)B3DAllocate(outBufferSize);

		AudioUtility::ConvertBitDepth(sampleBuffer, info.BitDepth, outBuffer, clipIO->BitDepth, info.NumSamples);

		info.BitDepth = clipIO->BitDepth;

		B3DFree(sampleBuffer);

		sampleBuffer = outBuffer;
		bufferSize = outBufferSize;
	}

	// Encode to Ogg Vorbis if needed
	SPtr<MemoryDataStream> sampleStream;
	if(clipIO->Format == AudioFormat::VORBIS)
	{
		// Note: If the original source was in Ogg Vorbis we could just copy it here, but instead we decode to PCM and
		// then re-encode which is redundant. If later we decide to copy be aware that the engine encodes Ogg in a
		// specific quality, and the the import source might have lower or higher bitrate/quality.
		sampleStream = OggVorbisEncoder::PCMToOggVorbis(sampleBuffer, info, bufferSize);

		B3DFree(sampleBuffer);
	}
	else
	{
		sampleStream = B3DMakeShared<MemoryDataStream>(sampleBuffer, bufferSize);
	}

	AUDIO_CLIP_DESC clipDesc;
	clipDesc.BitDepth = info.BitDepth;
	clipDesc.Format = clipIO->Format;
	clipDesc.Frequency = info.SampleRate;
	clipDesc.NumChannels = info.ChannelCount;
	clipDesc.ReadMode = clipIO->ReadMode;
	clipDesc.Is3D = clipIO->Is3D;

	SPtr<AudioClip> clip = AudioClip::CreatePtrInternal(sampleStream, bufferSize, info.NumSamples, clipDesc);

	const String fileName = filePath.GetFilename(false);
	clip->SetName(fileName);

	return clip;
}
