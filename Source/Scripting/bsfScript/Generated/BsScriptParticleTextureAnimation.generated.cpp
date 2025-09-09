//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleTextureAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleTextureAnimationSettings.generated.h"
#include "BsScriptParticleTextureAnimation.generated.h"

namespace b3d
{
	ScriptParticleTextureAnimation::ScriptParticleTextureAnimation(const SPtr<ParticleTextureAnimation>& nativeObject)
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
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}

	void ScriptParticleTextureAnimation::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create();
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}
}
