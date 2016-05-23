//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudio.h"
#include "BsOAAudioClip.h"
#include "BsOAAudioListener.h"
#include "BsOAAudioSource.h"
#include "BsMath.h"
#include "BsTaskScheduler.h"
#include "AL\al.h"

namespace BansheeEngine
{
	OAAudio::OAAudio()
		:mVolume(1.0f)
	{
		bool enumeratedDevices;
		if(_isExtensionSupported("ALC_ENUMERATE_ALL_EXT"))
		{
			const ALCchar* defaultDevice = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
			mDefaultDevice.name = toWString(defaultDevice);

			const ALCchar* devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

			Vector<wchar_t> deviceName;
			while(true)
			{
				if(*devices == 0)
				{
					if (deviceName.size() == 0)
						break;

					mAllDevices.push_back({ WString(deviceName.data(), deviceName.size()) });
					deviceName.clear();

					continue;
				}

				deviceName.push_back(*devices);
				devices++;
			}

			enumeratedDevices = true;
		}
		else
		{
			mAllDevices.push_back({ L"" });
			enumeratedDevices = false;
		}

		mActiveDevice = mDefaultDevice;

		String defaultDeviceName = toString(mDefaultDevice.name);
		if(enumeratedDevices)
			mDevice = alcOpenDevice(defaultDeviceName.c_str());
		else
			mDevice = alcOpenDevice(nullptr);

		if (mDevice == nullptr)
		{
			BS_EXCEPT(InternalErrorException, "Failed to open OpenAL device: " + defaultDeviceName);
			return;
		}

		rebuildContexts();
	}

	OAAudio::~OAAudio()
	{
		assert(mListeners.size() == 0 && mSources.size() == 0); // Everything should be destroyed at this point
		clearContexts();

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
		// TODO
	}

	bool OAAudio::isPaused() const
	{
		// TODO
		return false;
	}

	void OAAudio::_update()
	{
		auto worker = [this]() { updateStreaming(); };

		// If previous task still hasn't completed, just skip streaming this frame, queuing more tasks won't help
		if (mStreamingTask != nullptr && !mStreamingTask->isComplete())
			return;

		mStreamingTask = Task::create("AudioStream", worker, TaskPriority::VeryHigh);
		TaskScheduler::instance().addTask(mStreamingTask);
	}

	void OAAudio::setActiveDevice(const AudioDevice& device)
	{
		if (mAllDevices.size() == 1)
			return; // No devices to change to, keep the active device as is

		clearContexts();

		alcCloseDevice(mDevice);
		mActiveDevice = device;

		String narrowName = toString(device.name);
		mDevice = alcOpenDevice(narrowName.c_str());
		if (mDevice == nullptr)
		{
			BS_EXCEPT(InternalErrorException, "Failed to open OpenAL device: " + narrowName);
			return;
		}

		rebuildContexts();
	}

	bool OAAudio::_isExtensionSupported(const String& extension) const
	{
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

	void OAAudio::startStreaming(OAAudioSource* source, bool startPaused)
	{
		Lock(mMutex);

		StreamingCommand command;
		command.type = StreamingCommandType::Start;
		command.source = source;
		command.params[0] = startPaused;

		mStreamingCommandQueue.push_back(command);
	}

	void OAAudio::stopStreaming(OAAudioSource* source)
	{
		Lock(mMutex);

		StreamingCommand command;
		command.type = StreamingCommandType::Stop;
		command.source = source;

		mStreamingCommandQueue.push_back(command);
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
		return bs_shared_ptr_new<OAAudioClip>(samples, streamSize, numSamples, desc);
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
			Lock(mMutex);

			for(auto& command : mStreamingCommandQueue)
			{
				switch(command.type)
				{
				case StreamingCommandType::Start:
				{
					StreamingData data;
					data.sourceIDs = command.source->mSourceIDs;
					alGenBuffers(StreamingData::StreamBufferCount, data.streamBuffers);

					mStreamingSources.insert(std::make_pair(command.source, data));

					for(auto& )

					// TODO - Store source IDs, start playback (possibly paused)
				}
					break;
				case StreamingCommandType::Stop:
					auto& contexts = gOAAudio()._getContexts(); // TODO - Don't use contexts here, use the source IDs directly
					UINT32 numContexts = (UINT32)contexts.size();
					for (UINT32 i = 0; i < numContexts; i++)
					{
						if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
							alcMakeContextCurrent(contexts[i]);

						INT32 numQueuedBuffers;
						alGetSourcei(mSourceIDs[i], AL_BUFFERS_QUEUED, &numQueuedBuffers);

						UINT32 buffer;
						for (INT32 j = 0; j < numQueuedBuffers; j++)
							alSourceUnqueueBuffers(mSourceIDs[i], 1, &buffer);
					}

					alDeleteBuffers(StreamBufferCount, mStreamBuffers);

					command.source->destroyStreamBuffers();
					mStreamingSources.erase(command.source);
					break;
				default:
					break;
				}
			}

			mStreamingCommandQueue.clear();
		}

		for (auto& source : mStreamingSources)
			source->stream();
	}

	OAAudio& gOAAudio()
	{
		return static_cast<OAAudio&>(OAAudio::instance());
	}
}