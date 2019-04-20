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

namespace bs
{
	OAImporter::OAImporter()
		:SpecificImporter()
	{

	}

	bool OAImporter::isExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == u8"wav" || lowerCaseExt == u8"flac" || lowerCaseExt == u8"ogg";
	}

	bool OAImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		// Don't check for magic number, rely on extension
		return true;
	}

	SPtr<ImportOptions> OAImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<AudioClipImportOptions>();
	}

	SPtr<Resource> OAImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		AudioDataInfo info;
		UINT32 bytesPerSample;
		UINT32 bufferSize;
		UINT8* sampleBuffer;
		{
			Lock fileLock = FileScheduler::getLock(filePath);
			SPtr<DataStream> stream = FileSystem::openFile(filePath);

			String extension = filePath.getExtension();
			StringUtil::toLowerCase(extension);

			UPtr<AudioDecoder> reader(nullptr, nullptr);
			if (extension == u8".wav")
				reader = bs_unique_ptr<AudioDecoder>(bs_new<WaveDecoder>());
			else if (extension == u8".flac")
				reader = bs_unique_ptr<AudioDecoder>(bs_new<FLACDecoder>());
			else if (extension == u8".ogg")
				reader = bs_unique_ptr<AudioDecoder>(bs_new<OggVorbisDecoder>());

			if (reader == nullptr)
				return nullptr;

			if (!reader->isValid(stream))
				return nullptr;

			if (!reader->open(stream, info))
				return nullptr;

			bytesPerSample = info.bitDepth / 8;
			bufferSize = info.numSamples * bytesPerSample;
			sampleBuffer = (UINT8*)bs_alloc(bufferSize);

			reader->read(sampleBuffer, info.numSamples);
		}

		SPtr<const AudioClipImportOptions> clipIO = std::static_pointer_cast<const AudioClipImportOptions>(importOptions);

		// If 3D, convert to mono
		if(clipIO->is3D && info.numChannels > 1)
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
		if(clipIO->bitDepth != info.bitDepth)
		{
			UINT32 outBufferSize = info.numSamples * (clipIO->bitDepth / 8);
			UINT8* outBuffer = (UINT8*)bs_alloc(outBufferSize);

			AudioUtility::convertBitDepth(sampleBuffer, info.bitDepth, outBuffer, clipIO->bitDepth, info.numSamples);

			info.bitDepth = clipIO->bitDepth;

			bs_free(sampleBuffer);

			sampleBuffer = outBuffer;
			bufferSize = outBufferSize;
		}

		// Encode to Ogg Vorbis if needed
		if(clipIO->format == AudioFormat::VORBIS)
		{
			// Note: If the original source was in Ogg Vorbis we could just copy it here, but instead we decode to PCM and
			// then re-encode which is redundant. If later we decide to copy be aware that the engine encodes Ogg in a
			// specific quality, and the the import source might have lower or higher bitrate/quality.
			UINT8* encodedSamples = OggVorbisEncoder::PCMToOggVorbis(sampleBuffer, info, bufferSize);

			bs_free(sampleBuffer);
			sampleBuffer = encodedSamples;
		}

		SPtr<MemoryDataStream> sampleStream = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, bufferSize);

		AUDIO_CLIP_DESC clipDesc;
		clipDesc.bitDepth = info.bitDepth;
		clipDesc.format = clipIO->format;
		clipDesc.frequency = info.sampleRate;
		clipDesc.numChannels = info.numChannels;
		clipDesc.readMode = clipIO->readMode;
		clipDesc.is3D = clipIO->is3D;

		SPtr<AudioClip> clip = AudioClip::_createPtr(sampleStream, bufferSize, info.numSamples, clipDesc);

		const String fileName = filePath.getFilename(false);
		clip->setName(fileName);

		return clip;
	}
}