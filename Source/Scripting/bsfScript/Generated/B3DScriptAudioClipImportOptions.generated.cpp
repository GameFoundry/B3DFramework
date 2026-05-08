//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAudioClipImportOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptAudioClipImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptAudioClipImportOptions::ScriptAudioClipImportOptions(const TShared<AudioClipImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAudioClipImportOptions::~ScriptAudioClipImportOptions()
	{
		UnregisterEvents();
	}

	void ScriptAudioClipImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptAudioClipImportOptions::InternalGetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFormat", (void*)&ScriptAudioClipImportOptions::InternalSetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReadMode", (void*)&ScriptAudioClipImportOptions::InternalGetReadMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetReadMode", (void*)&ScriptAudioClipImportOptions::InternalSetReadMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIs3D", (void*)&ScriptAudioClipImportOptions::InternalGetIs3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIs3D", (void*)&ScriptAudioClipImportOptions::InternalSetIs3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBitDepth", (void*)&ScriptAudioClipImportOptions::InternalGetBitDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBitDepth", (void*)&ScriptAudioClipImportOptions::InternalSetBitDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptAudioClipImportOptions::InternalCreate);

	}

	MonoObject* ScriptAudioClipImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAudioClipImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		TShared<AudioClipImportOptions> nativeObject = AudioClipImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptAudioClipImportOptions>(nativeObject, scriptObject);
	}
	AudioFormat ScriptAudioClipImportOptions::InternalGetFormat(ScriptAudioClipImportOptions* self)
	{
		AudioFormat tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClipImportOptions*>(self->GetNativeObject())->Format;

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetFormat(ScriptAudioClipImportOptions* self, AudioFormat value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioClipImportOptions*>(self->GetNativeObject())->Format = value;
	}

	AudioReadMode ScriptAudioClipImportOptions::InternalGetReadMode(ScriptAudioClipImportOptions* self)
	{
		AudioReadMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClipImportOptions*>(self->GetNativeObject())->ReadMode;

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetReadMode(ScriptAudioClipImportOptions* self, AudioReadMode value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioClipImportOptions*>(self->GetNativeObject())->ReadMode = value;
	}

	bool ScriptAudioClipImportOptions::InternalGetIs3D(ScriptAudioClipImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClipImportOptions*>(self->GetNativeObject())->Is3D;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetIs3D(ScriptAudioClipImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioClipImportOptions*>(self->GetNativeObject())->Is3D = value;
	}

	uint32_t ScriptAudioClipImportOptions::InternalGetBitDepth(ScriptAudioClipImportOptions* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClipImportOptions*>(self->GetNativeObject())->BitDepth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAudioClipImportOptions::InternalSetBitDepth(ScriptAudioClipImportOptions* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AudioClipImportOptions*>(self->GetNativeObject())->BitDepth = value;
	}
#endif
}
