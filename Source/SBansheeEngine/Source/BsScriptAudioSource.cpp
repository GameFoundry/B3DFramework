//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAudioSource.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsAudioSource.h"
#include "BsScriptAudioClip.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptAudioSource::ScriptAudioSource(MonoObject* instance, const SPtr<AudioSource>& source)
		:ScriptObject(instance), mSource(source)
	{ }

	void ScriptAudioSource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptAudioSource::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptAudioSource::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", &ScriptAudioSource::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetVelocity", &ScriptAudioSource::internal_SetVelocity);
		metaData.scriptClass->addInternalCall("Internal_SetClip", &ScriptAudioSource::internal_SetClip);
		metaData.scriptClass->addInternalCall("Internal_SetVolume", &ScriptAudioSource::internal_SetVolume);
		metaData.scriptClass->addInternalCall("Internal_SetPitch", &ScriptAudioSource::internal_SetPitch);
		metaData.scriptClass->addInternalCall("Internal_SetIsLooping", &ScriptAudioSource::internal_SetIsLooping);
		metaData.scriptClass->addInternalCall("Internal_SetPriority", &ScriptAudioSource::internal_SetPriority);
		metaData.scriptClass->addInternalCall("Internal_SetMinDistance", &ScriptAudioSource::internal_SetMinDistance);
		metaData.scriptClass->addInternalCall("Internal_SetAttenuation", &ScriptAudioSource::internal_SetAttenuation);
		metaData.scriptClass->addInternalCall("Internal_Play", &ScriptAudioSource::internal_Play);
		metaData.scriptClass->addInternalCall("Internal_Pause", &ScriptAudioSource::internal_Pause);
		metaData.scriptClass->addInternalCall("Internal_Stop", &ScriptAudioSource::internal_Stop);
		metaData.scriptClass->addInternalCall("Internal_SetTime", &ScriptAudioSource::internal_SetTime);
		metaData.scriptClass->addInternalCall("Internal_GetTime", &ScriptAudioSource::internal_GetTime);
		metaData.scriptClass->addInternalCall("Internal_GetState", &ScriptAudioSource::internal_GetState);
	}

	void ScriptAudioSource::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<AudioSource> source = AudioSource::create();
		new (bs_alloc<ScriptAudioSource>()) ScriptAudioSource(instance, source);
	}

	void ScriptAudioSource::internal_Destroy(ScriptAudioSource* thisPtr)
	{
		thisPtr->mSource = nullptr;
	}

	void ScriptAudioSource::internal_SetPosition(ScriptAudioSource* thisPtr, Vector3* position)
	{
		thisPtr->mSource->setPosition(*position);
	}

	void ScriptAudioSource::internal_SetVelocity(ScriptAudioSource* thisPtr, Vector3* velocity)
	{
		thisPtr->mSource->setVelocity(*velocity);
	}

	void ScriptAudioSource::internal_SetClip(ScriptAudioSource* thisPtr, ScriptAudioClip* clip)
	{
		HAudioClip audioClip;
		if (clip != nullptr)
			audioClip = clip->getHandle();

		thisPtr->mSource->setClip(audioClip);
	}

	void ScriptAudioSource::internal_SetVolume(ScriptAudioSource* thisPtr, float volume)
	{
		thisPtr->mSource->setVolume(volume);
	}

	void ScriptAudioSource::internal_SetPitch(ScriptAudioSource* thisPtr, float pitch)
	{
		thisPtr->mSource->setPitch(pitch);
	}

	void ScriptAudioSource::internal_SetIsLooping(ScriptAudioSource* thisPtr, bool loop)
	{
		thisPtr->mSource->setIsLooping(loop);
	}

	void ScriptAudioSource::internal_SetPriority(ScriptAudioSource* thisPtr, UINT32 priority)
	{
		thisPtr->mSource->setPriority(priority);
	}

	void ScriptAudioSource::internal_SetMinDistance(ScriptAudioSource* thisPtr, float distance)
	{
		thisPtr->mSource->setMinDistance(distance);
	}

	void ScriptAudioSource::internal_SetAttenuation(ScriptAudioSource* thisPtr, float attenuation)
	{
		thisPtr->mSource->setAttenuation(attenuation);
	}

	void ScriptAudioSource::internal_Play(ScriptAudioSource* thisPtr)
	{
		thisPtr->mSource->play();
	}

	void ScriptAudioSource::internal_Pause(ScriptAudioSource* thisPtr)
	{
		thisPtr->mSource->pause();
	}

	void ScriptAudioSource::internal_Stop(ScriptAudioSource* thisPtr)
	{
		thisPtr->mSource->stop();
	}

	void ScriptAudioSource::internal_SetTime(ScriptAudioSource* thisPtr, float position)
	{
		thisPtr->mSource->setTime(position);
	}

	float ScriptAudioSource::internal_GetTime(ScriptAudioSource* thisPtr)
	{
		return thisPtr->mSource->getTime();
	}

	UINT32 ScriptAudioSource::internal_GetState(ScriptAudioSource* thisPtr)
	{
		return (UINT32)thisPtr->mSource->getState();
	}
}