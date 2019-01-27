//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudio.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Audio/BsAudio.h"
#include "BsScriptAudioDevice.generated.h"

namespace bs
{
	ScriptAudio::ScriptAudio(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptAudio::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setVolume", (void*)&ScriptAudio::Internal_setVolume);
		metaData.scriptClass->addInternalCall("Internal_getVolume", (void*)&ScriptAudio::Internal_getVolume);
		metaData.scriptClass->addInternalCall("Internal_setPaused", (void*)&ScriptAudio::Internal_setPaused);
		metaData.scriptClass->addInternalCall("Internal_isPaused", (void*)&ScriptAudio::Internal_isPaused);
		metaData.scriptClass->addInternalCall("Internal_setActiveDevice", (void*)&ScriptAudio::Internal_setActiveDevice);
		metaData.scriptClass->addInternalCall("Internal_getActiveDevice", (void*)&ScriptAudio::Internal_getActiveDevice);
		metaData.scriptClass->addInternalCall("Internal_getDefaultDevice", (void*)&ScriptAudio::Internal_getDefaultDevice);
		metaData.scriptClass->addInternalCall("Internal_getAllDevices", (void*)&ScriptAudio::Internal_getAllDevices);

	}

	void ScriptAudio::Internal_setVolume(float volume)
	{
		Audio::instance().setVolume(volume);
	}

	float ScriptAudio::Internal_getVolume()
	{
		float tmp__output;
		tmp__output = Audio::instance().getVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudio::Internal_setPaused(bool paused)
	{
		Audio::instance().setPaused(paused);
	}

	bool ScriptAudio::Internal_isPaused()
	{
		bool tmp__output;
		tmp__output = Audio::instance().isPaused();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudio::Internal_setActiveDevice(__AudioDeviceInterop* device)
	{
		AudioDevice tmpdevice;
		tmpdevice = ScriptAudioDevice::fromInterop(*device);
		Audio::instance().setActiveDevice(tmpdevice);
	}

	void ScriptAudio::Internal_getActiveDevice(__AudioDeviceInterop* __output)
	{
		AudioDevice tmp__output;
		tmp__output = Audio::instance().getActiveDevice();

		__AudioDeviceInterop interop__output;
		interop__output = ScriptAudioDevice::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptAudioDevice::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptAudio::Internal_getDefaultDevice(__AudioDeviceInterop* __output)
	{
		AudioDevice tmp__output;
		tmp__output = Audio::instance().getDefaultDevice();

		__AudioDeviceInterop interop__output;
		interop__output = ScriptAudioDevice::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptAudioDevice::getMetaData()->scriptClass->_getInternalClass());
	}

	MonoArray* ScriptAudio::Internal_getAllDevices()
	{
		Vector<AudioDevice> vec__output;
		vec__output = Audio::instance().getAllDevices();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptAudioDevice>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptAudioDevice::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}
}
