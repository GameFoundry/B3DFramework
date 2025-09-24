//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAudio.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Audio/B3DAudio.h"
#include "B3DScriptAudioDevice.generated.h"

namespace b3d
{
	ScriptAudio::ScriptAudio()
		:TScriptTypeDefinition()
	{
	}

	void ScriptAudio::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVolume", (void*)&ScriptAudio::InternalSetVolume);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVolume", (void*)&ScriptAudio::InternalGetVolume);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPaused", (void*)&ScriptAudio::InternalSetPaused);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPaused", (void*)&ScriptAudio::InternalIsPaused);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActiveDevice", (void*)&ScriptAudio::InternalSetActiveDevice);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetActiveDevice", (void*)&ScriptAudio::InternalGetActiveDevice);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDefaultDevice", (void*)&ScriptAudio::InternalGetDefaultDevice);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAllDevices", (void*)&ScriptAudio::InternalGetAllDevices);

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
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAudioDevice::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptAudio::InternalGetDefaultDevice(__AudioDeviceInterop* __output)
	{
		AudioDevice tmp__output;
		tmp__output = Audio::Instance().GetDefaultDevice();

		__AudioDeviceInterop interop__output;
		interop__output = ScriptAudioDevice::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAudioDevice::GetMetaData()->ScriptClass->GetInternalClass());
	}

	MonoArray* ScriptAudio::InternalGetAllDevices()
	{
		Vector<AudioDevice> nativeArray__output;
		nativeArray__output = Audio::Instance().GetAllDevices();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAudioDevice>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptAudioDevice::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
