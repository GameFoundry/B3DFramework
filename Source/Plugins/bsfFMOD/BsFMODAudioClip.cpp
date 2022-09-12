//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioClip.h"
#include "BsFMODAudio.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	FMOD_RESULT F_CALLBACK PcmReadCallback(FMOD_SOUND* sound, void *data, unsigned int dataLen)
	{
		FMODOggDecompressorData* decompressor = nullptr;
		((FMOD::Sound*)sound)->GetUserData((void**)&decompressor);

		const FMODAudioClip* clip = decompressor->clip;
		UINT32 bytesPerSample = (clip->GetBitDepth() / 8);

		assert(dataLen % bytesPerSample == 0);
		UINT32 numSamples = dataLen / bytesPerSample;

		decompressor->vorbisReader.Seek(decompressor->readPos);
		UINT32 readSamples = decompressor->vorbisReader.Read((UINT8*)data, numSamples);
		while(readSamples < numSamples) // Looping
		{
			decompressor->vorbisReader.Seek(0);

			UINT8* writePtr = (UINT8*)data;
			writePtr += readSamples * bytesPerSample;

			readSamples += decompressor->vorbisReader.Read(writePtr, numSamples - readSamples);
		}

		assert(readSamples == numSamples);

		decompressor->readPos += readSamples;
		decompressor->readPos %= clip->GetNumSamples();

		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK PcmSetPosCallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT posType)
	{
		FMODOggDecompressorData* decompressor = nullptr;
		((FMOD::Sound*)sound)->GetUserData((void**)&decompressor);

		const FMODAudioClip* clip = decompressor->clip;
		UINT32 bytesPerSample = (clip->GetBitDepth() / 8);

		switch(posType)
		{
		case FMOD_TIMEUNIT_MS:
			decompressor->readPos = (UINT32)((clip->GetFrequency() * clip->getNumChannels()) * (position / 1000.0f));
			break;
		case FMOD_TIMEUNIT_PCM:
			decompressor->readPos = clip->GetNumChannels() * position;
			break;
		case FMOD_TIMEUNIT_PCMBYTES:
			assert(position % bytesPerSample == 0);
			decompressor->readPos = position / bytesPerSample;
			break;
		default:
			BS_LOG(Error, Audio, "Invalid time unit.");
			break;
		}

		decompressor->readPos %= clip->GetNumSamples();
		decompressor->vorbisReader.Seek(decompressor->readPos);
		return FMOD_OK;
	}

	FMODAudioClip::FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc)
	{ }

	FMODAudioClip::~FMODAudioClip()
	{
		if(mSound != nullptr)
			mSound->Release();
	}

	void FMODAudioClip::Initialize()
	{
		AudioDataInfo info;
		info.bitDepth = mDesc.bitDepth;
		info.numChannels = mDesc.numChannels;
		info.numSamples = mNumSamples;
		info.sampleRate = mDesc.frequency;

		// If we need to keep source data, read everything into memory and keep a copy
		if (mKeepSourceData)
		{
			mStreamData->Seek(mStreamOffset);

			UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
			mStreamData->Read(sampleBuffer, mStreamSize);

			mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
			mSourceStreamSize = mStreamSize;
		}

		// If streaming is not required, create the sound right away
		if(!requiresStreaming())
		{
			SPtr<DataStream> stream;
			UINT32 offset = 0;
			if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
				stream = mSourceStreamData;
			else
			{
				stream = mStreamData;
				offset = mStreamOffset;
			}

			UINT32 bufferSize = info.numSamples * (info.bitDepth / 8);

			FMOD_CREATESOUNDEXINFO exInfo;
			memset(&exInfo, 0, sizeof(exInfo));
			exInfo.cbsize = sizeof(exInfo);
			exInfo.length = bufferSize;

			FMOD_MODE flags = FMOD_CREATESAMPLE | FMOD_OPENMEMORY;

			if (is3D())
				flags |= FMOD_3D;
			else
				flags |= FMOD_2D;

			if (mDesc.format == AudioFormat::PCM)
			{
				flags |= FMOD_OPENRAW;

				switch (mDesc.bitDepth)
				{
				case 8:
					exInfo.format = FMOD_SOUND_FORMAT_PCM8;
					break;
				case 16:
					exInfo.format = FMOD_SOUND_FORMAT_PCM16;
					break;
				case 24:
					exInfo.format = FMOD_SOUND_FORMAT_PCM24;
					break;
				case 32:
					exInfo.format = FMOD_SOUND_FORMAT_PCM32;
					break;
				default:
					assert(false);
					break;
				}

				exInfo.numchannels = mDesc.numChannels;
				exInfo.defaultfrequency = mDesc.frequency;
			}

			UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);
			stream->Seek(offset);
			stream->Read(sampleBuffer, bufferSize);

			FMOD::System* fmod = gFMODAudio()._getFMOD();
			if (fmod->CreateSound((const char*)sampleBuffer, flags, &exInfo, &mSound) != FMOD_OK)
			{
				BS_LOG(Error, Audio, "Failed creating sound.");
			}
			else
			{
				mSound->SetMode(FMOD_LOOP_OFF);
			}

			mStreamData = nullptr;
			mStreamOffset = 0;
			mStreamSize = 0;

			bs_stack_free(sampleBuffer);
		}
		else // Streaming
		{
			// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
			if(mDesc.readMode == AudioReadMode::LoadCompressed && mStreamData->IsFile())
			{
				if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					mStreamData = mSourceStreamData;
				else
				{
					UINT8* data = (UINT8*)bs_alloc(mStreamSize);

					mStreamData->Seek(mStreamOffset);
					mStreamData->Read(data, mStreamSize);

					mStreamData = bs_shared_ptr_new<MemoryDataStream>(data, mStreamSize);
				}

				mStreamOffset = 0;
			}
		}

		AudioClip::initialize();
	}

	FMOD::Sound* FMODAudioClip::createStreamingSound() const
	{
		if(!requiresStreaming() || mStreamData == nullptr)
		{
			BS_LOG(Error, Audio, "Invalid audio stream data.");
			return nullptr;
		}

		FMOD_MODE flags = FMOD_CREATESTREAM;
		const char* streamData;

		FMOD_CREATESOUNDEXINFO exInfo;
		memset(&exInfo, 0, sizeof(exInfo));
		exInfo.cbsize = sizeof(exInfo);

		String pathStr;
		if (mStreamData->IsFile())
		{
			// initialize() guarantees the data was loaded in memory if it's not streaming
			assert(mDesc.readMode == AudioReadMode::Stream);

			exInfo.length = mStreamSize;
			exInfo.fileoffset = mStreamOffset;

			SPtr<FileDataStream> fileStream = std::static_pointer_cast<FileDataStream>(mStreamData);
			pathStr = fileStream->GetPath().ToString();

			streamData = pathStr.c_str();
		}
		else
		{
			SPtr<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStreamData);

			if (mDesc.readMode == AudioReadMode::Stream)
			{
				// Note: I could use FMOD_OPENMEMORY_POINT here to save on memory, but then the caller would need to make
				// sure the memory is not deallocated. I'm ignoring this for now as streaming from memory should be a rare
				// occurence (normally only in editor)
				flags |= FMOD_OPENMEMORY;

				memStream->Seek(mStreamOffset);
				streamData = (const char*)memStream->GetCurrentPtr();

				exInfo.length = mStreamSize;
			}
			else // Load compressed
			{
				flags |= FMOD_OPENUSER;

				exInfo.decodebuffersize = mDesc.frequency;
				exInfo.pcmreadcallback = pcmReadCallback;
				exInfo.pcmsetposcallback = pcmSetPosCallback;

				AudioDataInfo info;
				info.bitDepth = mDesc.bitDepth;
				info.numChannels = mDesc.numChannels;
				info.numSamples = mNumSamples;
				info.sampleRate = mDesc.frequency;

				FMODOggDecompressorData* decompressorData = bs_new<FMODOggDecompressorData>();
				decompressorData->clip = this;

				if (!decompressorData->vorbisReader.Open(memStream, info, mStreamOffset))
				{
					BS_LOG(Error, Audio, "Failed decompressing AudioClip stream.");
					return nullptr;
				}

				exInfo.userdata = decompressorData;
				exInfo.length = mNumSamples * (mDesc.bitDepth / 8);

				streamData = nullptr;
			}
		}

		if (is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		if (mDesc.format == AudioFormat::PCM || mDesc.readMode == AudioReadMode::LoadCompressed)
		{
			switch (mDesc.bitDepth)
			{
			case 8:
				exInfo.format = FMOD_SOUND_FORMAT_PCM8;
				break;
			case 16:
				exInfo.format = FMOD_SOUND_FORMAT_PCM16;
				break;
			case 24:
				exInfo.format = FMOD_SOUND_FORMAT_PCM24;
				break;
			case 32:
				exInfo.format = FMOD_SOUND_FORMAT_PCM32;
				break;
			default:
				assert(false);
				break;
			}

			exInfo.numchannels = mDesc.numChannels;
			exInfo.defaultfrequency = mDesc.frequency;

			if(mDesc.readMode != AudioReadMode::LoadCompressed)
				flags |= FMOD_OPENRAW;
		}

		FMOD::Sound* sound = nullptr;
		FMOD::System* fmod = gFMODAudio()._getFMOD();
		if (fmod->CreateSound(streamData, flags, &exInfo, &sound) != FMOD_OK)
		{
			BS_LOG(Error, Audio, "Failed creating a streaming sound.");
			return nullptr;
		}

		sound->SetMode(FMOD_LOOP_OFF);
		return sound;
	}

	void FMODAudioClip::ReleaseStreamingSound(FMOD::Sound* sound)
	{
		FMODOggDecompressorData* decompressorData = nullptr;
		((FMOD::Sound*)sound)->GetUserData((void**)&decompressorData);

		if (decompressorData != nullptr)
			bs_delete(decompressorData);

		sound->Release();
	}

	SPtr<DataStream> FMODAudioClip::GetSourceStream(UINT32& size)
	{
		size = mSourceStreamSize;
		mSourceStreamData->Seek(0);

		return mSourceStreamData;
	}

	bool FMODAudioClip::RequiresStreaming() const
	{
		return mDesc.readMode == AudioReadMode::Stream ||
			(mDesc.readMode == AudioReadMode::LoadCompressed && mDesc.format == AudioFormat::VORBIS);
	}
}
