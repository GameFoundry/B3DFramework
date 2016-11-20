//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAudioClip.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsScriptMeshData.h"
#include "BsMonoClass.h"

namespace bs
{
	ScriptAudioClip::ScriptAudioClip(MonoObject* instance, const HAudioClip& audioClip)
		:TScriptResource(instance, audioClip)
	{

	}

	void ScriptAudioClip::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetBitDepth", &ScriptAudioClip::internal_GetBitDepth);
		metaData.scriptClass->addInternalCall("Internal_GetSampleRate", &ScriptAudioClip::internal_GetSampleRate);
		metaData.scriptClass->addInternalCall("Internal_GetNumChannels", &ScriptAudioClip::internal_GetNumChannels);
		metaData.scriptClass->addInternalCall("Internal_GetAudioFormat", &ScriptAudioClip::internal_GetAudioFormat);
		metaData.scriptClass->addInternalCall("Internal_GetReadMode", &ScriptAudioClip::internal_GetReadMode);
		metaData.scriptClass->addInternalCall("Internal_GetNumSamples", &ScriptAudioClip::internal_GetNumSamples);
		metaData.scriptClass->addInternalCall("Internal_GetDuration", &ScriptAudioClip::internal_GetDuration);
		metaData.scriptClass->addInternalCall("Internal_GetIs3D", &ScriptAudioClip::internal_GetIs3D);
	}

	UINT32 ScriptAudioClip::internal_GetBitDepth(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getBitDepth();
	}

	UINT32 ScriptAudioClip::internal_GetSampleRate(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getFrequency();
	}

	UINT32 ScriptAudioClip::internal_GetNumChannels(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getNumChannels();
	}

	AudioFormat ScriptAudioClip::internal_GetAudioFormat(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getFormat();
	}

	AudioReadMode ScriptAudioClip::internal_GetReadMode(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getReadMode();
	}

	UINT32 ScriptAudioClip::internal_GetNumSamples(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getNumSamples();
	}

	float ScriptAudioClip::internal_GetDuration(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->getLength();
	}

	bool ScriptAudioClip::internal_GetIs3D(ScriptAudioClip* thisPtr)
	{
		HAudioClip audioClip = thisPtr->getHandle();

		return audioClip->is3D();
	}

	MonoObject* ScriptAudioClip::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}