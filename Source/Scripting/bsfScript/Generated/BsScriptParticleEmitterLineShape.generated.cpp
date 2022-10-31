//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterLineShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_LINE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterLineShape.generated.h"

using namespace bs;
ScriptParticleEmitterLineShape::ScriptParticleEmitterLineShape(MonoObject* managedInstance, const SPtr<ParticleEmitterLineShape>& value)
	: TScriptReflectable(managedInstance, value)
{
	mInternal = value;
}

void ScriptParticleEmitterLineShape::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterLineShape::InternalSetOptions);
	metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterLineShape::InternalGetOptions);
	metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterLineShape::InternalCreate);
	metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterLineShape::InternalCreate0);
}

MonoObject* ScriptParticleEmitterLineShape::Create(const SPtr<ParticleEmitterLineShape>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptParticleEmitterLineShape>()) ScriptParticleEmitterLineShape(managedInstance, value);
	return managedInstance;
}

void ScriptParticleEmitterLineShape::InternalSetOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* options)
{
	PARTICLE_LINE_SHAPE_DESC tmpoptions;
	tmpoptions = ScriptPARTICLE_LINE_SHAPE_DESC::FromInterop(*options);
	thisPtr->GetInternal()->SetOptions(tmpoptions);
}

void ScriptParticleEmitterLineShape::InternalGetOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* __output)
{
	PARTICLE_LINE_SHAPE_DESC tmp__output;
	tmp__output = thisPtr->GetInternal()->GetOptions();

	__PARTICLE_LINE_SHAPE_DESCInterop interop__output;
	interop__output = ScriptPARTICLE_LINE_SHAPE_DESC::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_LINE_SHAPE_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptParticleEmitterLineShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_LINE_SHAPE_DESCInterop* desc)
{
	PARTICLE_LINE_SHAPE_DESC tmpdesc;
	tmpdesc = ScriptPARTICLE_LINE_SHAPE_DESC::FromInterop(*desc);
	SPtr<ParticleEmitterLineShape> instance = ParticleEmitterLineShape::Create(tmpdesc);
	new(B3DAllocate<ScriptParticleEmitterLineShape>()) ScriptParticleEmitterLineShape(managedInstance, instance);
}

void ScriptParticleEmitterLineShape::InternalCreate0(MonoObject* managedInstance)
{
	SPtr<ParticleEmitterLineShape> instance = ParticleEmitterLineShape::Create();
	new(B3DAllocate<ScriptParticleEmitterLineShape>()) ScriptParticleEmitterLineShape(managedInstance, instance);
}
