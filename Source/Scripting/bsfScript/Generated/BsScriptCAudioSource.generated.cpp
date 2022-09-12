//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCAudioSource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCAudioSource.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs
{
	ScriptCAudioSource::ScriptCAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCAudioSource::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setClip", (void*)&ScriptCAudioSource::Internal_setClip);
		metaData.scriptClass->AddInternalCall("Internal_getClip", (void*)&ScriptCAudioSource::Internal_getClip);
		metaData.scriptClass->AddInternalCall("Internal_setVolume", (void*)&ScriptCAudioSource::Internal_setVolume);
		metaData.scriptClass->AddInternalCall("Internal_getVolume", (void*)&ScriptCAudioSource::Internal_getVolume);
		metaData.scriptClass->AddInternalCall("Internal_setPitch", (void*)&ScriptCAudioSource::Internal_setPitch);
		metaData.scriptClass->AddInternalCall("Internal_getPitch", (void*)&ScriptCAudioSource::Internal_getPitch);
		metaData.scriptClass->AddInternalCall("Internal_setIsLooping", (void*)&ScriptCAudioSource::Internal_setIsLooping);
		metaData.scriptClass->AddInternalCall("Internal_getIsLooping", (void*)&ScriptCAudioSource::Internal_getIsLooping);
		metaData.scriptClass->AddInternalCall("Internal_setPriority", (void*)&ScriptCAudioSource::Internal_setPriority);
		metaData.scriptClass->AddInternalCall("Internal_getPriority", (void*)&ScriptCAudioSource::Internal_getPriority);
		metaData.scriptClass->AddInternalCall("Internal_setMinDistance", (void*)&ScriptCAudioSource::Internal_setMinDistance);
		metaData.scriptClass->AddInternalCall("Internal_getMinDistance", (void*)&ScriptCAudioSource::Internal_getMinDistance);
		metaData.scriptClass->AddInternalCall("Internal_setAttenuation", (void*)&ScriptCAudioSource::Internal_setAttenuation);
		metaData.scriptClass->AddInternalCall("Internal_getAttenuation", (void*)&ScriptCAudioSource::Internal_getAttenuation);
		metaData.scriptClass->AddInternalCall("Internal_setTime", (void*)&ScriptCAudioSource::Internal_setTime);
		metaData.scriptClass->AddInternalCall("Internal_getTime", (void*)&ScriptCAudioSource::Internal_getTime);
		metaData.scriptClass->AddInternalCall("Internal_setPlayOnStart", (void*)&ScriptCAudioSource::Internal_setPlayOnStart);
		metaData.scriptClass->AddInternalCall("Internal_getPlayOnStart", (void*)&ScriptCAudioSource::Internal_getPlayOnStart);
		metaData.scriptClass->AddInternalCall("Internal_play", (void*)&ScriptCAudioSource::Internal_play);
		metaData.scriptClass->AddInternalCall("Internal_pause", (void*)&ScriptCAudioSource::Internal_pause);
		metaData.scriptClass->AddInternalCall("Internal_stop", (void*)&ScriptCAudioSource::Internal_stop);
		metaData.scriptClass->AddInternalCall("Internal_getState", (void*)&ScriptCAudioSource::Internal_getState);

	}

	void ScriptCAudioSource::Internal_setClip(ScriptCAudioSource* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AudioClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AudioClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->setClip(tmpclip);
	}

	MonoObject* ScriptCAudioSource::Internal_getClip(ScriptCAudioSource* thisPtr)
	{
		ResourceHandle<AudioClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->getClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAudioSource::Internal_setVolume(ScriptCAudioSource* thisPtr, float volume)
	{
		thisPtr->GetHandle()->setVolume(volume);
	}

	float ScriptCAudioSource::Internal_getVolume(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPitch(ScriptCAudioSource* thisPtr, float pitch)
	{
		thisPtr->GetHandle()->setPitch(pitch);
	}

	float ScriptCAudioSource::Internal_getPitch(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getPitch();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setIsLooping(ScriptCAudioSource* thisPtr, bool loop)
	{
		thisPtr->GetHandle()->setIsLooping(loop);
	}

	bool ScriptCAudioSource::Internal_getIsLooping(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getIsLooping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPriority(ScriptCAudioSource* thisPtr, uint32_t priority)
	{
		thisPtr->GetHandle()->setPriority(priority);
	}

	uint32_t ScriptCAudioSource::Internal_getPriority(ScriptCAudioSource* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getPriority();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setMinDistance(ScriptCAudioSource* thisPtr, float distance)
	{
		thisPtr->GetHandle()->setMinDistance(distance);
	}

	float ScriptCAudioSource::Internal_getMinDistance(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setAttenuation(ScriptCAudioSource* thisPtr, float attenuation)
	{
		thisPtr->GetHandle()->setAttenuation(attenuation);
	}

	float ScriptCAudioSource::Internal_getAttenuation(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getAttenuation();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setTime(ScriptCAudioSource* thisPtr, float time)
	{
		thisPtr->GetHandle()->setTime(time);
	}

	float ScriptCAudioSource::Internal_getTime(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getTime();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_setPlayOnStart(ScriptCAudioSource* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->setPlayOnStart(enable);
	}

	bool ScriptCAudioSource::Internal_getPlayOnStart(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getPlayOnStart();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::Internal_play(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->play();
	}

	void ScriptCAudioSource::Internal_pause(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->pause();
	}

	void ScriptCAudioSource::Internal_stop(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->stop();
	}

	AudioSourceState ScriptCAudioSource::Internal_getState(ScriptCAudioSource* thisPtr)
	{
		AudioSourceState tmp__output;
		tmp__output = thisPtr->GetHandle()->getState();

		AudioSourceState __output;
		__output = tmp__output;

		return __output;
	}
}
