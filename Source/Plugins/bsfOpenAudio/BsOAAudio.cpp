//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAAudio.h"
#include "BsOAAudioClip.h"
#include "BsOAAudioListener.h"
#include "BsOAAudioSource.h"
#include "Math/BsMath.h"
#include "Threading/BsTaskScheduler.h"
#include "Audio/BsAudioUtility.h"
#include "AL/al.h"

namespace bs
{
	OAAudio::OAAudio()
	{
		bool enumeratedDevices;
		if(alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") != ALC_FALSE)
		{
			const ALCchar* defaultDevice = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
			mDefaultDevice.name = String(defaultDevice);

			const ALCchar* devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

			Vector<char> deviceName;
			while(true)
			{
				if(*devices == 0)
				{
					if (deviceName.Empty())
						break;

					// Clean up the name to get the actual hardware name
					String FixedName(deviceName.Data(), deviceName.size());
					fixedName = StringUtil::replaceAll(fixedName, u8"OpenAL Soft on ", u8"");

					mAllDevices.push_back({ fixedName });
					deviceName.Clear();

					devices++;
					continue;
				}

				deviceName.push_back(*devices);
				devices++;
			}

			enumeratedDevices = true;
		}
		else
		{
			mAllDevices.push_back({ u8"" });
			enumeratedDevices = false;
		}

		mActiveDevice = mDefaultDevice;

		String defaultDeviceName = mDefaultDevice.name;
		if(enumeratedDevices)
			mDevice = alcOpenDevice(defaultDeviceName.c_str());
		else
			mDevice = alcOpenDevice(nullptr);

		if (mDevice == nullptr)
			BS_LOG(Error, Audio, "Failed to open OpenAL device: {0}", defaultDeviceName);

		rebuildContexts();
	}

	OAAudio::~OAAudio()
	{
		stopManualSources();

		assert(mListeners.Empty() && mSources.empty()); // Everything should be destroyed at this point
		clearContexts();

		if(mDevice != nullptr)
			alcCloseDevice(mDevice);
	}

	void OAAudio::SetVolume(float volume)
	{
		mVolume = Math::clamp01(volume);
		
		for (auto& listener : mListeners)
			listener->Rebuild();
	}

	float OAAudio::GetVolume() const
	{
		return mVolume;
	}

	void OAAudio::SetPaused(bool paused)
	{
		if (mIsPaused == paused)
			return;

		mIsPaused = paused;

		for (auto& source : mSources)
			source->SetGlobalPause(paused);
	}

	void OAAudio::_update()
	{
		auto worker = [this]() { updateStreaming(); };

		// If previous task still hasn't completed, just skip streaming this frame, queuing more tasks won't help
		if (mStreamingTask != nullptr && !mStreamingTask->IsComplete())
			return;

		mStreamingTask = Task::create("AudioStream", worker, TaskPriority::VeryHigh);
		TaskScheduler::instance().AddTask(mStreamingTask);

		Audio::_update();
	}

	void OAAudio::SetActiveDevice(const AudioDevice& device)
	{
		if (mAllDevices.Size() == 1)
			return; // No devices to change to, keep the active device as is

		clearContexts();

		if(mDevice != nullptr)
			alcCloseDevice(mDevice);
		
		mActiveDevice = device;

		String narrowName = device.name;
		mDevice = alcOpenDevice(narrowName.c_str());
		if (mDevice == nullptr)
			BS_LOG(Error, Audio, "Failed to open OpenAL device: ", narrowName);

		rebuildContexts();
	}

	bool OAAudio::_isExtensionSupported(const String& extension) const
	{
		if (mDevice == nullptr)
			return false;

		if ((extension.Length() > 2) && (extension.substr(0, 3) == "ALC"))
			return AlcIsExtensionPresent(mDevice, extension.c_str()) != AL_FALSE;
		else
			return AlIsExtensionPresent(extension.c_str()) != AL_FALSE;
	}

	void OAAudio::_registerListener(OAAudioListener* listener)
	{
		mListeners.push_back(listener);

		rebuildContexts();
	}

	void OAAudio::_unregisterListener(OAAudioListener* listener)
	{
		auto iterFind = std::find(mListeners.Begin(), mListeners.end(), listener);
		if (iterFind != mListeners.End())
			mListeners.Erase(iterFind);

		rebuildContexts();
	}

	void OAAudio::_registerSource(OAAudioSource* source)
	{
		mSources.Insert(source);
	}

	void OAAudio::_unregisterSource(OAAudioSource* source)
	{
		mSources.Erase(source);
	}

	void OAAudio::StartStreaming(OAAudioSource* source)
	{
		Lock Lock(mMutex);

		mStreamingCommandQueue.push_back({ StreamingCommandType::Start, source });
		mDestroyedSources.Erase(source);
	}

	void OAAudio::StopStreaming(OAAudioSource* source)
	{
		Lock Lock(mMutex);

		mStreamingCommandQueue.push_back({ StreamingCommandType::Stop, source });
		mDestroyedSources.Insert(source);
	}

	ALCcontext* OAAudio::_getContext(const OAAudioListener* listener) const
	{
		if (mListeners.Size() > 0)
		{
			assert(mListeners.Size() == mContexts.size());

			UINT32 numContexts = (UINT32)mContexts.Size();
			for(UINT32 i = 0; i < numContexts; i++)
			{
				if (mListeners[i] == listener)
					return mContexts[i];
			}
		}
		else
			return mContexts[0];

		BS_LOG(Error, Audio, "Unable to find context for an audio listener.");
		return nullptr;
	}

	SPtr<AudioClip> OAAudio::createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
		const AUDIO_CLIP_DESC& desc)
	{
		return bs_core_ptr_new<OAAudioClip>(samples, streamSize, numSamples, desc);
	}

