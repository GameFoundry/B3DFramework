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
	ScriptParticleTextureAnimation::ScriptParticleTextureAnimation(MonoObject* managedInstance, const SPtr<ParticleTextureAnimation>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleTextureAnimation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleTextureAnimation::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleTextureAnimation::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleTextureAnimation::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleTextureAnimation::InternalCreate0);

	}

	MonoObject* ScriptParticleTextureAnimation::Create(const SPtr<ParticleTextureAnimation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleTextureAnimation>()) ScriptParticleTextureAnimation(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleTextureAnimation::InternalSetOptions(ScriptParticleTextureAnimation* self, PARTICLE_TEXTURE_ANIMATION_DESC* options)
	{
		self->GetInternal()->SetOptions(*options);
	}

	void ScriptParticleTextureAnimation::InternalGetOptions(ScriptParticleTextureAnimation* self, PARTICLE_TEXTURE_ANIMATION_DESC* __output)
	{
		PARTICLE_TEXTURE_ANIMATION_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleTextureAnimation::InternalCreate(MonoObject* managedInstance, PARTICLE_TEXTURE_ANIMATION_DESC* desc)
	{
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create(*desc);
		new (B3DAllocate<ScriptParticleTextureAnimation>())ScriptParticleTextureAnimation(managedInstance, nativeObject);
	}

	void ScriptParticleTextureAnimation::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleTextureAnimation> nativeObject = ParticleTextureAnimation::Create();
		new (B3DAllocate<ScriptParticleTextureAnimation>())ScriptParticleTextureAnimation(managedInstance, nativeObject);
	}
}
