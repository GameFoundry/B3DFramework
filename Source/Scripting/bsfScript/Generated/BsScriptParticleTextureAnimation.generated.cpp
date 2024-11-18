//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleTextureAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_TEXTURE_ANIMATION_DESC.generated.h"
#include "BsScriptParticleTextureAnimation.generated.h"

namespace bs
{
	ScriptParticleTextureAnimation::ScriptParticleTextureAnimation(const SPtr<ParticleTextureAnimation>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleTextureAnimation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleTextureAnimation::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleTextureAnimation::InternalGetOptions);
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
	void ScriptParticleTextureAnimation::InternalSetOptions(ScriptParticleTextureAnimation* self, PARTICLE_TEXTURE_ANIMATION_DESC* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleTextureAnimation*>(self->GetNativeObject())->SetOptions(*options);
	}

	void ScriptParticleTextureAnimation::InternalGetOptions(ScriptParticleTextureAnimation* self, PARTICLE_TEXTURE_ANIMATION_DESC* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_TEXTURE_ANIMATION_DESC tmp__output;
		tmp__output = static_cast<ParticleTextureAnimation*>(self->GetNativeObject())->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleTextureAnimation::InternalCreate(MonoObject* scriptObject, PARTICLE_TEXTURE_ANIMATION_DESC* desc)
	{
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create(*desc);
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}

	void ScriptParticleTextureAnimation::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create();
		ScriptObjectWrapper::Create<ScriptParticleTextureAnimation>(nativeObject, scriptObject);
	}
}
