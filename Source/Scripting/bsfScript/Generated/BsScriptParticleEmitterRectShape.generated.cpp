//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterRectShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_RECT_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterRectShape.generated.h"

namespace bs
{
	ScriptParticleEmitterRectShape::ScriptParticleEmitterRectShape(MonoObject* managedInstance, const SPtr<ParticleEmitterRectShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterRectShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterRectShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterRectShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterRectShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterRectShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterRectShape::Create(const SPtr<ParticleEmitterRectShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterRectShape>()) ScriptParticleEmitterRectShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterRectShape::InternalSetOptions(ScriptParticleEmitterRectShape* self, __PARTICLE_RECT_SHAPE_DESCInterop* options)
	{
		PARTICLE_RECT_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleRectShapeOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterRectShape::InternalGetOptions(ScriptParticleEmitterRectShape* self, __PARTICLE_RECT_SHAPE_DESCInterop* __output)
	{
		PARTICLE_RECT_SHAPE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_RECT_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleRectShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRectShapeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterRectShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_RECT_SHAPE_DESCInterop* desc)
	{
		PARTICLE_RECT_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleRectShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterRectShape>())ScriptParticleEmitterRectShape(managedInstance, nativeObject);
	}

	void ScriptParticleEmitterRectShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create();
		new (B3DAllocate<ScriptParticleEmitterRectShape>())ScriptParticleEmitterRectShape(managedInstance, nativeObject);
	}
}
