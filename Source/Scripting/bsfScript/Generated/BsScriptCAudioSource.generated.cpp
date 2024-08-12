//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	ScriptAudioSource::ScriptAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptAudioSource::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetClip", (void*)&ScriptAudioSource::InternalSetClip);
		metaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptAudioSource::InternalGetClip);
		metaData.ScriptClass->AddInternalCall("Internal_SetVolume", (void*)&ScriptAudioSource::InternalSetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_GetVolume", (void*)&ScriptAudioSource::InternalGetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_SetPitch", (void*)&ScriptAudioSource::InternalSetPitch);
		metaData.ScriptClass->AddInternalCall("Internal_GetPitch", (void*)&ScriptAudioSource::InternalGetPitch);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsLooping", (void*)&ScriptAudioSource::InternalSetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsLooping", (void*)&ScriptAudioSource::InternalGetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptAudioSource::InternalSetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptAudioSource::InternalGetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptAudioSource::InternalSetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptAudioSource::InternalGetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetAttenuation", (void*)&ScriptAudioSource::InternalSetAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_GetAttenuation", (void*)&ScriptAudioSource::InternalGetAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_SetTime", (void*)&ScriptAudioSource::InternalSetTime);
		metaData.ScriptClass->AddInternalCall("Internal_GetTime", (void*)&ScriptAudioSource::InternalGetTime);
		metaData.ScriptClass->AddInternalCall("Internal_SetPlayOnStart", (void*)&ScriptAudioSource::InternalSetPlayOnStart);
		metaData.ScriptClass->AddInternalCall("Internal_GetPlayOnStart", (void*)&ScriptAudioSource::InternalGetPlayOnStart);
		metaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptAudioSource::InternalPlay);
		metaData.ScriptClass->AddInternalCall("Internal_Pause", (void*)&ScriptAudioSource::InternalPause);
		metaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptAudioSource::InternalStop);
		metaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptAudioSource::InternalGetState);

	}

	void ScriptAudioSource::InternalSetClip(ScriptAudioSource* thisPtr, MonoObject* clip)
	{
		TResourceHandle<AudioClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AudioClip>(scriptObjectWrapperclip->GetHandle());
		thisPtr->GetHandle()->SetClip(tmpclip);
	}

	MonoObject* ScriptAudioSource::InternalGetClip(ScriptAudioSource* thisPtr)
	{
		TResourceHandle<AudioClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAudioSource::InternalSetVolume(ScriptAudioSource* thisPtr, float volume)
	{
		thisPtr->GetHandle()->SetVolume(volume);
	}

	float ScriptAudioSource::InternalGetVolume(ScriptAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPitch(ScriptAudioSource* thisPtr, float pitch)
	{
		thisPtr->GetHandle()->SetPitch(pitch);
	}

	float ScriptAudioSource::InternalGetPitch(ScriptAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPitch();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetIsLooping(ScriptAudioSource* thisPtr, bool loop)
	{
		thisPtr->GetHandle()->SetIsLooping(loop);
	}

	bool ScriptAudioSource::InternalGetIsLooping(ScriptAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIsLooping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPriority(ScriptAudioSource* thisPtr, uint32_t priority)
	{
		thisPtr->GetHandle()->SetPriority(priority);
	}

	uint32_t ScriptAudioSource::InternalGetPriority(ScriptAudioSource* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPriority();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetMinDistance(ScriptAudioSource* thisPtr, float distance)
	{
		thisPtr->GetHandle()->SetMinDistance(distance);
	}

	float ScriptAudioSource::InternalGetMinDistance(ScriptAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetAttenuation(ScriptAudioSource* thisPtr, float attenuation)
	{
		thisPtr->GetHandle()->SetAttenuation(attenuation);
	}

	float ScriptAudioSource::InternalGetAttenuation(ScriptAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAttenuation();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetTime(ScriptAudioSource* thisPtr, float time)
	{
		thisPtr->GetHandle()->SetTime(time);
	}

	float ScriptAudioSource::InternalGetTime(ScriptAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetTime();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPlayOnStart(ScriptAudioSource* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetPlayOnStart(enable);
	}

	bool ScriptAudioSource::InternalGetPlayOnStart(ScriptAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPlayOnStart();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalPlay(ScriptAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Play();
	}

	void ScriptAudioSource::InternalPause(ScriptAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Pause();
	}

	void ScriptAudioSource::InternalStop(ScriptAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Stop();
	}

	AudioSourceState ScriptAudioSource::InternalGetState(ScriptAudioSource* thisPtr)
	{
		AudioSourceState tmp__output;
		tmp__output = thisPtr->GetHandle()->GetState();

		AudioSourceState __output;
		__output = tmp__output;

		return __output;
	}
}
