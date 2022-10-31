//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleTextureAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_TEXTURE_ANIMATION_DESC.generated.h"
#include "BsScriptParticleTextureAnimation.generated.h"

using namespace bs;
ScriptParticleTextureAnimation::ScriptParticleTextureAnimation(MonoObject* managedInstance, const SPtr<ParticleTextureAnimation>& value)
	: TScriptReflectable(managedInstance, value)
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
	new(B3DAllocate<ScriptParticleTextureAnimation>()) ScriptParticleTextureAnimation(managedInstance, value);
	return managedInstance;
}

void ScriptParticleTextureAnimation::InternalSetOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* options)
{
	thisPtr->GetInternal()->SetOptions(*options);
}

void ScriptParticleTextureAnimation::InternalGetOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* __output)
{
	PARTICLE_TEXTURE_ANIMATION_DESC tmp__output;
	tmp__output = thisPtr->GetInternal()->GetOptions();

	*__output = tmp__output;
}

void ScriptParticleTextureAnimation::InternalCreate(MonoObject* managedInstance, PARTICLE_TEXTURE_ANIMATION_DESC* desc)
{
	SPtr<ParticleTextureAnimation> instance = ParticleTextureAnimation::Create(*desc);
	new(B3DAllocate<ScriptParticleTextureAnimation>()) ScriptParticleTextureAnimation(managedInstance, instance);
}

void ScriptParticleTextureAnimation::InternalCreate0(MonoObject* managedInstance)
{
	SPtr<ParticleTextureAnimation> instance = ParticleTextureAnimation::Create();
	new(B3DAllocate<ScriptParticleTextureAnimation>()) ScriptParticleTextureAnimation(managedInstance, instance);
}
