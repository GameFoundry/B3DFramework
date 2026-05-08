//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOAAudio.h"

#include "B3DApplication.h"
#include "B3DOAAudioClip.h"
#include "B3DOAAudioListener.h"
#include "B3DOAAudioSource.h"
#include "Math/B3DMath.h"
#include "Audio/B3DAudioUtility.h"
#include "AL/al.h"
#include "Threading/B3DSignalEvent.h"

using namespace b3d;

OAAudio::OAAudio()
	:mStreamingTaskSignal(SignalEvent::Mode::ManuallyReset, true)
{
	bool enumeratedDevices;
	if(alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") != ALC_FALSE)
	{
		const ALCchar* defaultDevice = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
		mDefaultDevice.Name = String(defaultDevice);

		const ALCchar* devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

		Vector<char> deviceName;
		while(true)
		{
			if(*devices == 0)
			{
				if(deviceName.empty())
					break;

				// Clean up the name to get the actual hardware name
				String fixedName(deviceName.data(), deviceName.size());
				fixedName = StringUtility::ReplaceAll(fixedName, u8"OpenAL Soft on ", u8"");

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

	String defaultDeviceName = mDefaultDevice.Name;
	if(enumeratedDevices)
		mDevice = alcOpenDevice(defaultDeviceName.c_str());
	else
		mDevice = alcOpenDevice(nullptr);

	if(mDevice == nullptr)
		B3D_LOG(Error, LogAudio, "Failed to open OpenAL device: {0}", defaultDeviceName);

	RebuildContexts();
}

OAAudio::~OAAudio()
{
	StopManualSources();

	B3D_ASSERT(mListeners.empty() && mSources.empty()); // Everything should be destroyed at this point
	ClearContexts();

	if(mDevice != nullptr)
		alcCloseDevice(mDevice);
}

void OAAudio::SetVolume(float volume)
{
	mVolume = Math::Clamp01(volume);

	for(auto& listener : mListeners)
		listener->Rebuild();
}

float OAAudio::GetVolume() const
{
	return mVolume;
}

void OAAudio::SetPaused(bool paused)
{
	if(mIsPaused == paused)
		return;

	mIsPaused = paused;

	for(auto& source : mSources)
		source->SetGlobalPause(paused);
}

void OAAudio::Update()
{
	// If previous task still hasn't completed, just skip streaming this frame, queuing more tasks won't help
	if(!mStreamingTaskSignal.IsSignalled())
		return;

	mStreamingTaskSignal.Reset();
	GetApplication().GetTaskScheduler().Post(SchedulerTask([this] { UpdateStreaming(); mStreamingTaskSignal.Signal(); }, "AudioStreaming"));

	Audio::Update();
}

void OAAudio::SetActiveDevice(const AudioDevice& device)
{
	if(mAllDevices.size() == 1)
		return; // No devices to change to, keep the active device as is

	ClearContexts();

	if(mDevice != nullptr)
		alcCloseDevice(mDevice);

	mActiveDevice = device;

	String narrowName = device.Name;
	mDevice = alcOpenDevice(narrowName.c_str());
	if(mDevice == nullptr)
		B3D_LOG(Error, LogAudio, "Failed to open OpenAL device: ", narrowName);

	RebuildContexts();
}

bool OAAudio::IsExtensionSupported(const String& extension) const
{
	if(mDevice == nullptr)
		return false;

	if((extension.length() > 2) && (extension.substr(0, 3) == "ALC"))
		return alcIsExtensionPresent(mDevice, extension.c_str()) != AL_FALSE;
	else
		return alIsExtensionPresent(extension.c_str()) != AL_FALSE;
}

void OAAudio::RegisterListener(OAAudioListener* listener)
{
	mListeners.push_back(listener);

	RebuildContexts();
}

void OAAudio::UnregisterListener(OAAudioListener* listener)
{
	auto iterFind = std::find(mListeners.begin(), mListeners.end(), listener);
	if(iterFind != mListeners.end())
		mListeners.erase(iterFind);

	RebuildContexts();
}

void OAAudio::RegisterSource(OAAudioSource* source)
{
	mSources.insert(source);
}

void OAAudio::UnregisterSource(OAAudioSource* source)
{
	mSources.erase(source);
}

void OAAudio::StartStreaming(OAAudioSource* source)
{
	Lock lock(mMutex);

	mStreamingCommandQueue.push_back({ StreamingCommandType::Start, source });
	mDestroyedSources.erase(source);
}

void OAAudio::StopStreaming(OAAudioSource* source)
{
	Lock lock(mMutex);

	mStreamingCommandQueue.push_back({ StreamingCommandType::Stop, source });
	mDestroyedSources.insert(source);
}

ALCcontext* OAAudio::GetContext(const OAAudioListener* listener) const
{
	if(mListeners.size() > 0)
	{
		B3D_ASSERT(mListeners.size() == mContexts.size());

		u32 numContexts = (u32)mContexts.size();
		for(u32 i = 0; i < numContexts; i++)
		{
			if(mListeners[i] == listener)
				return mContexts[i];
		}
	}
	else
		return mContexts[0];

	B3D_LOG(Error, LogAudio, "Unable to find context for an audio listener.");
	return nullptr;
}

TShared<AudioClip> OAAudio::CreateClip(const TShared<DataStream>& samples, u32 streamSize, u32 numSamples, const AudioClipCreateInformation& desc)
{
	return B3DMakeShared<OAAudioClip>(samples, streamSize, numSamples, desc);
}

TShared<IAudioListenerImplementation> OAAudio::CreateListener()
{
	return B3DMakeShared<OAAudioListener>();
}

TShared<IAudioSourceImplementation> OAAudio::CreateSource()
{
	return B3DMakeShared<OAAudioSource>();
}

void OAAudio::RebuildContexts()
{
	for(auto& source : mSources)
		source->Clear();

	ClearContexts();

	if(mDevice == nullptr)
		return;

	const u32 listenerCount = (u32)mListeners.size();
	const u32 contextCount = listenerCount > 1 ? listenerCount : 1;

	for(u32 i = 0; i < contextCount; i++)
	{
		ALCcontext* context = alcCreateContext(mDevice, nullptr);
		mContexts.push_back(context);
	}

	// If only one context is available keep it active as an optimization. Audio listeners and sources will avoid
	// excessive context switching in such case.
	alcMakeContextCurrent(mContexts[0]);

	for(auto& listener : mListeners)
		listener->Rebuild();

	for(auto& source : mSources)
		source->Rebuild();
}

void OAAudio::ClearContexts()
{
	alcMakeContextCurrent(nullptr);

	for(auto& context : mContexts)
		alcDestroyContext(context);

	mContexts.clear();
}

void OAAudio::UpdateStreaming()
{
	{
		Lock lock(mMutex);

		for(auto& command : mStreamingCommandQueue)
		{
			switch(command.Type)
			{
			case StreamingCommandType::Start:
				mStreamingSources.insert(command.Source);
				break;
			case StreamingCommandType::Stop:
				mStreamingSources.erase(command.Source);
				break;
			default:
				break;
			}
		}

		mStreamingCommandQueue.clear();
		mDestroyedSources.clear();
	}

	for(auto& source : mStreamingSources)
	{
		// Check if the source got destroyed while streaming
		{
			Lock lock(mMutex);

			auto iterFind = mDestroyedSources.find(source);
			if(iterFind != mDestroyedSources.end())
				continue;
		}

		source->Stream();
	}
}

ALenum OAAudio::GetOpenALBufferFormat(u32 numChannels, u32 bitDepth)
{
	switch(bitDepth)
	{
	case 8:
		{
			switch(numChannels)
			{
			case 1: return AL_FORMAT_MONO8;
			case 2: return AL_FORMAT_STEREO8;
			case 4: return alGetEnumValue("AL_FORMAT_QUAD8");
			case 6: return alGetEnumValue("AL_FORMAT_51CHN8");
			case 7: return alGetEnumValue("AL_FORMAT_61CHN8");
			case 8: return alGetEnumValue("AL_FORMAT_71CHN8");
			default:
				B3D_ASSERT(false);
				return 0;
			}
		}
	case 16:
		{
			switch(numChannels)
			{
			case 1: return AL_FORMAT_MONO16;
			case 2: return AL_FORMAT_STEREO16;
			case 4: return alGetEnumValue("AL_FORMAT_QUAD16");
			case 6: return alGetEnumValue("AL_FORMAT_51CHN16");
			case 7: return alGetEnumValue("AL_FORMAT_61CHN16");
			case 8: return alGetEnumValue("AL_FORMAT_71CHN16");
			default:
				B3D_ASSERT(false);
				return 0;
			}
		}
	case 32:
		{
			switch(numChannels)
			{
			case 1: return alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
			case 2: return alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
			case 4: return alGetEnumValue("AL_FORMAT_QUAD32");
			case 6: return alGetEnumValue("AL_FORMAT_51CHN32");
			case 7: return alGetEnumValue("AL_FORMAT_61CHN32");
			case 8: return alGetEnumValue("AL_FORMAT_71CHN32");
			default:
				B3D_ASSERT(false);
				return 0;
			}
		}
	default:
		B3D_ASSERT(false);
		return 0;
	}
}

void OAAudio::WriteToOpenALBuffer(u32 bufferId, u8* samples, const AudioDataInfo& info)
{
	if(info.ChannelCount <= 2) // Mono or stereo
	{
		if(info.BitDepth > 16)
		{
			if(IsExtensionSupported("AL_EXT_float32"))
			{
				u32 bufferSize = info.SampleCount * sizeof(float);
				float* sampleBufferFloat = (float*)B3DStackAllocate(bufferSize);

				AudioUtility::ConvertToFloat(samples, info.BitDepth, sampleBufferFloat, info.SampleCount);

				ALenum format = GetOpenALBufferFormat(info.ChannelCount, info.BitDepth);
				alBufferData(bufferId, format, sampleBufferFloat, bufferSize, info.SampleRate);

				B3DStackFree(sampleBufferFloat);
			}
			else
			{
				B3D_LOG(Warning, LogRenderBackend, "OpenAL doesn't support bit depth larger than 16. Your audio data will be truncated.");

				u32 bufferSize = info.SampleCount * 2;
				u8* sampleBuffer16 = (u8*)B3DStackAllocate(bufferSize);

				AudioUtility::ConvertBitDepth(samples, info.BitDepth, sampleBuffer16, 16, info.SampleCount);

				ALenum format = GetOpenALBufferFormat(info.ChannelCount, 16);
				alBufferData(bufferId, format, sampleBuffer16, bufferSize, info.SampleRate);

				B3DStackFree(sampleBuffer16);
			}
		}
		else if(info.BitDepth == 8)
		{
			// OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
			u32 bufferSize = info.SampleCount * (info.BitDepth / 8);
			u8* sampleBuffer = (u8*)B3DStackAllocate(bufferSize);

			for(u32 i = 0; i < info.SampleCount; i++)
				sampleBuffer[i] = ((i8*)samples)[i] + 128;

			ALenum format = GetOpenALBufferFormat(info.ChannelCount, 16);
			alBufferData(bufferId, format, sampleBuffer, bufferSize, info.SampleRate);

			B3DStackFree(sampleBuffer);
		}
		else
		{
			ALenum format = GetOpenALBufferFormat(info.ChannelCount, info.BitDepth);
			alBufferData(bufferId, format, samples, info.SampleCount * (info.BitDepth / 8), info.SampleRate);
		}
	}
	else // Multichannel
	{
		// Note: Assuming AL_EXT_MCFORMATS is supported. If it's not, channels should be reduced to mono or stereo.

		if(info.BitDepth == 24) // 24-bit not supported, convert to 32-bit
		{
			u32 bufferSize = info.SampleCount * sizeof(i32);
			u8* sampleBuffer32 = (u8*)B3DStackAllocate(bufferSize);

			AudioUtility::ConvertBitDepth(samples, info.BitDepth, sampleBuffer32, 32, info.SampleCount);

			ALenum format = GetOpenALBufferFormat(info.ChannelCount, 32);
			alBufferData(bufferId, format, sampleBuffer32, bufferSize, info.SampleRate);

			B3DStackFree(sampleBuffer32);
		}
		else if(info.BitDepth == 8)
		{
			// OpenAL expects unsigned 8-bit data, but engine stores it as signed, so convert
			u32 bufferSize = info.SampleCount * (info.BitDepth / 8);
			u8* sampleBuffer = (u8*)B3DStackAllocate(bufferSize);

			for(u32 i = 0; i < info.SampleCount; i++)
				sampleBuffer[i] = ((i8*)samples)[i] + 128;

			ALenum format = GetOpenALBufferFormat(info.ChannelCount, 16);
			alBufferData(bufferId, format, sampleBuffer, bufferSize, info.SampleRate);

			B3DStackFree(sampleBuffer);
		}
		else
		{
			ALenum format = GetOpenALBufferFormat(info.ChannelCount, info.BitDepth);
			alBufferData(bufferId, format, samples, info.SampleCount * (info.BitDepth / 8), info.SampleRate);
		}
	}
}

namespace b3d {
OAAudio& GetOAAudio()
{
	return static_cast<OAAudio&>(OAAudio::Instance());
}
} // namespace b3d
