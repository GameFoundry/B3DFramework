//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAudioSource.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DAudioSource.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Audio/B3DAudioClip.h"

namespace b3d
{
	ScriptAudioSource::ScriptAudioSource(const TGameObjectHandle<AudioSource>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAudioSource::~ScriptAudioSource()
	{
		UnregisterEvents();
	}

	void ScriptAudioSource::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClip", (void*)&ScriptAudioSource::InternalSetClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptAudioSource::InternalGetClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVolume", (void*)&ScriptAudioSource::InternalSetVolume);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVolume", (void*)&ScriptAudioSource::InternalGetVolume);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPitch", (void*)&ScriptAudioSource::InternalSetPitch);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPitch", (void*)&ScriptAudioSource::InternalGetPitch);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsLooping", (void*)&ScriptAudioSource::InternalSetIsLooping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsLooping", (void*)&ScriptAudioSource::InternalGetIsLooping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptAudioSource::InternalSetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptAudioSource::InternalGetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptAudioSource::InternalSetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptAudioSource::InternalGetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAttenuation", (void*)&ScriptAudioSource::InternalSetAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAttenuation", (void*)&ScriptAudioSource::InternalGetAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTime", (void*)&ScriptAudioSource::InternalSetTime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTime", (void*)&ScriptAudioSource::InternalGetTime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPlayOnStart", (void*)&ScriptAudioSource::InternalSetPlayOnStart);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlayOnStart", (void*)&ScriptAudioSource::InternalGetPlayOnStart);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptAudioSource::InternalPlay);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Pause", (void*)&ScriptAudioSource::InternalPause);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptAudioSource::InternalStop);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptAudioSource::InternalGetState);

	}

	MonoObject* ScriptAudioSource::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAudioSource::InternalSetClip(ScriptAudioSource* self, MonoObject* clip)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AudioClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AudioClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<AudioSource*>(self->GetNativeObject())->SetClip(tmpclip);
	}

	MonoObject* ScriptAudioSource::InternalGetClip(ScriptAudioSource* self)
	{
		TResourceHandle<AudioClip> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAudioSource::InternalSetVolume(ScriptAudioSource* self, float volume)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetVolume(volume);
	}

	float ScriptAudioSource::InternalGetVolume(ScriptAudioSource* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetVolume();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPitch(ScriptAudioSource* self, float pitch)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetPitch(pitch);
	}

	float ScriptAudioSource::InternalGetPitch(ScriptAudioSource* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetPitch();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetIsLooping(ScriptAudioSource* self, bool loop)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetIsLooping(loop);
	}

	bool ScriptAudioSource::InternalGetIsLooping(ScriptAudioSource* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetIsLooping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPriority(ScriptAudioSource* self, uint32_t priority)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetPriority(priority);
	}

	uint32_t ScriptAudioSource::InternalGetPriority(ScriptAudioSource* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetPriority();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetMinDistance(ScriptAudioSource* self, float distance)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetMinDistance(distance);
	}

	float ScriptAudioSource::InternalGetMinDistance(ScriptAudioSource* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetAttenuation(ScriptAudioSource* self, float attenuation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetAttenuation(attenuation);
	}

	float ScriptAudioSource::InternalGetAttenuation(ScriptAudioSource* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetAttenuation();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetTime(ScriptAudioSource* self, float time)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetTime(time);
	}

	float ScriptAudioSource::InternalGetTime(ScriptAudioSource* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetTime();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalSetPlayOnStart(ScriptAudioSource* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->SetPlayOnStart(enable);
	}

	bool ScriptAudioSource::InternalGetPlayOnStart(ScriptAudioSource* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetPlayOnStart();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioSource::InternalPlay(ScriptAudioSource* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->Play();
	}

	void ScriptAudioSource::InternalPause(ScriptAudioSource* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->Pause();
	}

	void ScriptAudioSource::InternalStop(ScriptAudioSource* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioSource*>(self->GetNativeObject())->Stop();
	}

	AudioSourceState ScriptAudioSource::InternalGetState(ScriptAudioSource* self)
	{
		AudioSourceState tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioSource*>(self->GetNativeObject())->GetState();

		AudioSourceState __output;
		__output = tmp__output;

		return __output;
	}
}
