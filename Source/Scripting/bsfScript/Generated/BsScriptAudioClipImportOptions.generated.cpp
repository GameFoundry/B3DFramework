//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioClipImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAudioClipImportOptions.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptAudioClipImportOptions::ScriptAudioClipImportOptions(MonoObject* managedInstance, const SPtr<AudioClipImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptAudioClipImportOptions::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getformat", (void*)&ScriptAudioClipImportOptions::Internal_getformat);
		metaData.scriptClass->addInternalCall("Internal_setformat", (void*)&ScriptAudioClipImportOptions::Internal_setformat);
		metaData.scriptClass->addInternalCall("Internal_getreadMode", (void*)&ScriptAudioClipImportOptions::Internal_getreadMode);
		metaData.scriptClass->addInternalCall("Internal_setreadMode", (void*)&ScriptAudioClipImportOptions::Internal_setreadMode);
		metaData.scriptClass->addInternalCall("Internal_getis3D", (void*)&ScriptAudioClipImportOptions::Internal_getis3D);
		metaData.scriptClass->addInternalCall("Internal_setis3D", (void*)&ScriptAudioClipImportOptions::Internal_setis3D);
		metaData.scriptClass->addInternalCall("Internal_getbitDepth", (void*)&ScriptAudioClipImportOptions::Internal_getbitDepth);
		metaData.scriptClass->addInternalCall("Internal_setbitDepth", (void*)&ScriptAudioClipImportOptions::Internal_setbitDepth);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptAudioClipImportOptions::Internal_create);

	}

	MonoObject* ScriptAudioClipImportOptions::create(const SPtr<AudioClipImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptAudioClipImportOptions>()) ScriptAudioClipImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptAudioClipImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<AudioClipImportOptions> instance = AudioClipImportOptions::create();
		new (bs_alloc<ScriptAudioClipImportOptions>())ScriptAudioClipImportOptions(managedInstance, instance);
	}
	AudioFormat ScriptAudioClipImportOptions::Internal_getformat(ScriptAudioClipImportOptions* thisPtr)
	{
		AudioFormat tmp__output;
		tmp__output = thisPtr->getInternal()->format;

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::Internal_setformat(ScriptAudioClipImportOptions* thisPtr, AudioFormat value)
	{
		thisPtr->getInternal()->format = value;
	}

	AudioReadMode ScriptAudioClipImportOptions::Internal_getreadMode(ScriptAudioClipImportOptions* thisPtr)
	{
		AudioReadMode tmp__output;
		tmp__output = thisPtr->getInternal()->readMode;

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::Internal_setreadMode(ScriptAudioClipImportOptions* thisPtr, AudioReadMode value)
	{
		thisPtr->getInternal()->readMode = value;
	}

	bool ScriptAudioClipImportOptions::Internal_getis3D(ScriptAudioClipImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->is3D;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::Internal_setis3D(ScriptAudioClipImportOptions* thisPtr, bool value)
	{
		thisPtr->getInternal()->is3D = value;
	}

	uint32_t ScriptAudioClipImportOptions::Internal_getbitDepth(ScriptAudioClipImportOptions* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->bitDepth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::Internal_setbitDepth(ScriptAudioClipImportOptions* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->bitDepth = value;
	}
#endif
}
