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
	ScriptParticleEmitterLineShape::ScriptParticleEmitterLineShape(const SPtr<ParticleEmitterLineShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleEmitterLineShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterLineShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterLineShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterLineShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterLineShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterLineShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterLineShape::InternalSetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* options)
	{
		PARTICLE_LINE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleLineShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterLineShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterLineShape::InternalGetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_LINE_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterLineShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_LINE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleLineShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleLineShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterLineShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_LINE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_LINE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleLineShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create(tmpdesc);
		B3DNew<ScriptParticleEmitterLineShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterLineShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create();
		B3DNew<ScriptParticleEmitterLineShape>(nativeObject, scriptObject);
	}
}
