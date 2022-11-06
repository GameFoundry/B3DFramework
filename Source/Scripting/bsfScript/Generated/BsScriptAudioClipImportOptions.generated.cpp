//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAudioClipImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAudioClipImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptAudioClipImportOptions::ScriptAudioClipImportOptions(MonoObject* managedInstance, const SPtr<AudioClipImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptAudioClipImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptAudioClipImportOptions::InternalGetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_SetFormat", (void*)&ScriptAudioClipImportOptions::InternalSetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_GetReadMode", (void*)&ScriptAudioClipImportOptions::InternalGetReadMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetReadMode", (void*)&ScriptAudioClipImportOptions::InternalSetReadMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetIs3D", (void*)&ScriptAudioClipImportOptions::InternalGetIs3D);
		metaData.ScriptClass->AddInternalCall("Internal_SetIs3D", (void*)&ScriptAudioClipImportOptions::InternalSetIs3D);
		metaData.ScriptClass->AddInternalCall("Internal_GetBitDepth", (void*)&ScriptAudioClipImportOptions::InternalGetBitDepth);
		metaData.ScriptClass->AddInternalCall("Internal_SetBitDepth", (void*)&ScriptAudioClipImportOptions::InternalSetBitDepth);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptAudioClipImportOptions::InternalCreate);

	}

	MonoObject* ScriptAudioClipImportOptions::Create(const SPtr<AudioClipImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAudioClipImportOptions>()) ScriptAudioClipImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptAudioClipImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<AudioClipImportOptions> instance = AudioClipImportOptions::Create();
		new (B3DAllocate<ScriptAudioClipImportOptions>())ScriptAudioClipImportOptions(managedInstance, instance);
	}
	AudioFormat ScriptAudioClipImportOptions::InternalGetFormat(ScriptAudioClipImportOptions* thisPtr)
	{
		AudioFormat tmp__output;
		tmp__output = thisPtr->GetInternal()->Format;

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetFormat(ScriptAudioClipImportOptions* thisPtr, AudioFormat value)
	{
		thisPtr->GetInternal()->Format = value;
	}

	AudioReadMode ScriptAudioClipImportOptions::InternalGetReadMode(ScriptAudioClipImportOptions* thisPtr)
	{
		AudioReadMode tmp__output;
		tmp__output = thisPtr->GetInternal()->ReadMode;

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetReadMode(ScriptAudioClipImportOptions* thisPtr, AudioReadMode value)
	{
		thisPtr->GetInternal()->ReadMode = value;
	}

	bool ScriptAudioClipImportOptions::InternalGetIs3D(ScriptAudioClipImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Is3D;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetIs3D(ScriptAudioClipImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Is3D = value;
	}

	uint32_t ScriptAudioClipImportOptions::InternalGetBitDepth(ScriptAudioClipImportOptions* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->BitDepth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetBitDepth(ScriptAudioClipImportOptions* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->BitDepth = value;
	}
#endif
}
