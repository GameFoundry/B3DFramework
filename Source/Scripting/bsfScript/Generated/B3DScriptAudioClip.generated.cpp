//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAudioClip.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Audio/B3DAudioClip.h"

namespace b3d
{
	ScriptAudioClip::ScriptAudioClip(const TResourceHandle<AudioClip>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAudioClip::~ScriptAudioClip()
	{
		UnregisterEvents();
	}

	void ScriptAudioClip::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptAudioClip::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBitDepth", (void*)&ScriptAudioClip::InternalGetBitDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFrequency", (void*)&ScriptAudioClip::InternalGetFrequency);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChannelCount", (void*)&ScriptAudioClip::InternalGetChannelCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptAudioClip::InternalGetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetReadMode", (void*)&ScriptAudioClip::InternalGetReadMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLength", (void*)&ScriptAudioClip::InternalGetLength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptAudioClip::InternalGetSampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Is3D", (void*)&ScriptAudioClip::InternalIs3D);

	}

	MonoObject* ScriptAudioClip::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptAudioClip::InternalGetRef(ScriptAudioClip* self)
	{
		return self->GetOrCreateResourceReference();
	}

	uint32_t ScriptAudioClip::InternalGetBitDepth(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetBitDepth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetFrequency(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetFrequency();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetChannelCount(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetChannelCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	AudioFormat ScriptAudioClip::InternalGetFormat(ScriptAudioClip* self)
	{
		AudioFormat tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetFormat();

		AudioFormat __output;
		__output = tmp__output;

		return __output;
	}

	AudioReadMode ScriptAudioClip::InternalGetReadMode(ScriptAudioClip* self)
	{
		AudioReadMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetReadMode();

		AudioReadMode __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAudioClip::InternalGetLength(ScriptAudioClip* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAudioClip::InternalGetSampleCount(ScriptAudioClip* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->GetSampleCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAudioClip::InternalIs3D(ScriptAudioClip* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AudioClip*>(self->GetNativeObject())->Is3D();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
