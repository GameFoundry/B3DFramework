//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAudio.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptAudioDevice::ScriptAudioDevice(MonoObject* instance, const AudioDevice& device)
		:ScriptObject(instance), mDevice(device)
	{ }

	void ScriptAudioDevice::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetName", &ScriptAudioDevice::internal_GetName);
	}

	MonoObject* ScriptAudioDevice::create(const AudioDevice& device)
	{
		MonoObject* instance = metaData.scriptClass->createInstance();

		new (bs_alloc<ScriptAudioDevice>()) ScriptAudioDevice(instance, device);
		return instance;
	}

	MonoString* ScriptAudioDevice::internal_GetName(ScriptAudioDevice* thisPtr)
	{
		return MonoUtil::wstringToMono(thisPtr->getDevice().name);
	}

	ScriptAudio::ScriptAudio(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptAudio::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetVolume", &ScriptAudio::internal_SetVolume);
		metaData.scriptClass->addInternalCall("Internal_GetVolume", &ScriptAudio::internal_GetVolume);
		metaData.scriptClass->addInternalCall("Internal_SetActiveDevice", &ScriptAudio::internal_SetActiveDevice);
		metaData.scriptClass->addInternalCall("Internal_GetActiveDevice", &ScriptAudio::internal_GetActiveDevice);
		metaData.scriptClass->addInternalCall("Internal_GetDefaultDevice", &ScriptAudio::internal_GetDefaultDevice);
		metaData.scriptClass->addInternalCall("Internal_GetAllDevices", &ScriptAudio::internal_GetAllDevices);
	}

	void ScriptAudio::internal_SetVolume(float volume)
	{
		gAudio().setVolume(volume);
	}

	float ScriptAudio::internal_GetVolume()
	{
		return gAudio().getVolume();
	}

	void ScriptAudio::internal_SetActiveDevice(ScriptAudioDevice* device)
	{
		gAudio().setActiveDevice(device->getDevice());
	}

	MonoObject* ScriptAudio::internal_GetActiveDevice()
	{
		return ScriptAudioDevice::create(gAudio().getActiveDevice());
	}

	MonoObject* ScriptAudio::internal_GetDefaultDevice()
	{
		return ScriptAudioDevice::create(gAudio().getDefaultDevice());
	}

	MonoArray* ScriptAudio::internal_GetAllDevices()
	{
		const Vector<AudioDevice>& devices = gAudio().getAllDevices();

		UINT32 numDevices = (UINT32)devices.size();
		ScriptArray scriptArray = ScriptArray::create<ScriptAudioDevice>(numDevices);

		for (UINT32 i = 0; i < numDevices; i++)
			scriptArray.set(i, ScriptAudioDevice::create(devices[i]));

		return scriptArray.getInternal();
	}
}