	SPtr<AudioListener> OAAudio::CreateListener()
	{
		return bs_shared_ptr_new<OAAudioListener>();
	}

	SPtr<AudioSource> OAAudio::CreateSource()
	{
		return bs_shared_ptr_new<OAAudioSource>();
	}

	void OAAudio::RebuildContexts()
	{
		for (auto& source : mSources)
			source->Clear();

		clearContexts();

		if (mDevice == nullptr)
			return;

		UINT32 numListeners = (UINT32)mListeners.Size();
		UINT32 numContexts = numListeners > 1 ? numListeners : 1;

		for(UINT32 i = 0; i < numContexts; i++)
		{
			ALCcontext* context = alcCreateContext(mDevice, nullptr);
			mContexts.push_back(context);
		}

		// If only one context is available keep it active as an optimization. Audio listeners and sources will avoid
		// excessive context switching in such case.
		alcMakeContextCurrent(mContexts[0]);

		for (auto& listener : mListeners)
			listener->Rebuild();

		for (auto& source : mSources)
			source->Rebuild();
	}

	void OAAudio::ClearContexts()
	{
		alcMakeContextCurrent(nullptr);

		for (auto& context : mContexts)
			alcDestroyContext(context);

		mContexts.Clear();
	}

	void OAAudio::UpdateStreaming()
	{
		{
			Lock Lock(mMutex);

			for(auto& command : mStreamingCommandQueue)
			{
				switch(command.type)
				{
				case StreamingCommandType::Start:
					mStreamingSources.Insert(command.source);
					break;
				case StreamingCommandType::Stop:
					mStreamingSources.Erase(command.source);
					break;
				default:
					break;
				}
			}

			mStreamingCommandQueue.Clear();
			mDestroyedSources.Clear();
		}

		for (auto& source : mStreamingSources)
		{
			// Check if the source got destroyed while streaming
			{
				Lock Lock(mMutex);

				auto iterFind = mDestroyedSources.Find(source);
				if (iterFind != mDestroyedSources.End())
					continue;
			}

			source->Stream();
		}
	}

	ALenum OAAudio::_getOpenALBufferFormat(UINT32 numChannels, UINT32 bitDepth)
	{
		switch (bitDepth)
		{
		case 8:
		{
			switch (numChannels)
			{
			case 1:  return AL_FORMAT_MONO8;
			case 2:  return AL_FORMAT_STEREO8;
			case 4:  return AlGetEnumValue("AL_FORMAT_QUAD8");
			case 6:  return AlGetEnumValue("AL_FORMAT_51CHN8");
			case 7:  return AlGetEnumValue("AL_FORMAT_61CHN8");
			case 8:  return AlGetEnumValue("AL_FORMAT_71CHN8");
			default:
				assert(false);
				return 0;
			}
		}
		case 16:
		{
			switch (numChannels)
			{
			case 1:  return AL_FORMAT_MONO16;
			case 2:  return AL_FORMAT_STEREO16;
			case 4:  return AlGetEnumValue("AL_FORMAT_QUAD16");
			case 6:  return AlGetEnumValue("AL_FORMAT_51CHN16");
			case 7:  return AlGetEnumValue("AL_FORMAT_61CHN16");
			case 8:  return AlGetEnumValue("AL_FORMAT_71CHN16");
			default:
				assert(false);
				return 0;
			}
		}
		case 32:
		{
			switch (numChannels)
			{
			case 1:  return AlGetEnumValue("AL_FORMAT_MONO_FLOAT32");
			case 2:  return AlGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
			case 4:  return AlGetEnumValue("AL_FORMAT_QUAD32");
			case 6:  return AlGetEnumValue("AL_FORMAT_51CHN32");
			case 7:  return AlGetEnumValue("AL_FORMAT_61CHN32");
			case 8:  return AlGetEnumValue("AL_FORMAT_71CHN32");
			default:
				assert(false);
				return 0;
			}
		}
		default:
			assert(false);
			return 0;
		}
	}

