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
		:mVolume(1.0f), mIsPaused(false), mDevice(nullptr)
	{
		bool enumeratedDevices;
		if(alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")!=ALC_FALSE)
		{
			const ALCchar* defaultDevice = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
			mDefaultDevice.name = String(defaultDevice);

			const ALCchar* devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

			Vector<char> deviceName;
			while(true)
			{
				if(*devices == 0)
				{
					if (deviceName.size() == 0)
						break;

					// Clean up the name to get the actual hardware name
					String fixedName(deviceName.data(), deviceName.size());
					fixedName = StringUtil::replaceAll(fixedName, u8"OpenAL Soft on ", u8"");

					mAllDevices.push_back({ fixedName });
					deviceName.clear();

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
			LOGERR("Failed to open OpenAL device: " + defaultDeviceName);

		rebuildContexts();
	}

	OAAudio::~OAAudio()
	{
		stopManualSources();

		assert(mListeners.empty() && mSources.empty()); // Everything should be destroyed at this point
		clearContexts();

		if(mDevice != nullptr)
			alcCloseDevice(mDevice);
	}

	void OAAudio::setVolume(float volume)
	{
		mVolume = Math::clamp01(volume);
		
		for (auto& listener : mListeners)
			listener->rebuild();
	}

	float OAAudio::getVolume() const
	{
		return mVolume;
	}

	void OAAudio::setPaused(bool paused)
	{
		if (mIsPaused == paused)
			return;

		mIsPaused = paused;

		for (auto& source : mSources)
			source->setGlobalPause(paused);
	}

	void OAAudio::_update()
	{
		auto worker = [this]() { updateStreaming(); };

		// If previous task still hasn't completed, just skip streaming this frame, queuing more tasks won't help
		if (mStreamingTask != nullptr && !mStreamingTask->isComplete())
			return;

		mStreamingTask = Task::create("AudioStream", worker, TaskPriority::VeryHigh);
		TaskScheduler::instance().addTask(mStreamingTask);

		Audio::_update();
	}

	void OAAudio::setActiveDevice(const AudioDevice& device)
	{
		if (mAllDevices.size() == 1)
			return; // No devices to change to, keep the active device as is

		clearContexts();

		if(mDevice != nullptr)
			alcCloseDevice(mDevice);
		
		mActiveDevice = device;

		String narrowName = device.name;
		mDevice = alcOpenDevice(narrowName.c_str());
		if (mDevice == nullptr)
			LOGERR("Failed to open OpenAL device: " + narrowName);

		rebuildContexts();
	}

	bool OAAudio::_isExtensionSupported(const String& extension) const
	{
		if (mDevice == nullptr)
			return false;

		if ((extension.length() > 2) && (extension.substr(0, 3) == "ALC"))
			return alcIsExtensionPresent(mDevice, extension.c_str()) != AL_FALSE;
		else
			return alIsExtensionPresent(extension.c_str()) != AL_FALSE;
	}

	void OAAudio::_registerListener(OAAudioListener* listener)
	{
		mListeners.push_back(listener);

		rebuildContexts();
	}

	void OAAudio::_unregisterListener(OAAudioListener* listener)
	{
		auto iterFind = std::find(mListeners.begin(), mListeners.end(), listener);
		if (iterFind != mListeners.end())
			mListeners.erase(iterFind);

		rebuildContexts();
	}

	void OAAudio::_registerSource(OAAudioSource* source)
	{
		mSources.insert(source);
	}

	void OAAudio::_unregisterSource(OAAudioSource* source)
	{
		mSources.erase(source);
	}

	void OAAudio::startStreaming(OAAudioSource* source)
	{
		Lock lock(mMutex);

		mStreamingCommandQueue.push_back({ StreamingCommandType::Start, source });
		mDestroyedSources.erase(source);
	}

	void OAAudio::stopStreaming(OAAudioSource* source)
	{
		Lock lock(mMutex);

		mStreamingCommandQueue.push_back({ StreamingCommandType::Stop, source });
		mDestroyedSources.insert(source);
	}

	ALCcontext* OAAudio::_getContext(const OAAudioListener* listener) const
	{
		if (mListeners.size() > 0)
		{
			assert(mListeners.size() == mContexts.size());

			UINT32 numContexts = (UINT32)mContexts.size();
			for(UINT32 i = 0; i < numContexts; i++)
			{
				if (mListeners[i] == listener)
					return mContexts[i];
			}
		}
		else
			return mContexts[0];

		LOGERR("Unable to find context for an audio listener.");
		return nullptr;
	}

	SPtr<AudioClip> OAAudio::createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
		const AUDIO_CLIP_DESC& desc)
	{
		return bs_core_ptr_new<OAAudioClip>(samples, streamSize, numSamples, desc);
	}

	SPtr<AudioListener> OAAudio::createListener()
	{
		return bs_shared_ptr_new<OAAudioListener>();
	}

	SPtr<AudioSource> OAAudio::createSource()
	{
		return bs_shared_ptr_new<OAAudioSource>();
	}

	void OAAudio::rebuildContexts()
	{
		for (auto& source : mSources)
			source->clear();

		clearContexts();

		if (mDevice == nullptr)
			return;

		UINT32 numListeners = (UINT32)mListeners.size();
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
			listener->rebuild();

		for (auto& source : mSources)
			source->rebuild();
	}

	void OAAudio::clearContexts()
	{
		alcMakeContextCurrent(nullptr);

		for (auto& context : mContexts)
			alcDestroyContext(context);

		mContexts.clear();
	}

	void OAAudio::updateStreaming()
	{
		{
			Lock lock(mMutex);

			for(auto& command : mStreamingCommandQueue)
			{
				switch(command.type)
				{
				case StreamingCommandType::Start:
					mStreamingSources.insert(command.source);
					break;
				case StreamingCommandType::Stop:
					mStreamingSources.erase(command.source);
					break;
				default:
					break;
				}
			}

			mStreamingCommandQueue.clear();
			mDestroyedSources.clear();
		}

		for (auto& source : mStreamingSources)
		{
			// Check if the source got destroyed while streaming
			{
				Lock lock(mMutex);

				auto iterFind = mDestroyedSources.find(source);
				if (iterFind != mDestroyedSources.end())
					continue;
			}

			source->stream();
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
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD8");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN8");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN8");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN8");
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
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD16");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN16");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN16");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN16");
			default:
				assert(false);
				return 0;
			}
		}
		case 32:
		{
			switch (numChannels)
			{
			case 1:  return alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
			case 2:  return alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD32");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN32");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN32");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN32");
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
					LOGWRN("OpenAL doesn't support bit depth larger than 16. Your audio data will be truncated.");

					UINT32 bufferSize = info.numSamples * 2;
					UINT8* sampleBuffer16 = (UINT8*)bs_stack_alloc(bufferSize);

					AudioUtility::convertBitDepth(samples, info.bitDepth, sampleBuffer16, 16, info.numSamples);

					ALenum format = _getOpenALBufferFormat(info.numChannels, 16);
					alBufferData(bufferId, format, sampleBuffer16, bufferSize, info.sampleRate);

					bs_stack_free(sampleBuffer16);
				}
			}
			else if(info.bitDepth == 8) 
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

	OAAudio& gOAAudio()
	{
		return static_cast<OAAudio&>(OAAudio::instance());
	}
}
