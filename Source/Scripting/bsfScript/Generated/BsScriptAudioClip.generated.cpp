//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioClip.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs
{
	ScriptAudioClip::ScriptAudioClip(MonoObject* managedInstance, const ResourceHandle<AudioClip>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptAudioClip::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptAudioClip::Internal_getRef);
		metaData.scriptClass->AddInternalCall("Internal_getBitDepth", (void*)&ScriptAudioClip::Internal_getBitDepth);
		metaData.scriptClass->AddInternalCall("Internal_getFrequency", (void*)&ScriptAudioClip::Internal_getFrequency);
		metaData.scriptClass->AddInternalCall("Internal_getNumChannels", (void*)&ScriptAudioClip::Internal_getNumChannels);
		metaData.scriptClass->AddInternalCall("Internal_getFormat", (void*)&ScriptAudioClip::Internal_getFormat);
		metaData.scriptClass->AddInternalCall("Internal_getReadMode", (void*)&ScriptAudioClip::Internal_getReadMode);
		metaData.scriptClass->AddInternalCall("Internal_getLength", (void*)&ScriptAudioClip::Internal_getLength);
		metaData.scriptClass->AddInternalCall("Internal_getNumSamples", (void*)&ScriptAudioClip::Internal_getNumSamples);
		metaData.scriptClass->AddInternalCall("Internal_is3D", (void*)&ScriptAudioClip::Internal_is3D);

	}

	 MonoObject*ScriptAudioClip::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptAudioClip::Internal_getRef(ScriptAudioClip* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	uint32_t ScriptAudioClip::Internal_getBitDepth(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getBitDepth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getFrequency(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getFrequency();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getNumChannels(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getNumChannels();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	AudioFormat ScriptAudioClip::Internal_getFormat(ScriptAudioClip* thisPtr)
	{
		AudioFormat tmp__output;
		tmp__output = thisPtr->GetHandle()->getFormat();

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	AudioReadMode ScriptAudioClip::Internal_getReadMode(ScriptAudioClip* thisPtr)
	{
		AudioReadMode tmp__output;
		tmp__output = thisPtr->GetHandle()->getReadMode();

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAudioClip::Internal_getLength(ScriptAudioClip* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getNumSamples(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getNumSamples();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAudioClip::Internal_is3D(ScriptAudioClip* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->is3D();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
