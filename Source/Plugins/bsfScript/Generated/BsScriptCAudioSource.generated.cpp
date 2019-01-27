//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCAudioSource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCAudioSource.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs
{
	ScriptCAudioSource::ScriptCAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCAudioSource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setClip", (void*)&ScriptCAudioSource::Internal_setClip);
		metaData.scriptClass->addInternalCall("Internal_getClip", (void*)&ScriptCAudioSource::Internal_getClip);
		metaData.scriptClass->addInternalCall("Internal_setVolume", (void*)&ScriptCAudioSource::Internal_setVolume);
		metaData.scriptClass->addInternalCall("Internal_getVolume", (void*)&ScriptCAudioSource::Internal_getVolume);
		metaData.scriptClass->addInternalCall("Internal_setPitch", (void*)&ScriptCAudioSource::Internal_setPitch);
		metaData.scriptClass->addInternalCall("Internal_getPitch", (void*)&ScriptCAudioSource::Internal_getPitch);
		metaData.scriptClass->addInternalCall("Internal_setIsLooping", (void*)&ScriptCAudioSource::Internal_setIsLooping);
		metaData.scriptClass->addInternalCall("Internal_getIsLooping", (void*)&ScriptCAudioSource::Internal_getIsLooping);
		metaData.scriptClass->addInternalCall("Internal_setPriority", (void*)&ScriptCAudioSource::Internal_setPriority);
		metaData.scriptClass->addInternalCall("Internal_getPriority", (void*)&ScriptCAudioSource::Internal_getPriority);
		metaData.scriptClass->addInternalCall("Internal_setMinDistance", (void*)&ScriptCAudioSource::Internal_setMinDistance);
		metaData.scriptClass->addInternalCall("Internal_getMinDistance", (void*)&ScriptCAudioSource::Internal_getMinDistance);
		metaData.scriptClass->addInternalCall("Internal_setAttenuation", (void*)&ScriptCAudioSource::Internal_setAttenuation);
		metaData.scriptClass->addInternalCall("Internal_getAttenuation", (void*)&ScriptCAudioSource::Internal_getAttenuation);
		metaData.scriptClass->addInternalCall("Internal_setTime", (void*)&ScriptCAudioSource::Internal_setTime);
		metaData.scriptClass->addInternalCall("Internal_getTime", (void*)&ScriptCAudioSource::Internal_getTime);
		metaData.scriptClass->addInternalCall("Internal_setPlayOnStart", (void*)&ScriptCAudioSource::Internal_setPlayOnStart);
		metaData.scriptClass->addInternalCall("Internal_getPlayOnStart", (void*)&ScriptCAudioSource::Internal_getPlayOnStart);
		metaData.scriptClass->addInternalCall("Internal_play", (void*)&ScriptCAudioSource::Internal_play);
		metaData.scriptClass->addInternalCall("Internal_pause", (void*)&ScriptCAudioSource::Internal_pause);
		metaData.scriptClass->addInternalCall("Internal_stop", (void*)&ScriptCAudioSource::Internal_stop);
		metaData.scriptClass->addInternalCall("Internal_getState", (void*)&ScriptCAudioSource::Internal_getState);

	}

	void ScriptCAudioSource::Internal_setClip(ScriptCAudioSource* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AudioClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AudioClip>(scriptclip->getHandle());
		thisPtr->getHandle()->setClip(tmpclip);
	}

	MonoObject* ScriptCAudioSource::Internal_getClip(ScriptCAudioSource* thisPtr)
	{
		ResourceHandle<AudioClip> tmp__output;
		tmp__output = thisPtr->getHandle()->getClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAudioSource::Internal_setVolume(ScriptCAudioSource* thisPtr, float volume)
	{
		thisPtr->getHandle()->setVolume(volume);
	}

	float ScriptCAudioSource::Internal_getVolume(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPitch(ScriptCAudioSource* thisPtr, float pitch)
	{
		thisPtr->getHandle()->setPitch(pitch);
	}

	float ScriptCAudioSource::Internal_getPitch(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getPitch();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setIsLooping(ScriptCAudioSource* thisPtr, bool loop)
	{
		thisPtr->getHandle()->setIsLooping(loop);
	}

	bool ScriptCAudioSource::Internal_getIsLooping(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getIsLooping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPriority(ScriptCAudioSource* thisPtr, uint32_t priority)
	{
		thisPtr->getHandle()->setPriority(priority);
	}

	uint32_t ScriptCAudioSource::Internal_getPriority(ScriptCAudioSource* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getPriority();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setMinDistance(ScriptCAudioSource* thisPtr, float distance)
	{
		thisPtr->getHandle()->setMinDistance(distance);
	}

	float ScriptCAudioSource::Internal_getMinDistance(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setAttenuation(ScriptCAudioSource* thisPtr, float attenuation)
	{
		thisPtr->getHandle()->setAttenuation(attenuation);
	}

	float ScriptCAudioSource::Internal_getAttenuation(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getAttenuation();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setTime(ScriptCAudioSource* thisPtr, float time)
	{
		thisPtr->getHandle()->setTime(time);
	}

	float ScriptCAudioSource::Internal_getTime(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getTime();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPlayOnStart(ScriptCAudioSource* thisPtr, bool enable)
	{
		thisPtr->getHandle()->setPlayOnStart(enable);
	}

	bool ScriptCAudioSource::Internal_getPlayOnStart(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getPlayOnStart();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_play(ScriptCAudioSource* thisPtr)
	{
		thisPtr->getHandle()->play();
	}

	void ScriptCAudioSource::Internal_pause(ScriptCAudioSource* thisPtr)
	{
		thisPtr->getHandle()->pause();
	}

	void ScriptCAudioSource::Internal_stop(ScriptCAudioSource* thisPtr)
	{
		thisPtr->getHandle()->stop();
	}

	AudioSourceState ScriptCAudioSource::Internal_getState(ScriptCAudioSource* thisPtr)
	{
		AudioSourceState tmp__output;
		tmp__output = thisPtr->getHandle()->getState();

		AudioSourceState __output;
		__output = tmp__output;

		return __output;
	}
}
