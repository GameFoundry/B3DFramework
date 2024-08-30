//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterCircleShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_CIRCLE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterCircleShape.generated.h"

namespace bs
{
	ScriptParticleEmitterCircleShape::ScriptParticleEmitterCircleShape(const SPtr<ParticleEmitterCircleShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitterCircleShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterCircleShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterCircleShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterCircleShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterCircleShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterCircleShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterCircleShape::InternalSetOptions(ScriptParticleEmitterCircleShape* self, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleCircleShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterCircleShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterCircleShape::InternalGetOptions(ScriptParticleEmitterCircleShape* self, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterCircleShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_CIRCLE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleCircleShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleCircleShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterCircleShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleCircleShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterCircleShape> nativeObject = ParticleEmitterCircleShape::Create(tmpdesc);
		B3DNew<ScriptParticleEmitterCircleShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterCircleShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterCircleShape> nativeObject = ParticleEmitterCircleShape::Create();
		B3DNew<ScriptParticleEmitterCircleShape>(nativeObject, scriptObject);
	}
}
