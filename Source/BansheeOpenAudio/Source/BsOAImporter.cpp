//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAImporter.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"
#include "BsOAWaveReader.h"
#include "BsOAFLACReader.h"
#include "BsOAOggVorbisReader.h"
#include "BsOAOggVorbisWriter.h"
#include "BsAudioClipImportOptions.h"

namespace BansheeEngine
{
	OAImporter::OAImporter()
		:SpecificImporter()
	{

	}

	OAImporter::~OAImporter()
	{

	}

	bool OAImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"wav" || lowerCaseExt == L"flac" || lowerCaseExt == L"ogg";
	}

	bool OAImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		// Don't check for magic number, rely on extension
		return true;
	}

	SPtr<Resource> OAImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<DataStream> stream = FileSystem::openFile(filePath);

		WString extension = filePath.getWExtension();
		StringUtil::toLowerCase(extension);

		UPtr<OAFileReader> reader(nullptr, nullptr);
		if(extension == L"wav")
			reader = bs_unique_ptr<OAFileReader>(bs_new<OAWaveReader>());
		else if(extension == L"flac")
			reader = bs_unique_ptr<OAFileReader>(bs_new<OAFLACReader>());
		else if(extension == L"ogg")
			reader = bs_unique_ptr<OAFileReader>(bs_new<OAOggVorbisReader>());

		if (reader == nullptr)
			return nullptr;

		AudioFileInfo info;
		if (!reader->isValid(stream))
			return nullptr;

		if (!reader->open(stream, info))
			return nullptr;

		UINT32 bufferSize = info.numSamples * info.bitDepth;
		UINT8* sampleBuffer = (UINT8*)bs_alloc(bufferSize);
		reader->read(sampleBuffer, info.numSamples);

		SPtr<const AudioClipImportOptions> clipIO = std::static_pointer_cast<const AudioClipImportOptions>(importOptions);

		// Encode to Ogg Vorbis if needed
		if(clipIO->getFormat() == AudioFormat::VORBIS)
		{
			// Note: If the original source was in Ogg Vorbis we could just copy it here, but instead we decode to PCM and
			// then re-encode which is redundant. If later we decide to copy be aware that the engine encodes Ogg in a
			// specific quality, and the the import source might have lower or higher bitrate/quality.
			UINT8* encodedSamples = OAOggVorbisWriter::PCMToOggVorbis(sampleBuffer, info, bufferSize);

			bs_free(sampleBuffer);
			sampleBuffer = encodedSamples;
		}

		SPtr<MemoryDataStream> sampleStream = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, bufferSize);

		AUDIO_CLIP_DESC clipDesc;
		clipDesc.bitDepth = info.bitDepth;
		clipDesc.format = clipIO->getFormat();
		clipDesc.frequency = info.sampleRate;
		clipDesc.numChannels = info.numChannels;
		clipDesc.readMode = clipIO->getReadMode();

		SPtr<AudioClip> clip = AudioClip::_createPtr(sampleStream, bufferSize, info.numSamples, clipDesc);

		WString fileName = filePath.getWFilename(false);
		clip->setName(fileName);

		return clip;
	}
}