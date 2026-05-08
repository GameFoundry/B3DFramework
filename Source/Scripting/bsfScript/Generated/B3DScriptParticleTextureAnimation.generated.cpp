//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleTextureAnimation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleTextureAnimationSettings.generated.h"
#include "B3DScriptParticleTextureAnimation.generated.h"

namespace b3d
{
	ScriptParticleTextureAnimation::ScriptParticleTextureAnimation(const TShared<ParticleTextureAnimation>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleTextureAnimation::~ScriptParticleTextureAnimation()
	{
		UnregisterEvents();
	}

	void ScriptParticleTextureAnimation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleTextureAnimation::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleTextureAnimation::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleTextureAnimation::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleTextureAnimation::InternalCreate0);

	}

	MonoObject* ScriptParticleTextureAnimation::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleTextureAnimation::InternalSetSettings(ScriptParticleTextureAnimation* self, ParticleTextureAnimationSettings* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleTextureAnimation*>(self->GetNativeObject())->SetSettings(*settings);
	}

	void ScriptParticleTextureAnimation::InternalGetSettings(ScriptParticleTextureAnimation* self, ParticleTextureAnimationSettings* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleTextureAnimationSettings tmp__output;
		tmp__output = static_cast<ParticleTextureAnimation*>(self->GetNativeObject())->GetSettings();

		*__output = tmp__output;
	}

	void ScriptParticleTextureAnimation::InternalCreate(MonoObject* scriptObject, ParticleTextureAnimationSettings* settings)
	{
		TShared<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}

	void ScriptParticleTextureAnimation::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create();
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}
}
