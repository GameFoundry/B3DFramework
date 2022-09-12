//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_setOptions", (void*)&ScriptParticleTextureAnimation::Internal_setOptions);
		metaData.scriptClass->AddInternalCall("Internal_getOptions", (void*)&ScriptParticleTextureAnimation::Internal_getOptions);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptParticleTextureAnimation::Internal_create);
		metaData.scriptClass->AddInternalCall("Internal_create0", (void*)&ScriptParticleTextureAnimation::Internal_create0);

	}

	MonoObject* ScriptParticleTextureAnimation::create(const SPtr<ParticleTextureAnimation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleTextureAnimation>()) ScriptParticleTextureAnimation(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleTextureAnimation::Internal_setOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* options)
	{
		thisPtr->GetInternal()->setOptions(*options);
	}

	void ScriptParticleTextureAnimation::Internal_getOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* __output)
	{
		PARTICLE_TEXTURE_ANIMATION_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->getOptions();

		*__output = tmp__output;
	}

	void ScriptParticleTextureAnimation::Internal_create(MonoObject* managedInstance, PARTICLE_TEXTURE_ANIMATION_DESC* desc)
	{
		SPtr<ParticleTextureAnimation> instance = ParticleTextureAnimation::create(*desc);
		new (bs_alloc<ScriptParticleTextureAnimation>())ScriptParticleTextureAnimation(managedInstance, instance);
	}

	void ScriptParticleTextureAnimation::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleTextureAnimation> instance = ParticleTextureAnimation::create();
		new (bs_alloc<ScriptParticleTextureAnimation>())ScriptParticleTextureAnimation(managedInstance, instance);
	}
}
