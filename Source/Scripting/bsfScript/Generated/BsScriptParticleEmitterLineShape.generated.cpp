//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterLineShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_LINE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterLineShape.generated.h"

namespace bs
{
	ScriptParticleEmitterLineShape::ScriptParticleEmitterLineShape(MonoObject* managedInstance, const SPtr<ParticleEmitterLineShape>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptParticleEmitterLineShape>()) ScriptParticleEmitterLineShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterLineShape::InternalSetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* options)
	{
		PARTICLE_LINE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleLineShapeOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterLineShape::InternalGetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_LINE_SHAPE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_LINE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleLineShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleLineShapeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterLineShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_LINE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_LINE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleLineShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterLineShape>())ScriptParticleEmitterLineShape(managedInstance, nativeObject);
	}

	void ScriptParticleEmitterLineShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create();
		new (B3DAllocate<ScriptParticleEmitterLineShape>())ScriptParticleEmitterLineShape(managedInstance, nativeObject);
	}
}
