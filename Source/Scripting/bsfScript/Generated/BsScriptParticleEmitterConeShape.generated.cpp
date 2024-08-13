//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterConeShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_CONE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterConeShape.generated.h"

namespace bs
{
	ScriptParticleEmitterConeShape::ScriptParticleEmitterConeShape(MonoObject* managedInstance, const SPtr<ParticleEmitterConeShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterConeShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterConeShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterConeShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterConeShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterConeShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterConeShape::Create(const SPtr<ParticleEmitterConeShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterConeShape>()) ScriptParticleEmitterConeShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterConeShape::InternalSetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* options)
	{
		PARTICLE_CONE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleConeShapeOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterConeShape::InternalGetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_CONE_SHAPE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_CONE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleConeShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleConeShapeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterConeShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_CONE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_CONE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleConeShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterConeShape>())ScriptParticleEmitterConeShape(managedInstance, nativeObject);
	}

	void ScriptParticleEmitterConeShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create();
		new (B3DAllocate<ScriptParticleEmitterConeShape>())ScriptParticleEmitterConeShape(managedInstance, nativeObject);
	}
}
