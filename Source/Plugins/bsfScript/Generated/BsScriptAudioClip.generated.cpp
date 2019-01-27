//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioClip.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs
{
	ScriptAudioClip::ScriptAudioClip(MonoObject* managedInstance, const ResourceHandle<AudioClip>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptAudioClip::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptAudioClip::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_getBitDepth", (void*)&ScriptAudioClip::Internal_getBitDepth);
		metaData.scriptClass->addInternalCall("Internal_getFrequency", (void*)&ScriptAudioClip::Internal_getFrequency);
		metaData.scriptClass->addInternalCall("Internal_getNumChannels", (void*)&ScriptAudioClip::Internal_getNumChannels);
		metaData.scriptClass->addInternalCall("Internal_getFormat", (void*)&ScriptAudioClip::Internal_getFormat);
		metaData.scriptClass->addInternalCall("Internal_getReadMode", (void*)&ScriptAudioClip::Internal_getReadMode);
		metaData.scriptClass->addInternalCall("Internal_getLength", (void*)&ScriptAudioClip::Internal_getLength);
		metaData.scriptClass->addInternalCall("Internal_getNumSamples", (void*)&ScriptAudioClip::Internal_getNumSamples);
		metaData.scriptClass->addInternalCall("Internal_is3D", (void*)&ScriptAudioClip::Internal_is3D);

	}

	 MonoObject*ScriptAudioClip::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptAudioClip::Internal_getRef(ScriptAudioClip* thisPtr)
	{
		return thisPtr->getRRef();
	}

	uint32_t ScriptAudioClip::Internal_getBitDepth(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getBitDepth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getFrequency(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getFrequency();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getNumChannels(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getNumChannels();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	AudioFormat ScriptAudioClip::Internal_getFormat(ScriptAudioClip* thisPtr)
	{
		AudioFormat tmp__output;
		tmp__output = thisPtr->getHandle()->getFormat();

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	AudioReadMode ScriptAudioClip::Internal_getReadMode(ScriptAudioClip* thisPtr)
	{
		AudioReadMode tmp__output;
		tmp__output = thisPtr->getHandle()->getReadMode();

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAudioClip::Internal_getLength(ScriptAudioClip* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::Internal_getNumSamples(ScriptAudioClip* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getNumSamples();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAudioClip::Internal_is3D(ScriptAudioClip* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->is3D();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
