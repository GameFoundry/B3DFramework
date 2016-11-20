//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAudioListener.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsAudioListener.h"

using namespace std::placeholders;

namespace bs
{
	ScriptAudioListener::ScriptAudioListener(MonoObject* instance, const SPtr<AudioListener>& listener)
		:ScriptObject(instance), mListener(listener)
	{ }

	void ScriptAudioListener::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptAudioListener::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptAudioListener::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", &ScriptAudioListener::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetDirection", &ScriptAudioListener::internal_SetDirection);
		metaData.scriptClass->addInternalCall("Internal_SetUp", &ScriptAudioListener::internal_SetUp);
		metaData.scriptClass->addInternalCall("Internal_SetVelocity", &ScriptAudioListener::internal_SetVelocity);
	}

	void ScriptAudioListener::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<AudioListener> listener = AudioListener::create();
		new (bs_alloc<ScriptAudioListener>()) ScriptAudioListener(instance, listener);
	}

	void ScriptAudioListener::internal_Destroy(ScriptAudioListener* thisPtr)
	{
		thisPtr->mListener = nullptr;
	}

	void ScriptAudioListener::internal_SetPosition(ScriptAudioListener* thisPtr, Vector3* position)
	{
		thisPtr->mListener->setPosition(*position);
	}

	void ScriptAudioListener::internal_SetDirection(ScriptAudioListener* thisPtr, Vector3* direction)
	{
		thisPtr->mListener->setDirection(*direction);
	}

	void ScriptAudioListener::internal_SetUp(ScriptAudioListener* thisPtr, Vector3* up)
	{
		thisPtr->mListener->setUp(*up);
	}

	void ScriptAudioListener::internal_SetVelocity(ScriptAudioListener* thisPtr, Vector3* velocity)
	{
		thisPtr->mListener->setVelocity(*velocity);
	}
}