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
	ScriptCAudioSource::ScriptCAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCAudioSource::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetClip", (void*)&ScriptCAudioSource::InternalSetClip);
		metaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptCAudioSource::InternalGetClip);
		metaData.ScriptClass->AddInternalCall("Internal_SetVolume", (void*)&ScriptCAudioSource::InternalSetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_GetVolume", (void*)&ScriptCAudioSource::InternalGetVolume);
		metaData.ScriptClass->AddInternalCall("Internal_SetPitch", (void*)&ScriptCAudioSource::InternalSetPitch);
		metaData.ScriptClass->AddInternalCall("Internal_GetPitch", (void*)&ScriptCAudioSource::InternalGetPitch);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsLooping", (void*)&ScriptCAudioSource::InternalSetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsLooping", (void*)&ScriptCAudioSource::InternalGetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptCAudioSource::InternalSetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptCAudioSource::InternalGetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptCAudioSource::InternalSetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptCAudioSource::InternalGetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetAttenuation", (void*)&ScriptCAudioSource::InternalSetAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_GetAttenuation", (void*)&ScriptCAudioSource::InternalGetAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_SetTime", (void*)&ScriptCAudioSource::InternalSetTime);
		metaData.ScriptClass->AddInternalCall("Internal_GetTime", (void*)&ScriptCAudioSource::InternalGetTime);
		metaData.ScriptClass->AddInternalCall("Internal_SetPlayOnStart", (void*)&ScriptCAudioSource::InternalSetPlayOnStart);
		metaData.ScriptClass->AddInternalCall("Internal_GetPlayOnStart", (void*)&ScriptCAudioSource::InternalGetPlayOnStart);
		metaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptCAudioSource::InternalPlay);
		metaData.ScriptClass->AddInternalCall("Internal_Pause", (void*)&ScriptCAudioSource::InternalPause);
		metaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptCAudioSource::InternalStop);
		metaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptCAudioSource::InternalGetState);

	}

	void ScriptCAudioSource::InternalSetClip(ScriptCAudioSource* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AudioClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AudioClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->SetClip(tmpclip);
	}

	MonoObject* ScriptCAudioSource::InternalGetClip(ScriptCAudioSource* thisPtr)
	{
		ResourceHandle<AudioClip> tmp__output;
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

	void ScriptCAudioSource::InternalSetVolume(ScriptCAudioSource* thisPtr, float volume)
	{
		thisPtr->GetHandle()->SetVolume(volume);
	}

	float ScriptCAudioSource::InternalGetVolume(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetPitch(ScriptCAudioSource* thisPtr, float pitch)
	{
		thisPtr->GetHandle()->SetPitch(pitch);
	}

	float ScriptCAudioSource::InternalGetPitch(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPitch();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetIsLooping(ScriptCAudioSource* thisPtr, bool loop)
	{
		thisPtr->GetHandle()->SetIsLooping(loop);
	}

	bool ScriptCAudioSource::InternalGetIsLooping(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIsLooping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetPriority(ScriptCAudioSource* thisPtr, uint32_t priority)
	{
		thisPtr->GetHandle()->SetPriority(priority);
	}

	uint32_t ScriptCAudioSource::InternalGetPriority(ScriptCAudioSource* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPriority();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetMinDistance(ScriptCAudioSource* thisPtr, float distance)
	{
		thisPtr->GetHandle()->SetMinDistance(distance);
	}

	float ScriptCAudioSource::InternalGetMinDistance(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetAttenuation(ScriptCAudioSource* thisPtr, float attenuation)
	{
		thisPtr->GetHandle()->SetAttenuation(attenuation);
	}

	float ScriptCAudioSource::InternalGetAttenuation(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAttenuation();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetTime(ScriptCAudioSource* thisPtr, float time)
	{
		thisPtr->GetHandle()->SetTime(time);
	}

	float ScriptCAudioSource::InternalGetTime(ScriptCAudioSource* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetTime();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalSetPlayOnStart(ScriptCAudioSource* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetPlayOnStart(enable);
	}

	bool ScriptCAudioSource::InternalGetPlayOnStart(ScriptCAudioSource* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPlayOnStart();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAudioSource::InternalPlay(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Play();
	}

	void ScriptCAudioSource::InternalPause(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Pause();
	}

	void ScriptCAudioSource::InternalStop(ScriptCAudioSource* thisPtr)
	{
		thisPtr->GetHandle()->Stop();
	}

	AudioSourceState ScriptCAudioSource::InternalGetState(ScriptCAudioSource* thisPtr)
	{
		AudioSourceState tmp__output;
		tmp__output = thisPtr->GetHandle()->GetState();

		AudioSourceState __output;
		__output = tmp__output;

		return __output;
	}
}
