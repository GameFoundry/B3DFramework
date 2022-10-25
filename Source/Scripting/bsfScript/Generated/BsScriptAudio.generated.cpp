//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudio.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Audio/BsAudio.h"
#include "BsScriptAudioDevice.generated.h"

namespace bs
{
	ScriptAudio::ScriptAudio(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{
	}

	void ScriptAudio::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetVolume", (void*)&ScriptAudio::InternalSetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_GetVolume", (void*)&ScriptAudio::InternalGetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_SetPaused", (void*)&ScriptAudio::InternalSetPaused);
		metaData.ScriptClass->AddInternalCall("Internal_IsPaused", (void*)&ScriptAudio::InternalIsPaused);
		metaData.ScriptClass->AddInternalCall("Internal_SetActiveDevice", (void*)&ScriptAudio::InternalSetActiveDevice);
		metaData.ScriptClass->AddInternalCall("Internal_GetActiveDevice", (void*)&ScriptAudio::InternalGetActiveDevice);
		metaData.ScriptClass->AddInternalCall("Internal_GetDefaultDevice", (void*)&ScriptAudio::InternalGetDefaultDevice);
		metaData.ScriptClass->AddInternalCall("Internal_GetAllDevices", (void*)&ScriptAudio::InternalGetAllDevices);
	}

	void ScriptAudio::InternalSetVolume(float volume)
	{
		Audio::Instance().SetVolume(volume);
	}

	float ScriptAudio::InternalGetVolume()
	{
		float tmp__output;
		tmp__output = Audio::Instance().GetVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudio::InternalSetPaused(bool paused)
	{
		Audio::Instance().SetPaused(paused);
	}

	bool ScriptAudio::InternalIsPaused()
	{
		bool tmp__output;
		tmp__output = Audio::Instance().IsPaused();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudio::InternalSetActiveDevice(__AudioDeviceInterop* device)
	{
		AudioDevice tmpdevice;
		tmpdevice = ScriptAudioDevice::FromInterop(*device);
		Audio::Instance().SetActiveDevice(tmpdevice);
	}

	void ScriptAudio::InternalGetActiveDevice(__AudioDeviceInterop* __output)
	{
		AudioDevice tmp__output;
		tmp__output = Audio::Instance().GetActiveDevice();

		__AudioDeviceInterop interop__output;
		interop__output = ScriptAudioDevice::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAudioDevice::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptAudio::InternalGetDefaultDevice(__AudioDeviceInterop* __output)
	{
		AudioDevice tmp__output;
		tmp__output = Audio::Instance().GetDefaultDevice();

		__AudioDeviceInterop interop__output;
		interop__output = ScriptAudioDevice::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAudioDevice::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	MonoArray* ScriptAudio::InternalGetAllDevices()
	{
		Vector<AudioDevice> vec__output;
		vec__output = Audio::Instance().GetAllDevices();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptAudioDevice>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptAudioDevice::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}
} // namespace bs