	void OAAudio::_writeToOpenALBuffer(UINT32 bufferId, UINT8* samples, const AudioDataInfo& info)
	{
		if (info.numChannels <= 2) // Mono or stereo
		{
			if (info.bitDepth > 16)
			{
				if (_isExtensionSupported("AL_EXT_float32"))
				{
					UINT32 bufferSize = info.numSamples * sizeof(float);
					float* sampleBufferFloat = (float*)bs_stack_alloc(bufferSize);

					AudioUtility::convertToFloat(samples, info.bitDepth, sampleBufferFloat, info.numSamples);

					ALenum format = _getOpenALBufferFormat(info.numChannels, info.bitDepth);
					alBufferData(bufferId, format, sampleBufferFloat, bufferSize, info.sampleRate);

					bs_stack_free(sampleBufferFloat);
				}
				else
				{
					BS_LOG(Warning, RenderBackend,
						"OpenAL doesn't support bit depth larger than 16. Your audio data will be truncated.");

					UINT32 bufferSize = info.numSamples * 2;
					UINT8* sampleBuffer16 = (UINT8*)bs_stack_alloc(bufferSize);

					AudioUtility::convertBitDepth(samples, info.bitDepth, sampleBuffer16, 16, info.numSamples);

					ALenum format = _getOpenALBufferFormat(info.numChannels, 16);
					alBufferData(bufferId, format, sampleBuffer16, bufferSize, info.sampleRate);

					bs_stack_free(sampleBuffer16);
				}
			}
			else If(info.bitDepth == 8)
			{
				// OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
				UINT32 bufferSize = info.numSamples * (info.bitDepth / 8);
				UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);

				for(UINT32 i = 0; i < info.numSamples; i++)
					sampleBuffer[i] = ((INT8*)samples)[i] + 128;

				ALenum format = _getOpenALBufferFormat(info.numChannels, 16);
				alBufferData(bufferId, format, sampleBuffer, bufferSize, info.sampleRate);

				bs_stack_free(sampleBuffer);
			}
			else
			{
				ALenum format = _getOpenALBufferFormat(info.numChannels, info.bitDepth);
				alBufferData(bufferId, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
			}
		}
		else // Multichannel
		{
			// Note: Assuming AL_EXT_MCFORMATS is supported. If it's not, channels should be reduced to mono or stereo.

			if (info.bitDepth == 24) // 24-bit not supported, convert to 32-bit
			{
				UINT32 bufferSize = info.numSamples * sizeof(INT32);
				UINT8* sampleBuffer32 = (UINT8*)bs_stack_alloc(bufferSize);

				AudioUtility::convertBitDepth(samples, info.bitDepth, sampleBuffer32, 32, info.numSamples);

				ALenum format = _getOpenALBufferFormat(info.numChannels, 32);
				alBufferData(bufferId, format, sampleBuffer32, bufferSize, info.sampleRate);

				bs_stack_free(sampleBuffer32);
			}
			else if (info.bitDepth == 8)
			{
				// OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
				UINT32 bufferSize = info.numSamples * (info.bitDepth / 8);
				UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);

				for (UINT32 i = 0; i < info.numSamples; i++)
					sampleBuffer[i] = ((INT8*)samples)[i] + 128;

				ALenum format = _getOpenALBufferFormat(info.numChannels, 16);
				alBufferData(bufferId, format, sampleBuffer, bufferSize, info.sampleRate);

				bs_stack_free(sampleBuffer);
			}
			else
			{
				ALenum format = _getOpenALBufferFormat(info.numChannels, info.bitDepth);
				alBufferData(bufferId, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
			}
		}
	}

	OAAudio& GOAAudio()
	{
		return static_cast<OAAudio&>(OAAudio::instance());
	}
}
