//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioClip.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs
{
	ScriptAudioClip::ScriptAudioClip(MonoObject* managedInstance, const TResourceHandle<AudioClip>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptAudioClip::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptAudioClip::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetBitDepth", (void*)&ScriptAudioClip::InternalGetBitDepth);
		metaData.ScriptClass->AddInternalCall("Internal_GetFrequency", (void*)&ScriptAudioClip::InternalGetFrequency);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumChannels", (void*)&ScriptAudioClip::InternalGetNumChannels);
		metaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptAudioClip::InternalGetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_GetReadMode", (void*)&ScriptAudioClip::InternalGetReadMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetLength", (void*)&ScriptAudioClip::InternalGetLength);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumSamples", (void*)&ScriptAudioClip::InternalGetNumSamples);
		metaData.ScriptClass->AddInternalCall("Internal_Is3D", (void*)&ScriptAudioClip::InternalIs3D);

	}

	 MonoObject*ScriptAudioClip::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptAudioClip::InternalGetRef(ScriptAudioClip* self)
	{
		return self->GetRRef();
	}

	uint32_t ScriptAudioClip::InternalGetBitDepth(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetBitDepth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetFrequency(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetFrequency();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetNumChannels(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetNumChannels();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	AudioFormat ScriptAudioClip::InternalGetFormat(ScriptAudioClip* self)
	{
		AudioFormat tmp__output;
		tmp__output = self->GetHandle()->GetFormat();

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	AudioReadMode ScriptAudioClip::InternalGetReadMode(ScriptAudioClip* self)
	{
		AudioReadMode tmp__output;
		tmp__output = self->GetHandle()->GetReadMode();

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAudioClip::InternalGetLength(ScriptAudioClip* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetNumSamples(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetNumSamples();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAudioClip::InternalIs3D(ScriptAudioClip* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->Is3D();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
