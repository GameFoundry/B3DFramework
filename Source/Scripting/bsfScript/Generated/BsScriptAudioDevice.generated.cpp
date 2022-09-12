//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioDevice.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAudioDevice::ScriptAudioDevice(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptAudioDevice::InitRuntimeData()
	{ }

	MonoObject*ScriptAudioDevice::box(const __AudioDeviceInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__AudioDeviceInterop ScriptAudioDevice::Unbox(MonoObject* value)
	{
		return *(__AudioDeviceInterop*)MonoUtil::unbox(value);
	}

	AudioDevice ScriptAudioDevice::FromInterop(const __AudioDeviceInterop& value)
	{
		AudioDevice output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;

		return output;
	}

	__AudioDeviceInterop ScriptAudioDevice::ToInterop(const AudioDevice& value)
	{
		__AudioDeviceInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;

		return output;
	}

}
