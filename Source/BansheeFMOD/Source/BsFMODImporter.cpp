//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODImporter.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"
#include "BsAudioClipImportOptions.h"
#include "BsAudioUtility.h"
#include "BsFMODAudio.h"

#include <fmod.hpp>

namespace BansheeEngine
{
	FMODImporter::FMODImporter()
		:SpecificImporter()
	{

	}

	FMODImporter::~FMODImporter()
	{

	}

	bool FMODImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"wav" || lowerCaseExt == L"flac" || lowerCaseExt == L"ogg" || lowerCaseExt == L"mp3" ||
			lowerCaseExt == L"wma" || lowerCaseExt == L"asf" || lowerCaseExt == L"wmv" || lowerCaseExt == L"midi" || 
			lowerCaseExt == L"fsb" || lowerCaseExt == L"aif" || lowerCaseExt == L"aiff";
	}

	bool FMODImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		// Don't check for magic number, rely on extension
		return true;
	}

	SPtr<ImportOptions> FMODImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<AudioClipImportOptions>();
	}

	SPtr<Resource> FMODImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		WString extension = filePath.getWExtension();
		StringUtil::toLowerCase(extension);

		AudioFileInfo info;

		FMOD::Sound* sound;
		String pathStr = filePath.toString();
		if(gFMODAudio()._getFMOD()->createSound(pathStr.c_str(), FMOD_CREATESAMPLE, nullptr, &sound) != FMOD_OK)
		{
			LOGERR("Failed importing audio file: " + pathStr);
			return nullptr;
		}

		FMOD_SOUND_FORMAT format;
		INT32 numChannels = 0;
		INT32 numBits = 0;

		sound->getFormat(nullptr, &format, &numChannels, &numBits);

		if(format != FMOD_SOUND_FORMAT_PCM8 && format != FMOD_SOUND_FORMAT_PCM16 && format != FMOD_SOUND_FORMAT_PCM24 
			&& format != FMOD_SOUND_FORMAT_PCM32 && format != FMOD_SOUND_FORMAT_PCMFLOAT)
		{
			LOGERR("Failed importing audio file, invalid imported format: " + pathStr);
			return nullptr;
		}

		float frequency = 0.0f;
		sound->getDefaults(&frequency, nullptr);

		UINT32 size;
		sound->getLength(&size, FMOD_TIMEUNIT_PCMBYTES);
		
		info.bitDepth = numBits;
		info.numChannels = numChannels;
		info.sampleRate = (UINT32)frequency;
		info.numSamples = size / (info.bitDepth / 8);

		UINT32 bytesPerSample = info.bitDepth / 8;
		UINT32 bufferSize = info.numSamples * bytesPerSample;
		UINT8* sampleBuffer = (UINT8*)bs_alloc(bufferSize);
		assert(bufferSize == size);
		
		UINT8* startData = nullptr;
		UINT8* endData = nullptr;
		UINT32 startSize = 0;
		UINT32 endSize = 0;
		sound->lock(0, size, (void**)&startData, (void**)&endData, &startSize, &endSize);

		if(format == FMOD_SOUND_FORMAT_PCMFLOAT)
		{
			assert(info.bitDepth == 32);

			UINT32* output = (UINT32*)sampleBuffer;
			for(UINT32 i = 0; i < info.numSamples; i++)
			{
				float value = *(((float*)startData) + i);
				*output = (UINT32)(value * 2147483647.0f);
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
		if (clipIO->getIs3D() && info.numChannels > 1)
		{
			UINT32 numSamplesPerChannel = info.numSamples / info.numChannels;

			UINT32 monoBufferSize = numSamplesPerChannel * bytesPerSample;
			UINT8* monoBuffer = (UINT8*)bs_alloc(monoBufferSize);

			AudioUtility::convertToMono(sampleBuffer, monoBuffer, info.bitDepth, numSamplesPerChannel, info.numChannels);

			info.numSamples = numSamplesPerChannel;
			info.numChannels = 1;

			bs_free(sampleBuffer);

			sampleBuffer = monoBuffer;
			bufferSize = monoBufferSize;
		}

		// Convert bit depth if needed
		if (clipIO->getBitDepth() != info.bitDepth)
		{
			UINT32 outBufferSize = info.numSamples * (clipIO->getBitDepth() / 8);
			UINT8* outBuffer = (UINT8*)bs_alloc(outBufferSize);

			AudioUtility::convertBitDepth(sampleBuffer, info.bitDepth, outBuffer, clipIO->getBitDepth(), info.numSamples);

			info.bitDepth = clipIO->getBitDepth();

			bs_free(sampleBuffer);

			sampleBuffer = outBuffer;
			bufferSize = outBufferSize;
		}

		// Encode to Ogg Vorbis if needed
		if (clipIO->getFormat() == AudioFormat::VORBIS)
		{



			// TODO - Encode to Ogg Vorbis!




		}

		SPtr<MemoryDataStream> sampleStream = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, bufferSize);

		AUDIO_CLIP_DESC clipDesc;
		clipDesc.bitDepth = info.bitDepth;
		clipDesc.format = clipIO->getFormat();
		clipDesc.frequency = info.sampleRate;
		clipDesc.numChannels = info.numChannels;
		clipDesc.readMode = clipIO->getReadMode();
		clipDesc.is3D = clipIO->getIs3D();

		SPtr<AudioClip> clip = AudioClip::_createPtr(sampleStream, bufferSize, info.numSamples, clipDesc);

		WString fileName = filePath.getWFilename(false);
		clip->setName(fileName);

		return clip;
	}
}