//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterCircleShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_CIRCLE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterCircleShape.generated.h"

namespace bs
{
	ScriptParticleEmitterCircleShape::ScriptParticleEmitterCircleShape(MonoObject* managedInstance, const SPtr<ParticleEmitterCircleShape>& value)
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterCircleShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterCircleShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterCircleShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterCircleShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterCircleShape::InternalCreate0);
	}

	MonoObject* ScriptParticleEmitterCircleShape::Create(const SPtr<ParticleEmitterCircleShape>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptParticleEmitterCircleShape>()) ScriptParticleEmitterCircleShape(managedInstance, value);
		return managedInstance;
	}

	void ScriptParticleEmitterCircleShape::InternalSetOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_CIRCLE_SHAPE_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterCircleShape::InternalGetOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_CIRCLE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_CIRCLE_SHAPE_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_CIRCLE_SHAPE_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterCircleShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_CIRCLE_SHAPE_DESC::FromInterop(*desc);
		SPtr<ParticleEmitterCircleShape> instance = ParticleEmitterCircleShape::Create(tmpdesc);
		new(bs_alloc<ScriptParticleEmitterCircleShape>()) ScriptParticleEmitterCircleShape(managedInstance, instance);
	}

	void ScriptParticleEmitterCircleShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterCircleShape> instance = ParticleEmitterCircleShape::Create();
		new(bs_alloc<ScriptParticleEmitterCircleShape>()) ScriptParticleEmitterCircleShape(managedInstance, instance);
	}
} // namespace bs
