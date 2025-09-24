//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAudioDevice.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptAudioDevice::ScriptAudioDevice()
	{ }

	MonoObject* ScriptAudioDevice::Box(const __AudioDeviceInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__AudioDeviceInterop ScriptAudioDevice::Unbox(MonoObject* value)
	{
		return *(__AudioDeviceInterop*)MonoUtil::Unbox(value);
	}

	AudioDevice ScriptAudioDevice::FromInterop(const __AudioDeviceInterop& value)
	{
		AudioDevice output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;

		return output;
	}

	__AudioDeviceInterop ScriptAudioDevice::ToInterop(const AudioDevice& value)
	{
		__AudioDeviceInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;

		return output;
	}

}
